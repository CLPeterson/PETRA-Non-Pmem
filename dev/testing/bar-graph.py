import matplotlib.pyplot as plt
import numpy as np
trans_file = "CLFLUSHOPT/key10000/50/50_lfttall_CLFLUSHOPT_10000_average.txt"
with open(trans_file, "r") as f:
    trans_sizes = (1,2,4,8,16)
    x_vals = np.arange(5)
    ops = []
    for line in f.readlines():
        if line.startswith("16 threads"):
            ops.append(float(line.split()[10]))
    plt.bar(x_vals,ops, align='center')
    plt.xticks(x_vals, trans_sizes)
    plt.ylabel('Ops/sec')
    plt.yscale("log")
    plt.xlabel('Transaction Size')
    plt.title('Effect of Transaction Sizes on Throughput')
    plt.savefig("bar.pdf", dpi=600)
    