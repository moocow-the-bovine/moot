#!/usr/bin/perl

$USAGE = "$0 FILE1.lex FILE2.lex";
if (@ARGV < 2) {
  print STDERR "Usage: $USAGE\n";
  exit 1;
}

($file1,$file2) = @ARGV;

open(F1,"<$file1") or die("$0: open failed for '$file1': $!");
open(F2,"<$file2") or die("$0: open failed for '$file2': $!");

## read_lex($fh,\%lxhash);
sub read_lex {
  my ($fh,$lxh) = @_;
  my (@fields,$tok,$tag);
  while (<$fh>) {
    chomp;
    next if ($_ =~ /^\s*%%/ || $_ =~ /^\s*$/);
    @fields = split(/\t+/,$_);
    $tok = shift(@fields);
    $count = shift(@fields);
    $lxh->{$tok} = $count;
    while (defined($tag = shift(@fields))) {
      $lxh->{"$tok\t$tag"} = shift(@fields)||0;
    }
  }
}

read_lex(\*F1,\%lx1);
read_lex(\*F2,\%lx2);

for $le (sort(keys(%lx1))) {
  if (!exists($lx2{$le})) {
    print "$le \t= $lx1{$le}\t/\t (NONE)\n";
  }
  elsif ($lx2{$le} != $lx1{$le}) {
    print "$le \t= $lx1{$le} /\t $lx2{$le}\n";
  }
}
for $le (sort(keys(%lx2))) {
  if (!exists($lx1{$le})) {
    print "$le \t= (NONE)\t/\t $lx2{$le}\n";
  }
}
