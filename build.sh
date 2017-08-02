#!/bin/bash

if [[ $1 == clean ]] 
then
rm -rf *.o test test_array
exit
fi

gcc -pedantic -std=c89 -c array.c -o array.o
gcc -pedantic -std=c89 -c expr_parser.c -o expr_parser.o			
gcc -pedantic -std=c89 test.c array.o expr_parser.o -o test
gcc -pedantic -std=c89 test_array.c array.o -o test_array
