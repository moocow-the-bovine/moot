use ExtUtils::MakeMaker;

##----------------------------------------------------------------------
## Configuration
##----------------------------------------------------------------------
if (system("pkg-config --atleast-version=2.0.8 moot") != 0) {
  die("Required module moot >= v2.0.8 not found!\n");
}

$MOOT_IFLAGS = join(' ', map { $s=`pkg-config --cflags-only-I $_`; chomp($s); $s } qw(moot));
$MOOT_LDFLAGS= join(' ', map { $s=`pkg-config --libs-only-L $_`; chomp($s); $s } qw(moot));
$MOOT_LIBS   = join(' ', map { $s=`pkg-config --libs-only-l $_`; chomp($s); $s } qw(moot));

##----------------------------------------------------------------------
## MAIN
##----------------------------------------------------------------------
WriteMakefile
  (
   NAME		=> 'Moot',
   DISTNAME	=> 'moot-perl',
   AUTHOR       => 'Bryan Jurish <jurish@uni-potsdam.de>',
   ABSTRACT	=> 'Perl interface to the libmoot part-of-speech tagging library',
   VERSION_FROM	=> 'Moot.pm',


   ##-- user variables
   CC           => 'c++',   ##-- your c++ compiler
   LIBS         => "$MOOT_LDFLAGS $MOOT_LIBS",      ##-- additional libraries
   DEFINE       => '-DHAVE_CONFIG_H',     ##-- additional defines
   INC          => $MOOT_IFLAGS,      ##-- additional includes
   CCFLAGS      => '-Wno-deprecated',          ##-- additional flags
   OPTIMIZE    => '-O2 -pipe', ##-- optimization flags
   OBJECT       => 'Moot.o MootPerl.o',

   ##-- install these
   EXE_FILES => [glob("moot*.perl")],

   ##-- DEBUG
   #MAKEAPERL=>1,
   #LINKTYPE=>'static',
   #MAP_TARGET=>'mootperl',
   #CCFLAGS=>'-g',
   ##-- /DEBUG

   ##-- stuff that probably doesn't need to be changed
   XSOPT        => '-C++',
   TYPEMAPS     => ['perlobject.map', 'typemap'],
  );


#-----------------------------------------------------------------------
# Extensions
#-----------------------------------------------------------------------
##...

sub MY::depend {
  package MY;
  my $inherited = shift->SUPER::depend(@_);
  $inherited .= (
		 "\n\n"
		 ##-- included xs dependencies
		 ."Moot.c: "
		 .(''
		   ." Moot.xs"
		   ." Constants.xs"
		   ." Lexfreqs.xs"
		   ." Ngrams.xs"
		   ." HMM.xs"
		  )
		 ."\n\n"
		 ##-- c->o dependencies
		 ."Moot.o: Moot.c MootPerl.h\n"
		 ."\n"
		 ."MootPerl.o: MootPerl.h MootPerl.cc\n"
		 ."\n"
		 ## -- more here
		 ."\n"
		);
  $inherited;
}

