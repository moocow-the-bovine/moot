#!/usr/bin/perl -w

($tag1,$tag2) = splice(@ARGV,0,2);
$tag1 = 'UNKNOWN' if (!defined($tag1));
$tag2 = 'UNKNOWN' if (!defined($tag2));

$lambda1 = 0.272507;
$lambda2 = 0.727493;

@p{($tag1,$tag2,"$tag1 $tag2")} = qw(0 0 0);

while (<>) {
  chomp;
  foreach $tag ($tag1,$tag2) {
    if (/^\d+\(\"$tag\"\)\s+([^\(\)\"]+)$/) {
      print $_, "\n";
      $p{$tag} = $1;
    }
  }
  if (/\d+\(\"$tag1\"\)\s+\d+\(\"$tag2\"\)([^\(\)\"]+)$/) {
    print $_, "\n";
    $p{$tag1.' '.$tag2} = $1;
    next;
  }
}

print
  (" ---\n",
   "p(t1=$tag1) = ", $p{$tag1}, "\n",
   "p(t2=$tag2) = ", $p{$tag2}, "\n",
   "p(t1=$tag1,t2=$tag2) = ", $p{"$tag1 $tag2"}, "\n",
   "\n",
   "L1 = $lambda1\n",
   "L2 = $lambda2\n",
   "\n",
   "smoothed1=(L1*p(t1)) = ", $s1=($lambda1*$p{$tag1}), "\n",
   "smoothed2=(L1*p(t2)) = ", $s2=($lambda1*$p{$tag2}), "\n",
   "smoothed12=(smoothed2+(L2*p(t1,t2))) = ", $s12=($s2+($lambda2*$p{"$tag1 $tag2"})), "\n",
   "\n",
   "log(smoothed1) = ", log($s1), "\n",
   "log(smoothed2) = ", log($s2), "\n",
   "log(smoothed12) = ", log($s12), "\n",
  );
