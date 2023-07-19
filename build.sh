#!/bin/bash
set -xe
gcc src/main.c src/jpeg.c -o bin/main -lm
