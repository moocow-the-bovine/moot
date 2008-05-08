#!/usr/bin/perl -w

use statfuncs;

$imin = 1;
$imax = 20;
$ntags = shift(@ARGV);

##--------------------------------------------------------
## get_uwstddev($i)
##  + gets stddev for [__$=1, A=$i, X_1=1,...,X_{$ntags-1}=1]
sub get_wustddev {
  my $i = shift;
  my $total = $ntags + $i;
  my %udist = (map { $_=>1/($ntags+1) } (0..$ntags));
  my %wdist = (0=>$i/$total, (map { ($_=>1/$total) } 1..$ntags));
  my $valsub = sub { return $wdist{$_[0]}; };
  return
    #1/($ntags)*  ##-- ??
    sqrt(expect(\%wdist,
		varfunc(expect(\%udist,$valsub),$valsub)));
}


##--------------------------------------------------------
## MAIN
@data = qw();
for ($i=$imin; $i<=$imax; $i++) {
  push(@data,[$i,get_wustddev($i)]);
}

##-- output
foreach $pair (@data) {
  print $pair->[0], ' ', $pair->[1], "\n";
}
