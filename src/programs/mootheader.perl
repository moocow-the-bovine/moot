#!/usr/bin/perl -w

if (!@ARGV) {
  print STDERR "Usage: $0 HMMFILE\n";
  exit 1;
}

my $hsize = 24;
my $infile = shift;
open(HMM,"<$infile")
  or die("$0: open failed for $infile: $!");

my ($buf);
read(HMM, $buf,$hsize)
  or die("$0: failed to read header from $infile: $!");

my $itype = 'L';
my ($magic,$version,$revision,$minver,$minrev,$flags) = unpack("${itype}6", $buf);

print
  ("File: $infile\n",
   sprintf("+ magic: %#x\n", $magic),
   sprintf("+ version: %d\n", $version),
   sprintf("+ revision: %d\n", $revision),
   sprintf("+ minver: %d\n", $minver),
   sprintf("+ minrev: %d\n", $minrev),
   sprintf("+ flags: %#x\n", $flags),
  );

