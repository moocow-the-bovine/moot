#!/usr/bin/perl -w

%cf = ();
%tags = ();
$total = 0;
while (<>) {
  next if (/^\s*$/ || /^\s*%%/);
  chomp;
  ($class,$ctotal,%ctags) = split(/\s*\t+\s*/,$_);
  $cf{$class} = {} if (!exists($cf{$class}));

  #$cf{$class}{TOTAL} += $ctotal;
  @{$cf{$class}}{keys(%ctags)} = values(%ctags);
  @tags{keys(%ctags)} = undef;

  $total += $ctotal;
}

sub n_pairs {
  my $n = 0;
  foreach my $se (values(%cf)) {
    $n += scalar(keys(%$se));
  }
  return $n;
}

sub n_impos {
  my $n = 0;
  my ($c,%ch,$cs,$t);
  foreach $c (keys(%cf)) {
    %ch = (map { ($_=>1) } split(/\s+/,$c));
    $cs = $cf{$c};
    foreach $t (keys(%$cs)) {
      $n++ if (!exists($ch{$t}));
    }
  }
  return $n;
}

sub sum_impos {
  my $sum = 0;
  my ($c,%ch,$cs,$t);
  foreach $c (keys(%cf)) {
    %ch = (map { ($_=>1) } split(/\s+/,$c));
    $cs = $cf{$c};
    foreach $t (keys(%$cs)) {
      $sum += $cs->{$t} if (!exists($ch{$t}));
    }
  }
  return $sum;
}

sub sum_total {
  my $sum = 0;
  my ($cs,$f);
  foreach $cs (values(%cf)) {
    foreach $f (values(%$cs)) {
      $sum += $f;
    }
  }
  return $sum;
}

printf("Total    : %8g\n", $total);
printf("NClasses : %8g\n", scalar(keys(%cf)));
printf("NTags    : %8g\n", scalar(keys(%tags)));
printf("NPairs   : %8g\n", n_pairs());
printf("NImpos   : %8g\n", n_impos());
printf("SumImpos : %8g\n",, sum_impos());
printf("SumTotal : %8g\n", sum_total());
