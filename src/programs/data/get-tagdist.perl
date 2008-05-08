#!/usr/bin/perl -w

use statfuncs;
use utils;

$quant = .01;
$max = .1;
$min = 0;

if (!@ARGV) {
  print STDERR "Usage: $0 NGRAMS\n";
  exit 1;
}

$ngfile = shift(@ARGV);
$ngs = load_123($ngfile);

%quanta = ();
for ($q = $min; $q <= $max; $q += $quant) {
  $quanta{$q/$quant} = 0;
}
$total = $ngs->{$TOTAL};
while (($ng,$ct) = each(%$ngs)) {
  @ngram = split(/\t/,$ng);
  next if (@ngram != 1);
  $p = $ct/$total;
  $q = int($p/$quant);
  ++$quanta{$q};
}

for ($q = $min; $q <= $max; $q += $quant) {
  print $q, "\t", $quanta{$q/$quant}, "\n";
}
