#!/usr/local/bin/perl -w

$cmax = $cmin = undef;
@dcounts = qw();
while (defined($line = <>)) {
  chomp($line);
  @data = split(/\s+/,$line);
  $dcounts[$#data] = 0 if (!defined($dcounts[$#data]));
  ++$dcounts[$#data];
  if ($#data == 1 || $#data == 3) {
    $cost = pop(@data);
    $cmin = $cost if (!defined($cmin) || $cost < $cmin);
    $cmax = $cost if (!defined($cmax) || $cost > $cmax);
  }
}
print STDERR
  ("$0 Summary:\n",
   "  Cost Range: $cmin .. $cmax\n",
   "  Data-lines: (",
   join(",", map { defined($_) ? "$_" : 0 } @dcounts),
   ")\n",
  );
