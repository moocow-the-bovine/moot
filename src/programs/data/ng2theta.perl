#!/usr/bin/perl -w

use IO::File;
use statfuncs;

if (!@ARGV) {
  print STDERR "Usage: $0 NGRAMS\n";
  exit 1;
}

$EOS = "__\$";
$TOTAL = '';

$ignore_eos_ugtotal = 1;
$ignore_eos_ntags = 0;   ##-- tnt reports 56 tags, which includes __$
$ignore_eos_theta = 0;   ##-- ditto

$ignore_hapax_ugtotal = 0;   ##-- ignore "tags" which only occur once
$ignore_hapax_theta   = 0;   ##-- ignore "tags" which only occur once
$ignore_hapax_ntags   = 0;   ##-- ignore "tags" which only occur once

$purge_outliers_theta = 0;   ##-- ignore outliers when computing $theta
$purge_outliers_ntags = 0;  ##-- ignore outliers when computing $ntags

$purge_outermost_theta = 0;   ##-- ignore worst outlier when computing $theta
$purge_outermost_ntags = 0;  ##-- ignore worst outlier when computing $ntags

$purge_uniform = 0;         ##-- whether to use uniform distribution when purging

$var_minus1 = 1; ##-- whether to inexplicably subtract 1 from $ntags when computing variance

##--------------------------------------------------------------
## \%ngrams = load_123($ngramfile)
##  + \%ngrams = { ''=>$total,
##                 "$tag1"              =>$count,
##                 "$tag1\t$tag2"       =>$count,
##                 "$tag1\t$tag2\t$tag3"=>$count
##                 __HAPAX__=>{$hapax_tag=>...}, }
sub load_123 {
  my ($ngfile) = shift;
  my $ngfh = IO::File->new("<$ngfile")
    or die("$0: open failed for $ngfile: $!");
  my $ngs = { __HAPAX__=>{} };
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

    if (@ngram == 1 && $count == 1) { $ngs->{__HAPAX__}{$ngram[0]} = 1; }
  }

  ##-- maybe subtract EOS tag from unigrams
  $ngs->{$TOTAL} -= $ngs->{$EOS} if (defined($ngs->{$EOS}) && $ignore_eos_ugtotal);

  ##-- maybe subtract hapax from unigram totals
  if ($ignore_hapax_ugtotal) {
    foreach (keys(%{$ngs->{__HAPAX__}})) {
      --$ngs->{$TOTAL};
    }
  }

  $ngfh->close();
  return $ngs;
}

##--------------------------------------------------------------
## expect(\%dist), var(\%dist), stddev(\%dist)
##  + %dist: { $key => $prob, ... }
##  + see statfuncs.pm

##--------------------------------------------------------------
## \%dist2 = purge_outliers(\%dist1);
## \%dist2 = purge_outliers(\%dist1,\&valfunc);
##  + removes outliers > 2 std deviations from the mean from \%dist1,
##    returns pruned distribution \%dist2
sub purge_outliers {
  my $dist = shift;
  my $valfunc = shift || \&idfunc;
  my $mean = expect($dist,$valfunc);
  my $stddev = stddev($dist,$valfunc);
  my $dist2 = {};
  my ($x,$px,$vx);
  while (($x,$px) = each(%$dist)) {
    $vx = &$valfunc($x);
    next if (abs($vx-$mean) > 2*$stddev);
    $dist2->{$x} = $px;
  }
  return $dist2;
}

##--------------------------------------------------------------
## \%dist2 = purge_outermost(\%dist1);
## \%dist2 = purge_outermost(\%dist1,\&valfunc);
##  + removes single worst outlier from the mean from \%dist1,
##    returns pruned distribution \%dist2
sub purge_outermost {
  my $dist = shift;
  my $valfunc = shift || \&idfunc;
  my $mean = expect($dist,$valfunc);
  my $worst_x = undef;
  my $worst_diff = undef;
  my ($x,$px,$vx);
  while (($x,$px) = each(%$dist)) {
    $vx = &$valfunc($x);
    if (!defined($worst_diff) || abs($vx-$mean) > $worst_diff) {
      $worst_x = $x;
      $worst_diff = abs($vx-$mean);
    }
  }
  my %dist2 = %$dist;
  delete($dist2{$worst_x}) if (defined($worst_x));
  return \%dist2;
}

