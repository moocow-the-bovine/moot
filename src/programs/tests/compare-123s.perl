#!/usr/bin/perl

$USAGE = "$0 FILE1.123 FILE2.123";
if (@ARGV < 2) {
  print STDERR "Usage: $USAGE\n";
  exit 1;
}

($file1,$file2) = @ARGV;

open(F1,"<$file1") or die("$0: open failed for '$file1': $!");
open(F2,"<$file2") or die("$0: open failed for '$file2': $!");

## read_123($fh,\%nghash);
sub read_123 {
  my ($fh,$ngh) = @_;
  my (@fields,$count);
  while (<$fh>) {
    chomp;
    next if ($_ =~ /^\s*%%/ || $_ =~ /^\s*$/);
    @fields = split(/\t+/,$_);
    $count = pop(@fields);
    $ngh->{join("\t",@fields)} = $count;
  }
}

read_123(\*F1,\%ng1);
read_123(\*F2,\%ng2);

for $ng (sort(keys(%ng1))) {
  if (!exists($ng2{$ng})) {
    print "$ng \t= $ng1{$ng}\t/\t (NONE)\n";
  }
  elsif ($ng2{$ng} != $ng1{$ng}) {
    print "$ng \t= $ng1{$ng} /\t $ng2{$ng}\n";
  }
}
for $ng (sort(keys(%ng2))) {
  if (!exists($ng1{$ng})) {
    print "$ng \t= (NONE)\t/\t $ng2{$ng}\n";
  }
}
