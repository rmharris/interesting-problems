#!/bin/sh

# Copyright (c) 2018, Robert Harris.

TMPFILE=`mktemp`
OUTCOME=0

for RESULT in *.success
do
	TEST=`basename $RESULT .success`
	../wc++ $TEST > $TMPFILE 2>&1
	diff $TMPFILE $RESULT > /dev/null 2>&1
	if [ $? -ne 0 ]
	then
		echo "Unexpected failure in test $TEST"
		cp $TMPFILE $TEST.result
		OUTCOME=1
	fi
done

for RESULT in *.failure
do
	TEST=`basename $RESULT .failure`
	../wc++ $TEST > $TMPFILE 2>&1
	if [ $? -eq 0 ]
	then
		echo "Unexpected success in test $TEST"
		OUTCOME=1
	fi
done

rm $TMPFILE

[ $OUTCOME -eq 0 ] && echo "all tests passed" || echo "failures encountered"
