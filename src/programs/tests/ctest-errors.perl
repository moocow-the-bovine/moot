#!/usr/bin/perl -w

while (<>) {
  chomp;
  @fields = split;
  $cat = substr($fields[1],1);
  if (!defined($fields[2]) || $fields[2] !~ /=$cat$/) {
    print join("\t", @fields), "\n";
  }
}
