uname=`uname -n`

case "$uname" in
    "carrot")
       fsm_base=/usr/local
       bumble_base=/usr
       stlp_base=/usr/local
       ;;
     "tuvok")
       fsm_base=/usr/local
       bumble_base=/usr/local
       stlp_base=/usr/local
       ;;
     *)
       fsm_base=/usr/local
       bumble_base=/usr/local
       stlp_base=/usr
       ;;
esac
      
dwdst_cflags="-DHAVE_CONFIG_H -I. -I/usr/local/include -I${bumble_base}/include/bumble -I${fsm_base}/include/FSM -I${stlp_base}/include/stlport -I${fsm_base}/include/FSMext -Wall -ftemplate-depth-24"

dwdst_oflags="-O3 -fomit-frame-pointer -funroll-loops -finline-limit-100000"

dwdst_ldflags="-L/usr/local/lib"

dwdst_libs='-lFSMext -lFSM -lFSMSymSpec -lstlport_gcc -lpthread -lz -lbumble++'
