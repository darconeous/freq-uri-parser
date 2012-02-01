#!/bin/bash

which abnfgen 1>/dev/null || { echo Missing abnfgen. ; exit 1 ; }

for ((i=0;i<100000;i++))
do
	URI=`abnfgen -c freq-uri-abnf.txt`
	echo Testing '<'$URI'>'
	./freq-parser $URI > /dev/null || exit -1

	URI=`abnfgen -c freq-uri-query-abnf.txt`
	echo Testing '<'$URI'>'
	./freq-parser $URI > /dev/null || exit -1
done
