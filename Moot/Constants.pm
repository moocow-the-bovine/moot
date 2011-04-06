package Moot;
use Carp;
use strict;

##=====================================================================
## Constants: TokenType

our (%TokType,@TokType);
BEGIN {
  %TokType = (
	      'unknown' => TokTypeUnknown(),
              'vanilla' => TokTypeVanilla(),
              'libxml'  => TokTypeLibXML(),
              'xmlraw'  => TokTypeXMLRaw(),
              'comment' => TokTypeComment(),
              'eos'     => TokTypeEOS(),
              'eof'     => TokTypeEOF(),
              'user'    => TokTypeUser(),
             );
  @TokType[values %TokType] = keys %TokType;
}

our (%vlevel,@vlevel);
BEGIN {
  %vlevel = (
	     'silent' => vlSilent(),
	     'errors' => vlErrors(),
	     'warnings'  => vlWarnings(),
	     'progress'  => vlProgress(),
	     'everything' => vlEverything(),
	    );
  @vlevel[values %vlevel] = keys %vlevel;
}



1; ##-- be happy

__END__

=pod

=head1 NAME

Moot::Constants - libmoot : constants

=head1 SYNOPSIS

  use Moot;

  ##=====================================================================
  ## Constants

  $version = Moot::library_version();

  ##-- Token Types: indices by name
  $i = Moot::TokTypeUnknown();  ##-- alias: $Moot::TokType{unknown}
  $i = Moot::TokTypeVanilla();  ##-- alias: $Moot::TokType{vanilla}
  $i = Moot::TokTypeLibXML();   ##-- alias: $Moot::TokType{libxml}
  $i = Moot::TokTypeXMLRaw();   ##-- alias: $Moot::TokType{xmlraw}
  $i = Moot::TokTypeComment();  ##-- alias: $Moot::TokType{comment}
  $i = Moot::TokTypeEOS();      ##-- alias: $Moot::TokType{eos}
  $i = Moot::TokTypeEOF();      ##-- alias: $Moot::TokType{eof}
  $i = Moot::TokTypeUser();     ##-- alias: $Moot::TokType{user}

  ##-- Token Types: names by index
  $name = $Moot::TokType[$i];

  ##-- verbosity levels: indices by name
  $i = Moot::vlSilent();        ##-- alias: $Moot::vlevel{silent}
  $i = Moot::vlErrors();        ##-- alias: $Moot::vlevel{errors}
  $i = Moot::vlWarnings();      ##-- alias: $Moot::vlevel{warnings}
  $i = Moot::vlProgress();      ##-- alias: $Moot::vlevel{progress}
  $i = Moot::vlEverything();    ##-- alias: $Moot::vlevel{everything}

  ##-- verbosity levels: names by index
  $name = $Moot::vlevel[$i];


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

