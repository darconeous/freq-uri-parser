/*	@file freq-uri.c
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

#include "assert-macros.h"
#include "freq-uri.h"
#include "url-helpers.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#if !HAS_STRNDUP
static char*
x_strndup(const char* str, size_t n) {
	char* ret = NULL;
	int i;

	require_string(ret=calloc(n+1,1),bail,"calloc failure");

	for(i=0;i<n && str[i];i++)
		ret[i]=str[i];

bail:
	return ret;
}
#define strndup	x_strndup
#endif

static double
unitmultiplier(char unit) {
	double fmult = 0;
	switch(tolower(unit)) {
		case 'h': fmult=1.0;  break;
		case 'k': fmult=1000.0;  break;
		case 'm': fmult=1000000.0;  break;
		case 'g': fmult=1000000000.0;  break;
		case 't': fmult=1000000000000.0;  break;
		default: assert_printf("Bad mult char: [%c] (%d)\n",unit,unit); break;
	}
	return fmult;
}

static uint32_t
strtofreq(const char * nptr, char ** endptr, char* multchar) {
	double fvalue=0.0,fmult=1000.0;
	char* ep = NULL;

	if(multchar)
		fmult = unitmultiplier(tolower(*multchar));

	if(endptr)
		ep = *endptr;

	fvalue = strtod(nptr,&ep);

	if(ep && ep[0] && isalpha(ep[0])) {
		double newmult = unitmultiplier(tolower(ep[0]));
		if(newmult) {
			fmult = newmult;
			if(multchar)
				*multchar = tolower(ep[0]);
			ep++;

			// Skip optional 'hz' suffix if present.
			if( ('h'==tolower(ep[0]))
			 && ('z'==tolower(ep[1]))
			)	ep += 2;
		}
	}

	if(endptr)
		*endptr = ep;

	return (uint32_t)(fvalue*fmult+0.5);
}

static void
parse_query_param(freq_t* freq,const char* key,const char* value) {

	assert(freq!=NULL);

	require(value!=NULL,bail);
	require(key!=NULL,bail);

	if(0==strcmp(key,"m")) {
		// Parse Modulation.
		if(0==strcmp(value,"am")) {
			freq->m = FREQ_MOD_AM;
		} else if(0==strcmp(value,"sb")) {
			freq->m = FREQ_MOD_SB;
		} else if(0==strcmp(value,"fm")) {
			freq->m = FREQ_MOD_FM;
		} else if(0==strcmp(value,"cw")) {
			freq->m = FREQ_MOD_CW;
		} else {
			freq->m = FREQ_MOD_UNKNOWN;
		}

	} else if(0==strcmp(key,"bw")) {
		// Parse Bandwidth.
		int i;

		// Check for a split.
		for(i=0;value[i];i++) {
			if(value[i]==':') {
				char multchar = 'k';
				// Zero-terminate at the seperator.
				freq->bw[0] = strtofreq(value,NULL,&multchar);
				freq->bw[1] = strtofreq(value+i+1,NULL,&multchar);
				return;
			}
		}

		// No split.
		freq->bw[0] =
		freq->bw[1] = strtofreq(value,NULL,NULL)/2;

	} else if(0==strcmp(key,"dv")) {
		// Parse Deviation.
		freq->dv = strtofreq(value,NULL,NULL);

	} else if(0==strcmp(key,"tp")) {
		// Parse Transmit Power.
		freq->tp = (uint32_t)(strtof(value,NULL)*1000.0f+0.5f);

	} else if(0==strcmp(key,"ts")) {
		// Parse the CTCSS tone frequency.
		freq->ts = (uint16_t)(strtof(value,NULL)*10.0f+0.5f);

	} else if(0==strcmp(key,"dcs")) {
		// Parse the DCS code.
		freq->dcs = strtol(value,NULL,10);

	} else if(0==strcmp(key,"std")) {
		// Undefined at this point.

	} else {
		// Ignore unknown query keys.

	}
bail:
	return;
}

static void
split_and_decode(char* value, char** value_rx, char** value_tx) {
	int i;

	assert(value_rx!=NULL);
	assert(value_tx!=NULL);

	*value_rx = value;
	*value_tx = value;

	require(value!=NULL,bail);

	for(i=0;value[i];i++) {
		if(value[i]=='/') {
			// Zero-terminate at the seperator.
			value[i++] = 0;

			*value_tx += i;
			url_decode_cstr_inplace(*value_tx);
			break;
		}
	}
	url_decode_cstr_inplace(*value_rx);

bail:
	return;
}

int
freq_parse_uri(
	const char* original_uri,
	size_t len,
	freq_t* freq_rx,
	freq_t* freq_tx
) {
	int ret = 0;
	int status = 0;
	char *uri=NULL;
	char *protocol=NULL;
	char *path=NULL;
	char *query=NULL;
	char multchar = 'k';
	uint32_t freq;
	char* key;
	char* value;

	require_action_string(original_uri,bail,
		ret=FREQ_STATUS_BAD_ARGUMENT,"uri is null");

	if(!len)
		len = strlen(original_uri);

	uri = strndup(original_uri,len);

	require_action_string(uri!=NULL,bail,
		ret=FREQ_STATUS_FAILURE,"strndup failed");

	status = url_parse(
		uri,
		&protocol,
		NULL,
		NULL,
		NULL,
		NULL,
		&path,
		&query
	);

	require_action_string(status>0,bail,
		ret=FREQ_STATUS_BAD_FORMAT,"url parse failure");

	// Make sure this is a freq or x-freq URI.
	if(0!=strcmp(protocol,"x-freq")
	 && 0!=strcmp(protocol,"freq")) {
		ret = FREQ_STATUS_BAD_PROTOCOL;
		goto bail;
	}

	// Parse the frequency.
	freq = strtofreq(path,&path,&multchar);

	require_action_string(freq!=0,bail,
		ret=FREQ_STATUS_BAD_FORMAT,"bad frequency");

	if(freq_rx)
		freq_rx->f = freq;

	if(freq_tx)
		freq_tx->f = freq;

	// Parse the split transmit frequency, if present.
	if(freq_tx) switch(path[0]) {
		case '+':
			path++;
			freq_tx->f += strtofreq(path,&path,&multchar);
			break;
		case '-':
			path++;
			freq_tx->f -= strtofreq(path,&path,&multchar);
			break;
		case '/':
			path++;
			freq_tx->f = strtofreq(path,&path,&multchar);
			break;
		default:
			break;
	}

	// If we don't have a query component, skip to the end.
	if(!query)
		goto bail;

	// Parse the query parameters.
	while(url_form_next_value(&query,&key,&value,false)) {
		char* value_rx = NULL;
		char* value_tx = NULL;

		if(!key)
			break;

		split_and_decode(value,&value_rx,&value_tx);

		if(freq_rx)
			parse_query_param(freq_rx,key,value_rx);

		if(freq_tx)
			parse_query_param(freq_tx,key,value_tx);
	}

bail:
	free(uri);
	return ret;
}

void
freq_dump(FILE* file, const freq_t* freq, const char* lineprefix) {
	if(!lineprefix)
		lineprefix = "";

	fprintf(file,"%sfreq = %u Hz\n",lineprefix,freq->f);
	if(freq->m) {
		fprintf(file,"%smod = ",lineprefix);
		switch(freq->m) {
			case FREQ_MOD_AM:
				fprintf(file,"AM\n");
				break;
			case FREQ_MOD_SB:
				fprintf(file,"SB\n");
				break;
			case FREQ_MOD_FM:
				fprintf(file,"FM\n");
				break;
			case FREQ_MOD_CW:
				fprintf(file,"CW\n");
				break;
			default:
			case FREQ_MOD_UNKNOWN:
				fprintf(file,"unknown\n");
				break;
		}
	}
	if(freq->dv)
		fprintf(file,"%sdeviation = (+/-)%u Hz\n",
			lineprefix,freq->dv);
	if(freq->bw[0] || freq->bw[1]) {
		fprintf(file,"%sbandwidth = ",lineprefix);
		if(freq->bw[0]==freq->bw[1])
			fprintf(file,"%u Hz\n",freq->bw[0]+freq->bw[1]);
		else
			fprintf(file,"%u:%u Hz\n",freq->bw[0],
				freq->bw[1]);
	}
	if(freq->ts)
		fprintf(file,"%sctcss = %.01f Hz\n",lineprefix,freq->ts*0.1);
	if(freq->dcs)
		fprintf(file,"%sdcs = %03u\n",lineprefix,freq->dcs);
	if(freq->tp)
		fprintf(file,"%spower = %u mW\n",lineprefix,freq->tp);
}
