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
   'derlei' => ['[PIAT thack=1]'],
   'deren' => ['[PDAT thack=1]'],
   'derer' => ['[PDS thack=1]'],
   'allesamt' => ['[ADV thack=1]'],
   'Töne' => ['[NN thack=1]'],
   'de' => ['[NE thack=1]'],
   'wieviel' => ['[PWAV thack=1]'],
   'Drogenabhängigen' => ['[NN thack=1]'],
   'Interessierte' => ['[NN thack=1]'],
   'Interessierten' => ['[NN thack=1]'],
   'anderem' => ['[PIS thack=1]'],
   'innen' => ['[TRUNC thack=1]'],
   'gegenüber' => ['[PTKVZ thack=1]'],
   'einein' => ['ADJA thack=1]'],
   'möchte' => ['[VMFIN thack=1]'],
   'möchten' => ['[VMFIN thack=1]', '[VMINF thack=1]'],
   'mag' => ['[VMFIN thack=1]'],
   'mögen' => ['[VMFIN thack=1]', '[VMINF thack=1]'],
   'möge' => ['[VMFIN thack=1]'],
   'liebsten' => ['[ADJD thack=1]'],
   'herum' => ['[APZR thack=1]'],
   'drauf' => ['[PAV thack=1]'],
   'seitdem' => ['[PAV thack=1]'],
   'na' => ['[ITJ thack=1]'],
   '§' => ['[NN thack=1]'],
   'reichlich' => ['[ADV thack=1]'],
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
   'genügend' => ['[PIAT thack=1]'],
   'genug' => ['[PIAT thack=1]'],
   'dabei' => ['[PTKVZ thack=1]'],
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
   's' => ['[PPER thack=1]'],
   'Kleinen' => ['[NN thack=1]'],
   'teil' => ['[PTKVZ thack=1]'],
   'Ostend' => ['[NE thack=1]'],
   'zustande' => ['[PTKVZ thack=1]'],
   'halt' => ['[PTKVZ thack=1]'],
   'öfter' => ['[ADJD thack=1]'],
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
   "[CARD]" => "[ADJA chack=1]\t[CARD]",
   "[ADV]\t[PTKVZ]" => "[ADV]\t[APZR chack=1]\t[PTKVZ]",
   "[ADJD]\t[ADV]" => "[ADJA]\t[ADJD]\t[ADV]",  # i.e. 'sechziger'
  );

##-- single-tag translation table (additive)
%xlate_tag =
  (
   'ADJC' => ['[ADJD hack=1]'],
   #XY   => ['[$. hack=1]'],
   ##--
   #NE => ['[NN hack=1]'],
   #NN => ['[NE hack=1]']
   ##--
   'NE' => ['[NN hack=1]', '[ADJA hack=1]' ],
   'NN' => ['[NE hack=1]', '[ADJA hack=1]' ],
   ##--
   'PIDAT' => ['[PIAT hack=1]'],
   ##--
   "[ADJA]" => ["[NN hack=1]", "[ADJD hack=1]"],
  );


while (<>) {
  if (/^\s*%%/ || /^\s*$/) { print;  next; }

  chomp;

  ##-- remove 'MorphConsole' redundant "empty analysis" marker
  s/\t+\s*\$Keine Analyse\$/\t/;

  %ans = ();
  ($token,@astrs) = split(/\s*\t+\s*/, $_);

  ##-- extract best, if requested
  $best = shift(@astrs) if ($have_best);

  ##-- beautify class
  foreach (@astrs) {
    s/([^\]])( <)/$1\]$2/; ## -- beautify
    s/\s*<[\d\.]+>\s*//g;    ## -- eliminate costs
  }

  ##-- apply class-based rules
  $class = join("\t", sort(@astrs));
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
