uname=`uname -n`

case "$uname" in
    "carrot")
       fsm_base=/usr/local
       bumble_base=/usr/local
       stlp_base=/usr/local
       ;;
     "tuvok")
       fsm_base=/usr/local
       bumble_base=/usr/local
       stlp_base=/usr/local
       ;;
     "lore")
       fsm_base=/usr
       bumble_base=/usr
       stlp_base=/usr/local
       ;;
     *)
       fsm_base=/usr/local
       bumble_base=/usr/local
       stlp_base=/usr
       ;;
esac
      
moot_cflags="-DHAVE_CONFIG_H -I. -I/usr/local/include -I${bumble_base}/include/bumble -I${fsm_base}/include/FSM -I${stlp_base}/include/stlport -I${fsm_base}/include/FSMext -Wall -ftemplate-depth-24"

moot_oflags="-O3 -fomit-frame-pointer -funroll-loops -finline-limit-100000"

moot_ldflags="-L/usr/local/lib"

moot_libs='-lFSMext -lFSM -lFSMSymSpec -lstlport_gcc -lpthread -lz -lbumble++'

xflags=''

xlibs=''

xcomp() {
  if [ -z "$base" ] ; then
    echo "varible \$base is not defined!"
  else
    g++ $xflags $moot_cflags $base.cc -o $base -L$PWD/.libs -static $xlibs $moot_libs \
      && echo "$base compiled ok" \
      || echo "$base compilation FAILED"
  fi
}
xdcomp() {
  if [ -z "$base" ] ; then
    echo "varible \$base is not defined!"
  else
    g++ $xflags $moot_cflags $base.cc -o $base -L$PWD/.libs -static -lmoot $xlibs $moot_libs \
      && echo "$base compiled ok" \
      || echo "$base compilation FAILED"
  fi
}

xdlink() {
  if [ -z "$base" ] ; then
    echo "varible \$base is not defined!"
  else
    g++ $xflags -o $base $base.o -L$PWD/.libs -static $xlibs -lmoot $moot_libs \
      && echo "$base linked ok" \
      || echo "$base linking FAILED"
  fi
}
