#!/usr/bin/perl -w

$imin = 1;
$imax = 20;

$ntags = shift(@ARGV);

##--------------------------------------------------------
## get_tnt_theta($n)
##  + gets theta value for ngrams [__$=1,A=$i]
sub get_tnt_theta {
  my $i = shift;
  open(TMP,">tmp.123") or die("$0: could not open 'tmp.123': $!");
  print TMP
    ("__\$\t1\n",
     "A\t$i\n");
  my $n;
  for ($n=1; $n < $ntags; $n++) {
    print TMP "X${n}\t1\n";
  }
  close TMP;

  my $out = `(tnt -a1 tmp /dev/null) 2>&1`;
  my $theta;
  if ($out =~ /^suffix theta = (\S+)$/m) {
    $theta = $1;
  }
  else {
    die("$0: could not get theta!\n");
  }
  return $theta;
}


##--------------------------------------------------------
## MAIN
@data = qw();
for ($i=$imin; $i<=$imax; $i++) {
  push(@data,[$i,get_tnt_theta($i)]);
}

##-- output
foreach $pair (@data) {
  print $pair->[0], ' ', $pair->[1], "\n";
}
