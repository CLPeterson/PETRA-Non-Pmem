#!/bin/bash

declare TMPDIR=/tmp DEST=${1:-google.com} DELAY=${2:-.25} MAXDATA=100
[ -d /dev/shm ] && [ -w /dev/shm ] && TMPDIR=/dev/shm
read -r TMPDAT < <(mktemp -p "$TMPDIR" --suffix .dat plot-XXXXXXXXXX)

exec {PFD}> >(exec gnuplot)
closExit() { exec {PFD}>&- ; [ -f "$TMPDAT" ] && rm "$TMPDAT" ; exit ;}
trap closExit 0 1 2 3 6 15

getDatas() {
    read -r data < <(
        ping -c 1 -n "$DEST" 2>&1 |
            sed -u 's/^64.*time=\([0-9.]\+\) .*$/\1/p;d'
    )
    now=$EPOCHREALTIME
    printf '%.6f %s\n' >>"$TMPDAT" "$now" "$data"
    (( cnt++ > MAXDATA )) && sed -e 1d -i "$TMPDAT"
    printf ' %8d %(%a %b %d %T)T.%s %s\n' \
            "$cnt" "${now%.*}" "${now#*.}" "$data"
}
getDatas

echo >&$PFD "set term wxt noraise persist title 'Ping $DEST';"
echo >&$PFD "set xdata time;"
echo >&$PFD "set timefmt '%s';"

while ! read -rsn 1 -t "$DELAY" ;do
    getDatas
    echo >&$PFD "plot '$TMPDAT' using 1:2 with line title 'ping $DEST';"
done
