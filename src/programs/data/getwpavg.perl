#!/usr/bin/perl -w

$imin = 1;
$imax = 20;
$ntags = shift(@ARGV);

##--------------------------------------------------------
## get_wpavg($i)
##  + gets average probability [__$=1, A=$i, X_1=1,...,X_{$ntags-1}=1]
sub get_wpavg {
  my $i = shift;
  my $total = $ntags + $i;
  #print "ntags=$ntags; total=$total\n";
  return ($ntags * (1/$total)**2 + ($i/$total)**2);
}


##--------------------------------------------------------
## MAIN
@data = qw();
for ($i=$imin; $i<=$imax; $i++) {
  push(@data,[$i,get_wpavg($i)]);
}

##-- output
foreach $pair (@data) {
  print $pair->[0], ' ', $pair->[1], "\n";
}
