#!/usr/bin/perl -w

use statfuncs;

$imin = 1;
$imax = 20;
$ntags = shift(@ARGV);

##--------------------------------------------------------
## get_wwstddev($i)
##  + gets weighted-weighted stddev for [__$=1, A=$i, X_1=1,...,X_{$ntags-1}=1]
sub get_wwstddev {
  my $i = shift;
  my $total = $ntags + $i;
  my %wdist = (0=>$i/$total, (map { ($_=>1/$total) } 1..$ntags));
  my $valsub = sub { return $wdist{$_[0]}; };
  return
    sqrt(expect(\%wdist,
		varfunc(expect(\%wdist,$valsub),$valsub)));
}


##--------------------------------------------------------
## MAIN
@data = qw();
for ($i=$imin; $i<=$imax; $i++) {
  push(@data,[$i,get_wwstddev($i)]);
}

##-- output
foreach $pair (@data) {
  print $pair->[0], ' ', $pair->[1], "\n";
}
