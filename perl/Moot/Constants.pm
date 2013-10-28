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
              'sb'      => TokTypeSB(),
              'wb'      => TokTypeWB(),
              'user'    => TokTypeUser(),
             );
  @TokType[values %TokType] = keys %TokType;
}

##=====================================================================
## Constants: vlevel
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


##=====================================================================
## Constants: wasteScannerType

our (%wScanType,@wScanType);
BEGIN {
  %wScanType = (
	       'eof' => Moot::wScanTypeEOF(),
	       'wb' => Moot::wScanTypeWB(),
	       'sb' => Moot::wScanTypeSB(),
	       'latin' => Moot::wScanTypeLatin(),
	       'greek' => Moot::wScanTypeGreek(),
	       'alpha' => Moot::wScanTypeAlpha(),
	       'newline' => Moot::wScanTypeNewline(),
	       'space' => Moot::wScanTypeSpace(),
	       'number' => Moot::wScanTypeNumber(),
	       'roman' => Moot::wScanTypeRoman(),
	       'hyphen' => Moot::wScanTypeHyphen(),
	       'punct' => Moot::wScanTypePunct(),
	       'link' => Moot::wScanTypeLink(),
	       'xml' => Moot::wScanTypeXML(),
	       'comment' => Moot::wScanTypeComment(),
	       'other' => Moot::wScanTypeOther(),
             );
  @wScanType[values %wScanType] = keys %wScanType;
}

##=====================================================================
## Constants: LexerType

