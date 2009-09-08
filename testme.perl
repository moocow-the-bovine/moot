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
  my $a0 = $as->front(); ##-- should puke, since $as is empty
  print STDERR "$0: test_token_analysis(): done\n";
}
#test_token_analysis();

##----------------------------------------------------------------------
## test: Token

sub test_token {
  my $t = moot::Token->new("foo", $moot::TokTypeVanilla);
  my $as = moot::TokenAnalyses->new();
  $t->insert(moot::TokenAnalysis->new('tag1','details1',1));
  $t->insert(moot::TokenAnalysis->new('tag2','details2',2));
  $t->insert(moot::TokenAnalysis->new('tag3','details3',3.14195));
  $t->{tok_besttag} = 'tag42';

  my $s = moot::Sentence->new();
  $s->push_back(moot::Token->new('foo'));
  $s->push_back(moot::Token->new('bar'));

  print STDERR "$0: test_token(): done\n";
}
#test_token();

##----------------------------------------------------------------------
## test: AssocVector

sub test_assoc_vector {
  my $av = moot::IdProbAssocVector->new();
  $av->insert(420,24.7);
  $av->insert(24, 48.96);

  print STDERR "$0: test_assoc_vector(): done\n";
}
#test_assoc_vector();


##----------------------------------------------------------------------
## test: HMM

sub test_hmm {
  my $hmm = moot::HMM->new();
  $hmm->{hash_ngrams} = 1;
  #my $model = '../src/programs/data/utrain';
  my $model = '../src/programs/data/utrain-yy.hmm';
  #my $model = 'dummy';
  $hmm->load_model($model) or warn("$0: load_model($model) failed: $!");
  #$hmm->txtdump('test.dump');

  my $s = moot::Sentence->new();
  $s->push_back(moot::Token->new($_)) foreach (qw(Dies ist ein Test .));
  $hmm->tag_sentence($s);

  ##-- test lexprobs
  my $lp = $hmm->{lexprobs};
  print "lp.size = ", $lp->size, "\n";

  ##-- test n-grams
  my $ngh = $hmm->{ngprobsh};
  print "ngh.size = ", $ngh->size, "\n";

  print STDERR "$0: test_hmm(): done\n";
}
test_hmm();


##----------------------------------------------------------------------
## dummy
foreach $i (1..3) {
  print STDERR "dummy[$i]\n";
}
