import numpy as np
import matplotlib.pyplot as plt

# my_timing = "timing_my_version.dat"
# fftw = "timing_fftw.dat"
my_timing = "timing_my_version_big.dat"
fftw = "timing_fftw_big.dat"
dim2 = "timing_2D.dat"
n_rep = 4

dat = np.loadtxt(my_timing)
fftdat = np.loadtxt(fftw)
D2dat = np.loadtxt(dim2)
n_p = dat[:,0]
time = dat[:,1]
fftw_time = fftdat[:,1]
time2 = D2dat[:,1]

lvec = len(time) / n_rep
nprocs = np.zeros(lvec)
time_def = np.zeros(lvec)
fftw_time_def = np.zeros(lvec)
D2_time_def = np.zeros(lvec)

err = np.zeros(lvec)
fftw_err = np.zeros(lvec)
D2_err = np.zeros(lvec)

t_tmp = 0.
fw_tmp = 0.
t2_tmp = 0.

i = 0
set_element = 0

while (i < dat.shape[0]):

    nprocs[set_element] = n_p[i]
    
    # mean value calculation
    for j in range(n_rep):
        t_tmp += time[i + j]
        fw_tmp += fftw_time[i + j]
        t2_tmp += time2[i + j]

    toset = t_tmp / n_rep
    fw_toset = fw_tmp / n_rep
    t2_toset = t2_tmp / n_rep

    time_def[set_element] = toset
    fftw_time_def[set_element] = fw_toset
    D2_time_def[set_element] = t2_toset

    t_tmp = 0.
    fw_tmp = 0.
    t2_tmp = 0.

    # error calculation
    for j in range(n_rep):
        t_tmp += (toset - time[i + j])**2
        fw_tmp += (fw_toset - fftw_time[i + j])**2
        t2_tmp += (t2_toset - time2[i + j])**2

    err[set_element] = (t_tmp / (n_rep - 1))**0.5
    fftw_err[set_element] = (fw_tmp / (n_rep - 1))**0.5
    D2_err[set_element] = (t2_tmp / (n_rep - 1))**0.5

    i += n_rep

    set_element += 1
    t_tmp = 0.
    fw_tmp = 0.
    t2_tmp = 0.

plt.figure()
plt.errorbar(nprocs, time_def, err, label = "MyVersion")
plt.errorbar(nprocs, fftw_time_def, fftw_err, label = "FFTW")
plt.errorbar(nprocs, D2_time_def, D2_err, label = "2D+1D")
plt.title("Timing")
plt.xlabel("NPES")
plt.ylabel("time (s)")
plt.legend()
# plt.savefig("timing.png")
plt.savefig("timing_with_2D.png")
plt.close('All')

plt.figure()
scaling = time_def[0] / time_def
fscaling = fftw_time_def[0] / fftw_time_def
D2scaling = D2_time_def[0] / D2_time_def

# plt.plot(nprocs, nprocs)
plt.plot(nprocs, nprocs/20)

plt.errorbar(nprocs, scaling, err, label = "MyVersion")
plt.errorbar(nprocs, fscaling, fftw_err, label = "FFTW")
plt.errorbar(nprocs, D2scaling, D2_err, label = "2D+1D")
plt.title("Scaling")
plt.xlabel("NPES")
plt.ylabel("speedup")
plt.legend(bbox_to_anchor = (0.325,1.))
# plt.savefig("scaling.png")
# plt.savefig("scaling_big.png")
plt.savefig("scaling_with_2D.png")
