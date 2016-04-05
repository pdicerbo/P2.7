/* Assignement:
 * Here you have to modify the includes, the array sizes and the fftw calls, to use the fftw-mpi
 *
 * Regarding the fftw calls. here is the substitution 
 * fftw_plan_dft_3d -> fftw_mpi_plan_dft_3d
 * ftw_execute_dft  > fftw_mpi_execute_dft 
 * use fftw_mpi_local_size_3d for local size of the arrays
 * 
 * Created by G.P. Brandino, I. Girotto, R. Gebauer
 * Last revision: March 2016
 *
 */ 

#include <stdbool.h>
#include <string.h>
#include "utilities.h"
#include <complex.h>
#include <fftw3.h>
#include <mpi.h>

double seconds(){
/* Return the second elapsed since Epoch (00:00:00 UTC, January 1, 1970) */
  struct timeval tmp;
  double sec;
  gettimeofday( &tmp, (struct timezone *)0 );
  sec = tmp.tv_sec + ((double)tmp.tv_usec)/1000000.0;
  return sec;
}

/* 
 *  Index linearization is computed following row-major order.
 *  For more informtion see FFTW documentation:
 *  http://www.fftw.org/doc/Row_002dmajor-Format.html#Row_002dmajor-Format
 *
 */
int index_f ( int i1, int i2, int i3, int n1, int n2, int n3){

  return n3*n2*i1 + n3*i2 + i3; 
}

void init_fftw(fftw_dist_handler *fft, int n1, int n2, int n3, MPI_Comm comm){
  
  int npes, mype;
  int buffer_size = 0;
  
  fft->mpi_comm = comm;

  /*
   *  Allocate a distributed grid for complex FFT using aligned memory allocation
   *  See details here:
   *  http://www.fftw.org/fftw3_doc/Allocating-aligned-memory-in-Fortran.html#Allocating-aligned-memory-in-Fortran
   *  HINT: initialize all global and local dimensions. Consider the first dimension being multiple of the number of processes
   *
   */

  MPI_Comm_size( comm, &npes );
  MPI_Comm_rank( comm, &mype );

  if( ( ( n1 % npes ) || ( n2 % npes ) ) && !mype ){
    
    fprintf( stdout, "\nN1 dimension must be multiple of the number of processes. The program will be aborted...\n\n" );
    MPI_Abort( comm, 1 );
  }

  /* Fill the missing parts */
  fft->n1 = n1;
  fft->n2 = n2;
  fft->n3 = n3;
  fft->local_n1 = n1 / npes;
  fft->local_n1_offset = n2 * n3 * mype;
  fft->global_size_grid = n1 * n2 * n3;
  fft->local_size_grid = n2 * n3 * n1 / npes;

  /*
   * Allocate fft->fftw_data and create an FFTW plan for each 1D FFT among all dimensions
   *
   */
  buffer_size = MAX(n1, n2);
  buffer_size = MAX(buffer_size, n3);

  fft->fftw_data = ( fftw_complex* ) fftw_malloc( buffer_size * sizeof( fftw_complex ) );

  // FORWARD'S PLANS
  fft->fw_plan_i1 = fftw_plan_dft_1d(n1, fft -> fftw_data, fft -> fftw_data,
				     FFTW_FORWARD, FFTW_ESTIMATE);

  fft->fw_plan_i2 = fftw_plan_dft_1d(n2, fft -> fftw_data, fft -> fftw_data,
				     FFTW_FORWARD, FFTW_ESTIMATE);

  fft->fw_plan_i3 = fftw_plan_dft_1d(n3, fft -> fftw_data, fft -> fftw_data,
				     FFTW_FORWARD, FFTW_ESTIMATE);

  // BACKWARD'S PLANS
  fft->bw_plan_i1 = fftw_plan_dft_1d(n1, fft -> fftw_data, fft -> fftw_data,
				     FFTW_BACKWARD, FFTW_ESTIMATE);

  fft->bw_plan_i2 = fftw_plan_dft_1d(n2, fft -> fftw_data, fft -> fftw_data,
				     FFTW_BACKWARD, FFTW_ESTIMATE);

  fft->bw_plan_i3 = fftw_plan_dft_1d(n3, fft -> fftw_data, fft -> fftw_data,
				     FFTW_BACKWARD, FFTW_ESTIMATE);

}

void close_fftw( fftw_dist_handler *fft ){

    fftw_destroy_plan( fft->bw_plan_i1 );
    fftw_destroy_plan( fft->bw_plan_i2 );
    fftw_destroy_plan( fft->bw_plan_i3 );

    fftw_destroy_plan( fft->fw_plan_i1 );
    fftw_destroy_plan( fft->fw_plan_i2 );
    fftw_destroy_plan( fft->fw_plan_i3 );

    fftw_free( fft->fftw_data );
}

/* This subroutine uses fftw to calculate 3-dimensional discrete FFTs.
 * The data in direct space is assumed to be real-valued
 * The data in reciprocal space is complex. 
 * direct_to_reciprocal indicates in which direction the FFT is to be calculated
 * 
 * Note that for real data in direct space (like here), we have
 * F(N-j) = conj(F(j)) where F is the array in reciprocal space.
 * Here, we do not make use of this property.
 * Also, we do not use the special (time-saving) routines of FFTW which
 * allow one to save time and memory for such real-to-complex transforms.
 *
 * f: array in direct space
 * F: array in reciprocal space
 * 
 * F(k) = \sum_{l=0}^{N-1} exp(- 2 \pi I k*l/N) f(l)
 * f(l) = 1/N \sum_{k=0}^{N-1} exp(+ 2 \pi I k*l/N) F(k)
 * 
 */

