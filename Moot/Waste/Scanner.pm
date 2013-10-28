package Moot::Waste::Scanner;
#use Moot::Waste::Scanner;
#use Moot::Waste::Lexer;
#use Moot::Waste::Recycler;
use strict;


1; ##-- be happy

__END__

=pod

=head1 NAME

Moot::Waste:Scanner - libmoot : WASTE tokenizer : low-level scanner

=head1 SYNOPSIS

  use Moot::Waste;

  ##=====================================================================
  ## Constructors etc

  $scanner = Moot::Waste::Scanner->new();  ##-- create a new scanner
  $scanner->reset();                       ##-- reset scanner data

  ##=====================================================================
  ## Input Selection

  $scanner->close();			  ##-- close current input source
  $bool = $scanner->opened();             ##-- true iff opened

  $scanner->from_file($filename);         ##-- input from named file
  $scanner->from_fh($fh);                 ##-- input from filehandle
  $scanner->from_string($buffer);         ##-- input from string buffer

  ##=====================================================================
  ## Token-Level Access

  $token = $scanner->get_token();         ##-- get next token

  ##=====================================================================
  ## Diagnostics

  $name = $scanner->name();               ##-- get/set scanner name
  $name = $scanner->name($name);

  $n = $scanner->line_number();           ##-- get/set line number
  $n = $scanner->line_number($n);

  $n = $scanner->column_number();         ##-- get/set column number (BUGGY)
  $n = $scanner->column_number($n);

  $n = $scanner->byte_number();           ##-- get/set byte offset
  $n = $scanner->byte_number($n);

=head1 DESCRIPTION

The Moot::Waste module provides an object-oriented interface to the WASTE tokenization
system included in the libmoot library for Hidden Markov Model decoding. Really just a wrapper
for

 use Moot::Waste::Scanner;
 use Moot::Waste::Lexer;

=head1 SEE ALSO

Moot(3perl),
Moot::Waste::Scanner(3perl),
Moot::Waste::Lexer(3perl),
moot(1),
perl(1).

=head1 AUTHOR

Bryan Jurish E<lt>moocow@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2013 by Bryan Jurish

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.10.1 or,
at your option, any later version of Perl 5 you may have available.

=cut

