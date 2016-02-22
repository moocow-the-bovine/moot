##-*- Mode: CPerl -*-
use strict;
use warnings;

use Alien::Base::ModuleBuild;

my $builder = Alien::Base::ModuleBuild->new
  (
   module_name => 'Alien::Moot',
   license => 'perl',
   configure_requires => {
			  'Alien::Base'   => '0.001',
			  'Module::Build' => '0.38',
			 },
   test_requires => {
		     #'File::Which'   => 0,
		    },
   requires => {
		'perl'        => '5.8.9',
		'Alien::Base' => '0.005',
	       },
   dist_author => 'Bryan Jurish <moocow@cpan.org>',
   alien_name => 'moot',
   alien_repository => {
			protocol => 'http',
			host     => 'kaskade.dwds.de',
			location => '~moocow/mirror/projects/moot/',
			pattern  => qr/^moot-([\d\.\-]+)\.tar\.gz$/,
		       },
   alien_build_commands => [
			    '%c --prefix=%s --disable-doc --enable-shared --enable-static',
			    'make -j4',
			   ],
   ##
   alien_isolate_dynamic => 0, ##-- 'share'-mode builds don't work for Moot module with alien_isolate_dynamic=>1
   ##
   #sign => 1,
   #meta_merge => { ... },
   ##
   create_readme => 1,
  );
$builder->create_build_script;

