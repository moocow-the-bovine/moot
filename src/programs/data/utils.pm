package main;

use IO::File;

##--------------------------------------------------------------
## \%ngrams = load_123($ngramfile)
##  + \%ngrams = { ''=>$total,
##                 "$tag1"              =>$count,
##                 "$tag1\t$tag2"       =>$count,
##                 "$tag1\t$tag2\t$tag3"=>$count
##                 __HAPAX__=>{$hapax_tag=>...}, }

our $ignore_eos_ugtotal = 1;
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
## \%ugs = ngs2ugs(\%ngrams,\%options)
##   + %options:
##     ignore_eos=>$bool=0
##     ignore_total=>$bool=0
sub ngs2ugs {
  my ($ngrams,$options) = @_;
  $options = {} if (!defined($options));
  my $ugs = {};
  my ($k,$v);
  while (($k,$v) = each(%$ngs)) {
    next if ($k eq $TOTAL && $options->{ignore_total});
    next if ($k eq $EOS   && $options->{ignore_eos});
    next if ($k =~ /\t/);
    $ugs->{$k} = $v;
  }
  return $ugs;
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

##--------------------------------------------------------------
## MLE

## \%probs = mle(\%counts)
##  + do general ML estimation
sub mle {
  my $counts = shift;
  my $total = sum([values(%$counts)]);
  my $probs = { %$counts };
  foreach (values(%$probs)) {
    $_ /= $total;
  }
  return $probs;
}



##--------------------------------------------------------------
## Samuelsson smoothing

## $M = cs_M(\@probs)
## $M = cs_M(\@probs,$logbase)
## $M = cs_M(\%probs)
## $M = cs_M(\%probs,$logbase)
##  + defined s.t. entropy(uniform($M)) = entropy(\@probs)
sub cs_M {
  my ($probs,$logbase) = @_;
  $logbase = (defined($logbase) ? exp(1) : 2) if (!$logbase);
  my $H = entropyN($probs,$logbase);
  my $M = $logbase ** $H;
  return $M;
}

## $sigma0 = cs_sigma0(\@probs)
## $sigma0 = cs_sigma0(\%dist)
##  + get std dev of uniform dist w/ entropy of \%dist
sub cs_sigma0 {
  my $M = cs_M(@_);
  return sqrt(($M**2 - 1)/12);
}

## $sigma = cs_sigma(\@probs_C_k_minus_1, $sizeof_C_k)
sub cs_sigma {
  my $pCk1 = shift;
  my $sizeofCk = shift || distsize($pCk1);
  return cs_sigma0($pCk1) / sqrt($sizeofCk);
}


1;
