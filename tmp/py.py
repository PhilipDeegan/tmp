def extract(fn):
    return tuple([int(s) for s in  (fn.split("_")[-2],fn.split("_")[-1].split(".")[-2])])

import glob
import pandas as pds
import matplotlib.pyplot as plt

files = glob.glob("speedup_*")
print(files)
extract(files[0])

speedups={}
for f in files:
    params = extract(f)
    speedups.update({params:pds.read_table(f, sep=" ", index_col=0, header=None)})


def plot(N):
    fig, ax = plt.subplots()
    for params,data in  speedups.items():
        if (params[0]==N):
            ax.plot(data.index, data.values, label="{}".format(params))
            ax.plot(data.index, data.index, ls="--", color="k")
    ax.set_xlabel("nthread")
    ax.set_ylabel("speedup")
    ax.legend()
    plt.show()

plot(50)
