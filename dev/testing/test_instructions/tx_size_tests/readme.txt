This folder contains the raw data used for comparing the effect of different transaction sizes between Petra and Romulus.
PETRAvsROMULUS is an excel spreadsheet with the raw data. The "petraall" and "romulusall" sub directories contain the raw 
data for our tests, as well as formatted data taking the median and average of the test runs to be fed into the 
plotting script.

Steps for running multiple transaction Petra vs Romulus comparison tests:

1. To run the tests, navigate to the folder containing run_tests_mod.py and run the command "python3 run_tests_mod.py" with the --romulusall and 
--petraall flags to run the tests with multiple transaction sizes. The test suite should begin with the petra tests first. You will first be prompted 
to enter a relative path to the compiled petra binary, then for a space separated list of transaction sizes you would like to test, and then a key range for the test.
Once the test finishes, in your current directory you should see two new files with file name formats similar to the "50_lfttall_CLFLUSHOPT_10000_median.txt" format for 
Petraall with average and median, as well as a raw file with the prefix "raw. You will then receive similar input prompts for the romulus tests, and similarly will get 
two output files containing romulusall formatted data, and a raw data file. 

2. Once those files have been generated, navigate to the directory containing "gen_plots.py". To create a plot comparing Petra and Romulus results, run the command
"python3 gen_plots.py" followed by the paths two of the formatted romulus and petra files. You can run "python3 gen_plots.py --example" for an example of this, or use the
-h flag to view other options. The script should prompt you for an output filename unless you supply one at the command line with the --filename flag. Keep in mind that 
only valid plot formats are supports creating plots with ps, pdf, pgf, png, raw, rgba, svg, svgz, jpg, jpeg, tif, and tiff formats. The default dpi for the plots are 600