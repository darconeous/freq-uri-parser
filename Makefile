# 
# This code was written by Robert Quattlebaum <darco@deepdarc.com>.
# 
# This work is provided as-is. Unless otherwise provided in writing,
# Robert Quattlebaum makes no representations or warranties of any
# kind concerning this work, express, implied, statutory or otherwise,
# including without limitation warranties of title, merchantability,
# fitness for a particular purpose, non infringement, or the absence
# of latent or other defects, accuracy, or the present or absence of
# errors, whether or not discoverable, all to the greatest extent
# permissible under applicable law.
# 
# To the extent possible under law, Robert Quattlebaum has waived all
# copyright and related or neighboring rights to this work. This work
# is published from the United States.
# 
# I, Robert Quattlebaum, dedicate any and all copyright interest in
# this work to the public domain. I make this dedication for the
# benefit of the public at large and to the detriment of my heirs and
# successors. I intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to
# this code under copyright law. In jurisdictions where this is not
# possible, I hereby release this code under the Creative Commons
# Zero (CC0) license.
# 
#  * <http://creativecommons.org/publicdomain/zero/1.0/>
#

freq-parser: main.o freq-uri.o url-helpers.o
	$(CC) -o $@ $^

clean:
	$(RM) main.o freq-uri.o url-helpers.o freq-parser

test: freq-parser
	@./freq-parser 'x-freq:145.23m-.6?m=fm;dv=5;ts=100'
	@./freq-parser 'x-freq:145.922m/435.758?bw=:16/16:'
	@./freq-parser 'x-freq:107.9m'
	@./freq-parser 'x-freq:61.25m?m=am;bw=1.5m:4.5'
	@./freq-parser 'x-freq:2.6?m=cw'
	@./freq-parser 'x-freq:441.3m+5?m=fm;dv=2.5;ts=100/123.0'

main.o: main.c freq-uri.h
url-helpers.o: url-helpers.c url-helpers.h assert-macros.h
freq-uri.o: freq-uri.c freq-uri.h assert-macros.h
