/*
**	assert_macros.h
**	8/31/10
**
**	This file was written by Robert Quattlebaum <darco@deepdarc.com>.
**
**	This work is provided as-is. Unless otherwise provided in writing,
**	Robert Quattlebaum makes no representations or warranties of any
**	kind concerning this work, express, implied, statutory or otherwise,
**	including without limitation warranties of title, merchantability,
**	fitness for a particular purpose, non infringement, or the absence
**	of latent or other defects, accuracy, or the present or absence of
**	errors, whether or not discoverable, all to the greatest extent
**	permissible under applicable law.
**
**	To the extent possible under law, Robert Quattlebaum has waived all
**	copyright and related or neighboring rights to this work. This work
**	is published from the United States.
**
**	I, Robert Quattlebaum, dedicate any and all copyright interest in
**	this work to the public domain. I make this dedication for the
**	benefit of the public at large and to the detriment of my heirs and
**	successors. I intend this dedication to be an overt act of
**	relinquishment in perpetuity of all present and future rights to
**	this code under copyright law. In jurisdictions where this is not
**	possible, I hereby release this code under the Creative Commons
**	Zero (CC0) license.
**
**	 * <http://creativecommons.org/publicdomain/zero/1.0/>
**
**	See <http://www.deepdarc.com/> for other cool stuff.
*/

#ifndef __DARC_ASSERT_MACROS__
#define __DARC_ASSERT_MACROS__

#if __CONTIKI__
#define assert_error_stream     stdout
#else
#define assert_error_stream     stderr
#endif

#if HAS_ASSERTMACROS_H
 #include <AssertMacros.h>
#else
#include <stdio.h>
#include <assert.h>
#if !DEBUG
 #define check_string(c, s)   do { } while(0)
 #define require_action_string(c, l, a, s) \
    do { if(!(c)) { \
			 a; goto l; \
		 } \
	} while(0)
#else
 #if __AVR__
  #define check_string(c, s) \
    do { if(!(c)) fprintf_P(assert_error_stream, \
				PSTR(__FILE__ ":%d: Check Failed (%s)\n"), \
				__LINE__, \
				s); } while(0)
  #define require_action_string(c, l, a, s) \
    do { if(!(c)) { \
			 fprintf_P( \
				assert_error_stream, \
				PSTR(__FILE__ ":%d: Assert Failed (%s)\n"), \
				__LINE__, \
				s); a; goto l; \
		 } \
	} while(0)
 #else
  #define check_string(c, s) \
    do { if(!(c)) fprintf(assert_error_stream, \
				__FILE__ ":%d: Check Failed (%s)\n", \
				__LINE__, \
				s); } while(0)
  #define require_action_string(c, l, a, s) \
    do { if(!(c)) { \
			 fprintf( \
				assert_error_stream, \
				__FILE__ ":%d: Assert Failed (%s)\n", \
				__LINE__, \
				s); a; goto l; \
		 } \
	} while(0)
 #endif
#endif

 #define check(c)   check_string(c, # c)
 #define require_quiet(c, l)   do { if(!(c)) goto l; } while(0)
 #define require(c, l)   require_action_string(c, l, {}, # c)

 #define require_noerr(c, l)   require((c) == 0, l)
 #define require_action(c, l, a)   require_action_string(c, l, a, # c)
 #define require_string(c, l, s) \
    require_action_string(c, l, \
	    do {} while(0), s)
#endif

#endif
