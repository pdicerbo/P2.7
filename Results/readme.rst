==================
Results
==================

Here I report the timing and the scaling obtained comparing the 3D Fast Fourier Transform (FFT) calculation
with the fftw library and my versions. The "pdicerbo-P2.7/D2-exercise2/provided_code/C/fft_wrapper.c" file
contains two implementations of the function that performs the FFT. The "old_fft_3d" function performs the
FFT through the calculation of three 1D FFT (one for each dimension), while "fft_3d" comes to the result
performing first a 2D FFT and then performing a 1D FFT on the remaining dimension. Because of the algorithms
we expect better results with fft_3d (that need less copies).

In the following plot the label "MyVersion" refers to data obtained with the "old_fft_3d" function while
the label "2D+1D" refers to data obtained with the "fft_3d" function.

Plots
------------------

These plots are obtained with a matrix with size **480x480x96** and performing only 2 steps of integration.

.. image:: timing_2D_small.png

Here the scaling obtained:
	   
.. image:: ./scaling_2D_small.png

As one can see, if NPES <= 20 (namely we are using one single node), the best performance (in terms of
time needed to run the program) is obtained with the "2D+1D" version of the code.

However, increasing the size of the matrix to **1440x1440x100** and using multiple nodes, the results
change drammatically. These are the same plots as before obtained using a number of nodes starting
from 1 to 8.

.. image:: timing_2D_big.png

Here the corresponding scaling:
	   
.. image:: ./scaling_2D_big.png

The obtained results seems to be a bit weird, but I don't know the real reason of that. I'm sorry, but I
don't have enough time to analize these results.
