#!/usr/local/bin/perl -w

########################################################################
#        File: analyse-parfile.perl
#      Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
# Description: analyse a DWDST parameter file
########################################################################

use Getopt::Long;
use Pod::Usage;

# USAGE = $0 < DWDST_PAR_FILE

########################################################################
# Globals
########################################################################
our $VERSION = 0.01;

########################################################################
# Command-line processing
########################################################################
$wdsep = "\t";
$verbose = 1;
GetOptions(## -- General
	   'help|h|?'=>\$help,
	   'man'=>\$man,
	   'verbose|v=i'=>\$verbose,
	   'version'=>\$version,
	   ## -- Format
	   'wdsep|w'=>\$wdsep,
	  );

if ($version) {
  print STDERR "\n$0 version $VERSION by Bryan Jurish <moocow\@ling.uni-potsdam.de>\n\n";
  exit 1;
}
pod2usage({-verbose=>2}) if ($man);
pod2usage({-verbose=>0}) if ($help);

###############################################################
# MAIN
#  + Data Structures
#    + %ngrams
#      ( $ng1 => $wt1, ... )
#    + %weights
#      ( $wt1 => { $ng1_1=>undef, ... } )
#    + %ambigs
#      ( $awt1 => { $ng1_1=>undef, ... } )
###############################################################

## -- read in parameter files
%ngrams = ();
while (defined($_ = <>)) {
  chomp($_);
  next if (/^\s*(?:%%.*)?$/); # skip comments and blank lines

  @sline = split(/\s*\t+\s*/,$_);
  ($weight) = pop(@sline);
  $ngram = join($wdsep, @sline);

  ## -- forward-index
  if (exists($ngrams{$ngram})) {
    $ngrams{$ngram} += $weight;
  } else {
    $ngrams{$ngram} = $weight;
  }

  ## -- reverse-index
  $weights{$weight}{$ngram} = undef;
}

%ambigs = ();
while (($weight,$ngset) = each(%weights)) {
  # examine
  next if (scalar(keys(%$ngset)) <= 1);  # ignore singletons
 NG1:
  foreach $ng1 (keys(%$ngset)) {
    @tags1 = split($wdsep,$ng1);
  NG2:
    foreach $ng2 (keys(%$ngset)) {
      next if ($ng1 eq $ng2);
      @tags2 = split($wdsep,$ng2);
      next if ($#tags1 != $#tags2);

      for ($i = 0; $i < $#tags1; ++$i) {
	next NG2 if ($tags1[$i] ne $tags2[$i]);
      }
      ## -- it's a real ambiguity
      $ambigs{"$weight"}{$ng2}
	= $ambigs{"$weight"}{$ng1} = undef;

      ## -- DEBUG:
      if ($verbose > 0) {
	print("> Found ambiguity for weight $weight :\n",
	      ">     Ngram_1: '$ng1'\n",
	      ">  \!= Ngram_2: '$ng2'\n\n");
      }
    }
  }
}

## -- get max,min,avg ambiguity-set size
$maxamb = $minamb = $avgamb = undef;
while (($weight,$aset) = each(%ambigs)) {
  $asize = scalar(keys(%$aset));
  $maxamb = $asize if (!defined($maxamb) || $asize > $maxamb);
  $minamb = $asize if (!defined($minamb) || $asize < $minamb);
  $avgamb += $asize;
}
$nambs = scalar(keys(%ambigs));
$maxamb = 0 if (!defined($maxamb));
$minamb = 0 if (!defined($minamb));
$avgamb = ($nambs == 0 ? '-NaN-' : $avgamb / $nambs);

## --- Summary
print STDERR
  (
   #"-" x 72, "\n",
   "$0 Summary:\n",
   "  + Total Number of distinct N-Grams: ", scalar(keys(%ngrams)), "\n",
   "  + Total Number of distinct Weights: ", scalar(keys(%weights)), "\n",
   "  + Total Number of distinct Ambiguities : $nambs\n",
   "    - Minimum Ambiguity-set size: $minamb\n",
   "    - Maximum Ambiguity-set size: $maxamb\n",
   "    - Average Ambiguity-set size: $avgamb\n",
   #"-" x 72, "\n"
  );

__END__
###############################################################
# Program Usage
###############################################################
=pod

=head1 NAME

analyse-parfile.perl - analyse a DWDST parameter file.

=head1 SYNOPSIS

 analyse-parfile.perl [OPTIONS] [PARFILE]

 General Options:
   -help
   -man
   -version
   -verbose LEVEL

 Format Options:
   -wdsep WORD_SEPARATOR

=cut

###############################################################
# Description
###############################################################
=pod

=head1 DESCRIPTION


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
# Footer
###############################################################
=pod

=head1 ACKNOWLEDGEMENTS

perl by Larry Wall.

This package was developed as part of the DWDS/Collocations project
("Kollokationen im digitalen Wörterbuch der deutschen Sprache"
= "collocations in the digital dictionary of the German language") at the
Berlin-Brandenburgische Akademie der Wissenschaften.
For details on the DWDS project, visit: http://www.dwds.de.

=head1 AUTHOR

Bryan Jurish E<lt>moocow@ling.uni-potsdam.deE<gt>

=head1 SEE ALSO

perl(1).

=cut
