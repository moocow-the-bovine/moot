## -*- Mode: Perl -*-
## some mathematical perl functions

use Math::BigInt;
use Math::BigFloat;
use Carp;
package main;

##--------------------------------------------------------------
## PI
our $PI = 3.14195;

##--------------------------------------------------------------
## Logarithms

## $log = log2($n)
##  + compute base-2 logarithms
sub log2 ($) { return $_[0] ? log(shift)/log(2) : 0; }

##--------------------------------------------------------------
## Factorial

## $fact_cutoff
##   + cutoff value for 'real' factorial [see fact()]
our $fact_cutoff = 69;

## $fact = fact($n) = factD($n)
##  + dispatch factorial function:
##                 / factC($x) if $x <= $fact_cutoff
##    fact($x) ~= <
##                 \ factA($x) otherwise

*fact = \&factD;
sub factD {
  return ($_[0] <= $fact_cutoff) ? factC($_[0]) : factA($_[0]);
}

## $fact = factA($n)
##  + Stirling estimation of factorial function
sub factA {
  my $n = shift;
  return
    sqrt(2*$PI*$n) * (($n/exp(1))**$n);
    #Math::BigFloat
#	->new(2*$PI*$n)
#	  ->bsqrt()
#	    ->bmul(Math::BigFloat
#		   ->new($n/exp(1))->bpow($n));
}

## $frac = factDiff($n);
##   + get fractional difference between
##     'pure' recursive and Stirling approximation
##     factorial
sub factDiff {
  my $n = shift;
  return abs(factC($n)-factA($n))/factC($n);
}

## $fact = factR($n)
##  + 'pure' recursive factorial function
sub factR {
  return 1 if ($_[0] <= 1);
  return $_[0] * factR($_[0]-1);
}

