#!/bin/sh

# This seems to require libaio library
# sudo apt-get install libaio1

for src in madbus maddevb maddevc 
do
	cd $src/KERN_SRC
	make
	cd ..
done
# wait

# for src in madsimui madtest madtestb madtestc 
# do
# 	cd $src/Debug
# 	make &
# 	cd ..
# done
# wait

