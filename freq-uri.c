
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
		default: printf("Bad mult char: [%c] (%d)\n",unit,unit); break;
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
		}
	}

	if(endptr)
		*endptr = ep;

	return (uint32_t)(fvalue*fmult+0.5);
}

static void
parse_query_param(freq_t* freq,const char* key,const char* value) {
	if(0==strcmp(key,"m")) {
		// Parse Modulation.
		if(0==strcmp(value,"am")) {
			freq->mod = FREQ_MOD_AM;
		} else if(0==strcmp(value,"sb")) {
			freq->mod = FREQ_MOD_SB;
		} else if(0==strcmp(value,"fm")) {
			freq->mod = FREQ_MOD_FM;
		} else if(0==strcmp(value,"cw")) {
			freq->mod = FREQ_MOD_CW;
		} else {
			freq->mod = FREQ_MOD_UNKNOWN;
		}

	} else if(0==strcmp(key,"bw")) {
		// Parse Bandwidth.
		int i;

		// Check for a split.
		for(i=0;value[i];i++) {
			if(value[i]==':') {
				char multchar = 'k';
				// Zero-terminate at the seperator.
				freq->bandwidth[0] = strtofreq(value,NULL,&multchar);
				freq->bandwidth[1] = strtofreq(value+i+1,NULL,&multchar);
				return;
			}
		}

		// No split.
		freq->bandwidth[0] =
		freq->bandwidth[1] = strtofreq(value,NULL,NULL)/2;

	} else if(0==strcmp(key,"dv")) {
		// Parse Deviation.
		freq->deviation = strtofreq(value,NULL,NULL);

	} else if(0==strcmp(key,"tp")) {
		// Parse Transmit Power.
		freq->power = (uint32_t)(strtof(value,NULL)*1000.0f+0.5f);

	} else if(0==strcmp(key,"ts")) {
		// Parse the CTCSS tone frequency.
		freq->ctcss = (uint16_t)(strtof(value,NULL)*10.0f+0.5f);

	} else if(0==strcmp(key,"dcs")) {
		// Parse the DCS code.
		freq->dcs = strtol(value,NULL,10);

	} else if(0==strcmp(key,"std")) {
		// Undefined at this point.

	} else {
		// Ignore unknown query keys.

	}
}

static void
split_and_decode(char* value, char** value_rx, char** value_tx) {
	int i;

	*value_rx = value;
	*value_tx = value;

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
		freq_rx->freq = freq;

	if(freq_tx)
		freq_tx->freq = freq;

	// Parse the split transmit frequency, if present.
	if(freq_tx) switch(path[0]) {
		case '+':
			path++;
			freq_tx->freq += strtofreq(path,&path,&multchar);
			break;
		case '-':
			path++;
			freq_tx->freq -= strtofreq(path,&path,&multchar);
			break;
		case '/':
			path++;
			freq_tx->freq = strtofreq(path,&path,&multchar);
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

	fprintf(file,"%sfreq = %u Hz\n",lineprefix,freq->freq);
	if(freq->mod) {
		fprintf(file,"%smod = ",lineprefix);
		switch(freq->mod) {
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
	if(freq->deviation)
		fprintf(file,"%sdeviation = (+/-)%u Hz\n",
			lineprefix,freq->deviation);
	if(freq->bandwidth[0] || freq->bandwidth[1]) {
		fprintf(file,"%sbandwidth = ",lineprefix);
		if(freq->bandwidth[0]==freq->bandwidth[1])
			fprintf(file,"%u Hz\n",freq->bandwidth[0]+freq->bandwidth[1]);
		else
			fprintf(file,"%u:%u Hz\n",freq->bandwidth[0],
				freq->bandwidth[1]);
	}
	if(freq->ctcss)
		fprintf(file,"%sctcss = %.01f Hz\n",lineprefix,freq->ctcss*0.1);
	if(freq->dcs)
		fprintf(file,"%sdcs = %03u\n",lineprefix,freq->dcs);
	if(freq->power)
		fprintf(file,"%spower = %u mW\n",lineprefix,freq->power);
}