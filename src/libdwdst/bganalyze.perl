#!/usr/bin/perl -w

use DWDS::Tagger::Ngrams;

$ngrams = DWDS::Tagger::Ngrams->new();
$ngrams->load(\*STDIN);

%ctxcounts = ();
foreach $ng (keys(%{$ngrams->{counts}})) {
  @ng = split("\t",$ng);
  next if (@ng != 2);
  $ctxcounts{$ng[0]}++;
}

##-- compute distribution classes & average
$class_size = 5;
$ctxavg = 0;
@ctxclasses = qw();
foreach $tag (keys(%ctxcounts)) {
  $ctxclasses[int($ctxcounts{$tag}/$class_size)]++ if ($ctxcounts{$tag});
  $ctxavg += $ctxcounts{$tag};
}
$ntags = scalar(keys(%ctxcounts));
$ctxavg /= $ntags;

##-- report
print STDERR
  ("$0 Summary:\n",
   "  Avg\t: ", sprintf("%.2f (%.2f%%)\n", $ctxavg, 100*$ctxavg/$ntags),
  );
for ($i = 0; $i <= $#ctxclasses; $i++) {
  next if (!defined($ctxclasses[$i]));
  $minsize = $i*$class_size;
  print STDERR
    (
     "  $minsize-", $minsize+$class_size,
     "\t: ",
     ($ctxclasses[$i] || 0),
     "\t (", sprintf("%.2f", 100*$ctxclasses[$i]/$ntags), "%)",
     "\n"
    );

  print STDOUT $minsize+($class_size/2), "\t", $ctxclasses[$i], "\n";  ##-- for gnuplot
}

