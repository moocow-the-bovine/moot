#!/usr/bin/perl -w

$nchars = shift;
$nchars = 8180 if (!defined($nchars));

print 'x' x $nchars, "\n";

