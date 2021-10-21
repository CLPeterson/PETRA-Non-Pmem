# Set first two line styles to blue (#0060ad) and red (#dd181f)
set print "-"

set style line 1 \
    linecolor rgb '#dd181f' \
    linetype 1 linewidth 2.5 \
    pointtype 7 pointsize 1.5
set style line 2 \
    linecolor rgb '#0060ad' \
    linetype 1 linewidth 2 \
    pointtype 5 pointsize 1.5

set style line 3 \
    linecolor rgb '#A020F0' \
    linetype 1 linewidth 2 \
    pointtype 9 pointsize 1.5

set term wxt size 1200,720

set key font ",18"

set xlabel "Number of Threads" font ",16"

set ylabel "Operations per Second" font ",16"

set tics font ",12"

#set print 'stdout.txt'

x = 0
stats [*:*][*:*] 'outputOnefile.txt' index 0 u 0 nooutput
while (x < 2) {
     stats [*:*][*:*] 'outputOnefile.txt' index 0 u 0 nooutput
     x = STATS_columns
}

NUM_ROWS = 4

x = 0
while (x < NUM_ROWS) {
     pause 1
     stats [*:*][*:*] 'outputOnefile.txt' index 0 u 0 nooutput
     plot 'outputPetra.txt' index 0 every ::0::STATS_records with linespoints linestyle 1 title 'Petra List', \
     'outputRomulus.txt'  index 0 every ::0::STATS_records with linespoints linestyle 2 title 'Romulus List', \
     'outputOnefile.txt'  index 0 every ::0::STATS_records with linespoints linestyle 3 title 'Onefile List'
     x = STATS_records
     #print sprintf("Block 1 STATS_records %d",STATS_records)
} 


while(STATS_blank < 2) {
     stats [*:*][*:*] 'outputOnefile.txt' u 0 nooutput
     plot 'outputPetra.txt' index 0 every ::0::NUM_ROWS with linespoints linestyle 1 title 'Petra List', \
     'outputRomulus.txt'  index 0 every ::0::NUM_ROWS with linespoints linestyle 2 title 'Romulus List', \
     'outputOnefile.txt'  index 0 every ::0::NUM_ROWS with linespoints linestyle 3 title 'Onefile List'
}

set term wxt 1

x = 0
while (x < NUM_ROWS)  {
     pause 1
     stats [*:*][*:*] 'outputOnefile.txt' index 1 u 0 nooutput
     plot 'outputPetra.txt' index 1 every ::0::STATS_records with linespoints linestyle 1 title 'Petra Skiplist', \
     'outputRomulus.txt'  index 1 every ::0::STATS_records with linespoints linestyle 2 title 'Romulus Skiplist', \
     'outputOnefile.txt'  index 1 every ::0::STATS_records with linespoints linestyle 3 title 'Onefile Skiplist'
     x = STATS_records
     #print sprintf("Block 2 STATS_records %d",STATS_records)
} 

while(STATS_blank < 4) {
     stats [*:*][*:*] 'outputOnefile.txt' u 0 nooutput
     plot 'outputPetra.txt' index 1 every ::0::NUM_ROWS with linespoints linestyle 1 title 'Petra Skiplist', \
     'outputRomulus.txt'  index 1 every ::0::NUM_ROWS with linespoints linestyle 2 title 'Romulus Skiplist', \
     'outputOnefile.txt'  index 1 every ::0::NUM_ROWS with linespoints linestyle 3 title 'Onefile Skiplist'
}

set term wxt 2

x = 0
while (x < NUM_ROWS)  {
     pause 1
     stats [*:*][*:*] 'outputOnefile.txt' index 2 u 0 nooutput
     plot 'outputPetra.txt' index 2 every ::0::STATS_records with linespoints linestyle 1 title 'Petra MDlist', \
     'outputRomulus.txt'  index 2 every ::0::STATS_records with linespoints linestyle 2 title 'Romulus MDlist', \
     'outputOnefile.txt'  index 2 every ::0::STATS_records with linespoints linestyle 3 title 'Onefile MDlist'
     x = STATS_records
     #print sprintf("Block 3 STATS_records %d",STATS_records)
} 

while(STATS_blank < 6) {
     stats [*:*][*:*] 'outputOnefile.txt' u 0 nooutput
     plot 'outputPetra.txt' index 2 every ::0::NUM_ROWS with linespoints linestyle 1 title 'Petra MDlist', \
     'outputRomulus.txt'  index 2 every ::0::NUM_ROWS with linespoints linestyle 2 title 'Romulus MDlist', \
     'outputOnefile.txt'  index 2 every ::0::NUM_ROWS with linespoints linestyle 3 title 'Onefile MDlist'
}

set term wxt 3

x = 0
while (x < NUM_ROWS)  {
     pause 1
     stats [*:*][*:*] 'outputOnefile.txt' index 3 u 0 nooutput
     plot 'outputPetra.txt' index 3 every ::0::STATS_records with linespoints linestyle 1 title 'Petra Map', \
     'outputRomulus.txt'  index 3 every ::0::STATS_records with linespoints linestyle 2 title 'Romulus Map', \
     'outputOnefile.txt'  index 3 every ::0::STATS_records with linespoints linestyle 3 title 'Onefile Map'
     x = STATS_records
     #print sprintf("Block 3 STATS_records %d",STATS_records)
} 

while(STATS_blank < 8) {
     stats [*:*][*:*] 'outputOnefile.txt' u 0 nooutput
     plot 'outputPetra.txt' index 3 every ::0::NUM_ROWS with linespoints linestyle 1 title 'Petra Map', \
     'outputRomulus.txt'  index 3 every ::0::NUM_ROWS with linespoints linestyle 2 title 'Romulus Map', \
     'outputOnefile.txt'  index 3 every ::0::NUM_ROWS with linespoints linestyle 3 title 'Onefile Map'
}

