#!/bin/sh

TMPFILE=`mktemp`
OUTCOME=0

for INPUT in *.in
do
	ID=`basename $INPUT .in`
	for DIRECTION in left right
	do
		rm -f $TMPFILE
		../rotate $DIRECTION $INPUT $TMPFILE 2>&1
		diff $TMPFILE $ID.$DIRECTION.expected > /dev/null 2>&1
		if [ $? -ne 0 ]
		then
			echo "Unexpected failure in test $ID/$DIRECTION"
			cp $TMPFILE $ID.$DIRECTION.actual
			OUTCOME=1
		fi
	done
done

rm $TMPFILE

[ $OUTCOME -eq 0 ] && echo "all tests passed" || echo "failures encountered"
