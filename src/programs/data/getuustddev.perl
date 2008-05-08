#!/usr/bin/perl -w

use statfuncs;

$imin = 1;
$imax = 20;
$ntags = shift(@ARGV);

##--------------------------------------------------------
## get_uustddev($i)
##  + gets uniform-uniform stddev for [__$=1, A=$i, X_1=1,...,X_{$ntags-1}=1]
sub get_uustddev {
  my $i = shift;
  my $total = $ntags + $i;
  my %dist = (map { $_=>1/($ntags+1) } (0..$ntags));
  return stddev(\%dist, sub { return $_[0]==0 ? $i/$total : 1/$total; });
}


##--------------------------------------------------------
## MAIN
@data = qw();
for ($i=$imin; $i<=$imax; $i++) {
  push(@data,[$i,get_uustddev($i)]);
}

##-- output
foreach $pair (@data) {
  print $pair->[0], ' ', $pair->[1], "\n";
}
