#!/usr/bin/perl -w

use Getopt::Long;
$have_best = 0;
GetOptions("best|b!" => \$have_best);

##-- token translation table (additive)
%xlate_tok =
  (
   #'wie' => ['[PWAV thack=1]'],
   'ganz' => ['[ADV thack=1]', '[ADJA thack=1]'],
   'einiges' => ['[PIS thack=1]'],
   'ja' => ['[ADV thack=1]'],
   'anderem' => ['[PIS thack=1]'],
   'gegen�ber' => ['[PTKVZ thack=1]'],
   'einein' => ['ADJA thack=1]'],
   'm�chte' => ['[VMFIN thack=1]'],
   'm�chten' => ['[VMFIN thack=1]', '[VMINF thack=1]'],
   'mag' => ['[VMFIN thack=1]'],
   'm�gen' => ['[VMFIN thack=1]', '[VMINF thack=1]'],
   'was' => ['[PRELS thack=1]'],
   'etwas' => ['[ADV thack=1]'],
   'gut' => ['[ADV thack=1]'],
   'besten' => ['[ADJD thack=1]'],
   'Unrecht' => ['[NN thack=1]'],
   'Unbekannten' => ['[NN thack=1]'],
   'siehe' => ['[VVIMP thack=1]'],
   'lang' => ['[APPO thack=1]'],
   'lange' => ['[ADJD thack=1]'],
   'seit' => ['[KOUS thack=1]'],
   'gen�gend' => ['[PIAT thack=1]'],
   'dabei' => ['[PKTVZ thack=1]'],
   'recht' => ['[ADV thack=1]'],
   'selber' => ['[ADV thack=1]'],
   #'einfach' => ['[ADV thack=1]'],
   #'dessen' => ['[PDAT thack=1]'],  ## PDS|PRELAT|PRELS
   'deshalb' => ['[PAV thack=1]'],
   '&' => ['[KON thack=1]'],
   #'wenige' => ['[PIAT thack=1]'],
   'weniger' => ['[ADV thack=1]'],
   'statt' => ['[PTKVZ thack=1]'],
   '...' => ['[$( thack=1]'],
   'als' => ['[KON thack=1]'],
   'teil' => ['[PKTVZ thack=1]'],
   #'solche' => ['[PIAT thack=1]'],
  );
##-- case-insenstize token table
while (($tok,$vals) = each(%xlate_tok)) {
  $xlate_tok{"\u$tok"} = $vals;
}

##-- whole-class translation table (destructive)
%xlate_class =
  (
   #"[NN]" => "[NE chack=1]\t[NN]",
   #"[NE]" => "[NN chack=1]\t[NE]",
   #"[NE]\t[XY]" => "[NN chack=1]\t[NE]\t[XY]",
   "[ADV]\t[KOKOM]\t[KON]\t[KOUS]" => "[ADV]\t[KOKOM]\t[KON]\t[KOUS]\t[PWAV chack=1]",
   "[ADJD]" => "[ADJD]\t[ADV]",
   "[PTKA]\t[PTKABT]\t[PTKANT]" => "[ADV]\t[PTKA]\t[PTKABT]\t[PTKANT]",
   "[PDS]\t[PRELAT]\t[PRELS]" => "[PDAT chack=1]\t[PDS]\t[PRELAT]\t[PRELS]",
   "[CARD]" => "[ADJA chack=1]\t[CARD]"
  );

##-- single-tag translation table (additive)
%xlate_tag =
  (
   ADJC => ['[ADJD hack=1]'],
   #XY   => ['[$. hack=1]'],
   ##--
   #NE => ['[NN hack=1]'],
   #NN => ['[NE hack=1]']
   ##--
   NE => ['[NN hack=1]', '[ADJA hack=1]' ],
   NN => ['[NE hack=1]', '[ADJA hack=1]' ],
   ##--
   PIDAT => ['[PIAT hack=1]'],
  );


while (<>) {
  if (/^\s*%%/ || /^\s*$/) { print;  next; }

  chomp;
  %ans = ();
  ($token,@class) = split(/\s*\t+\s*/, $_);

  ##-- extract best, if requested
  $best = shift(@class) if ($have_best);

  ##-- apply class-based rules
  $class = join("\t", sort(@class));
  $class = $xlate_class{$class} if (exists($xlate_class{$class}));

  ##-- apply tag-based rules on translated class
  foreach $an (split(/\t/,$class)) {
    next if (!defined($an));
    $ans{$an} = 1;
    foreach $src (keys(%xlate_tag)) {
      if ($an =~ m/(?:\b|_)$src\b/) {
	@ans{@{$xlate_tag{$src}}} = 1;
      }
    }
  }

  ##-- apply token-based rules
  @ans{@{$xlate_tok{$token}}} = 1 if (exists($xlate_tok{$token}));

  print join("\t",
	     $token,
	     ($have_best ? $best : qw()),
	     sort(keys(%ans))), "\n";
}
