#!/bin/bash

MAX=40
gnuplot -e "while(1){pause 0.1;stats 'file.dat' u 0 nooutput;
    lPnts=STATS_records<$MAX?0: STATS_records-$MAX;
    plot 'file.dat' using 1:2 skip lPnts title 'google' w st  }"
