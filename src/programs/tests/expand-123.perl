#!/usr/bin/perl -w

@prev = qw();
@cur  = qw();
while (<>) {
  chomp;
  next if (/^\s*%%/ || /^\s*$/);
  @fields = split(/\t/,$_);
  for ($i = 0; $i < 3 && $i < @fields; $i++) {
    if (!defined($fields[$i]) || $fields[$i] =~ /^\s*$/) {
      $cur[$i] = $prev[$i];
    } else {
      $cur[$i] = $fields[$i];
    }
  }
  print join("\t", grep { defined($_) } (@cur, @fields[3..$#fields])), "\n";
  @prev = @cur;
  @cur = qw();
}
