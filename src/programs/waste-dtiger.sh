#!/bin/bash

waste="`dirname $0`/waste"
rcdir="`dirname $0`/dtiger+qq/common"
args=(-v2 -y -a"$rcdir/dta_abbrevs_flat.lex" -j"$rcdir/dta_conjunctions.lex" -w"$rcdir/dta_stopwords.lex")
exec "$waste" "${args[@]}" "$@"
