#!/usr/bin/perl -w

@scounts = qw();
$slen = 0;
$nsents = 0;

##-- get counts
while (<>) {
  next if ($_ =~ /^\s*\%/);
  if ($_ =~ /^\s*$/) {
    $scounts[$slen]++;
    $nsents++;
    $slen = 0;
    next;
  }
  $slen++;
}
$scounts[$slen]++ if ($slen);

##-- classify
$class_size = 5;
@sclasses = qw();
for ($i = 0; $i <= $#scounts; $i++) {
  $sclasses[int($i/$class_size)] += $scounts[$i] if ($scounts[$i]);
}

##-- report
print STDERR "$0 Summary:\n";
for ($i = 0; $i <= $#sclasses; $i++) {
  $minsize = $i*$class_size;
  next if (!defined($sclasses[$i]));
  print
    STDERR
    ("  $minsize-", $minsize+$class_size,
     "\t: ",
     ($sclasses[$i] || 0),
     "\t (", sprintf("%.2f", 100*$sclasses[$i]/$nsents), "%)",
     "\n",
    );
  print STDOUT $minsize+($class_size/2), "\t", $sclasses[$i], "\n";  ##-- for gnuplot
}
print STDERR "  Total\t: $nsents\n";
