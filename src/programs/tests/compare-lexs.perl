#!/usr/bin/perl

$USAGE = "$0 FILE1.lex FILE2.lex";
if (@ARGV < 2) {
  print STDERR "Usage: $USAGE\n";
  exit 1;
}

($file1,$file2) = @ARGV;

open(F1,"<$file1") or die("$0: open failed for '$file1': $!");
open(F2,"<$file2") or die("$0: open failed for '$file2': $!");

$keylen=0;
$countlen=6;

## read_lex($fh,\%lxhash);
sub read_lex {
  my ($fh,$lxh) = @_;
  my (@fields,$tok,$tag,$pair);
  while (<$fh>) {
    chomp;
    next if ($_ =~ /^\s*%%/ || $_ =~ /^\s*$/);
    @fields = split(/\t+/,$_);
    $tok = shift(@fields);
    $count = shift(@fields);
    $lxh->{$tok} = $count;

    #-- update lengths
    $keylen = length($tok) if ($keylen < length($tok));
    $countlen = length($count) if ($countlen < length($count));

    while (defined($tag = shift(@fields))) {
      $count = shift(@fields)||0;
      $pair = "$tok $tag";
      $keylen = length($pair) if (length($pair) > $keylen);
      $countlen = length($count) if (length($count) > $countlen);
      $lxh->{$pair} = $count;
    }
  }
}

read_lex(\*F1,\%lx1);
read_lex(\*F2,\%lx2);

$diff = 0;
for $le (sort(keys(%lx1))) {
  if (!exists($lx2{$le})) {
    $mydiff = 0;
    $mydiff = $lx1{$le} if ($le =~ / /);
    $diff += $mydiff;
    printf("%-${keylen}s = %${countlen}g / %${countlen}s : %d (%d)\n",
	   $le, $lx1{$le}, "(NONE)", $mydiff, $diff);
  }
  elsif ($lx2{$le} != $lx1{$le}) {
    $mydiff = 0;
    $mydiff = ($lx1{$le} - $lx2{$le}) if ($le =~ / /);
    $diff += $mydiff;
    printf("%-${keylen}s = %${countlen}g / %${countlen}g : %d (%d)\n",
	   $le, $lx1{$le}, $lx2{$le}, $mydiff, $diff);
  }
}
for $le (sort(keys(%lx2))) {
  $mydiff = 0;
  if (!exists($lx1{$le})) {
    $mydiff = -$lx2{$le} if ($le =~ / /);
    $diff += $mydiff;
    printf("%-${keylen}s = %${countlen}s / %${countlen}g : %d (%d)\n",
	   $le, "(NONE)", $lx2{$le}, $mydiff, $diff);
  }
}
