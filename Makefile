

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