void fft_3d( fftw_dist_handler* fft, double *data_direct, fftw_complex* data_rec, bool direct_to_reciprocal ){

  double fac;
  int i1, i2, i3, index; //, start_index, end_index, index_buf, i2_loc;
  int n2 = fft->n2, n3 = fft->n3, n1 = fft->n1, npes, block_dim; //, nblock;

  /* Allocate buffers to send and receive data */

  MPI_Comm_size( fft->mpi_comm, &npes );
    
  // Now distinguish in which direction the FFT is performed
  if( direct_to_reciprocal ){

    /* among i3 dimension */
    for( i1 = 0; i1 < fft->local_n1; i1++ ){
      for( i2 = 0; i2 < n2; i2++ ){
	for( i3 = 0; i3 < n3; i3++ ){

	  index = index_f(i1, i2, i3, fft -> local_n1, n2, n3);
	  fft -> fftw_data[i3] = data_direct[index] + 0.0 * I;

	}

	fftw_execute_dft( fft -> fw_plan_i3, fft -> fftw_data, fft -> fftw_data);

	index = index_f(i1, i2, 0, fft -> local_n1, n2, n3);
	memcpy(&(data_rec[index]), fft->fftw_data, n3 * sizeof(fftw_complex));

      }
    }
      
    /* among i2 dimension */
    for( i1 = 0; i1 < fft->local_n1; i1++ ){
      for( i3 = 0; i3 < n3; i3++ ){
	for( i2 = 0; i2 < n2; i2++ ){

	  index = index_f(i1, i2, i3, fft -> local_n1, n2, n3);
	  fft -> fftw_data[i2] = data_rec[index];

	}

	fftw_execute_dft( fft -> fw_plan_i2, fft -> fftw_data, fft -> fftw_data);
	
	for(i2 = 0; i2 < n2; i2++){

	  index = index_f(i1, i2, i3, fft -> local_n1, n2, n3);
	  data_rec[index] = fft -> fftw_data[i2];

	}
      }
    }

    /*
     * Reorder the different data blocks to be contigous in memory.
     * The new distribution will allow to use the Alltoall function
     *
     */

    // Perform an Alltoall communication 

    /*  among i1 dimension */

    block_dim = n2 / npes;

    for( i3 = 0; i3 < n3; i3++ ){
      for( i2 = 0; i2 < block_dim; i2++ ){
	for( i1 = 0; i1 < n1; i1++ ){
	  
	  index = index_f(i1, i2, i3, n1, block_dim, n3);
	  fft -> fftw_data[i1] = data_rec[index];

	}

	fftw_execute_dft( fft -> fw_plan_i1, fft -> fftw_data, fft -> fftw_data);

	for(i1 = 0; i1 < n1; i1++){

	  index = index_f(i1, i2, i3, n1, block_dim, n3);
	  data_rec[index] = fft -> fftw_data[i1];

	}
      }
    }

    // Perform an Alltoall communication 

    /*
     * Reoder the different data blocks to be consistent with the initial distribution.
     *
     */      

  }
  else{

    fac = 1.0 / ( n1 * n2 * n3 );

    /* Implement the reverse transform */
    /* among i3 dimension */
    for( i1 = 0; i1 < fft->local_n1; i1++ ){
      for( i2 = 0; i2 < n2; i2++ ){
	for( i3 = 0; i3 < n3; i3++ ){

	  index = index_f(i1, i2, i3, fft -> local_n1, n2, n3);
	  fft -> fftw_data[i3] = data_rec[index];

	}

	fftw_execute_dft( fft -> bw_plan_i3, fft -> fftw_data, fft -> fftw_data);

	index = index_f(i1, i2, 0, fft -> local_n1, n2, n3);
	memcpy(&(data_rec[index]), fft->fftw_data, n3 * sizeof(fftw_complex));

      }
    }

    /* among i2 dimension */
    for( i1 = 0; i1 < fft->local_n1; i1++ ){
      for( i3 = 0; i3 < n3; i3++ ){
	for( i2 = 0; i2 < n2; i2++ ){
	    
	  index = index_f(i1, i2, i3, fft -> local_n1, n2, n3);
	  fft -> fftw_data[i2] = data_rec[index];

	}

	fftw_execute_dft( fft -> bw_plan_i2, fft -> fftw_data, fft -> fftw_data);
	
	for(i2 = 0; i2 < n2; i2++){

	  index = index_f(i1, i2, i3, fft -> local_n1, n2, n3);
	  data_rec[index] = fft -> fftw_data[i2];

	}
      }
    }

    /*
     * Reorder the different data blocks to be contigous in memory.
     * The new distribution will allow to use the Alltoall function
     *
     */

    // Perform an Alltoall communication 

    /*  among i1 dimension */
    block_dim = n2 / npes;

    for( i3 = 0; i3 < n3; i3++ ){
      for( i2 = 0; i2 < block_dim; i2++ ){
	for( i1 = 0; i1 < n1; i1++ ){
	  
	  index = index_f(i1, i2, i3, n1, block_dim, n3);
	  fft -> fftw_data[i1] = data_rec[index];

	}

	fftw_execute_dft( fft -> bw_plan_i1, fft -> fftw_data, fft -> fftw_data);

	for(i1 = 0; i1 < n1; i1++){

	  index = index_f(i1, i2, i3, n1, block_dim, n3);
	  data_direct[index] = creal(fft -> fftw_data[i1]) * fac;

	}
      }
    }

    // Perform an Alltoall communication 

    /*
     * Reoder the different data blocks to be consistent with the initial distribution.
     *
     */      

  }
  
}
