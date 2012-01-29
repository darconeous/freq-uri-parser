/*	@file url-helpers.c
**	@author Robert Quattlebaum <darco@deepdarc.com>
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

#include <ctype.h>
#include <string.h>
#include "assert-macros.h"
#include "url-helpers.h"

static bool
isurlchar(int src_char) {
	return isalnum(src_char)
	       || (src_char == '$')
	       || (src_char == '-')
	       || (src_char == '_')
	       || (src_char == '.')
	       || (src_char == '+')
	       || (src_char == '!')
	       || (src_char == '*')
	       || (src_char == '\'')
	       || (src_char == '(')
	       || (src_char == ')')
	       || (src_char == ',');
}

#if __AVR__
#include <avr/pgmspace.h>
static char int_to_hex_digit(uint8_t x) {
	return pgm_read_byte_near(PSTR(
			"0123456789ABCDEF") + (x & 0xF));
}
#else
static char int_to_hex_digit(uint8_t x) {
	return "0123456789ABCDEF"[x &
	    0xF];
}
#endif

size_t
url_encode_cstr(
	char *dest, const char*src, size_t max_size
) {
	size_t ret = 0;

	max_size--;

	while(true) {
		const char src_char = *src++;
		if(!src_char)
			break;

		if(!max_size) {
			ret++;
			break;
		}

		if(isurlchar(src_char)) {
			*dest++ = src_char;
			ret++;
			max_size--;
		} else if(src_char == ' ') {
			*dest++ = '+';  // Stupid legacy space encoding.
		} else {
			if(max_size < 3) {
				// Too small for the next character.
				ret++;
				break;
			}

			*dest++ = '%';
			*dest++ = int_to_hex_digit(src_char >> 4);
			*dest++ = int_to_hex_digit(src_char & 0xF);
			ret += 3;
			max_size -= 3;
		}
	}

	*dest = 0;

	return ret;
}

static char
hex_digit_to_int(char c) {
	switch(c) {
	case '0': return 0; break;
	case '1': return 1; break;
	case '2': return 2; break;
	case '3': return 3; break;
	case '4': return 4; break;
	case '5': return 5; break;
	case '6': return 6; break;
	case '7': return 7; break;
	case '8': return 8; break;
	case '9': return 9; break;
	case 'A':
	case 'a': return 10; break;
	case 'B':
	case 'b': return 11; break;
	case 'C':
	case 'c': return 12; break;
	case 'D':
	case 'd': return 13; break;
	case 'E':
	case 'e': return 14; break;
	case 'F':
	case 'f': return 15; break;
	}
	return 0;
}

size_t
url_decode_cstr(
	char *dest, const char*src, size_t max_size
) {
	size_t ret = 0;

	max_size--;

	while(true) {
		const char src_char = *src++;
		if(!src_char)
			break;

		if(!max_size) {
			ret++;
			break;
		}

		if((src_char == '%') && src[0] && src[1]) {
			*dest++ = (hex_digit_to_int(src[0]) << 4)
				+ hex_digit_to_int(src[1]);
			src += 2;
		} else if(src_char == '+') {
			*dest++ = ' ';  // Stupid legacy space encoding.
		} else {
			*dest++ = src_char;
		}

		ret++;
		max_size--;
	}

	*dest = 0;

	return ret;
}

void
url_decode_cstr_inplace(char *str) {
	url_decode_cstr(str, str, 0);
}

size_t
url_form_next_value(
	char** form_string, char** key, char** value, bool decodeValue
) {
	size_t bytes_parsed = 0;
	char c = **form_string;

	if(!c) {
		*key = NULL;
		*value = NULL;
		goto bail;
	}

	*key = *form_string;

	while(true) {
		c = **form_string;

		if(!c) {
			*value = NULL;
			goto bail;
		}

		if(c == '=')
			break;

		bytes_parsed++;

		if((c == ';') || (c == '&')) {
			*(*form_string)++ = 0;
			    (*form_string)++;
			*value = NULL;
			goto bail;
		}

		    (*form_string)++;
	}

	// Zero terminate the key.
	*(*form_string)++ = 0;
	bytes_parsed++;

	*value = *form_string;

	while(true) {
		c = **form_string;

		if(!c || (c == ';') || (c == '&'))
			break;

		bytes_parsed++;
		    (*form_string)++;
	}

	// Zero terminate the value
	*(*form_string)++ = 0;
	bytes_parsed++;

bail:
	if(*value && decodeValue)
		url_decode_cstr_inplace(*value);

	return bytes_parsed;
}


int
url_parse(
	char*	uri,
	char**	protocol,
	char**	username,
	char**	password,
	char**	host,
	char**	port,
	char**	path,
	char**	query
) {
	int bytes_parsed = 0;

	require_string(uri, bail, "NULL uri parameter");

	if(protocol)
		*protocol = uri;

	while(*uri != ':') {
		require_string(*uri, bail, "unexpected end of string");
		uri++;
		bytes_parsed++;
	}

	// Zero terminate the protocol;
	*uri++ = 0;
	bytes_parsed++;

	require_string(*uri, bail, "unexpected end of string");

	if(uri[0] == '/' && uri[1] == '/') {
		char* addr_end;
		char* addr_begin;
		bool got_port = false;

		// Contains a hostname. Parse it.
		uri += 2;
		bytes_parsed += 2;

		addr_begin = uri;

		while((isurlchar(*uri) || (*uri == '[') || (*uri == ']') ||
		            (*uri == ':') || (*uri == '@')) && (*uri != '/')) {
			uri++;
			bytes_parsed++;
		}

		addr_end = uri - 1;

		if(*uri) {
			*uri++ = 0;
			bytes_parsed++;
		}

		for(;
		        (addr_end >= addr_begin) && (*addr_end != '@') &&
		        (*addr_end != '[');
		    addr_end--) {
			if((*addr_end == ']')) {
				*addr_end = 0;
				got_port = true;
			} else if(!got_port && (*addr_end == ':')) {
				if(port)
					*port = addr_end + 1;
				*addr_end = 0;
				got_port = true;
			}
		}
		if(host)
			*host = addr_end + 1;
	}

	if(path)
		*path = uri;

	// Move to the end of the path.
	while((isurlchar(*uri) || (*uri == '/') || (*uri == '[') ||
	            (*uri == ']') || (*uri == ':') || (*uri == '=') ||
	            (*uri == '&') ||
	            (*uri == '%')) && (*uri != '#') && (*uri != '?')) {
		uri++;
		bytes_parsed++;
	}

	// Handle query component
	if(*uri == '?') {
		*uri++ = 0;
		bytes_parsed++;

		if(query)
			*query = uri;

		// Move to the end of the query.
		while((isurlchar(*uri) || (*uri == '[') || (*uri == ']') ||
		            (*uri == ':') || (*uri == '/') || (*uri == '=') ||
		            (*uri == '&') ||
		            (*uri == ';') || (*uri == '%')) && (*uri != '#')) {
			uri++;
			bytes_parsed++;
		}
	}

	// Zero terminate
	if(*uri) {
		*uri = 0;
		bytes_parsed++;
	}

bail:
	return bytes_parsed;
}

