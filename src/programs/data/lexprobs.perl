#!/usr/bin/perl -w

use IO::File;

if (!@ARGV) {
  print STDERR "Usage: $0 LEXFILE [WORD...]\n";
  exit 1;
}

$uthresh = 1;
#$ugtotal_add = 37085;

$UNKNOWN = "\@UNKNOWN";

##--------------------------------------------------------------
## \%lex = load_lex($lexfile)
##   + structure : { ''=>$total,$token=>{''=>$total,$tag=>$count}, ...}
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
## \%ngrams = load_123($ngramfile)
##  + \%ngrams = { ''=>$total, {$tag1=>{''=>$total,$tag2=>{''=>$total,$tag3=>$count}}}}
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
    for ($nngs=$ngs, $tag=shift(@ngram); @ngram; $tag=shift(@ngram)) {
      $nngs_next = $nngs->{$tag} = {} if (!defined($nngs_next = $nngs->{$tag}));
      $nngs = $nngs_next;
    }
    $nngs->{$tag} += $count;
  }

  $ngfh->close();
  return $ngs;
}


##--------------------------------------------------------------
## lookup
sub lookup {
  my ($lex,$ngs) = (shift,shift);
  my ($tok,$tag,$tokh,$plex,$ptag);
  foreach $tok (@_) {
    $tokh = $lex->{$tok};
    if (!$tokh) {
      print "$tok\t--EMPTY--\n";
      next;
    }
    foreach $tag (sort keys %$tokh) {
      $plex = $tokh->{$tag} / $ngs->{$tag};
      #$plex = (.5+$tokh->{$tag}) / (.5+$ngs->{$tag});
      $ptag = $ngs->{$tag}  / $ngs->{''};
      printf("p_lex( $tok | $tag ) = %.5e \t; p_uni( $tag ) = %.5e \t; product = %.5e\n",
	     $plex, $ptag, $plex*$ptag);
    }
    print "\n";
  }
}
sub lkp { lookup($lex,$ngs,@_); }

##--------------------------------------------------------------
## main
$lexfile = shift(@ARGV);
$lex = load_lex("$lexfile");
$ngs = lex2unigrams($lex);
#$ngs = load_123("$model.123");

if (@ARGV) {
  lookup($lex,$ngs,@ARGV);
}
else {
  lookup($lex,$ngs,(map { chomp; $_ } <STDIN>));
}

print
  "LEXTOTAL = ", $lex->{''}, " ; UGTOTAL = ", $ngs->{''}, "\n";

