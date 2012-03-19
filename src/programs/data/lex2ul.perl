#!/usr/bin/perl -w

##-- do I/O in utf8
use open ':utf8';
BEGIN {
  binmode(\*STDIN,':utf8');
  binmode(\*STDOUT,':utf8');
}

my %xf =
  (
   '@LOWER' => {},
   '@UPPER' => {},
  );
my $maxf = 1;


while (<>) {
  chomp;
  if (/^\s*$/ || /^\s*\%\%/) {
    print $_, "\n";
    next;
  }

  ($w,$wf,%tf) = split(/\t/,$_);
  next if (exists($xf{$w}));

  print $_, "\n";
  next if ($w =~ /^\@/ || $wf > $maxf);

  $wxf = $w=~/^[[:upper:]]/ ? $xf{'@UPPER'} : ($w=~/^[[:lower:]]/ ? $xf{'@LOWER'} : undef);
  next if (!defined($wxf));
  while (($t,$f)=each(%tf)) {
    $wxf->{$t} += $f;
  }
  $wxf->{''} += $wf;
}

##-- dump
foreach $w (sort keys %xf) {
  $wxf = $xf{$w};
  print join("\t",
	     $w, ($wxf->{''}||0),
	     map {($_,$wxf->{$_})} grep {$_ ne ''} sort keys %$wxf), "\n";
}
