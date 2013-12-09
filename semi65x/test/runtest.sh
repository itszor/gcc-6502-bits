#!/bin/sh
set -e
pasta test.s -o test
../semi65x -l 0xe00 test
