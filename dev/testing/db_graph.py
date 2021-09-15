import matplotlib.pyplot as plt
import numpy as np
import brewer2mpl
from collections import OrderedDict

cmap_file = "db_tests/cmap/cmap_bench.txt"
dlftt_file = "db_tests/dlfttmap/dlfttmap_bench.txt"
num_groups = 2
bar_width = 0.1
opacity = 0.8

index = np.arange(num_groups)
bmap = brewer2mpl.get_map("Set1", "Qualitative", 7)
colors = bmap.mpl_colors
benchmark = input("Benchmark: ")

index_map = {1: 1, 2: 2, 3: 4, 4: 8, 5: 16, 6: 48, 7: 96}
avg_dict = {1: [], 2: [], 4: [], 8: [], 16: [], 48: [], 96: []}

cmap_color = 2
dlftt_color = 1
plt.rcParams.update({'font.size': '28'})

with open(cmap_file, "r") as cfile, open(dlftt_file, "r") as dfile:
    for line in cfile.readlines():
        if line.split(" ")[0] == benchmark:
            vals = line.split()
            for i in range(1, len(vals)):
                avg_dict[index_map[i]].append(float(vals[i]))

    for line in dfile.readlines():
        if line.split()[0] == benchmark:
            vals = line.split()
            for i in range(1, len(vals)):
                avg_dict[index_map[i]].append(float(vals[i]))

for i, thread in enumerate(avg_dict.keys()):
    print(f"plotting thead {thread}")
    plt.bar(
        0 + bar_width * i,
        avg_dict[index_map[i + 1]][0],
        bar_width,
        color=colors[cmap_color],
        alpha=opacity,
        edgecolor="black",
    )
    plt.bar(
        1 + bar_width * i,
        avg_dict[index_map[i + 1]][1],
        bar_width,
        color=colors[dlftt_color],
        alpha=opacity,
        edgecolor="black",
    )


plt.xlabel("Datastore Engines")
plt.ylabel("\u03BCs/operation")
plt.title(benchmark)
plt.tight_layout()
plt.xticks(index + (bar_width * 7 / 2), ("cmap", "dlfttmap"))

plt.savefig(f"{benchmark}.pdf", dpi=600)
