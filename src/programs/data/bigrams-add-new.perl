#!/usr/bin/perl -w

my $f_new = 0.5;
my $t_new = '@NEW';

##-- get unigram total, passing data through
my $N=0;
my %f1=qw();
while (<>) {
  if (/^(\S+)\s+(\S+)$/) {
    $N += $2;
    $f1{$1} += $2;
  }
  print;
}

##-- add '@NEW' tag if not already present
if (!exists($f1{$t_new})) {
  $f1{$t_new} = $f_new;
  $N += $f_new;
  print "$t_new\t$f_new\n";
} else {
  $f_new = $f1{$t_new};
}

##-- create new bigrams
foreach (sort(keys(%f1))) {
  $fwnew = $f_new * $f1{$_}/$N;
  print
    ("$t_new\t$_\t$fwnew\n",
     "$_\t$t_new\t$fwnew\n",
    );
}
