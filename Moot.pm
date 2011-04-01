package Moot;

use 5.008004;
use strict;
use warnings;
use Carp;
use AutoLoader;
use Exporter;

our @ISA = qw(Exporter);

our $VERSION = '0.01';

require XSLoader;
XSLoader::load('Moot', $VERSION);

# Preloaded methods go here.
#require Moot::Alphabet;
#require Moot::Automaton;
#require Moot::Automaton::Indexed;

# Autoload methods go after =cut, and are processed by the autosplit program.

##======================================================================
## Exports
##======================================================================
our @EXPORT = qw();
our %EXPORT_TAGS = qw();

##======================================================================
## Constants
##======================================================================

##------------------------------------------------------------
## Constants: ...

##======================================================================
## Exports: finish
##======================================================================
our @EXPORT_OK = map { @$_ } values(%EXPORT_TAGS);
$EXPORT_TAGS{constants} = \@EXPORT_OK;


1;

__END__

# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Moot - Perl interface to the libmoot part-of-speech tagging library

=head1 SYNOPSIS

  use Moot;

  ##... stuff happens

=head1 DESCRIPTION

The Moot module provides an object-oriented interface to the libmoot library
for Hidden Markov Model part-of-speech tagging.

=head1 SEE ALSO

Moot::constants(3perl),
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