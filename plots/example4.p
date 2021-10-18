# Set first two line styles to blue (#0060ad) and red (#dd181f)
set style line 1 \
    linecolor rgb '#0060ad' \
    linetype 1 linewidth 2 \
    pointtype 7 pointsize 1.5
set style line 2 \
    linecolor rgb '#dd181f' \
    linetype 1 linewidth 2 \
    pointtype 5 pointsize 1.5

plot 'plotting_data3.dat' index 0 with linespoints linestyle 1, \
     ''                   index 1 with linespoints linestyle 2, \
     ''                   index 1 with linespoints linestyle 2
