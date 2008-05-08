#!/usr/bin/gnuplot -persist

set xlabel "f(A)"
set ylabel "theta"
set key right bottom
plot \
   "1tags.dat" with linespoints, \
   "2tags.dat" with linespoints, \
   "3tags.dat" with linespoints, \
   \
   "1btheta.dat" with points


##-- sample estimators
#   "1btheta.dat" with points, \
#   "2btheta.dat" with points, \
#   "3btheta.dat" with points, \

#   "1uustddev.dat" with points, \
#   "2uustddev.dat" with points, \
#   "3uustddev.dat" with points

##-- poisson estimators (?)
#   "1poisson.dat" with points, \
#   "2poisson.dat" with points, \
#   "3poisson.dat" with points

##-- weighted estimators
#   "1uwstddev.dat" with points, \
#   "2uwstddev.dat" with points, \
#   "3uwstddev.dat" with points

#   "1wwstddev.dat" with points, \
#   "2wwstddev.dat" with points, \
#   "3wwstddev.dat" with points

#   "1wustddev.dat" with points, \
#   "2wustddev.dat" with points, \
#   "3wustddev.dat" with points

##-- averages
#   "1wavg.dat" with linespoints, \
#   "2wavg.dat"  with linespoints, \
#   "3wavg.dat" with linespoints

#   "1wavg.dat" using 1:(sqrt($2)) with linespoints, \
#   "2wavg.dat" using 1:(sqrt($2)) with linespoints, \
#   "3wavg.dat" using 1:(sqrt($2)) with linespoints, \

#   "1avg.dat"  with lines, \
#   "2avg.dat"  with lines, \
#   "3avg.dat"  with lines, \
