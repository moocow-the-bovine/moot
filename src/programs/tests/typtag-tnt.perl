#!/usr/bin/perl -w

# $USAGE= "$0 < IN.t > OUT.typt";

use vars qw($total);

$tmpfile ="$0.tmp";
open(TMP, ">$tmpfile") or die("$0: could not open temp file '$tmpfile': $!");
while (<>) {
  chomp;
  next if (/^\s*%%/);
  if (/^\s*$/) { print TMP "\n"; next; }

  @fields = split(/\s*\t+\s*/,$_);
  print TMP $fields[0], "\t", "TXT=$fields[0]\n"
}
close(TMP);

$tmplex ="$0.lex";
die("$0: tnt-para run failed: $!") if (system("tnt-para -l $0.tmp") != 0);

open(LEX,"$tmplex") or die("$0: could not open temp lexicon '$tmplex': $!");
%specials = ();
while (<LEX>) {
  chomp;
  next if (/^\s*%%/ || /^\s*$/);

  ($tok,$total,%tagcts) = split(/\s*\t+\s*/, $_);
  next if ($tok !~ /^\@/);

  foreach $tag (keys(%tagcts)) {
    if ($tag =~ /^TXT=(.*)/) {
      $itok = $1;
      $specials{$itok} = $tok;
    }
    else {
      warn("cannot re-parse tag '$tag' for special '$tok'!");
    }
  }
}
close(LEX);

open(TMP, "<$tmpfile") or die("$0: could not re-open temp file '$tmpfile': $!");
while (<TMP>) {
  chomp;
  next if (/^\s*%%/);
  if (/^\s*$/) { print "\n"; next; }

  @fields = split(/\s*\t+\s*/,$_);
  $tok = shift(@fields);
  $typ = $specials{$tok} || "\@ALPHA";
  print $tok, "\t", $typ, "\n";
}
close(TMP);
