/*	@file main.c
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

#include <stdio.h>
#include "freq-uri.h"

void
parse_and_print(const char* uri) {
	freq_t rx_freq = {};
	freq_t tx_freq = {};

	freq_parse_uri(uri,0,&rx_freq,&tx_freq);

	fprintf(stdout,"%s\n",uri);

	fprintf(stdout,"  rx_freq:\n");
	freq_dump(stdout,&rx_freq,"    ");

	fprintf(stdout,"  tx_freq:\n");
	freq_dump(stdout,&tx_freq,"    ");
}

int main(int argc, char* argv[]) {

	if(argc>1) {
		int i;
		for(i=1;i<argc;i++) {
			parse_and_print(argv[i]);
			printf("\n");
		}
	}
	else
	{
		printf("Syntax:\n");
		printf("\t%s [x-freq-uri] ...\n",argv[0]);
	}
	return 0;
}

