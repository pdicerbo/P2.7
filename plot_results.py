import numpy as np
import matplotlib.pyplot as plt

my_timing = "timing_my_version.dat"
n_rep = 4

dat = np.loadtxt(my_timing)
n_p = dat[:,0]
time = dat[:,1]
lvec = len(time) / n_rep
nprocs = np.zeros(lvec)
time_def   = np.zeros(lvec)
err = np.zeros(lvec)

t_tmp = 0.
i = 0
set_element = 0

while (i < dat.shape[0]):

    nprocs[set_element] = n_p[i]
    
    # mean value calculation
    for j in range(n_rep):
        t_tmp += time[i + j]

    toset = t_tmp / n_rep
    time_def[set_element] = toset

    t_tmp = 0.
    # error calculation
    for j in range(n_rep):
        t_tmp += (toset - time[i + j])**2

    err[set_element] = (t_tmp / (n_rep - 1))**0.5

    i += n_rep

    set_element += 1
    t_tmp = 0.
    
plt.figure()
plt.errorbar(nprocs, time_def, err)
plt.title("Timing (My Version)")
plt.xlabel("NPES")
plt.ylabel("time (s)")
plt.savefig("first_timing.png")
plt.close('All')

plt.figure()
scaling = time_def[0] / time_def
plt.plot(nprocs, nprocs)
plt.errorbar(nprocs, scaling, err)
plt.title("Scaling (My Version)")
plt.xlabel("NPES")
plt.ylabel("speedup")
plt.savefig("first_scaling.png")
