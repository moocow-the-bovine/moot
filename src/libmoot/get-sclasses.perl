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

@scounts = qw();
$slen = 0;
$nsents = 0;

##-- get counts
while (<>) {
  next if ($_ =~ /^\s*\%/);
  if ($_ =~ /^\s*$/) {
    $scounts[$slen]++;
    $nsents++;
    $slen = 0;
    next;
  }
  $slen++;
}
$scounts[$slen]++ if ($slen);

##-- classify
$class_size = 5;
@sclasses = qw();
for ($i = 0; $i <= $#scounts; $i++) {
  $sclasses[int($i/$class_size)] += $scounts[$i] if ($scounts[$i]);
}

##-- report
print STDERR "$0 Summary:\n";
for ($i = 0; $i <= $#sclasses; $i++) {
  $minsize = $i*$class_size;
  next if (!defined($sclasses[$i]));
  print
    STDERR
    ("  $minsize-", $minsize+$class_size,
     "\t: ",
     ($sclasses[$i] || 0),
     "\t (", sprintf("%.2f", 100*$sclasses[$i]/$nsents), "%)",
     "\n",
    );
  print STDOUT $minsize+($class_size/2), "\t", $sclasses[$i], "\n";  ##-- for gnuplot
}
print STDERR "  Total\t: $nsents\n";
