#!/usr/bin/gnuplot -persist

set autoscale

# set term postscript eps enhanced color
set term postscript eps noenhanced color
set output 'matrix.eps'

#set pointsize 5
#set grid linewidth 0.75

unset key

set xlabel "Producer"
set ylabel "Consumer"
set zlabel "Communication in Bytes"

set xlabel font "Times-Roman, 14" offset 0,-2
set ylabel font "Times-Roman, 14" offset 0,-1

set key samplen 2 spacing .5 font ",8"

#set xrange [0:3]
#set yrange [0:3]
#set zrange [0:100]
#set cbrange [0:100]

# set style fill solid border -1

#set ticslevel 0

#set boxwidth 0.25
set title "Communication Matrix"

XTICS="`awk 'BEGIN{getline}{printf "%s ",$1}' matrix.out`"
YTICS="`head -1 matrix.out`"

set for [i=1:words(XTICS)] xtics ( word(XTICS,i) i-1 )
set for [i=1:words(YTICS)] ytics ( word(YTICS,i) i-1 )

set xtics nomirror rotate by 45 scale 0 font ",7"
set ytics nomirror rotate by 45 scale 0 font ",7"

set grid x
set grid y

#set palette defined (0 "white", 1 "yellow", 2 "green", 3 "blue", 4 "brown", 5 "red")
set palette defined ( 0 "white", 1 "green", 2 "blue", 3 "brown", 4 "red")
# set palette defined (0 0 0 0, 1 0 0 1, 3 0 1 0, 4 1 0 0, 6 1 1 1)

# 7,5,15   ... traditional pm3d (black-blue-red-yellow)
# 3,11,6   ... green-red-violet
# 23,28,3  ... ocean (green-blue-white); try also all other permutations
# 21,22,23 ... hot (black-red-yellow-white)
# 30,31,32 ... color printable on gray (black-blue-violet-yellow-white)
# 33,13,10 ... rainbow (blue-green-yellow-red)
# 34,35,36 ... AFM hot (black-red-yellow-white)
# set palette rgb 34,35,36;

set pm3d map
set pm3d interpolate 2,2
splot "<awk '{$1=\"\"}1' matrix.out | sed '1 d'" matrix with points pointtype 5\
        pointsize 2 palette linewidth 15 title ""
