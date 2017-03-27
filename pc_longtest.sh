#!/bin/bash

mkdir -p tmp
for i in `seq 1 1000`; do
dd if=/dev/urandom bs=1 count=8 of=tmp/testfile.dat 2>/dev/null
dd if=/dev/urandom bs=1 count=8 of=tmp/keyfile.key 2>/dev/null
 
k=`hexdump -C tmp/keyfile.key |cut -c 10-50|tr -d ' '|head -n 1`
openssl enc -nosalt -des-ecb -in tmp/testfile.dat -out tmp/testfile.ossl.enc -K $k -p
./des_test
if [ $? -ne 0 ];then
	echo "FAIL"
	exit 1
fi

done
