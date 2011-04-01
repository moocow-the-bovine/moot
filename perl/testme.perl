#!/usr/bin/perl -w

use lib qw(./blib/lib ./blib/arch);
use Moot;

##--------------------------------------------------------------
## test: basic

sub test_libversion {
  print STDERR "Moot libversion=", Moot::library_version, "\n";
}
#test_libversion();

##--------------------------------------------------------------
## test: lexfreqs

sub test_lexfreqs {
  my $lffile = shift || 'test0.lex';
  my $lf = Moot::Lexfreqs->new() or die ("Moot::Lexfreqs->new() failed: $!");
  $lf->load($lffile) or die("Lexfreqs->load($lffile) failed: $!");
  print "$0: loaded '$lffile'; n_tokens=", $lf->n_tokens, "; n_pairs=", $lf->n_pairs, "\n";
  $lf->add_count('added_text','added_tag',42);
  print "$0: post-add: n_tokens=", $lf->n_tokens, "; n_pairs=", $lf->n_pairs, "\n";
  $lf->save("-") or die("Lexfreqs->save(-) failed: $!");
}
test_lexfreqs(@ARGV);

##--------------------------------------------------------------
## MAIN

##-- dummy
sub main_dummy {
  foreach $i (1..3) {
    print "--dummy($i)--\n";
  }
}
main_dummy();

