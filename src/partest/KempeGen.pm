# -*- Mode: Perl -*-

#############################################################################
#
# File: KempeGen.perl
# Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
# Description: Utilities for Kempe(2000a)-style FST generation
#
#############################################################################

package KempeGen;

use Carp;
use IO::File;

########################################################################
# Globals
########################################################################
our $VERSION = 0.03;

our @cbrackets = ('{','}');
our $cjoinstr = ',';
our $njoinstr = ',';

our $FSMCOMPRE  = 'FSMcompre';
our $FSMSTRINGS = 'FSMstrings';

########################################################################
# Constructor
# + Object structure: hash of hashes:
#   {
#    tags=>{tag1=>tag1regex,...},
#    classes=>{class1=>class1regex,...},
#    tag2z=>{tag1=>ztag1,...},
#    class2z=>{class1=>zclass1,...},
#    classtags=>{class1=>{tag1_1=>undef, ... }},
#   }
########################################################################
sub new {
  my ($that,%args) = @_;
  my $class = ref($that) || $that;
  my $gen = bless {
		   ## -- base data
		   tags=>{},        # tag-regexes by tag-name
		   classes=>{},     # class-regexes by class-name
		   tag2z=>{},       # tag-abbrevs by tag-name (COMPILED ONLY)
		   class2z=>{},     # class-abbrevs by class-name (COMPILED ONLY)
		   classtags=>{},   # tag-name-hashes by class-name (COMPILED ONLY)
		   ## -- flags
		   eos=>'EOS',
		   bottom=>'BOTTOM',
		   compiled=>0,
		   verbose=>0,
		   ## -- counters
		   tagid=>0,
		   classid=>0,
		   ## -- ngram parameters
		   counts=>{},
		   besttags=>{},    # cache
		  }, $class;
  foreach (keys(%args)) {
    if (ref($args{$_}) && ref($args{$_}) eq 'HASH') {
      %{$gen->{$_}} = %{$args{$_}};
    } else {
      $gen->{$_} = $args{$_};
    }
  }
  return $gen;
}

#-------------------------------------------------------
# $gen = $gen->clear();
#  + clears all data
sub clear {
  my $gen = shift;
  %{$gen->{tags}} = ();
  %{$gen->{classes}} = ();
  %{$gen->{tag2z}} = ();
  %{$gen->{class2z}} = ();
  %{$gen->{classtags}} = ();
  $gen->{tagid} = 0;
  $gen->{classid} = 0;
  %{$gen->{counts}} = ();
  %{$gen->{besttags}} = ();
  return $gen;
}

###############################################################
# Compilation
###############################################################

#-------------------------------------------------------
# $gen = $gen->compile();
#  + compiles 'tag2z', 'class2z', and 'classtags'
#    from 'tags' and 'classes'.
sub compile {
  my $gen = shift;

  ## -- insert 'BOTTOM' into our sets as a literal
  if (!exists($gen->{tags}{$gen->{bottom}})) {
    $gen->{tags}{$gen->{bottom}} = '['.$gen->{bottom}.']';
  }
  if (!exists($gen->{tag2z}{$gen->{bottom}})) {
    $gen->{tag2z}{$gen->{bottom}} = $gen->{bottom};
  }
  if (!exists($gen->{classes}{$gen->{bottom}})) {
    $gen->{classes}{"$gen->{bottom}"} = '['.$gen->{bottom}.']';
  }
  if (!exists($gen->{class2z}{$gen->{bottom}})) {
    $gen->{class2z}{$gen->{bottom}} = $gen->{bottom};
  }

  my ($class,$tag,$classtags);

  ## -- compile short tag-names
  foreach $tag (sort(keys(%{$gen->{tags}}))) {
    if (!exists($gen->{tag2z}{$tag})) {
      $gen->{tag2z}{$tag} = 't'.++$gen->{tagid};
    }
  }

  ## -- compile short class-names
  foreach $class (sort(keys(%{$gen->{classes}}))) {
    if (!exists($gen->{class2z}{$class})) {
      $gen->{class2z}{$class} = 'c'.++$gen->{classid};
    }
    ## -- compile class-tags
    if ($class eq $gen->{bottom}) {
      @{$gen->{classtags}{$class}}{keys(%{$gen->{tags}})} = undef;
    } else {
      $classtags = $class;
      $classtags =~ s/^$cbrackets[0]//;
      $classtags =~ s/$cbrackets[1]$//;
      @{$gen->{classtags}{$class}}{split(/,/, $classtags)} = undef;
    }
  }

  $gen->{compiled} = 1;
  return $gen;
}

