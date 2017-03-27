#!/bin/bash

# generate test_vector_long.data file for DES testing in arduino. 

mkdir -p tmp

cp /dev/null tmp/test_vector_long.data

for i in `seq 1 1000`; do
dd if=/dev/urandom bs=1 count=8 of=tmp/testfile.dat 2>/dev/null
dd if=/dev/urandom bs=1 count=8 of=tmp/keyfile.key 2>/dev/null
 
k=`hexdump -C tmp/keyfile.key |cut -c 10-50|tr -d ' '|head -n 1`
openssl enc -nosalt -des-ecb -in tmp/testfile.dat -out tmp/testfile.ossl.enc -K $k -p

cat tmp/testfile.dat >> tmp/test_vector_long.data
cat tmp/keyfile.key >> tmp/test_vector_long.data
echo -n -e "\001" >> tmp/test_vector_long.data
dd if=tmp/testfile.ossl.enc bs=8 count=1 >> tmp/test_vector_long.data 2>/dev/null

done
