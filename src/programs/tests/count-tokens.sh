#!/bin/sh

cat $* | grep -v '^%%' | grep . | wc -l

