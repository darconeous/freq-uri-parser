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

