#!/usr/bin/perl -w

($tag1,$tag2) = splice(@ARGV,0,2);
$tag1 = 'UNKNOWN' if (!defined($tag1));
$tag2 = 'UNKNOWN' if (!defined($tag2));

$lambda1 = 0.272507;
$lambda2 = 0.727493;

while (<>) {
  chomp;
  foreach $tag ($tag1,$tag2) {
    if (/^0\(\"UNKNOWN\"\)\s+\d+\(\"$tag\"\)\s+([^\(\)\"]+)$/) {
      print $_, "\n";
      $lp{$tag} = $1;
    }
  }
  if (/\d+\(\"$tag1\"\)\s+\d+\(\"$tag2\"\)([^\(\)\"]+)$/) {
    print $_, "\n";
    $lp{$tag1.' '.$tag2} = $1;
    next;
  }
}

print
  (" ---\n",
   "lp(t1=$tag1) = ", $lp{$tag1}, "\n",
   "lp(t2=$tag2) = ", $lp{$tag2}, "\n",
   "lp(t1=$tag1,t2=$tag2) = ", $lp{"$tag1 $tag2"}, "\n",
   "\n",
   "L1 = $lambda1\n",
   "L2 = $lambda2\n",
   "\n",
   "smoothed1=exp(lp(t1))=(L1*p(t1)) = ", $s1=exp($lp{$tag1}), "\n",
   "smoothed2=exp(lp(t2))=(L1*p(t2)) = ", $s2=exp($lp{$tag2}), "\n",
   "smoothed12=exp(lp(t1,t2))=(L1*p(t2))+(L2*p(t1,t2)) = ",
     $s12=exp($lp{"$tag1 $tag2"}), "\n",
   "\n",
   "unsmoothed1=(smoothed1/L1) = ", $s1/$lambda1, "\n",
   "unsmoothed2=(smoothed2/L1) = ", $s2/$lambda1, "\n",
   "unsmoothed12=((smoothed12-smoothed2)/L2) = ", ($s12-$s2)/$lambda2, "\n",
  );
