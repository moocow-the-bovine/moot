#!/usr/bin/perl -w

use lib qw(./blib/lib ./blib/arch);
use moot;

##----------------------------------------------------------------------
## test: Types

sub test_types {
  my $wrap_version = $moot::VERSION;
  my $lib_version = $moot::PACKAGE_VERSION;
  my $prob_eps = $moot::mootProbEpsilon;
  my $prob_zero = $moot::MOOT_PROB_ZERO;
  my $prob_neg = $moot::MOOT_PROB_NEG;
  my $prob_one = $moot::MOOT_PROB_ONE;
  my $prob_none = $moot::MOOT_PROB_NONE;
  print STDERR "$0: test_types(): done\n";
}
#test_types();

##----------------------------------------------------------------------
## test: Token: Analysis

sub test_token_analysis {
  my $a1 = moot::TokenAnalysis->new('mytag','mydetails',42.0);
  my $a2 = moot::TokenAnalysis->new('mytag2','mydetails2',24.0);

  my $as = moot::TokenAnalyses->new();
  print STDERR "$0: test_token_analysis(): done\n";
}
#test_token_analysis();

##----------------------------------------------------------------------
## test: Token

sub test_token {
  my $t = moot::Token->new("foo", $moot::TokTypeVanilla);
  $t->insert(moot::TokenAnalysis->new('tag1','details1',1));
  $t->insert(moot::TokenAnalysis->new('tag2','details2',2));
  $t->insert(moot::TokenAnalysis->new('tag3','details3',3.14195));
  $t->{tok_besttag} = 'tag42';

  print STDERR "$0: test_token(): done\n";
}
test_token();


##----------------------------------------------------------------------
## dummy
foreach $i (1..3) {
  print STDERR "dummy[$i]\n";
}
