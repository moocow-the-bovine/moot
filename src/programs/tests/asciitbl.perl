#!/usr/bin/perl -w

for ($i = 0; $i < 256; $i++) {
  print "chr($i)='", chr($i), "'\n";
}

for ($i = 0; $i < 256; $i++) {
  print chr($i);
}
print "\n";
