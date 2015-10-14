#!/bin/sh

cd kern/conf
./config $1
cd ../compile/$1
pmake depend
pmake
pmake install
cd ../../../root
sys161 kernel