our (%wLexType,@wLexType);
BEGIN {
  %wLexType = (
	      'dot' => Moot::wLexerTypeDot(),
	      'comma' => Moot::wLexerTypeComma(),
	      'hyph' => Moot::wLexerTypeHyph(),
	      'apostrophe' => Moot::wLexerTypeApostrophe(),
	      'quote' => Moot::wLexerTypeQuote(),
	      'monetary' => Moot::wLexerTypeMonetary(),
	      'percent' => Moot::wLexerTypePercent(),
	      'plus' => Moot::wLexerTypePlus(),
	      'lbr' => Moot::wLexerTypeLBR(),
	      'rbr' => Moot::wLexerTypeRBR(),
	      'slash' => Moot::wLexerTypeSlash(),
	      'colon' => Moot::wLexerTypeColon(),
	      'semicolon' => Moot::wLexerTypeSemicolon(),
	      'eos' => Moot::wLexerTypeEOS(),
	      'punct' => Moot::wLexerTypePunct(),
	      'space' => Moot::wLexerTypeSpace(),
	      'newline' => Moot::wLexerTypeNewline(),
	      'number' => Moot::wLexerTypeNumber(),
	      'roman' => Moot::wLexerTypeRoman(),
	      'lower' => Moot::wLexerTypeAlphaLower(),
	      'upper' => Moot::wLexerTypeAlphaUpper(),
	      'caps' => Moot::wLexerTypeAlphaCaps(),
	      'trunc' => Moot::wLexerTypeAlphaTrunc(),
	      'other' => Moot::wLexerTypeOther(),
             );
  @wLexType[values %wLexType] = keys %wLexType;
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

  ##------------------------------------------------------------
  ## version
  $version = Moot::library_version();

  ##------------------------------------------------------------
  ## Token Types

  $i = Moot::TokTypeUnknown();  ##-- alias: $Moot::TokType{unknown}
  $i = Moot::TokTypeVanilla();  ##-- alias: $Moot::TokType{vanilla}
  $i = Moot::TokTypeLibXML();   ##-- alias: $Moot::TokType{libxml}
  $i = Moot::TokTypeXMLRaw();   ##-- alias: $Moot::TokType{xmlraw}
  $i = Moot::TokTypeComment();  ##-- alias: $Moot::TokType{comment}
  $i = Moot::TokTypeEOS();      ##-- alias: $Moot::TokType{eos}
  $i = Moot::TokTypeEOF();      ##-- alias: $Moot::TokType{eof}
  $i = Moot::TokTypeSB();       ##-- alias: $Moot::TokType{sb}
  $i = Moot::TokTypeWB();       ##-- alias: $Moot::TokType{wb}
  $i = Moot::TokTypeUser();     ##-- alias: $Moot::TokType{user}
  $name = $Moot::TokType[$i];   ##-- name by index

  ##------------------------------------------------------------
  ## Verbosity Levels

  $i = Moot::vlSilent();        ##-- alias: $Moot::vlevel{silent}
  $i = Moot::vlErrors();        ##-- alias: $Moot::vlevel{errors}
  $i = Moot::vlWarnings();      ##-- alias: $Moot::vlevel{warnings}
  $i = Moot::vlProgress();      ##-- alias: $Moot::vlevel{progress}
  $i = Moot::vlEverything();    ##-- alias: $Moot::vlevel{everything}
  $name = $Moot::vlevel[$i];    ##-- verbosity levels: names by index

  ##------------------------------------------------------------
  ## WASTE Scanner Types

  $i = Moot::wScanTypeEOF();		##-- alias: $Moot::wScanType{eof}
  $i = Moot::wScanTypeWB();		##-- alias: $Moot::wScanType{wb}
  $i = Moot::wScanTypeSB();		##-- alias: $Moot::wScanType{sb}
  $i = Moot::wScanTypeLatin();		##-- alias: $Moot::wScanType{latin}
  $i = Moot::wScanTypeGreek();		##-- alias: $Moot::wScanType{greek}
  $i = Moot::wScanTypeAlpha();		##-- alias: $Moot::wScanType{alpha}
  $i = Moot::wScanTypeNewline();	##-- alias: $Moot::wScanType{newline}
  $i = Moot::wScanTypeSpace();		##-- alias: $Moot::wScanType{space}
  $i = Moot::wScanTypeNumber();		##-- alias: $Moot::wScanType{number}
  $i = Moot::wScanTypeRoman();		##-- alias: $Moot::wScanType{roman}
  $i = Moot::wScanTypeHyphen();		##-- alias: $Moot::wScanType{hyphen}
  $i = Moot::wScanTypePunct();		##-- alias: $Moot::wScanType{punct}
  $i = Moot::wScanTypeLink();		##-- alias: $Moot::wScanType{link}
  $i = Moot::wScanTypeXML();		##-- alias: $Moot::wScanType{xml}
  $i = Moot::wScanTypeComment();	##-- alias: $Moot::wScanType{comment}
  $i = Moot::wScanTypeOther();		##-- alias: $Moot::wScanType{other}
  $name = $Moot::wScanType[$i];         ##-- WASTE Scanner Types: names by index

  ##------------------------------------------------------------
  ## WASTE Lexer Types

  $i = Moot::wLexerTypeDot();		##-- alias: $Moot::wLexType{dot}
  $i = Moot::wLexerTypeComma();		##-- alias: $Moot::wLexType{comma}
  $i = Moot::wLexerTypeHyph();		##-- alias: $Moot::wLexType{hyph}
  $i = Moot::wLexerTypeApostrophe();	##-- alias: $Moot::wLexType{apostrophe}
  $i = Moot::wLexerTypeQuote();		##-- alias: $Moot::wLexType{quote}
  $i = Moot::wLexerTypeMonetary();	##-- alias: $Moot::wLexType{monetary}
  $i = Moot::wLexerTypePercent();	##-- alias: $Moot::wLexType{percent}
  $i = Moot::wLexerTypePlus();		##-- alias: $Moot::wLexType{plus}
  $i = Moot::wLexerTypeLBR();		##-- alias: $Moot::wLexType{lbr}
  $i = Moot::wLexerTypeRBR();		##-- alias: $Moot::wLexType{rbr}
  $i = Moot::wLexerTypeSlash();		##-- alias: $Moot::wLexType{slash}
  $i = Moot::wLexerTypeColon();		##-- alias: $Moot::wLexType{colon}
  $i = Moot::wLexerTypeSemicolon();	##-- alias: $Moot::wLexType{semicolon}
  $i = Moot::wLexerTypeEOS();		##-- alias: $Moot::wLexType{eos}
  $i = Moot::wLexerTypePunct();		##-- alias: $Moot::wLexType{punct}
  $i = Moot::wLexerTypeSpace();		##-- alias: $Moot::wLexType{space}
  $i = Moot::wLexerTypeNewline();	##-- alias: $Moot::wLexType{newline}
  $i = Moot::wLexerTypeNumber();	##-- alias: $Moot::wLexType{number}
  $i = Moot::wLexerTypeRoman();		##-- alias: $Moot::wLexType{roman}
  $i = Moot::wLexerTypeAlphaLower();	##-- alias: $Moot::wLexType{lower}
  $i = Moot::wLexerTypeAlphaUpper();	##-- alias: $Moot::wLexType{upper}
  $i = Moot::wLexerTypeAlphaCaps();	##-- alias: $Moot::wLexType{caps}
  $i = Moot::wLexerTypeAlphaTrunc();	##-- alias: $Moot::wLexType{trunc}
  $i = Moot::wLexerTypeOther();		##-- alias: $Moot::wLexType{other}
  $name = $Moot::wLexType[$i];           ##-- Lexer Types: names by index

=head1 DESCRIPTION

The Moot module provides an object-oriented interface to the libmoot library
for Hidden Markov Model part-of-speech tagging.

=head1 SEE ALSO

Moot::constants(3perl),
moot(1),
perl(1).

=head1 AUTHOR

Bryan Jurish E<lt>moocow@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2011-2013 by Bryan Jurish

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.10.1 or,
at your option, any later version of Perl 5 you may have available.

=cut

