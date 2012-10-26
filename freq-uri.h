/*	@file freq-uri.h
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

#ifndef __FREQ_URI_H__
#define __FREQ_URI_H__ 1

#include <stdlib.h>
#include <stdint.h>

typedef enum {
	FREQ_MOD_UNDEFINED=0,
	FREQ_MOD_UNKNOWN=1,
	FREQ_MOD_AM=2,
	FREQ_MOD_FM=3,
	FREQ_MOD_SB=4,
	FREQ_MOD_CW=5,
} freq_mod_t;

enum {
	FREQ_STATUS_OK=0,
	FREQ_STATUS_FAILURE=1,
	FREQ_STATUS_BAD_PROTOCOL=2,
	FREQ_STATUS_BAD_FORMAT=3,
	FREQ_STATUS_BAD_ARGUMENT=4,
};

typedef struct {
	uint32_t f;			//!^ Frequency: in Hertz
	freq_mod_t m;		//!^ Modulation
	uint32_t bw[2];		//!^ Bandwidth: For AM and SB only
	uint16_t dv;		//!^ Deviation: For FM only
	uint16_t ts;		//!^ CTCSS Tone: in 1/10ths of a Hertz
	uint16_t dcs;		//!^ DCS Code
	uint32_t tp;		//!^ Transmit Power: in milliwatts
} freq_t;

extern int freq_parse_uri(
	const char* uri,
	size_t len,
	freq_t* rx_freq,
	freq_t* tx_freq
);

extern void freq_dump(
	FILE* file,
	const freq_t* freq,
	const char* lineprefix
);

#endif
