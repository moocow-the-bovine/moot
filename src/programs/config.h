/*
   moot-utils version 1.0.4 : moocow's part-of-speech tagger
   Copyright (C) 2002-2003 by Bryan Jurish <moocow@cpan.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

/* just include the config.h for libmoot */
#include <mootConfig.h>

/* PACKAGE does not jive well with multiple-program pacakges
   using 'gengetopt'.  'autoheader' doesn't like to undefine
   it, so we include this extra little header file to do that
*/
/*
#ifdef PACKAGE
# undef PACKAGE
#endif
*/

//#endif // _MOOT_CONFIG_H

