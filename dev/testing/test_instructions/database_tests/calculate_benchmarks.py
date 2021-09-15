import os

dir = input("Path to raw data: ")
os.chdir(dir)
index_map = {"1": 0, "2": 1, "4": 2, "8": 3, "16": 4, "48": 5, "96": 6}
averages = {
    "fillrandom": [0] * 7,
    "deleterandom": [0] * 7,
    "overwrite": [0] * 7,
    "fillseq": [0] * 7,
    "readrandom": [0] * 7,
    "readseq": [0] * 7,
    "readmissing": [0] * 7,
    "deleteseq": [0] * 7,
    "readrandomwriterandom": [0] * 7,
}
for filename in os.listdir("."):
    benchmarks = {
        "fillrandom": [],
        "deleterandom": [],
        "overwrite": [],
        "fillseq": [],
        "readrandom": [],
        "readseq": [],
        "readmissing": [],
        "deleteseq": [],
        "readrandomwriterandom": [],
    }
    with open(filename, "r") as f:
        for line in f.readlines():
            words = line.split()
            if len(words) == 0:
                continue
            if words[0] in benchmarks:
                benchmarks[words[0]].append(float(words[2]))
    for key, val in benchmarks.items():
        avg = sum(val) / 10
        thread_count = 3 if "petramap" in filename else 4
        index = index_map[filename.strip(".txt").split("_")[thread_count]]
        averages[key][index] = avg
with open(input("Output filename: "), "w") as output:
    for benchmark, values in averages.items():
        output.write(f"{benchmark} ")
        for val in values:
            output.write(f"{val} ")
        output.write("\n")
