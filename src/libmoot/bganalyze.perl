#!/usr/bin/perl -w

/*
   libmoot version 1.0.4 : moocow's part-of-speech tagging library
   Copyright (C) 2003 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

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

