#!/usr/bin/perl -w

open(RC,"|recode latin1..utf8")
  or die("$0: could not open pipe to recode: $!");

while (<>) {
  #s/&bullet;/·/g;
  #s/&sqmet;/m^2/g;
  #s/&/&amp;/g;
  #s/</&lt;/g;
  #s/>/&gt;/g;
  #s/\"/&quot;/g;

  s/&bullet;/*bullet;/g;
  s/&sqmet;/*sqmet;/g;

  print RC $_;
}
