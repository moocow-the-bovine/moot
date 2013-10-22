#!/usr/bin/perl -w

use open ':std',':utf8';
use utf8;

sub hexbytes_re2c {
  return map {sprintf("[\\x%0.2x]",$_)} unpack('U0C*',$_[0]);
}
sub hexbytes_c {
  return map {sprintf("\\x%0.2x",$_)} unpack('U0C*',$_[0]);
}
sub quoteit {
  my $s = quotemeta(shift);
  $s =~ s/\\\r/\\r/g;
  $s =~ s/\\\n/\\n/g;
  $s =~ s/\\\t/\\t/g;
  $s =~ s/\\\v/\\v/g;
  $s =~ s/\\\f/\\f/g;
  $s =~ s/\{/[LBRACE]/g;
  $s =~ s/\}/[RBRACE]/g;
  return $s;
}

while (<>) {
  if (/^\s*%%ULC%%\s*$/) {
    ##-- expand: lower-case rules
    foreach $i (1..65535) {
      next if ($i >= 0xD800 && $i <= 0xDFFF); #+++++++ utf16 surrogates, not legal utf8: IGNORE +++++++
      #no warnings '';
      $c  = pack('U0U*',$i);
      $lc = lc($c);
      #next if ($c eq $lc);
      print hexbytes_re2c($c),"\t{ ls += \"",hexbytes_c($lc),"\"; ",sprintf("\t/* %s -> %s == U+%0.4x -> U+%0.4x */ }\n", quoteit($c),quoteit($lc),ord($c),ord($lc));
    }
    next;
  }
  print;
}

