#!/bin/sh

for file in madbus mad_cdev madtest
do
	cd $file
	make &
	cd ..
done
wait
