#!/bin/sh
gcc control.c -o control
gcc -pthread test1.c -o test1 -lpthread -lrt
