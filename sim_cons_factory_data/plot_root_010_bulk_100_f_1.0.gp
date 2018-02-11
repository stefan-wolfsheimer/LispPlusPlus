#!/usr/bin/gnuplot
root_conses=10
bulk_conses=100
fraction=1.0
quantile_file=sprintf("quantile_R%03d_B%03d_F%3.1f.csv", root_conses, bulk_conses, fraction)
fraction_file=sprintf("cfraction_R%03d_B%03d_F%3.1f.csv", root_conses, bulk_conses, fraction)

set title sprintf("Root conses: %d, bulk conses: %d, edge-fraction: %f", root_conses, bulk_conses, fraction)
set key autotitle columnhead
set datafile separator ","
set key outside
set key right top
set xlabel "simulation step"
set ylabel "number of conses / edges"
set terminal pngcairo size 600,300 enhanced font 'Verdana,10'
set output sprintf('quantile_R%03d_B%03d_F%3.1f.png', root_conses, bulk_conses, fraction)

plot quantile_file u 1:7  w l,\
     quantile_file u 1:18 w l,\
     quantile_file u 1:29 w l,\
     quantile_file u 1:40 w l,\
     quantile_file u 1:51 w l,\
     quantile_file u 1:62 w l,\
     quantile_file u 1:73 w l

set output sprintf('fractions_R%03d_B%03d_F%3.1f.png', root_conses, bulk_conses, fraction)
set xlabel "simulation step"
set ylabel "cons fraction"

plot fraction_file u 1:2 w l,\
     fraction_file u 1:3 w l,\
     fraction_file u 1:4 w l,\
     fraction_file u 1:5 w l