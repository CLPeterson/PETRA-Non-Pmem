# Set first two line styles to blue (#0060ad) and red (#dd181f)
set style line 1 \
    linecolor rgb '#0060ad' \
    linetype 1 linewidth 2 \
    pointtype 7 pointsize 1.5
set style line 2 \
    linecolor rgb '#dd181f' \
    linetype 1 linewidth 2 \
    pointtype 5 pointsize 1.5

set style line 3 \
    linecolor rgb '#A020F0' \
    linetype 1 linewidth 2 \
    pointtype 5 pointsize 1.5

x=2
while (x < 6) {
	plot 'outputPetra.txt' index 0 every ::0::x with linespoints linestyle 1, \
     'outputRomulus.txt'  index 0 every ::0::x with linespoints linestyle 2, \
     'outputOnefile.txt'  index 0 every ::0::x with linespoints linestyle 3
	x = x + 1
	pause 1
}
