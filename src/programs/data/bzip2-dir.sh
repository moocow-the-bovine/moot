#!/bin/sh

# compress all files in a directory with size > $minsize

minsize=1024

if [ $# -lt 1 ] ; then
  echo "Usage: $0 DIR(s)..."
  exit 1
fi

case "$0" in
  *bzip2*)
    ZIPPROG=bzip2
    SUFFIX=bz2
    ;;
  *gzip*)
    ZIPPROG=gzip
    SUFFIX=gz
    ;;
  *)
  ZIPPROG=gzip
    SUFFIX=gz
    ;;
esac

squeeze_file() {
  if [ -s "$1" ] ; then
    fsize=`stat -c "%s" $1`
    if [ -n "$fsize" -a $fsize -gt $minsize ] ; then
       echo "$1"
       $ZIPPROG "$1"
    fi 
  fi
}

squeeze_dir() {
  for f in `find "$1" -type f` ; do
    test -s "$f" && squeeze_file "$f"
  done
}

for d in $* ; do
  if [ -d "$d" ] ; then
    squeeze_dir "$d"
  else
    squeeze_file "$d"
  fi
done
