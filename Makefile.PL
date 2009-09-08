use ExtUtils::MakeMaker;

##----------------------------------------------------------------------
## Configuration
##----------------------------------------------------------------------
our $moot_min_version = "2.0.6";
if (system("pkg-config --atleast-version=$moot_min_version moot") != 0) {
  die("Required module moot >= v$moot_min_version not found!\n");
}

our $use_moot_oflags = 1;
$MOOT_OFLAGS = '-O0 -pipe' if (!$use_moot_oflags);

@MOOT_ALL_CFLAGS = map { $s=`pkg-config --cflags $_`; chomp($s); split(/\s+/,$s) } qw(moot);
$MOOT_IFLAGS = join(' ', map { $s=`pkg-config --cflags-only-I $_`; chomp($s); $s } qw(moot));
foreach (@MOOT_ALL_CFLAGS) {
  next if ($MOOT_IFLAGS =~ m/\b$_\b/);
  if (/^\-[fpmO]/) {
    $MOOT_OFLAGS .= " $_" if ($use_moot_oflags);
    next;
  }
  $MOOT_CFLAGS .= " $_";
}
$MOOT_LDFLAGS= join(' ', map { $s=`pkg-config --libs-only-L $_`; chomp($s); $s } qw(moot));
$MOOT_LIBS   = join(' ', map { $s=`pkg-config --libs-only-l $_`; chomp($s); $s } qw(moot));

@SWIG_DEPS = glob("swig/*.i");

##----------------------------------------------------------------------
## MAIN
##----------------------------------------------------------------------
WriteMakefile
  (
   NAME		=> 'moot',
   DISTNAME	=> 'moot-perl',
   AUTHOR       => 'Bryan Jurish <moocow@ling.uni-potsdam.de>',
   ABSTRACT	=> 'Perl interface to libmoot part-of-speech tagging library',
   #VERSION_FROM => 'moot.pm',
   VERSION      => '0.01',


   ##-- user variables
   CC           => 'g++',   ##-- your c++ compiler
   LIBS         => "$MOOT_LDFLAGS $MOOT_LIBS",      ##-- additional libraries
   DEFINE       => '-DHAVE_CONFIG_H',   ##-- additional defines
   INC          => $MOOT_IFLAGS,        ##-- additional includes
   CCFLAGS      => $MOOT_CFLAGS,        ##-- additional flags
   OPTIMIZE     => $MOOT_OFLAGS,        ##-- optimization flags
   OBJECT       => 'moot_wrap.o',

   ##-- DEBUG
   #MAKEAPERL=>1,
   #LINKTYPE=>'static',
   #MAP_TARGET=>'gfsmperl',
   #CCFLAGS=>'-g',
   ##-- /DEBUG

   ##-- stuff that probably doesn't need to be changed
   #XSOPT        => '-C++',
   #TYPEMAPS     => ['perlobject.map', 'typemap'],
  );


#-----------------------------------------------------------------------
# Extensions
#-----------------------------------------------------------------------
##...

#-----------------------------------------------------------------------
sub MY::macro {
  package MY;
  my $inherited = shift->SUPER::macro(@_);
  $inherited .= (
		 "\n\n"
		 ##-- useful macros
		 ."MOOT_CFLAGS = $::MOOT_CFLAGS\n"
		 ."MOOT_IFLAGS = $::MOOT_IFLAGS\n"
		 ."MOOT_OFLAGS = $::MOOT_OFLAGS\n"
		 ."MOOT_LDFLAGS = $::MOOT_LDFLAGS\n"
		 ."MOOT_LIBS = $::MOOT_LIBS\n"
		 ."SWIG_DEPS = ".join(' ', @::SWIG_DEPS)."\n"
		 ."\n"
		);
  $inherited;
}

#-----------------------------------------------------------------------
sub MY::depend {
  package MY;
  my $inherited = shift->SUPER::depend(@_);
  $inherited .= (
		 "\n\n"
		 ##-- swig dependencies
		 ."moot_wrap.cxx: swig.stamp\n\n"
		 ."moot.pm: swig.stamp\n\n"

		 #."clean_subdirs: clean_swig\n\n"
		 ."realclean_subdirs: clean_swig\n\n"
		 ."\n\n"
		 ##-- c->o dependencies
		 ."moot_wrap.o: swig.stamp moot_wrap.cxx\n\n"
		 ## -- more here
		 ."\n"
		);
  $inherited;
}

#-----------------------------------------------------------------------
sub MY::special_targets {
  package MY;
  my $inherited = shift->SUPER::special_targets(@_);
  $inherited .= (
		 "\n\n"
		 ##-- swig call
		 ."swig.stamp: \$(SWIG_DEPS)\n"
		 ."\tswig -c++ -perl -outdir . -o moot_wrap.cxx \$(MOOT_IFLAGS) swig/moot.i\n"
		 ."\ttouch \$@\n"
		 ."\n"
		 ##-- c->o dependencies
		 ."moot_wrap.o: moot_wrap.cxx\n\n"
		 ## -- more here
		 ."\n"
		);
  $inherited;
}

#-----------------------------------------------------------------------
sub MY::clean {
  package MY;
  my $inherited = shift->SUPER::clean(@_);
  $inherited .= (
		 "\n\n"
		 ##-- swig call
		 ."clean_swig:\n"
		 ."\trm -f swig.stamp moot_wrap.cxx moot.pm\n"
		 ##-- more here
		 ##-- done
		 ."\n"
		);
  $inherited;
}
