#!/usr/bin/perl -w

use lib qw(./blib/lib ./blib/arch);
use moot;
use Getopt::Long;
use File::Basename qw(basename);
use Pod::Usage;

##------------------------------------------------------------------------------
## Constants & Globals
##------------------------------------------------------------------------------
our $prog = basename($0);
our $compact_ngrams = 1;
our $outfile = '-';

##------------------------------------------------------------------------------
## Command-line
##------------------------------------------------------------------------------
GetOptions(##-- General
	   'help|h' => \$help,

	   ##-- misc
	   'verbose-ngrams|verbose|v|N!' => sub { $compact_ngrams=!$_[1]; },
	   'compact-ngrams|compact!' => \$compact_ngrams,

	   ##-- I/O
	   'output|o=s' => \$outfile,
	  );

pod2usage({-exitval=>0, -verbose=>0}) if ($help);

##------------------------------------------------------------------------------
## MAIN

our $ng = moot::Ngrams->new();
foreach $file (@ARGV) {
  $ng->load($file) || die("$prog: load failed for file '$file': $!");
}
$ng->save($outfile,($compact_ngrams ? 1 : 0));


__END__

=pod

=head1 NAME

 mootmerge-123.perl - merge moot N-gram model files

=head1 SYNOPSIS

 mootmerge-123.perl [OPTIONS] NGRAM_FILE(s)...

 Options:
  -help                     # this help message
  -verbose , -noverbose     # do/don't generate verbose n-gram output (default=no)
  -output OUTFILE           # save output to OUTFILE (default=STDOUT)

=cut

##------------------------------------------------------------------------------
## Options and Arguments
##------------------------------------------------------------------------------
=pod

=head1 OPTIONS AND ARGUMENTS

not yet written.

=cut

##------------------------------------------------------------------------------
## Description
##------------------------------------------------------------------------------
=pod

=head1 DESCRIPTION

not yet written.

=cut


##------------------------------------------------------------------------------
## Footer
##------------------------------------------------------------------------------
=pod

=head1 AUTHOR

Bryan Jurish E<lt>moocow@bbaw.deE<gt>

=cut
