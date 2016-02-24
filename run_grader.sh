#!/bin/bash

number_of_graders=$1

if [ -z "$number_of_graders" ];
	then php index.php grader run "1-single"
else
	for i in `seq 1 $number_of_graders`;
	do
		php index.php grader run $i &
		sleep .1s
	done
fi

