#!/usr/bin/perl -w

$imin = 1;
$imax = 20;
$ntags = shift(@ARGV);

##--------------------------------------------------------
## get_pavg($n)
##  + gets average probability [__$=1, A=$i, X_1=1,...,X_{$ntags-1}=1]
sub get_pavg { return 1/($ntags+1); }


##--------------------------------------------------------
## MAIN
@data = qw();
for ($i=$imin; $i<=$imax; $i++) {
  push(@data,[$i,get_pavg($i)]);
}

##-- output
foreach $pair (@data) {
  print $pair->[0], ' ', $pair->[1], "\n";
}
