#!/bin/bash

for ((i=0; i<10; i++))
do
    ./hash_table.exe result_test/res_test.txt < test/test.txt
done