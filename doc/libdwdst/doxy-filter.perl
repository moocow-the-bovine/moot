#!/usr/local/bin/perl -w

#----------------------------------------------------------------------
# File: doxy-filter.perl
# Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
# Description: doxygen filter for use with the GNU C preprocessor
#
# Usage: $0 INPUT_FILENAME
#-----------------------------------------------------------------------

use Getopt::Long;
use File::Basename qw(basename fileparse);
use vars qw($filepath $filesuff $pppath);

$PP_ONLY = 0;
$DO_CPP = 1;
$logfile = undef;
$cmdline = join(' ', $0, @ARGV);
GetOptions(
	   "only-preprocess|o!"=>\$PP_ONLY,
	   "preprocess|do-cpp|p!"=>\$DO_CPP,
	   "no-preprocess|no-cpp|P"=>sub { $DO_CPP = 0; },
	   "logfile|l=s"=>\$logfile,
	  );

#-----------------------------------------------------------------------
# Globals
#-----------------------------------------------------------------------
$CPP = $ENV{CPP} || 'cpp';
$CPPFLAGS = ('-C '         ## -- preserve comments
	     .'-x c++'     ## -- parse c++ code
	     .($ENV{CPPFLAGS} || '')
	    );

$ppextra = '(?s:\bbison\.h\b)|(?s:\bflexskel\.h\b)';
%ppextra =
  ('(?:Lexer)' => '(?s:\bflexskel\.h\b)',
   '(?:Parser)' => '(?s:\bbison\.h\b)',
  );
@suffixes = qw(\.c \.h \.cc \.cpp \.cxx \.l \.y \.ll \.yy);

#-----------------------------------------------------------------------
# DEBUG
#-----------------------------------------------------------------------
#$DEBUG = 2;
$DEBUG = 1;
sub logopen {
  $logfile = 'doxy-filter.log' if (!defined($logfile));
  if ($logfile ne '-') {
    open(LOG,">>$logfile") or die("$0: open failed for log-file '$logfile': $!");
    open(STDERR,">&LOG") or die("$0: could not redirect STDERR to LOG: $!");
  } else {
    open(LOG, ">&STDERR") or die ("$0: could not redirect LOG to STDERR: $!");
  }
  print LOG
    ("\n", ("-" x 72), "\n", `date`,
     "> ARGV=$cmdline\n",
     "> PWD=", `pwd`,
     "> PP_ONLY=", $PP_ONLY ? 1 : 0, "\n",
     "> DO_CPP=", $DO_CPP ? 1 : 0, "\n",
    );
}

#-----------------------------------------------------------------------
# MAIN
#-----------------------------------------------------------------------
push(@ARGV,'-') if (!@ARGV);
logopen() if ($DEBUG>0);
foreach $file (@ARGV) {
  print LOG ("> CPPCMD=$CPP $CPPFLAGS $file |\n") if ($DEBUG>0);

  if ($DO_CPP) {
    open(CPP,"$CPP $CPPFLAGS $file|")
      or die("$0: could not open pipe from '$CPP $CPPFLAGS $file': $!");
  } else {
    open(CPP, "${file}.pp")
      or die("$0: could not open '${file}.pp': $!");
  }


  ($filebase,$filepath,$filesuff) = fileparse($file, @suffixes);
  $ppfile = undef;
  while (defined($line = <CPP>)) {
    if (!$PP_ONLY) {
      if ($line =~ /^\#\s*\d+\s*\"([^\"]*)\"/) {
	## -- data from a different file
	$ppfile = $1;
	($ppbase,$pppath,$ppsuff) = fileparse($ppfile, @suffixes);

	if ($DEBUG>2) {
	  print LOG ("> filebase='$filebase', ppbase='$ppbase', ppsuff='$ppsuff'\n");
	}
	
	if ($ppfile eq $file) {
	  $wantline = 1;
	  print LOG ("> ACCEPT: literal filename match: $line") if ($DEBUG>1);
	}
	elsif (grep { $file =~ $_ && $ppfile =~ $ppextra{$_} } keys(%ppextra)) {
	  $wantline = 1;
	  print LOG ("> ACCEPT: filename convention match: $line\n") if ($DEBUG>1);
	}
	elsif (grep { $ppbase eq $filebase && $ppsuff =~ /$_$/ } @suffixes) {
	  $wantline = 1;
	  print LOG ("> ACCEPT: suffix match: $line\n") if ($DEBUG>1);
	}
	else {
	  $wantline = 0;
	  print LOG ("> REJECT: $line") if ($DEBUG>1);
	}

	next;
      }
      next if (!defined($ppfile) || !$wantline);
    }
    print $line;
  }

  close(CPP);
}

close(LOG) if ($DEBUG>0);