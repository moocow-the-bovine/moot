#!/usr/bin/perl -w

while (<>) {
  chomp;
  print $_, "\n" if (-r $_);
}
