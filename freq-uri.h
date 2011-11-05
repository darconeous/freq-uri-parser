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
	uint32_t freq;			//!^ in Hertz
	freq_mod_t mod;
	uint32_t bandwidth[2];	//!^ For AM and SB only
	uint16_t deviation;		//!^ For FM only
	uint16_t ctcss;			//!^ in 1/10ths of a Hertz
	uint16_t dcs;
	uint32_t power;			//!^ in milliwatts
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
