#!/usr/bin/perl -w

use statfuncs;

$imin = 1;
$imax = 20;
$ntags = shift(@ARGV);

##--------------------------------------------------------
## get_upoisson($i)
##  + gets uniform poisson stddev for [__$=1, A=$i, X_1=1,...,X_{$ntags-1}=1]
sub get_b2ktheta {
  my $i = shift;
  my $total = $ntags + $i;
  my $pavg  = 1/($ntags+1);       ##-- uniform E(P(t)): a la Brants2k
  my $theta =
    sqrt(
	 1/($ntags+1)
	 * (($ntags * exp(-2 * 1/$total))
	    +
	    exp(-2*$i/$total))
	);
  return $theta;
}


##--------------------------------------------------------
## MAIN
@data = qw();
for ($i=$imin; $i<=$imax; $i++) {
  push(@data,[$i,get_b2ktheta($i)]);
}

##-- output
foreach $pair (@data) {
  print $pair->[0], ' ', $pair->[1], "\n";
}