###############################################################
# Alphabet Generation
###############################################################

#-------------------------------------------------------
# $gen_or_undef = $gen->load_tnt_file($file);
#  + populates 'tags' and 'classes' from a tnt-style
#    dwdst output file.
*analyse_tnt_file = \&analyze_tnt_file;
sub analyze_tnt_file {
  my ($gen,$file) = @_;

  my $TNTFILE = ref($file) ? $file : IO::File->new("<$file");
  if (!$TNTFILE) {
    carp(__PACKAGE__ ,
	 "::load_tnt_file(): could not open TnT file '$file' for read: $!");
    return undef;
  }

  $gen->{compiled} = 0;

  my($line,@rawtags,@tags,$rawclass,$class);
  while (defined($line=<$TNTFILE>)) {
    chomp($line);
    next if ($line =~ /^\s*(?:\#.*)?$/); # skip comments and blank lines
    next if ($line !~ /\t/);

    if ($line =~ /^\S+\t\s*(.*\S)$/) {
      @rawtags = sort(split(/\s*\t+\s*/, $1));

      shift(@rawtags) if ($rawtags[0] =~ /^\s*$/);
      @tags = @rawtags;
      @tags = map {
	$_ =~ s/\s+/\./g;
	$_ =~ s/[\[\]]+/\./g;
	$_ =~ s/[_\.]+$//;
	$_ =~ s/^[_\.]+//;
	$_;
      } @tags;

      @{$gen->{tags}}{@tags} = @rawtags;

      $class = $cbrackets[0].join($cjoinstr, @tags).$cbrackets[1];
      $rawclass = '('.join('|', map { '('.$_.')' } @rawtags).')';
      $gen->{classes}{$class} = $rawclass;
    } else {
      carp(__PACKAGE__ ,
	   "::load_tnt_file(): could not parse line '$line' -- ignoring.\n");
    }
  }
  $TNTFILE->close() if (!ref($file));

  return $gen;
}

###############################################################
# Alphabet load/save
###############################################################

#-------------------------------------------------------
# $gen_or_undef = $gen->load_alphabet_file($file);
#  + loads an alphabet file
sub load_alphabet_file {
  my ($gen,$file) = @_;
  my $ALPHAFILE = ref($file) ? $file : IO::File->new("<$file");

  if (!$ALPHAFILE) {
    carp(__PACKAGE__ ,
	 "::load_alphabet_file(): could not open alphabet file '$file' for read: $!");
    return undef;
  }

  $gen->{compiled} = 0;

  my ($line,$type,$zsym,$sym,$regex);
  while (defined($line = <$ALPHAFILE>)) {
    chomp($line);
    next if ($line =~ /^\s*(?:\#.*)?$/); # skip comments and blank lines
    ($type,$zsym,$sym,$regex) = split(/\s*\t+\s*/,$line);

    ## -- sanity check
    if (defined($type) && defined($zsym) && defined($sym) && defined($regex))
      {
	if ($type eq 'Tag') {
	  $gen->{tags}{$sym} = $regex;
	  $gen->{tag2z}{$sym} = $zsym;
	}
	elsif ($type eq 'Class') {
	  $gen->{classes}{$sym} = $regex;
	  $gen->{class2z}{$sym} = $zsym;
	} else {
	  warn("$0: unknown TYPE field '$type' in input file -- ignoring.\n");
	}
      }
    else
      {
	carp(__PACKAGE__ ,
	     "::load_alphabet_file(): could not parse input line '$_' -- ignoring.\n")
      }
  }

  $ALPHAFILE->close() if (!ref($file));
  return $gen;
}

#-------------------------------------------------------
# $gen_or_undef = $gen->save_alphabet_file($file);
#  + saves an alphabet file
sub save_alphabet_file {
  my ($gen,$file) = @_;

  my $ALPHAFILE = ref($file) ? $file : IO::File->new(">$file");
  if (!$ALPHAFILE) {
    carp(__PACKAGE__ , "::save_alphabet_file(): could not open file '$file' for write: $!");
    return undef;
  }

  $gen->compile() if (!$gen->{compiled});

  $ALPHAFILE->print(
		    "# dwdst meta-alphabet file\n",
		    "#   + Automatically generated by ", __PACKAGE__ , "::save_alphabet_file()\n",
		    "#   + Edit at your own risk\n",
		   );
  my ($sym);
  foreach $sym (sort(keys(%{$gen->{tags}}))) {
    $ALPHAFILE->print("Tag\t $gen->{tag2z}{$sym}\t $sym\t $gen->{tags}{$sym}\n");
  }
  foreach $sym (sort(keys(%{$gen->{classes}}))) {
    $ALPHAFILE->print("Class\t $gen->{class2z}{$sym}\t $sym\t $gen->{classes}{$sym}\n");
  }

  $ALPHAFILE->close() if (!ref($file));
}

###############################################################
# Symbol generation
###############################################################

#-------------------------------------------------------
# $gen_or_undef = $gen->save_symbols_file($file,$compact=1,$flat=0);
#  + $compact : compaction-level (0..2)
#  + $flag    : boolean flag
sub save_symbols_file {
  my ($gen,$file,$compact,$flat) = @_;

  my $SYMFILE = ref($file) ? $file : IO::File->new(">$file");
  if (!$SYMFILE) {
    carp(__PACKAGE__ ,
	 "::save_symbols_file(): could not open file '$file' for write: $!");
    return undef;
  }

  $flat = 0 if (!defined($flat));
  $compact = 1 if (!defined($compact));

  $gen->compile() if (!$gen->{compiled});

  my ($sym,$psym);
  my $line = "Tag\t";
  foreach $sym (sort(keys(%{$gen->{tags}}))) {
    $psym = $compact>1 ? $gen->{tag2z}{$sym} : $sym;
    if ($flat || length($line) + length($psym) > 71) {
      $SYMFILE->print($line, "\n");
      $line = "Tag\t";
    }
    $line .= " $psym";
  }
  $SYMFILE->print(($line ne "Tag\t" ? $line : qw()), "\n\n");

  $line = "Class\t";
  foreach $sym (sort(keys(%{$gen->{classes}}))) {
    $psym = $compact>0 ? $gen->{class2z}{$sym} : $sym;
    if ($flat || length($line) + length($psym) > 70) {
      $SYMFILE->print($line,"\n");
      $line = "Class\t";
    }
    $line .= " $psym";
  }
  $SYMFILE->print(($line ne "Class\t" ? $line : qw()), "\n\n");

  $SYMFILE->close() if (!ref($file));
}

###############################################################
# N-gram Parameter Analysis
###############################################################

#-------------------------------------------------------
# $gen_or_undef = $gen->analyze_param_file($file)
#   + does not require any compilation
#   + incorporates no new tags or classes into the object
*analyse_param_file = \&analyze_param_file;
sub analyze_param_file {
  my ($gen,$file) = @_;

  my $PARFILE = ref($file) ? $file : IO::File->new("<$file");
  if (!$PARFILE) {
    carp(__PACKAGE__ ,
	 "::analyze_param_file(): could not open parameter file '$file' for read: $!");
    return undef;
  }

  ## -- always count bottom
  $gen->{counts}{$gen->{bottom}} = 0 if (!exists($gen->{counts}{$gen->{bottom}}));

  my ($line,@ngram,$ngram,$count);
  while (defined($line = <$PARFILE>)) {
    chomp($line);
    next if ($line =~ /^\s*(?:\%\%.*)?$/); # ignore comments and blank lines
    @ngram = (
	      map {
		$_ =~ s/^\s*//;
		$_ =~ s/\s*$//;
		$_ =~ s/\s+/\./g;
		$_ =~ s/[\[\]]+/\./g;
		$_ =~ s/[_\.]+$//;
		$_ =~ s/^[_\.]+//;
		$_;
	      } split(/\s*\t+\s*/,$line)
	     );
    $count = pop(@ngram);
    shift(@ngram) if (@ngram && $ngram[0] =~ /^\s*$/);
    next if (!@ngram);

    $ngram = join($njoinstr, @ngram);
    if (!exists($gen->{counts}{$ngram})) {
      $gen->{counts}{$ngram} = $count;
    } else {
      $gen->{counts}{$ngram} += $count;
    }

    $gen->{counts}{$gen->{bottom}} += $count if (scalar(@ngram)==1);
  }

  $PARFILE->close() if (!ref($file));
  return $gen;
}

###############################################################
# Arclist Generation
###############################################################

#-------------------------------------------------------
# $tag = $gen->find_best_tag($prevtag,\%curtags)
#   + find the best tag to follow '$prevtag',
#     given bigrams in $gen->{counts}
sub find_best_tag {
  my ($gen,$prevtag,$curtags) = @_;
  my (@curtags,$tag,$ngram,%best,$best,$bestcount);
  %best = ();
  $bestcount = undef;

  ## -- single-tag might be an easy answer
  @curtags = keys(%$curtags);
  if (scalar(@curtags) == 1) {
    return $curtags[0] if ($curtags[0] ne $gen->{bottom});
    ## -- otherwise, expand curtags here...
    $curtags = $gen->{tags};
    @curtags = keys(%$curtags);
  }

  ## -- try tags in \%curtags first
  if ($prevtag ne $gen->{bottom}) {
    foreach $tag (@curtags) {
      next if ($tag eq $gen->{bottom} || $tag eq $gen->{eos}); # -- ignore special pseudo-tags
      $ngram = $prevtag.$njoinstr.$tag;

      next if (!exists($gen->{counts}{$ngram}));
      if (!defined($bestcount) || $bestcount < $gen->{counts}{$ngram}) {
	## -- found a better match
	$bestcount = $gen->{counts}{$ngram};
	%best = ($tag=>undef);
      } elsif ($bestcount == $gen->{counts}{$ngram}) {
	## -- found an equivalent match
	$best{$tag} = undef;
      }
    }
  }

  if (!%best) {
    ## -- fallback #1: try unigrams
    if ($gen->{verbose}>0 && $prevtag ne $gen->{bottom} && $curtags ne $gen->{tags}) {
      print STDERR
	(__PACKAGE__ ,
	 "::find_best_tag()",
	 ": unigram fallback: context=<$prevtag>, curtags={",
	 join(',',sort(keys(%$curtags))), "}",
	 "\n"
	);
    }
    foreach $tag (keys(%$curtags)) {
      next if ($tag eq $gen->{bottom} || $tag eq $gen->{eos}); # -- ignore special pseudo-tags
      $ngram = $tag;
      next if (!exists($gen->{counts}{$ngram}));
      if (!defined($bestcount) || $bestcount < $gen->{counts}{$ngram}) {
	## -- found a better match
	$bestcount = $gen->{counts}{$ngram};
	$best = $tag;
      }
    }
  }

  if (!defined($best)) {
    ## -- break ties by unigram counts
    $bestcount = undef;
    foreach $tag (keys(%best)) {
      if (!defined($bestcount) || $bestcount < $gen->{counts}{$tag}) {
	$bestcount = $gen->{counts}{$tag};
	$best = $tag;
      }
    }
  }

  if (!defined($best)) {
    ## -- it's still not defined...
    if ($curtags ne $gen->{tags}) {
      ## -- fallback to any known tag...
      print STDERR
	(__PACKAGE__ ,
	 "::find_best_tag()",
	 ": ANYTAG fallback: context=<$prevtag>, curtags={",
	 join(',',sort(keys(%$curtags))), "}",
	 "\n"
	) if ($gen->{verbose} > 0);
      $best = $gen->find_best_tag($prevtag,$gen->{tags});
    } else {
      print STDERR
	(__PACKAGE__ ,
	 "::find_best_tag()",
	 ": ARGH: no best successor for <$prevtag> found -- using '$gen->{bottom}'",
	 "\n"
	) if ($gen->{verbose} > 0);
      $best = $gen->{bottom};
    }
  }

  return $best
}


#-------------------------------------------------------
# $gen_or_undef = $gen->save_arclist_file($file,$compact=1)
#   + write an arclist file: requires loaded parameters!
sub save_arclist_file {
  my ($gen,$file,$compact) = @_;
  my $ARCFILE = ref($file) ? $file : IO::File->new(">$file");
  if (!$ARCFILE) {
    carp(__PACKAGE__ ,
	 "::save_arclist_file(): could not open file '$file' for write: $!");
    return undef;
  }

  $gen->compile() if (!$gen->{compiled});

  $ARCFILE->print("$gen->{eos}\n");
  my @allclasses = sort(keys(%{$gen->{classes}}));
  my ($class1,$pclass1,$best1,$pbest1,$class2,$pclass2,$best2key,$best2,$pbest2);
  foreach $class1 (@allclasses) {
    $best1 = $gen->find_best_tag($gen->{eos},$gen->{classtags}{$class1});
    $pbest1 = $compact>1 ? $gen->{tag2z}{$best1} : $best1;
    $pclass1 = $compact>0 ? $gen->{class2z}{$class1} : $class1;
    $ARCFILE->print("$gen->{eos}\t $pclass1\t [$pclass1]:[$pbest1]\n");
    foreach $class2 (@allclasses) {
      $pclass2 = $compact>0 ? $gen->{class2z}{$class2} : $class2;
      $best2key = $best1.$njoinstr.$class2;
      $best2 = $gen->{besttags}{$best2key};
      if (!defined($best2)) {
	$best2 = $gen->{besttags}{$best2key} =
	  $gen->find_best_tag($best1,$gen->{classtags}{$class2});
      }
      $pbest2 = $compact>1 ? $gen->{tag2z}{$best2} : $best2;
      $ARCFILE->print("$pclass1\t $pclass2\t [$pclass2]:[$pbest2]\n");
    }
    $ARCFILE->print("$pclass1\n");
  }

  $ARCFILE->close() if (!ref($file));
  return $gen;
}

###############################################################
# Vectormap Generation
###############################################################

#-------------------------------------------------------
# $gen_or_undef = $gen->save_arclist_file($file,$compact=1)
#   + write an arclist file: requires loaded parameters!
sub save_vectormap_file {
  my ($gen,$file,$symfile,$compact) = @_;
  my $VMAPFILE = ref($file) ? $file : IO::File->new(">$file");
  if (!$VMAPFILE) {
    carp(__PACKAGE__ ,
	 "::save_vectormap_file(): could not open file '$file' for write: $!");
    return undef;
  }

  $gen->compile() if (!$gen->{compiled});

  my ($class,$pipe,$line,$vec);
  foreach $class (sort(keys(%{$gen->{classes}}))) {
    #next if ($class eq $cbrackets[0].$gen->{bottom}.$cbrackets[1]); # ignore bottom
    next if ($class eq $gen->{bottom}); # ignore bottom

    $pipe = IO::File->new(qq($FSMCOMPRE -s "$symfile" "$gen->{classes}{$class}" ).
			  qq(| $FSMSTRINGS -n |));
    if (!$pipe) {
      carp(__PACKAGE__ ,
	   "save_vectormap_file(): open failed for pipe from FSM tools: $!");
      $VMAPFILE->close() if (!ref($file));
      return undef;
    }

    $line = <$pipe>;  # skip initial line
    $vec = '';
    while (defined($line = <$pipe>)) {
      chomp($line);
      $line =~ s/^\s*//;
      $line =~ s/\s*$//;
      $vec .= ' '.$line;
    }
    $pipe->close();

    $vec =~ s/^\s*//;
    $vec =~ s/\s*$//;
    $VMAPFILE->print(($compact>0 ? $gen->{class2z}{$class} : $class), "\t$vec\n");
  }

  $VMAPFILE->close() if (!ref($file));
  return $gen;
}


1; # make perl happy

__END__

###############################################################
=pod

=head1 NAME

KempeGen.pm - generation utilities for Kempe(2000a)-style
FST-based disambiguation.

=head1 SYNOPSIS

 use KempeGen;

 ... not yet written

=cut

###############################################################
# Description
###############################################################
=pod

=head1 DESCRIPTION

Generation utilities for Kempe(2000a)-style FST-based disambiguation
for use with dwdst.

=cut

###############################################################
# Methods
###############################################################
=pod

=head1 METHODS

=cut

###############################################################
# Constructor
###############################################################
=pod

=head2 Constructor

=over 4

=item * C<new(%args)>

Creates a new KempeGen object based on keyword-arguments
%args.

Available keywords:

=over 4

=item * C<eos =E<gt> EOS>

Use EOS as the end-of-string marker.
Default='EOS'.

=item * C<bottom =e<gt> BOTTOM>

Use BOTTOM as the name of the "universal" class.
Default='BOTTOM'.

=back

=cut

###############################################################
# Compilation Methods
###############################################################
=pod

=head2 Compilation Methods

=over 4

=item * C<compile()>

Compiles internal data structures.  Should be called
by other API methods automagically as needed.

=back

=cut

###############################################################
# Alphabet Methods
###############################################################
=pod

=head2 Alphabet Methods

=over 4

=item * C<load_tnt_file($filename)>

Populates 'tags' and 'classes' keys by
loading an input file in TnT-style format, as output
by 'dwdst -T'.

=item * C<save_alphabet_file($filename)>

Saves alphabet meta-file to $filename.

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

