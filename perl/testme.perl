#!/usr/bin/perl -w

use lib qw(./blib/lib ./blib/arch);
use IO::Handle;
use Data::Dumper;
use Encode qw(encode decode encode_utf8 decode_utf8);
use Moot;
use Devel::Peek;

BEGIN {
  *refcnt = \&Devel::Peek::SvREFCNT;
}

##--------------------------------------------------------------
## test: basic

sub test_libversion {
  print STDERR "Moot libversion=", Moot::library_version, "\n";
}
#test_libversion();

##--------------------------------------------------------------
## test: lexfreqs

sub test_lexfreqs {
  my $lffile = shift || 'test0.lex';
  my $lf = Moot::Lexfreqs->new() or die ("Moot::Lexfreqs->new() failed: $!");
  $lf->load($lffile) or die("Lexfreqs->load($lffile) failed: $!");
  print "$0: loaded '$lffile'; n_tokens=", $lf->n_tokens, "; n_pairs=", $lf->n_pairs, "\n";
  $lf->add_count('mytext','mytag',42);
  print "$0: post-add: n_tokens=", $lf->n_tokens, "; n_pairs=", $lf->n_pairs, "\n";

  $lf->add_count('mytext','mytag1',1);
  $lf->add_count('mytext','mytag2',2);
  print "$0: post-add1+2: n_tokens=", $lf->n_tokens, "; n_pairs=", $lf->n_pairs, "\n";
  print "$0: f_word(mytext)=", $lf->f_word('mytext'), ' : ', $lf->lookup('mytext'), "\n";
  print "$0: f_tag(mytag1)=", $lf->f_tag('mytag1'), ' : ', $lf->lookup(undef,'mytag1'), "\n";
  print "$0: f_tag(mytag2)=", $lf->f_tag('mytag2'), ' : ', $lf->lookup(undef,'mytag2'), "\n";
  print "$0: f_word_tag(mytext,mytag)=", $lf->f_word_tag('mytext','mytag'), ' : ', $lf->lookup(qw(mytext mytag)), "\n";
  ##
  print "$0: saveFile(-)\n";
  $lf->saveFile("-") or die("failed: $!");

  print "$0: saveFh(STDOUT)\n";
  $lf->save(\*STDOUT) or die("failed: $!");

  print "$0: save(-)\n";
  $lf->save("-") or die("failed: $!");

  if (0) {
    ##--segfaults
    print "$0: save(STRING_HANDLE)\n";
    my $ioh=IO::Handle->new;
    my $str='';
    open($ioh,'>',\$str);
    $lf->save($ioh) or die "failed: $!";
    print $str;
  }
}
#test_lexfreqs(@ARGV);

##--------------------------------------------------------------
## test: ngrams

sub test_ngrams {
  my $ngfile = shift || 'test0.123';
  my $ng = Moot::Ngrams->new() or die ("Moot::Ngrams->new() failed: $!");
  $ng->load($ngfile) or die("Ngrams->load($ngfile) failed: $!");

  print "$0: loaded '$ngfile'; n_tokens=", $ng->n_tokens, "; n_1,2,3=", join(", ", $ng->n_1grams, $ng->n_2grams, $ng->n_3grams), "\n";
  $ng->add_count(qw(tag1 42));
  $ng->add_count(qw(tag1 tag2 2));
  $ng->add_count(qw(tag1 tag2 tag3 3));
  print "$0: post-add: n_1,2,3=", join(", ", $ng->n_1grams, $ng->n_2grams, $ng->n_3grams), "\n";

  print "$0: lookup(tag1)=", $ng->lookup(qw(tag1)), "\n";
  print "$0: lookup(tag1 tag2)=", $ng->lookup(qw(tag1 tag2)), "\n";
  print "$0: lookup(tag1 tag2 tag3)=", $ng->lookup(qw(tag1 tag2 tag3)), "\n";

  print "$0: saveFile(-)\n";
  $ng->saveFile("-") or die("failed: $!");

  print "$0: saveFh(STDOUT)\n";
  $ng->save(\*STDOUT) or die("failed: $!");

  print "$0: save(-)\n";
  $ng->save("-") or die("failed: $!");

  print "$0: save(-,1)\n";
  $ng->save("-",1) or die("failed: $!");
}
#test_ngrams(@ARGV);


##--------------------------------------------------------------
## test: HMM

sub hmmstats {
  my ($hmm,$label) = @_;
  print
    ("$0: HMM stats ($label)\n",
     map {"$_=".$hmm->can($_)->($hmm)."\n"}
     qw(verbose ndots save_ambiguities save_flavors save_mark_unknown),
     qw(hash_ngrams relax use_lex_classes start_tagid),
     qw(unknown_lex_threshhold unknown_class_threshhold),
     qw(nglambda1 nglambda2 nglambda3),
     qw(wlambda0 wlambda1 clambda0 clambda1),
     qw(beamwd),
     qw(nsents ntokens nnewtokens nunclassed nnewclasses nunknown nfallbacks),
    );
}

