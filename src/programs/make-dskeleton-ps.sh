#!/bin/sh

base=dskeleton

do_cmd() {
    echo "$cmd"
    eval $cmd
}

cmd="FSMprint $base.fsa > $base.tfsa"
do_cmd
cmd="fsmcompile $base.tfsa -F $base.afsa"
do_cmd
cmd="fsmdraw -i $base.lab $base.afsa > $base.dot"
do_cmd
cmd="dot -Tps $base.dot > $base.ps"
do_cmd

