/*	@file url-helpers.c
**	@author Robert Quattlebaum <darco@deepdarc.com>
**
**	Originally published 2010-8-31.
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

#ifdef __SDCC
#include <malloc.h>
#endif

static bool
isurlchar(char src_char) {
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
	return pgm_read_byte_near(
		PSTR("0123456789ABCDEF") + (x & 0xF)
	);
}
#else
static char int_to_hex_digit(uint8_t x) {
	return "0123456789ABCDEF"[x & 0xF];
}
#endif

size_t
url_encode_cstr(
	char *dest, const char*src, size_t max_size
) {
	size_t ret = 0;

	if(!max_size--)
		return 0;

	while(true) {
		const char src_char = *src++;
		if(src_char==0)
			break;

		if(max_size==0) {
			ret++;
			break;
		}

		if(isurlchar(src_char)) {
			*dest++ = src_char;
			ret++;
			max_size--;
#if URL_ENCODE_SPACES_AS_PLUSES
		} else if(src_char == ' ') {
			*dest++ = '+';  // Stupid legacy space encoding.
			ret++;
			max_size--;
#endif
		} else {
			if(max_size < 3) {
				// Dest buffer too small for the next character.
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
	if(c>='0' && c<='9')
		return c-'0';
	if(c>='A' && c<='F')
		return 10+c-'A';
	if(c>='a' && c<='f')
		return 10+c-'a';
	return 0;
}

size_t
url_decode_str(
	char *dest,
	size_t max_size,
	const char* src,		// Length determined by src_len.
	size_t src_len
) {
	size_t ret = 0;

	if(!max_size--)
		return 0;

	while(src_len--) {
		const char src_char = *src++;
		if(!src_char)
			break;

		if(!max_size) {
			ret++;
			break;
		}

		if((src_char == '%')
			&& (src_len>=2)
			&& src[0]
			&& src[1]
		) {
			*dest++ = (hex_digit_to_int(src[0]) << 4) + hex_digit_to_int(
				src[1]);
			src += 2;
			src_len -= 2;
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

size_t
url_decode_cstr(
	char *dest,
	const char*src,
	size_t max_size
) {
	size_t ret = 0;
#if DEBUG
	char*const dest_check = dest;
#endif

	if(!max_size--)
		goto bail;

	while(true) {
		const char src_char = *src++;
		if(!src_char) {
			break;
		}

		if(!max_size) {
			ret++;
			break;
		}

		if((src_char == '%')
			&& src[0]
			&& src[1]
			&& !(src[0] == '0' && src[1] == '0')
		) {
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

bail:
	*dest = 0;
#if DEBUG
	assert(strlen(dest_check)==ret);
#endif
	return ret;
}

void
url_decode_cstr_inplace(char *str) {
	url_decode_cstr(str, str, -1);
}

size_t
quoted_cstr(
	char *dest,
	const char* src,		// Must be zero-terminated.
	size_t dest_max_size
) {
	char* ret = dest;

	require(dest_max_size,bail);
	dest_max_size--;		// For zero termination.

	require(dest_max_size,bail);
	*dest++ = '"';
	dest_max_size--;

	require(dest_max_size,bail);

	while(dest_max_size-1) {
		char src_char = *src++;

		if(!src_char)
			break;

		if((src_char == '/') && src[0] == '"')
			src_char = *src++;
		*dest++ = src_char;
		dest_max_size--;
	}

	*dest++ = '"';
	dest_max_size--;
bail:
	*dest = 0;

	return dest-ret;
}

size_t
url_form_next_value(
	char** form_string, char** key, char** value, bool decodeValue
) {
	size_t bytes_parsed = 0;
	char c = **form_string;
	char* v = NULL;

	if(!c) {
		*key = NULL;
		goto bail;
	}

	*key = *form_string;

	while(true) {
		c = **form_string;

		if(!c)
			goto bail;

		if(c == '=')
			break;

		bytes_parsed++;

		if((c == ';') || (c == '&')) {
			*(*form_string)++ = 0;
			goto bail;
		}

		(*form_string)++;
	}

	// Zero terminate the key.
	if(value)
		**form_string = 0;

	(*form_string)++;

	bytes_parsed++;

	v = *form_string;

	while(true) {
		c = **form_string;

		if(!c || (c == ';') || (c == '&'))
			break;

		bytes_parsed++;
		(*form_string)++;
	}

	// Zero terminate the value
	if(*form_string[0]) {
		*(*form_string)++ = 0;
		bytes_parsed++;
	}

bail:
	if(v && decodeValue)
		url_decode_cstr_inplace(v);
	if(value)
		*value = v;

	return bytes_parsed;
}

size_t
url_path_next_component(
	char** path_string, char** component
) {
	size_t bytes_parsed = 0;
	char c = **path_string;

	if(!c) {
		*component = NULL;
		goto bail;
	}

	*component = *path_string;

	while(true) {
		c = **path_string;

		if(!c || (c == '/'))
			break;

		bytes_parsed++;
		(*path_string)++;
	}

	// Zero terminate the value
	if(*path_string[0]) {
		*(*path_string)++ = 0;
		bytes_parsed++;
	}

bail:
	if(*component)
		url_decode_cstr_inplace(*component);

	return bytes_parsed;
}

int
url_parse(
	char* uri,
	struct url_components_s* components
) {
	int bytes_parsed = 0;
	char tmp;

	if(!url_is_absolute(uri))
		goto skip_absolute_url_stuff;

	check_string(uri, "NULL URI parameter");

	components->protocol = uri;

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

		while( (tmp=*uri) != '/'
			&& ( isurlchar(tmp)
				|| (tmp == '[')
				|| (tmp == ']')
				|| (tmp == ':')
				|| (tmp == '%')		// Necessary for scoped addresses
				|| (tmp == '@')
			)
		) {
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
		    addr_end--
		) {
			if(*addr_end == ']') {
				*addr_end = 0;
				got_port = true;
			} else if(!got_port && (*addr_end == ':')) {
				components->port = addr_end + 1;
				*addr_end = 0;
				got_port = true;
			}
		}
		components->host = addr_end + 1;

		if(*addr_end=='@') {
			*addr_end = 0;
			for(;
				(addr_end >= addr_begin) && (*addr_end != '/');
				addr_end--
			) {
				if(*addr_end==':') {
					*addr_end = 0;
					components->password = addr_end + 1;
				}
			}
			if(*addr_end=='/') {
				components->username = addr_end + 1;
			}
		}
	}

skip_absolute_url_stuff:

	components->path = uri;

	// Move to the end of the path.
	while( ((tmp=*uri) != '#')
		&& (tmp != '?')
		&& ( isurlchar(tmp)
			|| (tmp == '[')
			|| (tmp == ']')
			|| (tmp == ':')
			|| (tmp == '/')
			|| (tmp == '=')
			|| (tmp == '&')
			|| (tmp == '%')
		)
	) {
		uri++;
		bytes_parsed++;
	}

	// Handle query component
	if(*uri == '?') {
		*uri++ = 0;
		bytes_parsed++;

		components->query = uri;

		// Move to the end of the query.
		while( ((tmp=*uri) != '#')
			&& ( isurlchar(tmp)
				|| (tmp == '[')
				|| (tmp == ']')
				|| (tmp == ':')
				|| (tmp == '/')
				|| (tmp == '=')
				|| (tmp == '&')
				|| (tmp == '%')
				|| (tmp == ';')
			)
		) {
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

bool
url_is_absolute(const char* uri) {
	bool ret = false;
	int bytes_parsed = 0;

	require(uri, bail);

	while(*uri && (isalpha(*uri) || *uri=='-') && (*uri != ':')) {
		require(0 != *uri, bail);
		uri++;
		bytes_parsed++;
	}

	if(bytes_parsed && *uri == ':')
		ret = true;

bail:
	return ret;
}

bool
url_is_root(const char* url) {
	bool ret = false;

	require(url, bail);

	if(!isalpha(url[0])) goto bail;

	if(url_is_absolute(url)) {
		while(*url && isalpha(*url) && (*url != ':')) {
			require(*url, bail);
			url++;
		}
		if(	(url[0] != ':')
			|| (url[1] != '/')
			|| (url[2] != '/')
		)	goto bail;

		url+=3;

		while(*url && (*url != '/')) {
			url++;
		}
	}

	while(url[0]=='/' && url[1]=='/') url++;
	ret = (url[0]==0) || ((url[0]=='/') && (url[1]==0));

bail:
	return ret;
}

//!< Used to identify IPv6 address strings
bool
string_contains_colons(const char* str) {
	if(!str)
		return false;
	for(; (*str != ':') && *str; str++) ;
	return *str == ':';
}

