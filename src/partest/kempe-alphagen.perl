#!/usr/local/bin/perl -w

#############################################################################
#
# File: kempe-alphagen.perl
# Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
# Description: generate meta-alphabets a la Kempe(2000a) from dwdst output files
#              (TnT format)
#
#############################################################################

use Getopt::Long;
use Pod::Usage;
use IO::File;

use lib qw(.);
use KempeGen;

########################################################################
# Globals
########################################################################
our $VERSION = 0.02;

########################################################################
# Command-line processing
########################################################################
$EOS = 'EOS';
$bottom = 'BOTTOM';

GetOptions(## General Options
	   'help|h|?' => \$help,
	   'man' => \$man,
	   'version' => \$version,
	   ## Generation options
	   'eos|e=s' => \$EOS,
	   #'join|j=s' => \$joinstr,
	   'bottom|b=s' => \$bottom,
	   ## I/O Options
	   'output|o=s' => \$alphafile,
	  );

if ($version) {
  print STDERR "\n$0 version $VERSION by Bryan Jurish <moocow\@ling.uni-potsdam.de>\n\n";
  exit 1;
}
pod2usage({-verbose=>2}) if ($man);
pod2usage({-verbose=>0}) if ($help);

#-----------------------------------------------------------------------
# Process Options: I/O
#-----------------------------------------------------------------------
$alphafile = '-' if (!defined($alphafile));

$ALPHAFILE = IO::File->new(">$alphafile") ||
  die("$0: could not open output alphabet-file '$alphafile' for write: $!");
$ALPHAFILE->close();

###############################################################
# Data:
#  $gen : KempeGen object
###############################################################
$gen = KempeGen->new(
		     bottom=>$bottom,
		     eos=>$EOS,
		    );

###############################################################
# MAIN
###############################################################
push(@ARGV,'-') if (!@ARGV);
foreach $tntfile (@ARGV) {
  $gen->analyze_tnt_file($tntfile);
}
$gen->save_alphabet_file($alphafile);

__END__

###############################################################
=pod

=head1 NAME

kempe-alphagen.perl -- generate a meta-alphabet for FST-based disambiguation.

=head1 SYNOPSIS

 kempe-alphagen.perl [OPTIONS] TNT_OUTPUT_FILE(s)

 General Options:
   -help
   -man
   -version

 Generation Options:
   -eos EOS
   -bottom BOTTOM

 I/O Options:
   -o OUTPUT_FILE

=cut

###############################################################
# Description
###############################################################
=pod

=head1 DESCRIPTION

Generates an intermediate alphabet similar to that described by Kempe(2000a)
for FST-based disambiguation from dwdst output files in TnT format.

=cut

###############################################################
# Options
###############################################################
=pod

=head1 OPTIONS

=cut

###############################################################
# General Options
###############################################################
=pod

=head2 General Options

=over 4

=item * C<-help>

Display a brief help message.

=item * C<-man>

Display this manual page.

=item * C<-version>

Display version information.

=back

=cut

###############################################################
# Generation Options
###############################################################
=pod

=head2 Generation Options

=over 4

=item * C<-eos EOS>

Use EOS as the end-of-string marker.
Default='EOS'.

=begin comment text

=item * C<-join SEPARATOR>
Join individual tags into classes with SEPARATOR (not whitespace!).
Default='|' (vertical bar).

=end comment text

=item * C<-bottom BOTTOM>

Use BOTTOM as the name of the "universal" class.
Default='BOTTOM'.

=back

=cut


###############################################################
# I/O Options
###############################################################
=pod

=head2 I/O Options

=over 4

=item * C<-o ALPHABET_FILE>

Write generated alphabet to the file ALPHABET_FILE.
Default=STDOUT.

File format: lines of the form:

 TYPE  SHORTNAME LONGNAME REGEX

where fields are separated by one or more TAB characters
and optional additional whitespace, and:

 TYPE      is one of 'Tag' or 'Class',
 SHORTNAME is a generated symbol-name
 LONGNAME  is long generated symbol-name
 REGEX     is a regular expression for the entry

=back

=cut


###############################################################
# Bugs
###############################################################
=pod

=head1 BUGS

Probably many.

=cut


###############################################################
# Footer
###############################################################
=pod

=head1 ACKNOWLEDGEMENTS

perl by Larry Wall.

This package was developed as part of the KDWDS project
("Kollokationen im Digitalen Wörterbuch der deutschen Sprache"
= "collocations in the digital dictionary of the German language") at the
Berlin-Brandenburgische Akademie der Wissenschaften.
For details on the DWDS project, visit: http://www.dwds.de.

=head1 AUTHOR

Bryan Jurish E<lt>moocow@ling.uni-potsdam.deE<gt>

=head1 SEE ALSO

perl(1).

=cut

