#!/bin/sh

#AM_MAKEFLAGS=-j2
exec make DISTCHECK_CONFIGURE_FLAGS="--enable-debug --disable-shared CFLAGS=-pipe CXXFLAGS=-pipe DISTCHECK=y" "$@" distcheck
