#!/usr/local/bin/perl -w

#############################################################################
#
# File: kempe-arcgen.perl
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
$EOS = 'EOS';
$bottom = 'BOTTOM';
$compact = 1;
$verbose = 1;
GetOptions(## General Options
	   'help|h|?' => \$help,
	   'man' => \$man,
	   'version' => \$version,
	   'verbose|v:i' => \$verbose,
	   'v0' => sub { $verbose=0; },
	   'v1' => sub { $verbose=1; },
	   'v2' => sub { $verbose=2; },
	   ## Generation options
	   'eos|e=s' => \$EOS,
	   #'join|j=s' => \$joinstr,
	   'compact|z:i' => \$compact,
	   'z0' => sub { $compact=0; },
	   'z1' => sub { $compact=1; },
	   'z2' => sub { $compact=2; },
	   'bottom|b=s' => \$bottom,
	   ## I/O Options
	   'alphabet|a=s' => \@alphabets,
	   'output|o=s' => \$arcfile,
	  );

if ($version || $verbose>0) {
  print STDERR "\n$0 version $VERSION by Bryan Jurish <moocow\@ling.uni-potsdam.de>\n\n";
  exit 1 if ($version);
}
pod2usage({-verbose=>2}) if ($man);
pod2usage({-verbose=>0}) if ($help);

pod2usage({-verbose=>0,
	   -message=>"You must specify at least one alphabet!\n"}) if (!@alphabets);

#-----------------------------------------------------------------------
# Process Options: I/O
#-----------------------------------------------------------------------
$arcfile = '-' if (!defined($arcfile));
$ARCFILE = IO::File->new(">$arcfile") ||
  die("$0: could not open output arclist-file '$arcfile' for write: $!");
$ARCFILE->close();

###############################################################
# Data:
#  $gen : KempeGen object
###############################################################
$gen = KempeGen->new(
		     eos=>$EOS,
		     bottom=>$bottom,
		     verbose=>$verbose,
		    );


###############################################################
# MAIN
###############################################################
select(STDERR); $| = 1;
select(STDOUT);

push(@ARGV,'-') if (!@ARGV);
foreach $alphafile (@alphabets) {
  print STDERR ("Loading alphabet file '$alphafile'...") if ($verbose>0);
  $gen->load_alphabet_file($alphafile);
  print STDERR (" loaded.\n") if ($verbose>0);
}
foreach $parfile (@ARGV) {
  print STDERR ("Loading parameter file '$parfile'...") if ($verbose>0);
  $gen->analyse_param_file($parfile);
  print STDERR (" loaded.\n") if ($verbose>0);
}


print STDERR ("Compiling internal data structures...") if ($verbose>0);
$gen->compile();
print STDERR (" compiled.\n") if ($verbose>0);


print STDERR ("Generating arclist file '$arcfile'...") if ($verbose>0);
$gen->save_arclist_file($arcfile,$compact);
print STDERR (" saved.\n") if ($verbose>0);


__END__


###############################################################
=pod

=head1 NAME

kempe-arcgen.perl -- generate a disambiguation FST-arclist
from a dwdst/TnT parameter file and an alphabet specification.

=head1 SYNOPSIS

 kempe-arcgen.perl [OPTIONS] ALPHABET_FILE(s)

 General Options:
   -help
   -man
   -version
   -verbose LEVEL

 Generation Options:
   -eos EOS
   -bottom BOTTOM
   -compact LEVEL

 I/O Options:
   -a ALPHABET_FILE
   -o ARCLIST_FILE

=cut

###############################################################
# Description
###############################################################
=pod

=head1 DESCRIPTION

Generates an FST-based disambiguator
based on a dwdst/TnT paramater file and an intermediate alphabet,
similar to that described by Kempe(2000a).

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

=item * C<-verbose LEVEL>

Verbosity level.  Range: 0..1.
Default=1.

=back

=cut

###############################################################
# Generation Options
###############################################################
=pod

=head2 Generation Options

=over 4

=item * C<-compact LEVEL>

Compact arclist file by using shorter names.
Available values for LEVEL:

 0 : use long symbol names
 1 : use long tag-names and short class-names
 2 : use short tag-names and short symbol-names

Default=1.

=item * C<-eos EOS>

Use EOS as the end-of-string marker.
Default='EOS'.

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

=item * C<-a ALPHABET_FILE>

Load alphabet specifications from ALPHABET_FILE.
You should specify at least one of these.
Default=none.

=item * C<-o ARCLIST_FILE>

Write generated arclist to ARCLIST_FILE.
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
