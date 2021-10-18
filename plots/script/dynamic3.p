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

set term x11 0


plot 'outputPetra.txt' index 0 every ::0::5 with linespoints linestyle 1 title 'Petra List', \
     'outputRomulus.txt'  index 0 every ::0::5 with linespoints linestyle 2 title 'Romulus List', \
     'outputOnefile.txt'  index 0 every ::0::5 with linespoints linestyle 3 title 'Onefile List'


set term x11 1

plot 'outputPetra.txt' index 1 every ::0::5 with linespoints linestyle 1 title 'Petra Skiplist', \
     'outputRomulus.txt'  index 1 every ::0::5 with linespoints linestyle 2 title 'Romulus Skiplist', \
     'outputOnefile.txt'  index 1 every ::0::5 with linespoints linestyle 3 title 'Onefile Skiplist'
