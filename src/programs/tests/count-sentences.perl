#!/usr/bin/perl -w

$last_was_eos = 1;

$nsents = 0;
$ntokens = 0;
while (<>) {
  next if (/^\s*%%/);
  if (/^\s*$/s) {
    $nsents++ if (!$last_was_eos);
    $last_was_eos = 1;
    next;
  }
  $last_was_eos = 0;
  $ntokens++;
}

$nsents = 1 if (!$nsents && $ntokens);
print
  (sprintf("Sentences            : %12d\n", $nsents),
   sprintf("Tokens               : %12d\n", $ntokens),
   sprintf("Avg. Sentence Length : %15.2f\n", $ntokens/$nsents));

