package Moot::HMM;
use Carp;
use strict;

##======================================================================
## wrappers: config

## $hmm = $CLASS->new()
## $hmm = $CLASS->new($opts)
sub new {
  my ($that,$opts) = @_;
  my $hmm = $that->_new();
  $hmm->config($opts) if ($opts);
  return $hmm;
}

## \%opts = $hmm->config()
## \%opts = $hmm->config(\%opts)
##  + get/set HMM options
our %hmmOpts = (map {($_=>__PACKAGE__->can($_))}
		qw(verbose ndots),
		qw(save_ambiguities save_flavors save_mark_unknown),
		qw(hash_ngrams relax use_lex_classes),
		qw(start_tagid),
		qw(unknown_lex_threshhold unknown_class_threshhold),
		qw(nglambda1 nglambda2 nglambda3),
		qw(wlambda0 wlambda1),
		qw(clambda0 clambda1),
		qw(beamwd),
		qw(nsents ntokens nnewtokens nunclassed nnewclasses nunknown nfallbacks),
	       );
sub config {
  my ($hmm,$opts) = @_;
  if ($opts) {
    $hmmOpts{$_}->($hmm,$opts->{$_}) foreach (grep {$hmmOpts{$_}} keys %$opts);
  }
  return {map {($_=>$hmmOpts{$_}->($hmm))} keys %hmmOpts};
}

##======================================================================
## wrappers: I/O

BEGIN {
  *loadModel = \&load;
}

sub load {
  my ($that,$modelfile) = @_;
  $that = $that->new if (!ref($that));
  return undef if (!$that->_load_model($modelfile));
  return $that;
}

sub loadBin {
  my ($that,$binfile) = @_;
  $that = $that->new if (!ref($that));
  return undef if (!$that->_load($binfile));
  return $that;
}

BEGIN {
  *saveBin = \&_save;
}


1; ##-- be happy

__END__

=pod

=head1 NAME

Moot::HMM - libmoot : HMM

=head1 SYNOPSIS

  use Moot;

  ##=====================================================================
  ## Constructors etc
  $hmm  = Moot::HMM->new(\%opts);
  $opts = $hmm->config(\%opts);
  $opts = $hmm->config();

  ##=====================================================================
  ## Accessors
  ##  + all of the following are get/set methods, e.g.
  ##    `$hmm->verbose()' gets the value of the 'verbose' property, and
  ##    `$hmm->verbose($i)' sets it

  $val = $hmm->verbose();
  $ndots = $hmm->ndots();

  $save_ambiguities = $hmm->save_ambiguities();
  $save_flavors = $hmm->save_flavors();
  $save_mark_unknown = $hmm->save_mark_unknown();

  $hash_ngrams = $hmm->hash_ngrams();
  $relax = $hmm->relax();
  $use_lex_classes = $hmm->use_lex_classes();

  $start_tagid = $hmm->start_tagid();

  $unknown_lex_threshhold = $hmm->unknown_lex_threshhold();
  $unknown_class_threshhold = $hmm->unknown_class_threshhold();

  $nglambda1 = $hmm->nglambda1();
  $nglambda2 = $hmm->nglambda2();
  $nglambda3 = $hmm->nglambda3();

  $wlambda0 = $hmm->wlambda0();
  $wlambda1 = $hmm->wlambda1();

  $clambda0 = $hmm->clambda0();
  $clambda1 = $hmm->clambda1();

  $beamwd = $hmm->beamwd();

  $nsents = $hmm->nsents();
  $ntokens = $hmm->ntokens();
  $nnewtokens = $hmm->nnewtokens();
  $nunclassed = $hmm->nunclassed();
  $nnewclasses = $hmm->nnewclasses();
  $nunknown = $hmm->nunknown();
  $nfallbacks = $hmm->nfallbacks();

  ##=====================================================================
  ## Tagging

  ## sentences are tagged in-place; structure:
  @sent = (
           {text=>'This'},
           {text=>'is',    tag=>'this_will_be_overwritten'},
           {text=>'a'      tag=>'this_too'},
           {text=>'test',  analyses=>[{tag=>'N',details=>'test/N'},
                                      {tag=>'V',details=>'test/V',cost=>42}] },
           {text=>'.'      analyses=>[{tag=>'$.'}]},
          );

  $hmm->tag_sentence(\@sent); ##-- clobbers 'tag' key of each token


  ##=====================================================================
  ## I/O

  $hmm = $CLASS_OR_OBJECT->load($model);
  $hmm = $CLASS_OR_OBJECT->loadBin($binfile);

  $bool = $hmm->saveBin($binfile, $zlevel=-1);
  undef = $hmm->txtdump($filename='-');

=head1 DESCRIPTION

The Moot module provides an object-oriented interface to the libmoot library
for Hidden Markov Model part-of-speech tagging.

=head1 SEE ALSO

Moot::Constants(3perl),
moot(1),
perl(1).

=head1 AUTHOR

Bryan Jurish E<lt>jurish@uni-potsdam.deE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2011 by Bryan Jurish

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.10.1 or,
at your option, any later version of Perl 5 you may have available.

=cut