sub tagit {
  my ($hmm,$s,$utf8) = @_;
  $s = [map {ref($_) ? $_ : {text=>$_}} @$s];
  print "$0: tag_sentence[utf8=", ($utf8 ? 1 : 0), "]:  IN = (", join(' ', map {$_->{text}} @$s), ")\n";
  $hmm->tag_sentence($s,$utf8);
  print "$0: tag_sentence[utf8=", ($utf8 ? 1 : 0), "]: OUT = (", join(' ', map {"$_->{text}/$_->{tag}"} @$s), ")\n\n";
  return $s;
}

sub test_hmm {
  my $modelfile = shift || 'negra-train';
  my ($hmm);

  print "$0: loadModel($modelfile)";
  $hmm = Moot::HMM->new() or die ("Moot::HMM->new() failed: $!");
  $hmm->load($modelfile) or die "failed: $!";
  #hmmstats($hmm,"model=$modelfile");

  if (0) {
    ##-- ok
    my $binfile = shift || 'negra-train.hmm';
    print "$0: load($binfile) [bin]";
    $hmm = Moot::HMM->new();
    $hmm->load($binfile) or die "failed: $!";
    hmmstats($hmm,"bin=$binfile");
  }

  ##-- try to tag a sentence
  my @s  = qw(Dies ist ein Test .);
  print "$0: tag_sentence(", join(' ', @s), ")\n";
  my $s  = [map {{text=>$_}} @s];
  $hmm->tag_sentence($s);
  print "$0: got: (", join(' ', map {"$_->{text}/$_->{tag}"} @$s), ")\n";

  ##-- try utf8
  print "::raw\n";
  tagit($hmm, [split(/ /,"a \x{f6}de test")], 0); ##-- bad
  tagit($hmm, [split(/ /,"a \x{f6}de test")], 1); ##-- good
  ##
  print "::encode_utf8(raw)\n";
  tagit($hmm, [split(/ /,encode_utf8("a \x{f6}de test"))], 0); ##-- good
  tagit($hmm, [split(/ /,encode_utf8("a \x{f6}de test"))], 1); ##-- bad (double-encoded)
  ##
  print "::decode_utf8(encode_utf8(raw))\n";
  tagit($hmm, [split(/ /,decode_utf8(encode_utf8("a \x{f6}de test")))], 0); ##-- good
  tagit($hmm, [split(/ /,decode_utf8(encode_utf8("a \x{f6}de test")))], 1); ##-- good

  if (0) {
    ##-- test for memory leaks: looks good
    while (1) {
      $hmm->tag_sentence([map {{text=>$_}} @s]);
    }
  }
}
#test_hmm(@ARGV);

##--------------------------------------------------------------
## wasteScanner

sub test_wscan {
  my ($which,$what) = @_ ? @_ : (file=>'scanme.txt');
  my $ws = Moot::Waste::Scanner->new()
    or die("$0: Moot::Waste::Scanner->new() failed");

  my ($fh);
  if ($which eq 'file') {
    $ws->from_file($what);
  }
  elsif ($which eq 'fh') {
    $ws->from_fh($what);
  }
  elsif ($which eq 'string') {
    ##-- from_fh() doesn't work with string-handles if wrapped with FILE* type
    #open($fh, "<", \$what) or die("$0: open failed for string filehandle");
    #$ws->from_fh($fh);
    ##
    #$ws->from_string($what);
    ##
    ##-- from_fh() doesn't work with string-handles if wrapped with FILE* type
    open($fh, "<", \$what) or die("$0: open failed for string filehandle");
    $ws->from_fh($fh);
  }
  else {
    die("$0: unknown 'which'=$which in test_wscan()");
  }

  ##-- ye olde loope
  my ($tok);
  $Data::Dumper::Indent = 0;
  $Data::Dumper::Terse = 1;
  $Data::Dumper::Pair = '=>';
  $Data::Dumper::Sortkeys = 1;
  $|=1;
  while (defined($tok=$ws->get_token)) {
    print Dumper($tok), "\n";
  }

  ##-- cleanup
  $ws->close();
  undef $ws;

  print STDERR "test_wscan() done\n";
}
#test_wscan(file=>'scanme.txt');
#test_wscan(fh=>\*STDIN);
test_wscan(string=>"Test 123\nfoo bar.");
##
#$buf = ("Test 123.456\nfoo bar." x 1024); test_wscan(string=>$buf);

##--------------------------------------------------------------
## MAIN

##-- dummy
sub main_dummy {
  foreach $i (1..3) {
    print "--dummy($i)--\n";
  }
}
main_dummy();

