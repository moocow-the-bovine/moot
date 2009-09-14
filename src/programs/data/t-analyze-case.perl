#!/usr/bin/perl -w

##-- for pseudo-frequencies
our ($f_id,$f_lc,$f_uc0,$f_uc);

our $costs_are_freqs = 0;
if ($costs_are_freqs) {
  ##-- set analysis 'costs' as pseudo-frequencies
  $f_id   = 2;
  $f_lc   = 1;
  $f_uc0  = 1;
  $f_uc   = 0.5;
} else {
  ##-- set analysis 'costs' as distances
  $f_id   = 0;
  $f_lc   = 1;
  $f_uc0  = 1;
  $f_uc   = 2;
}

my ($rest);

while (<>) {
  if (/^\%%/ || /^\s*$/) {
    print;
    next;
  }
  chomp;
  ($w,$rest) = split(/\t+/,$_,2);

  $w_std = $w;
  $w_std =~ s/^(.)(.*)$/$1\L$2\E/;  ##-- $w_std: lower-cased non-initial characters

  $w_lc  = lc($w_std);      ##-- $w_lower: all lower
  $w_uc0 = ucfirst($w_std); ##-- $w_upper: upper-cased first character
  $w_uc  = uc($w_std);

  @an = (
	 ("[$w] <$f_id>"),                           ##-- literal (identity)
	 ($w_lc  eq $w ? qw() : "[$w_lc] <$f_lc>"),   ##-- lower-cased
	 ($w_uc0 eq $w ? qw() : "[$w_uc0] <$f_uc0>"), ##-- upper-cased initial only
	 ($w_uc  eq $w ? qw() : "[$w_uc]  <$f_uc>"),  ##-- upper-cased all
	);
  print join("\t", $w, @an)."\n";
}