##--------------------------------------------------------------
## compute_theta
sub ng2theta {
  my $ngs = shift;
  my ($ng,$ct,@ngram);
  my $ugtotal = $ngs->{$TOTAL};

  ##-- get unigram table
  my %ugs = ();
  my %ugs_ntags = ();
  while (($ng,$ct) = each(%$ngs)) {
    @ngram = split(/\t/, $ng);
    next if (@ngram != 1 || $ngram[0] eq '__HAPAX__');

    $ugs_ntags{$ng} = $ct / $ugtotal
      unless (($ignore_eos_ntags && $ngram[0] eq $EOS)
	      &&
	      ($ignore_hapax_ntags && exists($ngs->{__HAPAX__}{$ngram[0]})));

    $ugs{$ng} = $ct / $ugtotal
      unless (($ignore_eos_theta && $ngram[0] eq $EOS)
	      ||
	      ($ignore_hapax_theta && exists($ngs->{__HAPAX__}{$ngram[0]})));
  }

  ##-- maybe remove outliers: ntags
  if ($purge_outliers_ntags || $purge_outermost_ntags) {
    my %ugs_unif = ($purge_uniform
		    ? (map { $_=>1/scalar(keys(%ugs_ntags)) } keys(%ugs_ntags))
		    : %ugs_ntags);

    if ($purge_outliers_ntags) {
      %ugs_unif = %{purge_outliers(\%ugs_ntags,
				   sub {return $ugs_ntags{$_[0]};}
				  )};
      foreach (keys(%ugs_ntags)) {
	delete($ugs_ntags{$_}) if (!exists($ugs_unif{$_}));
      }
    }

    if ($purge_outermost_ntags) {
      %ugs_unif = %{purge_outermost(\%ugs_ntags,
				    sub {return $ugs_ntags{$_[0]};}
				   )};
      foreach (keys(%ugs_ntags)) {
	delete($ugs_ntags{$_}) if (!exists($ugs_unif{$_}));
      }
    }
  }

  ##-- maybe remove outliers: ngs
  if ($purge_outliers_theta || $purge_outermost_theta) {
    my %ugs_unif = ($purge_uniform
		    ? (map { $_=>1/scalar(keys(%ugs_ntags)) } keys(%ugs_ntags))
		    : %ugs_ntags);

    if ($purge_outliers_theta) {
      %ugs_unif = %{purge_outliers(\%ugs,
				   sub { return $ugs{$_[0]}; },
				   )};
      foreach (keys(%ugs)) {
	delete($ugs{$_}) if (!exists($ugs_unif{$_}));
      }
    }

    if ($purge_outermost_theta) {
      %ugs_unif = %{purge_outermost(\%ugs,
				   sub { return $ugs{$_[0]}; },
				    )};
      foreach (keys(%ugs)) {
	delete($ugs{$_}) if (!exists($ugs_unif{$_}));
      }
    }
  }


  ##-- compute p_avg
  my $ntags = scalar(keys(%ugs_ntags));
  my $pavg = 1/$ntags;

  ##-- compute E(P_tag(·))
  my $eptag = 0;
  foreach (values(%ugs)) {
    $eptag += $_**2;
  }

  ##-- debug
  print
    "ntags=$ntags ; nugs=", scalar(keys(%ugs)), " ; pavg=$pavg ; eptag=$eptag\n";

  ##-- compute uniform / weighted theta, stddev
  $uu_theta = 0; # uniform  Var( uniform  E() ) : a la Brants 2000
  $uw_theta = 0; # uniform  Var( weighted E() ) : hybrid
  $wu_theta = 0; # weighted Var( uniform  E() ) : standard deviation
  $ww_theta = 0; # weighted Var( weighted E() ) : standard deviation
  foreach $ptag (values %ugs) {
    $uu_theta += ($ptag - $pavg)**2;
    $uw_theta += ($ptag - $eptag)**2;
    $wu_theta += $ptag * ($ptag - $pavg)**2;
    $ww_theta += $ptag * ($ptag - $eptag)**2;
  }
  $uu_theta = sqrt($uu_theta/($var_minus1 ? ($ntags-1) : $ntags));
  $uw_theta = sqrt($uw_theta/($var_minus1 ? ($ntags-1) : $ntags));
  $wu_theta = sqrt($wu_theta);
  $ww_theta = sqrt($ww_theta);

  ##-- debug
  printf "uu_theta=%.5e ; uw_theta=%.5e ; wu_theta=%.5e ; ww_theta=%.5e\n",
    $uu_theta, $uw_theta, $wu_theta, $ww_theta;

  ##-- compute cross-thetas
  $ux_theta = 0; # weighted-vs.-unweighted, uniform Var()
  $wx_theta = 0; # weighted-vs.-unweighted, weighted Var()
  foreach $ptag (values %ugs) {
    $ux_theta += ($eptag - $pavg)**2;
    $wx_theta += $ptag * ($eptag - $pavg)**2;
  }
  $ux_theta = sqrt($ux_theta/($ntags-1));
  $wx_theta = sqrt($wx_theta);

  ##-- debug
  printf "ux_theta=%.5e ; wx_theta=%.5e\n",
    $ux_theta, $wx_theta;
}


##--------------------------------------------------------------
## main
$ngfile = shift(@ARGV);
$ngs = load_123($ngfile);

 ng2theta($ngs);

print
  "UGTOTAL = ", $ngs->{$TOTAL}, "\n";

