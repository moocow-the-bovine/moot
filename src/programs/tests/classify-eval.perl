#!/usr/bin/perl -w

while (<>) {
  if (/^\s*%%/ || /^\s*$/) { print; next; }
  chomp;
  @tokens = split(/\s*\t+\s*\/\s*\t+\s*/, $_);
  ($status,@tok1) = split(/\s*\t+\s*/, $tokens[0]);
  @tok2           = split(/\s*\t+\s*/, $tokens[1]);
  $tok1[0] = $tok2[0] = '(text)';
  print join("\t", $status, @tok1, '/', @tok2), "\n";
}
