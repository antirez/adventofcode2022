#!/bin/sh
cc prog.c -Wall -W --pedantic
./a.out
rm -f a.out
