#!/usr/bin/perl -w

use vars qw($tokct);

$USAGE= "$0 SUFFIX LEXFILE";
if (@ARGV < 2) {
  print STDERR "Usage: $USAGE\n";
  exit 1;
}

$suffix   = shift(@ARGV);
$sufidx   = -length($suffix);
$suftotal = 0;
%ugcounts = qw();
%counts = qw();
while (<>) {
  next if (/^\s*$/ || /^\s*%%/);
  chomp;
  ($tok,$tokct,%tagcts) = split(/\s*\t+\s*/,$_);

  if (substr($tok,$sufidx) eq $suffix) {
    #print "tok=$tok\n";
    while (($tag,$ct) = each(%tagcts)) {
      #print " : tag=$tag ; ct=$ct\n";
      $suftotal += $ct;
      $counts{$tag} = 0 if (!exists($counts{$tag}));
      $counts{$tag} += $ct;

      $ugcounts{$tag} = 0 if (!exists($ugcounts{$tag}));
      $ugcounts{$tag} += $ct;
    }
  }
  else {
    ##-- just get unigram totals
    while (($tag,$ct) = each(%tagcts)) {
      $ugcounts{$tag} = 0 if (!exists($ugcounts{$tag}));
      $ugcounts{$tag} += $ct;
    }
  }
}

##-- output
foreach $tag (sort
	      {
		$counts{$a}/$ugcounts{$a} <=> $counts{$b}/$ugcounts{$b}
	      }
	      keys(%counts))
  {
    printf
      ("$tag\t %9u / %9u ==> p(suf|tag) = %-9.2g\n",
       $counts{$tag},
       $ugcounts{$tag},
       $counts{$tag} / $ugcounts{$tag},
      );
  }
