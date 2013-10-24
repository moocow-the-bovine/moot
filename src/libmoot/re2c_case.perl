#!/usr/bin/perl -w

use open ':std',':utf8';
use utf8;

sub hexbytes_re {
  return join('',map {sprintf("[\\x%0.2x]",$_)} unpack('U0C*',$_[0]));
}
sub hexbytes_c {
  return join('',map {sprintf("\\x%0.2x",$_)} unpack('U0C*',$_[0]));
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
sub rulestr {
  my ($c,$xc) = @_;
  return sprintf("%s\t{ ls += \"%s\"; continue; \t/* %s -> %s == U+%0.4X -> U+%0.4X */ }\n",
		 hexbytes_re($c), hexbytes_c($xc), quoteit($c), quoteit($xc), ord($c),ord($xc));
}

##-- build rule-sets
my @rules_lc = qw();
my @rules_uc = qw();
sub build_rules {
  my @id_lc  = qw(); ##-- $id[bytes::length($inchar)]{$inbytes_str} = $codepoint;
  my @id_uc  = qw();
  my ($i,$c,$lc,$clen);
  foreach $i (0..65535) {
    #no warnings '';
    next if ($i >= 0xD800 && $i <= 0xDFFF); #+++++++ utf16 surrogates, not legal utf8: IGNORE +++++++
    $c  = pack('U0U*',$i);
    $lc = lc($c);
    $uc = uc($c);
    if ($c eq $lc) {
      ##-- lc: identity
      use bytes;
      $id_lc[bytes::length($c)]{ hexbytes_re($c) } = $i;
    }
    else {
      ##-- lc: non-identity
      push(@rules_lc, rulestr($c,$lc));
    }
    if ($c eq $uc) {
      ##-- uc: identity
      use bytes;
      $id_uc[bytes::length($c)]{ hexbytes_re($c) } = $i;
    }
    else {
      ##-- uc: non-identity
      push(@rules_uc, rulestr($c,$uc));
    }
  }

  ##-- lc: append identity rules
  foreach my $i (grep {$id_lc[$_]} (1..$#id_lc)) {
    push(@rules_lc,
	 "\n",
	 "LcId${i}\t= ", join("|", map {"($_)"} sort keys %{$id_lc[$i]}), ";\n",
	 "LcId${i}\t{ ls.append(sp-$i, sp); continue; /* tolower() identity fallback, $i byte(s)/char */ }\n",
	);
  }

  ##-- uc: append identity rules
  foreach my $i (grep {$id_uc[$_]} (1..$#id_uc)) {
    push(@rules_uc,
	 "\n",
	 "UcId${i}\t= ", join("|", map {"($_)"} sort keys %{$id_uc[$i]}), ";\n",
	 "UcId${i}\t{ ls.append(sp-$i, sp); continue; /* toupper() identity fallback, $i byte(s)/char */ }\n",
	);
  }
}
build_rules();


##-- main loop
while (<>) {
  if (/^\s*%%ULC%%\s*$/) {
    print @rules_lc;
  }
  elsif (/^\s*%%UUC%%\s*$/) {
    print @rules_uc;
  }
  else {
    print;
  }
}
