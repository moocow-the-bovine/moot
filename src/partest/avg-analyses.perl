#!/usr/local/bin/perl -w

$nanals = 0;
$ntokens = 0;

$min_anals = $max_anals = $minnz_anals = undef;
$min_word = $max_word = $minnz_word = undef;

$total_alen = 0;
$min_alen = $max_alen = undef;
$min_asyms = $max_asyms = undef;

%allsyms = ();
%allsymsets = ();
$symset = '';

while (defined($_ = <>)) {
  chomp;

  if ($symset && $_ =~ /^\s*$/) {
    $allsymsets{$symset} = '';
  }

  if ($_ =~ /(\S+)\: (\d+) Analyse/) {
    $symset = '';
    ($word,$anals) = ($1,$2);

    if (!defined($min_anals) || $anals < $min_anals) {
      $min_word = $word;
      $min_anals = $anals;
    }
    if ($anals != 0 && (!defined($minnz_anals) || $anals < $minnz_anals)) {
      $minnz_word = $word;
      $minnz_anals = $anals;
    }
    if (!defined($max_anals) || $anals > $max_anals) {
      $max_word = $word;
      $max_anals = $anals;
    }

    $nanals += $anals;
    $ntokens++;
  }
  elsif ($_ =~ /^\t/) {
    # it's an analysis
    $_ =~ s/^\s*//;
    $_ =~ s/\s*$//;
    @syms = split(/\[/);
    $nsyms = scalar(@syms);
    if (!defined($min_alen) || $nsyms < $min_alen) {
      $min_alen = $nsyms;
      $min_asyms = $_;
    }
    if (!defined($max_alen) || $nsyms > $max_alen) {
      $max_alen = $nsyms;
      $max_asyms = $_;
    }
    $allsyms{$_} = undef;
    $total_alen += $nsyms;
    $symset .= ' & '.$_;
  }
}

print
  ("$0 Summary:\n",
   "       Total Tokens: $ntokens\n",
   "     Total Analyses: $nanals\n",
   "   Average Analyses: ", $nanals/$ntokens, "\n",
   "\n",
   "       Min Analyses: $min_anals ($min_word)\n",
   "      Min (nonzero): $minnz_anals ($minnz_word)\n",
   "       Max Analyses: $max_anals ($max_word)\n",
   "\n",
   " Total Analysis-Len: $total_alen\n",
   "  Avg Syms/Analysis: ", $total_alen/$nanals, "\n",
   "\n",
   "   Min Analysis-Len: $min_alen ($min_asyms)\n",
   "   Max Analysis-Len: $max_alen ($max_asyms)\n",
   "\n",
   "    num/pseudotags: ", scalar(keys(%allsyms)), "\n",
   "      num/ptagsets: ", scalar(keys(%allsymsets)), "\n",
  );


sub show_symsets {
  print
    ("Symsets:\n");
  $i = 0;
  foreach (sort(keys(%allsymsets))) {
    print "$i\t $_\n";
    $i++;
  }
}
