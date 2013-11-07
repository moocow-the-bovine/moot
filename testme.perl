#!/usr/bin/perl -w

use lib qw(./blib/lib ./blib/arch);
use IO::Handle;
use Data::Dumper;
use Encode qw(encode decode encode_utf8 decode_utf8);
use Moot;
use Devel::Peek;

use open ':std',':utf8';

BEGIN {
  *refcnt = \&Devel::Peek::SvREFCNT;
  *svdump = \&Devel::Peek::Dump;
  *svdumpa = \&Devel::Peek::DumpArray;
}
sub svdumph {
  my ($k,$v);
  while (@_) {
    ($k,$v)=splice(@_,0,2);
    print STDERR "--\n$k = ";
    svdump($v);
  }
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

sub scanner_pump {
  my $tr = shift;

  $Data::Dumper::Indent = 0;
  $Data::Dumper::Terse = 1;
  $Data::Dumper::Pair = '=>';
  $Data::Dumper::Sortkeys = 1;
  $|=1;

  my ($w);
  while (defined($w=$tr->get_token)) {
    $w->{type} .= ' '.$Moot::TokType[$w->{type}];
    $w->{utf8}  = utf8::is_utf8($w->{text});
    print ' ',Dumper($w),"\n";
  }
}

sub test_wscan {
  my ($which,$what) = @_ ? @_ : (file=>'scanme.txt');
  my $ws = Moot::Waste::Scanner->new( )
    or die("$0: Moot::Waste::Scanner->new() failed");

  reader_from($ws,$which,$what);
  scanner_pump($ws);

  ##-- cleanup
  $ws->close();
  undef $ws;

  print STDERR "test_wscan() done\n"; exit 0;
}
#test_wscan(file=>'scanme.txt');
#test_wscan(fh=>\*STDIN);
#test_wscan(string=>"Test 123\nfoo bar.");
#test_wscan(stringfh=>"Test 123\nfoo bar.");
#test_wscan(file=>'argh.txt');
##
#$buf = ("Test 123.456\nfoo bar." x 1024); test_wscan(stringfh=>$buf);

##--------------------------------------------------------------
## I/O formats

sub test_iofmt {
  my $i  = Moot::tiofWellDone();
  my $i1 = Moot::tiofText | Moot::tiofTagged | Moot::tiofAnalyzed;
  my $i2 = $Moot::ioFormat{welldone};
  my $i3 = Moot::TokenIO::parse_format_string('wd');
  my $i4 = Moot::TokenIO::guess_filename_format('foo.wd');

  my $rc = $i==$i1 && $i==$i2 && $i==$i3 && $i==$i4;
  print "i/o formats: numeric: ", ($rc ? 'ok' : 'NOT ok'), "\n";

  my $s1 = Moot::TokenIO::format_canonical_string($i);
  my $s2 = Moot::TokenIO::format_canonical_string($i|Moot::tiofNative|Moot::tiofLocation);
  $rc = ($s1 eq 'Text,Analyzed,Tagged') && ($s2 eq "Native,$s1,Location");
  print "i/o formats: names: ", ($rc ? 'ok' : 'NOT ok'), "\n";

  print STDERR "test_iofmt() done\n";
}
#test_iofmt();

##--------------------------------------------------------------
## TokenIO: common

sub fmt2str {
 my $fmt = shift // 0;
  return sprintf("%d=0x%x=%s", $fmt, $fmt, Moot::TokenIO::format_canonical_string($fmt));
}
sub sent2str {
  my $s = shift;
  $Data::Dumper::Indent = 0;
  $Data::Dumper::Terse = 1;
  $Data::Dumper::Pair = '=>';
  $Data::Dumper::Sortkeys = 1;
  $|=1;
  return ("[\n"
	  .join('',map {'  '.Dumper($_).",\n"} @$s)
	  ."]\n");
}
sub reader_from {
  my ($tr,$which,$what) = @_;
  my ($fh);

  if ($which eq 'file') {
    $tr->from_file($what);
  }
  elsif ($which eq 'filefh') {
    open($fh, "<", $what) or die("$0: open failed for filefh filehandle");
    $tr->from_fh($fh);
  }
  elsif ($which eq 'fh') {
    $tr->from_fh($what);
  }
  elsif ($which eq 'string') {
    $tr->from_string($what);
  }
  elsif ($which eq 'stringfh') {
    ##-- from_fh() doesn't work with string-handles if wrapped with FILE* type
    #open($fh, "<", \$what) or die("$0: open failed for string filehandle");
    #$tr->from_fh($fh);
    ##
    #$tr->from_string($what);
    ##
    ##-- from_fh() doesn't work with string-handles if wrapped with FILE* type
    open($fh, "<", \$what) or die("$0: open failed for string filehandle");
    $tr->from_fh($fh);
  }
  else {
    die("$0: unknown 'which'=$which in reader_from()");
  }
  return $tr;
}
sub writer_to {
  my $tw = shift;
  my $which = shift;
  my $whatr = \$_[0];
  my ($fh);

  if ($which eq 'file') {
    $tw->to_file($$whatr);
  }
  if ($which eq 'filefh') {
    open($fh, ">", $$whatr) or die("$0: open failed for filefh filehandle");
    $tw->to_fh($fh);
  }
  elsif ($which eq 'fh') {
    $tw->to_fh($$whatr);
  }
  elsif ($which eq 'string') {
    $tw->to_string($$whatr);
  }
  elsif ($which eq 'stringfh') {
    open($fh, ">", $whatr) or die("$0: open failed for string filehandle");
    $tw->to_fh($fh);
  }
  else {
    die("$0: unknown 'which'=$which in writer_to()");
  }
  return $tw;
}

sub reader_pump {
  my $tr = shift;
  my ($s);
  while (defined($s=$tr->get_sentence)) {
    foreach (@$s) {
      $_->{type} .= ' '.$Moot::TokType[$_->{type}];
      $_->{utf8}  = utf8::is_utf8($_->{text});
    }
    print sent2str($s) if (@$s);
  }
}
sub rw_churn {
  my ($tr,$tw) = @_;
  my ($s);
  while (defined($s=$tr->get_sentence)) {
    $tw->put_sentence($s) if (@$s);
  }
}


##--------------------------------------------------------------
sub test_trnative {
  my $tr = Moot::TokenReader::Native->new( Moot::tiofWellDone );
  print "tr=$tr ; name=", $tr->name(), " ; format=", fmt2str($tr->format()), "\n";
  reader_from($tr,@_);
  reader_pump($tr);
  exit 0;
}
#test_trnative(@ARGV ? (@ARGV>1 ? @ARGV : (file=>$ARGV[0])) : (file=>'in.wd'));

##--------------------------------------------------------------
sub test_new_reader {
  my ($req,$filename) = @_;
  my $fmt = Moot::TokenIO::parse_format_request($req, $filename, 0,0);
  print "guessed format = ", fmt2str($fmt), "\n";
  my $tr  = Moot::TokenIO::new_reader($fmt);

  print "tr=$tr ; name=", $tr->name(), " ; format=", fmt2str($tr->format()), "\n";
  reader_from($tr,file=>$filename);
  reader_pump($tr);
}
#test_new_reader(@ARGV ? @ARGV : ('','testme.wd'));

##--------------------------------------------------------------
sub test_buf_writer {
  my $tw = Moot::TokenWriter::Native->new( Moot::tiofWellDone|Moot::tiofLocation|Moot::tiofCost );
  #print STDERR "$0: say please? "; $_=<STDIN>;

  print STDERR "$0: to_string()\n";
  my ($buf);
  $tw->to_string($buf);

  print STDERR "$0: put_comment()\n";
  $tw->put_comment(" test Moot::TokenWriter perl buffer interface");

  print STDERR "$0: put_raw()\n";
  $tw->put_raw("(raw data)\n");
  foreach (@_) {
    print STDERR "$0: put(", ($_//'-undef-'), ")\n";
    if (ref($_)) {
      if    (ref($_) eq 'ARRAY') { $tw->put_tokens($_); }
      elsif (ref($_) eq 'Moot::Sentence') { $tw->put_sentence($_); }
      else { $tw->put_token($_); }
    }
    elsif (!defined($_)) {
      $tw->put_token({type=>Moot::TokTypeEOS});
    }
    else {
      $tw->put_token({text=>$_});
    }
  }
  $tw->close();

  ##-- dump buffer
  print $buf;
  print STDERR "$0: test_buf_writer() done\n"; exit 0;
}
#test_buf_writer(qw(test 123),undef,{text=>456,tag=>'NN',offset=>42,length=>247,analyses=>[{details=>'foo',prob=>1},{details=>'bar',prob=>2}]});
#test_buf_writer([{text=>'foo'},{text=>'bar'}],[{text=>'baz'},{text=>'bonk'}]);
#test_buf_writer(map {bless($_,'Moot::Sentence')} [{text=>'foo'},{text=>'bar'}],[{text=>'baz'},{text=>'bonk'}]);
#test_buf_writer('foo',{type=>Moot::TokTypeSB},'bar',{type=>Moot::TokTypeWB});

##--------------------------------------------------------------
sub test_churn {
  my ($ifile,$ofile, $ireq,$oreq) = @_;
  my $tr = Moot::TokenIO::file_reader($ifile,$ireq, 0,0);
  my $tw = Moot::TokenIO::file_writer($ofile,$oreq, 0,0);

  #$|=1; print STDERR "say please? "; $_=<STDIN>;
  print STDERR "$0: tr=$tr ; file=$ifile ; name=", $tr->name(), " ; format=", fmt2str($tr->format()), "\n";
  print STDERR "$0: tw=$tw ; file=$ofile ; name=", $tw->name(), " ; format=", fmt2str($tw->format()), "\n";

  rw_churn($tr,$tw);
  #print STDERR "undef tr\n"; undef $tr;
  #print STDERR "undef tw\n"; undef $tw;

  print STDERR "$0: test_churn() done\n";
}
#test_churn(@ARGV ? @ARGV : ('in.wd','-', '','wd,loc'));

##--------------------------------------------------------------
sub test_waste_lexicon {
  my $lexer = Moot::Waste::Lexer->new();
  my $sw    = $lexer->stopwords();
  #print STDERR "say please? "; $_=<STDIN>;

  #$sw->load("stop.lex");
  ##--
  #my $tr=Moot::TokenIO::file_reader("stop.lex", "native,rare"); $sw->load($tr);
  ##--
  #open(my $fh,"<stop.lex"); my $tr=Moot::TokenIO::new_reader(Moot::tiofNative|Moot::tiofRare); $tr->from_fh($fh); $sw->load($tr);
  ##--
  my ($fh); open($fh,"<stop.lex"); $sw->load($fh);
  ##--
  print "SIZE = ", $sw->size, "\n", (map {"LEX: $_ [utf8:".(utf8::is_utf8($_) ? 1 : 0)."]\n"} @{$sw->to_array(1)});

  print STDERR "$0: test_waste_lexicon() done\n"; exit 0;
}
#test_waste_lexicon();

##--------------------------------------------------------------
sub test_waste_lexer {
  my ($ftype,$file) = @_ ? @_ : ('txt'=>'scanme2.txt');
  my $lexer = Moot::Waste::Lexer->new();
  $lexer->stopwords->load("waste-dtiger/common/dta_stopwords.lex");
  $lexer->abbrevs->load("waste-dtiger/common/dta_abbrevs.lex");
  $lexer->conjunctions->load("waste-dtiger/common/dta_conjunctions.lex");
  $lexer->dehyphenate(1);

  my ($scanner);
  if ($ftype eq 'txt' || $ftype eq 'raw') {
    $scanner = Moot::Waste::Scanner->new();
    $scanner->from_file($file);
  }
  else {
    $scanner = Moot::TokenIO::file_reader($file);
  }
  #svdumph(scanner=>$scanner);

  print STDERR "$0: _set_scanner(\$scanner)\n";
  $lexer->scanner($scanner);
  #svdumph(scanner=>$scanner);
  #print "$0: scanner_refcnt = ", $lexer->_scanner_refcnt, "\n";

  print STDERR "$0: scanner=", ($scanner//'(undef)'), " ; lexer=$lexer \n";

  print STDERR "$0: undef \$scanner\n";
  undef $scanner;
  #svdumph(scanner=>$scanner);
  #print "$0: scanner_refcnt = ", $lexer->_scanner_refcnt, "\n";

  print STDERR "$0: \$scanner = \$lexer->_get_scanner()\n";
  $scanner = $lexer->scanner();
  #svdumph(scanner=>$scanner);
  #print "$0: scanner_refcnt = ", $lexer->_scanner_refcnt, "\n";

  print STDERR "=" x 64, "\n";
  print STDERR "$0: scanner_pump()\n";
  scanner_pump($lexer);
  print STDERR "=" x 64, "\n";

  #print STDERR "$0: lexer->close()\n";
  #$lexer->close;

  print STDERR "$0: undef \$lexer\n";
  undef $lexer;

  #svdumph(scanner=>$scanner);
  print STDERR "$0: undef \$scanner\n";
  undef $scanner;

  print STDERR "$0: test_waste_lexer() done\n"; exit 0;
}
#test_waste_lexer(@ARGV);

##--------------------------------------------------------------
sub test_waste_decoder {
  my $file = shift || 'decodeme.wd';
  my $reader  = Moot::TokenIO::file_reader($file, 'wd,loc');
  my $decoder = Moot::Waste::Decoder->new();
  my $writer  = Moot::TokenIO::file_writer('-', 'm,loc');

  $decoder->sink($writer);
  undef $writer;
  my ($tok);
  while (defined($tok=$reader->get_token)) {
    $decoder->put_token($tok);
  }
  $reader->close() if ($reader);
  $decoder->close() if ($decoder);
  $writer->close() if ($writer);

  print STDERR "$0: test_waste_decoder() done\n"; exit 0;
}
test_waste_decoder(@ARGV);

##--------------------------------------------------------------
## MAIN

##-- dummy
sub main_dummy {
  foreach $i (1..3) {
    print "--dummy($i)--\n";
  }
}
main_dummy();

