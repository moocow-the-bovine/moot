#!/usr/bin/perl -w

while (<>) {
  if (/^\s*%%/ || /^\s*$/) { print; next; }
  chomp;
  @fields = split(/\s*\t+\s*/, $_);
  $fields[0] = '(text)';
  print join("\t", @fields), "\n";
}
