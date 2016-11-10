#!/bin/sh

##-- buggy source file: from grenzboten sources, 2016-11-08

exec ./waste -v2 -Owd,loc --abbrevs=/home/ddc-dstar/dstar/resources/waste/abbr.lex --stopwords=/home/ddc-dstar/dstar/resources/waste/stop.lex --conjunctions=/home/ddc-dstar/dstar/resources/waste/conj.lex --model=/home/ddc-dstar/dstar/resources/waste/model.hmm data_src_grenzboten_341588_105276.txt.txt -o out.t
