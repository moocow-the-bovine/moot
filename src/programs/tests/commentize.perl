#!/usr/bin/perl -w

$eos = 1;
$i = 0;
while (<>) {
  if (/^\s*$/) { $eos = 1; next; }

  if ($eos) {
    print "%% Sentence ", ++$i, "\n";
    $eos = 0;
  }
} continue {
  print;
}
