This folder contains the raw data used for comparing the various database benchmarks present in our paper, as well as scripts needed to replicate the tests and create similar graphs. calculate_benchmarks.py converts raw test data into a structured format consumed by db_graph.py which actually creates the plots. 
Db_benchmarks.xlsx is an excel spreadsheet with the raw data from the tests. cmap_bench.txt and petramap_bench.txt contain the formatted version of the raw data from the tests and can be fed into db_graph.py for an example graph. 
The db_tests subfolder contains folders with the raw data we gathered after running our tests for petramap (then called dlfttmap) and cmap. run_tests_mod.py is the main driver for running the tests from scratch. It can be found one directory level up from this folder. 
If you would like to generate raw data and start from the very beginning, that script is the entry point. 


Steps for running the Database Benchmark tests

1. To run the tests, go to the folder containing run_tests_mod.py (it should be up one directory) and run the command 'python3 run_tests_mod.py' with the --db flag to run the database tests. If you would also like verbose debug output, add the -d flag as well. The script will prompt you to enter paths to the directory with the benchmarktest binary, the build directory to run make. 
and to enter a space seperated list of thread counts. When the tests are complete, the script should create .txt files with the raw output in the same directory with the prefix "raw_db_".

2. Once you have the raw data for each thread count of cmap and petramap, seperate the raw results into two folders, one for cmap raw data and one for petramap raw data. Run the command "python3 calculate_benchmarks.py". It should prompt you to enter the relative path to a folder containing either the raw petramap or cmap data.
The script should then prompt you for an output file name and save that file to the current directory. The contents of the file will be the processed data for each benchmark. Each line should contain the name of the benchmark, and then space seperated values for the data in count order.
Repeat this process for the other set of raw data. At the end of this step you should have two files that contain the processed data. 


3. To convert the processed data into a plot similar to the paper you can use db_graph.py. Run the command "python3 db_graph.py". It will prompt you for the file path to the processed cmap and petra data and then begin
processing that data and making the plot. Once that is complete, it will prompt you for the name of the output file. Note that matplotlib only supports creating plots with ps, pdf, pgf, png, raw, rgba, svg, svgz, jpg, jpeg, tif, and tiff formats.
the default dpi for the plots are 600
