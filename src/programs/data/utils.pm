package main;

use IO::File;

##--------------------------------------------------------------
## \%ngrams = load_123($ngramfile)
##  + \%ngrams = { ''=>$total,
##                 "$tag1"              =>$count,
##                 "$tag1\t$tag2"       =>$count,
##                 "$tag1\t$tag2\t$tag3"=>$count
##                 __HAPAX__=>{$hapax_tag=>...}, }

our $ignore_eos_ugtotal = 0;
our $EOS = "__\$";
our $TOTAL = '';

sub load_123 {
  my ($ngfile) = shift;
  my $ngfh = IO::File->new("<$ngfile")
    or die("$0: open failed for $ngfile: $!");
  my $ngs = {};
  my (@ngram,@pngram,$count,$i,$nngs,$nngs_next,$tag);
  while (<$ngfh>) {
    next if (/^\s*%%/ || /^\s*$/);
    chomp;

    ## -- get ngram
    @ngram = map { s/^\s*|\s*$//g; $_ } split(/\t/,$_);

    ## -- extract count
    $count = pop(@ngram);

    ## -- parse abbreviated forms
    for ($i = 0;
	 $i <= $#pngram && $i <= $#ngram && (!defined($ngram[$i]) || $ngram[$i] eq '');
	 ++$i)
      {
	$ngram[$i] = $pngram[$i];
      }

    ## -- add ngram to table
    $ngs->{join("\t",@ngram)} += $count;
    if (@ngram == 1) { $ngs->{$TOTAL} += $count; }
  }

  ##-- maybe subtract EOS tag from unigrams
  $ngs->{$TOTAL} -= $ngs->{$EOS}
    if (defined($ngs->{$EOS}) && $ignore_eos_ugtotal);

  $ngfh->close();
  return $ngs;
}

##--------------------------------------------------------------
## \%lex = load_lex($lexfile)
##   + structure : { ''=>$total,$token=>{''=>$total,$tag=>$count}, ...}

our $UNKNOWN = '\@UNKNOWN';

sub load_lex {
  my $lexfile = shift;
  my $lex = {};  ## { ''=>$total,$token=>{''=>$total,$tag=>$count}, ...}
  $lex->{$UNKNOWN} = {};
  my ($tok,$toktotal,%tagcts,$tag,$ct,$lextok);
  my $total = 0;
  my $lexfh = IO::File->new("<$lexfile")
    or die("$0: open failed for $lexfile: $!");
  while (<$lexfh>) {
    next if (/^\s*%%/ || /^\s*$/);
    chomp;
    ($tok,$toktotal,%tagcts) = split(/\s*\t+\s*/,$_);
    $total += $toktotal if ($tok !~ /^\@/);
    $lextok = $lex->{$tok} = {} if (!defined($lextok = $lex->{$tok}));
    while (($tag,$ct) = each(%tagcts)) {
      $lextok->{$tag} += $ct;
      $lex->{$UNKNOWN}{$tag} += $ct if ($toktotal <= $uthresh)
    }
  }
  $lex->{''} = $total;
  $lexfh->close();
  return $lex;
}

##--------------------------------------------------------------
## \%1grams = lex2unigrams($lex)
##  + \%1grams = { ''=>$total, $tag=>$count, ...}
sub lex2unigrams {
  my $lex = shift;
  my $ngs = {};
  my ($tag,$ct,$tok,$tokh);
  while (($tok,$tokh) = each(%$lex)) {
    next if ($tok =~ /^\@/);
    while (($tag,$ct) = each(%$tokh)) {
      $ngs->{$tag} += $ct;
    }
  }
  $ngs->{''} = $lex->{''};

  ##-- add extra unigram thingies
  $ngs->{''} += $ugtotal_add if ($ugtotal_add);

  return $ngs;
}

1;
