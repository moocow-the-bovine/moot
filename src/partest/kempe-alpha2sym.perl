#!/usr/local/bin/perl -w

#############################################################################
#
# File: kempe-alph2sym.perl
# Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
# Description: generate symbol-files from alphabet-files.
#              (native format)
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
#$EOS = 'EOS';
$bottom = 'BOTTOM';
$compact = 1;
GetOptions(## General Options
	   'help|h|?' => \$help,
	   'man' => \$man,
	   'version' => \$version,
	   ## Generation options
	   #'eos|e=s' => \$EOS,
	   #'join|j=s' => \$joinstr,
	   'flat!' => \$flat,
	   'compact|z:i' => \$compact,
	   'z0' => sub { $compact=0; },
	   'z1' => sub { $compact=1; },
	   'z2' => sub { $compact=2; },
	   'bottom|b=s' => \$bottom,
	   ## I/O Options
	   'output|o=s' => \$symfile,
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
$symfile = '-' if (!defined($symfile));
$SYMFILE = IO::File->new(">$symfile") ||
  die("$0: could not open output symbols-file '$symfile' for write: $!");
$SYMFILE->close();

###############################################################
# Data:
#  $gen : hash of hashes
#   {
#    tags=>{tag1=>tag1regex,...}
#    classes=>{class1=>class1regex,...}
#    tag2z=>{tag1=>ztag1,...}
#    class2z=>{class1=>zclass1,...}
#   }
###############################################################
$gen = KempeGen->new(
		     #eos=>$EOS,
		     bottom=>$bottom,
		    );


###############################################################
# MAIN
###############################################################

push(@ARGV,'-') if (!@ARGV);
foreach $alphafile (@ARGV) {
  $gen->load_alphabet_file($alphafile);
}

$gen->save_symbols_file($symfile,$compact,$flat);


__END__


###############################################################
=pod

=head1 NAME

kempe-alpha2sym.perl -- generate a symbols-file from an alphabet file.

=head1 SYNOPSIS

 kempe-alpha2sym.perl [OPTIONS] ALPHABET_FILE(s)

 General Options:
   -help
   -man
   -version

 Generation Options:
   -bottom BOTTOM
   -compact LEVEL
   -flat

 I/O Options:
   -o SYMBOLS_FILE

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

=item * C<-flat>

Output a 'flat' symbols file.  Default is to try and
write multiple symbols onto each line.

=item * C<-compact LEVEL>

Compact symbols file by using shorter names.
Available values for LEVEL:

 0 : use long symbol names
 1 : use long tag-names and short class-names
 2 : use short tag-names and short symbol-names

Default=1.

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

=item * C<-o SYMBOLS_FILE>

Write generated alphabet to symbols-file SYMBOLS_FILE.
Default=STDOUT.

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