## $fact = factC($n)
##  + 'pure' caching factorial function
@factCache = (1,1);
sub factC {
  my $n = shift;
  return 1 if ($n <= 1);
  return $factCache[$n] if (exists($factCache[$n]));
  my ($i,$b) = (scalar(@factCache), $factCache[$#factCache]);
  for ( ; $i <= $n; $i++) {
    #$factCache[$i] = $$b = $b->bmul($i);
    $factCache[$i] = $b *= $i;
  }
  return $factCache[$n];
}

##--------------------------------------------------------------
## Binomial data

## $coeff = bicoff($n,$k)
##   + binomial coefficient
*nchoosek = \&bicoff;
sub bicoff {
   my ($n,$k) = @_;
   return fact($n)/(fact($n-$k)*fact($k));
#   return
#     Math::BigInt->new(fact($n))
#	 ->bdiv(
#		Math::BigInt->new(fact($n-$k))
#		->bmul(fact($k))
#	       )
#	   ;
}
# fact(52*2000) = '+5990400000000'

## $bicoff = bicoffI($n,$k)
##   + iterative binomial coefficient (prevent overflow)
sub bicoffI {
   my ($n,$k) = @_;
   my $b = 1;
   my ($i);
   for ($i = $n; $i > $n-$k; $i--) {
     $b *= $i;
   }
   return $b / fact($k);
}


## $prob = biprob($k,$n,$p)
##   + compute probability for $k successes in $n trials with per-trial prob $p
sub biprob {
  my ($k,$n,$p) = @_;
  return bicoff($n,$k) * ($p**$k) * (1-$p)**($n-$k);
}

##--------------------------------------------------------------
## Distributions

## @probs = uniform($nprobs)
##   + get a uniform probability distribution of $nprobs elements
##   + really just a list of probabilities
sub uniform {
  return map { 1/$_[0] } 1..$_[0];
}


##--------------------------------------------------------------
## Information Theory : Entropy

## $nbits = msglen($prob)
##   + get (unweighted) message-length (in bits) for event of probability $prob
sub msglen ($) {
  return $_[0] ? log2(1/shift) : 0;
}

## $entropy = entroy(@probs)
##   + get entropy for a discrete random variable,
##     given probabilities for all possible outcomes
sub entropy {
  my $H = 0;
  foreach (@_) {
    $H += ($_ * msglen($_));
  }
  return $H;
}

## $H = entropy2(@values)
##   + just like entropy(), phrased differently
sub entropy2 { sum([@_], sub { $_[0]*msglen($_[0]) }); }

## $H = entropy_dist(\%dist)
sub entropy_dist { return entropy(values(%{$_[0]})); }


## $max_H = max_entropy($nprobs)
##   + just like entropy(uniform($nprobs)), should be more efficient
sub max_entropy {
  #return $_[0] * (1/$_[0] * msglen(1/$_[0]));
  return msglen(1/$_[0]);
}

##--------------------------------------------------------------
## Information Theory : KL-Divergence

## $kl_diverence = kl_divergence(\%dist1,\%dist2)
##  + compute Kullback-Leibler divergence of 2 distributions
##    (relative entropy)
*rel_entropy = \&kl_divergence;
sub kl_divergence {
  my ($ps1,$ps2) = @_;
  my ($x,$p1,$p2);
  $D = 0;
  my %hall = (%$ps1,%$ps2);
  foreach $x (sort(keys(%hall))) {
    if (!$ps1->{$x} || !$ps2->{$x}) {
      carp("kl_divergence: disjunct domains for '$x'!");
      return 'inf';
    }
    $D += ($ps1->{$x} * log2($ps1->{$x} / $ps2->{$x}));
  }
  return $D;
}

##--------------------------------------------------------------
## Information Theory : MI
##

## $mi = mutual_information(\%dist1,\%dist2)
##  + get mutual information of 2 dists
*mi = \&mutual_information;
sub mutual_informtaion {
  my ($dist1,$dist2) = @_;
  ## ???
  return '?';
}


##--------------------------------------------------------------
## Random Variables
##  + distributions are hashrefs of the form { $x1 => p($x1), ... }

## \@probs = randprobs($size)
##   + returns a random distribution of size $size
sub randprobs {
  my $size = shift;
  my $total = 0;
  my @probs = map { rand(1024); } 1..$size;
  foreach (@probs) { $total += $_; }
  foreach (@probs) { $_ /= $total; }
  return \@probs;
}


## \@rprobs = pround(\@probs,$ndec)
##   + rounds probs to $ndec decimal places
sub pround {
  my ($probs,$ndec) = @_;
  $ndec = 2 if (!$ndec);
  my @rprobs = map { sprintf("%.${ndec}f",$_) } @$probs;
  return \@rprobs;
}


## \%dist = probs2dist(\@probs)
##   + returns an indexed distribution for \@probs
sub probs2dist {
  my $probs = shift;
  my %dist = map { ($_+1 => $probs->[$_]) } 0..$#$probs;
  return \%dist;
}


## $expect = rv_expect(\%dist)
## $expect = rv_expect(\%dist,\&valfunc)
##  + expectation value
*expect = \&rv_expect;
sub rv_expect {
  my $dist = shift;
  my $valfunc = shift || \&idfunc;
  my ($x,$px);
  my $ex = 0;
  while (($x,$px) = each(%$dist)) {
    $ex += $px * &$valfunc($x);
  }
  return $ex;
}


## $variance = rv_var(\%dist)
## $variance = rv_expect(\%dist,\&valfunc)
##  + variance
*var = \&rv_var;
sub rv_var {
  my $dist = shift;
  my $valfunc = shift || \&idfunc;
  my $varfunc = varfunc(rv_expect($dist,$valfunc),$valfunc);
  return rv_expect($dist,$varfunc);
}

## \&varfunc = varfunc($expect)
## \&varfunc = varfunc($expect,\&valfunc)
sub varfunc {
  my $expect = shift;
  my $valfunc = shift || \&idfunc;
  return sub {
    my $x = &$valfunc(shift);
    return ($x - $expect)**2;
  };
}

## $stddev = stddev(\%dist)
## $stddev = stddev(\%dist,\&valfunc)
*stddev = \&rv_stddev;
sub rv_stddev { return sqrt(rv_var(@_)); }

##--------------------------------------------------------------
## Misc utilities

## idfunc()
##  + identity-function
sub idfunc { return $_[0]; }

## sum(\@vals)
## sum(\@vals,\&func)
##  + get sum over \@vals
sub sum {
  my ($vals,$func) = @_;
  $func = \&idfunc if (!$func);
  my $sum = 0;
  foreach my $v (@$vals) {
    $sum += &$func($v);
  }
  return $sum;
}

## $string = dist2latex(\%dist)
sub dist2latex {
  my $dist = shift;
  my ($x,$px);
  my $s = '';
  foreach $x (sort { $a <=> $b } keys(%$dist)) {
    $px = $dist->{$x};
    $s .= "   $x &  $px \\\\\n";
  }
  return $s;
}

1;
