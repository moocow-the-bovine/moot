#!/bin/sh

cmd="./dwdst-fstgen -d -n test.par -p test.tags -P test.tags -F dskeleton.fsa"
echo $cmd
exec $cmd
