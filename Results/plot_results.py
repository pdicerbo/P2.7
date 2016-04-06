import numpy as np
import matplotlib.pyplot as plt

my_timing = "timing_my_version.dat"
fftw = "timing_fftw.dat"
n_rep = 4

dat = np.loadtxt(my_timing)
fftdat = np.loadtxt(fftw)
n_p = dat[:,0]
time = dat[:,1]
fftw_time = fftdat[:,1]

lvec = len(time) / n_rep
nprocs = np.zeros(lvec)
time_def = np.zeros(lvec)
fftw_time_def = np.zeros(lvec)
err = np.zeros(lvec)
fftw_err = np.zeros(lvec)

t_tmp = 0.
fw_tmp = 0.
i = 0
set_element = 0

while (i < dat.shape[0]):

    nprocs[set_element] = n_p[i]
    
    # mean value calculation
    for j in range(n_rep):
        t_tmp += time[i + j]
        fw_tmp += fftw_time[i + j]

    toset = t_tmp / n_rep
    fw_toset = fw_tmp / n_rep
    time_def[set_element] = toset
    fftw_time_def[set_element] = fw_toset
    
    t_tmp = 0.
    fw_tmp = 0.

    # error calculation
    for j in range(n_rep):
        t_tmp += (toset - time[i + j])**2
        fw_tmp += (fw_toset - fftw_time[i + j])**2

    err[set_element] = (t_tmp / (n_rep - 1))**0.5
    fftw_err[set_element] = (fw_tmp / (n_rep - 1))**0.5

    i += n_rep

    set_element += 1
    t_tmp = 0.
    fw_tmp = 0.
    
plt.figure()
plt.errorbar(nprocs, time_def, err, label = "MyVersion")
plt.errorbar(nprocs, fftw_time_def, fftw_err, label = "FFTW")
plt.title("Timing")
plt.xlabel("NPES")
plt.ylabel("time (s)")
plt.legend()
plt.savefig("timing.png")
plt.close('All')

plt.figure()
scaling = time_def[0] / time_def
fscaling = fftw_time_def[0] / fftw_time_def
plt.plot(nprocs, nprocs)
plt.errorbar(nprocs, scaling, err, label = "MyVersion")
plt.errorbar(nprocs, fscaling, fftw_err, label = "FFTW")
plt.title("Scaling (My Version)")
plt.xlabel("NPES")
plt.ylabel("speedup")
plt.legend(bbox_to_anchor = (0.325,1.))
plt.savefig("scaling.png")
