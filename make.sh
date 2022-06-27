#!/bin/sh

git pull 
for file in madbus mad_cdev madtest
do
	cd $file
	make -j4 &
	cd ..
done
wait
