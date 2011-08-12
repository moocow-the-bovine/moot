dnl-*- Mode: Autoconf -*-
## ax_run_sizeof.m4 
dnl Author: Bryan Jurish <jurish@uni-potsdam.de>
dnl Description: get size of arbitrary type using AX_RUN_IFELSE
dnl
dnl Usage: AX_RUN_SIZEOF(type, value-if-cross-compiling)
dnl
dnl Effects:
dnl   + sets shell variable SIZEOF_${type}=SIZE_IN_BYTES


AC_DEFUN([AX_RUN_SIZEOF],[

  AC_MSG_CHECKING([sizeof($1)])
  AC_RUN_IFELSE(
    AC_LANG_PROGRAM(
	[
#include <stdio.h>
	],
	[
FILE *f = fopen("conftest.out","w");
fprintf(f,"%d\n",sizeof($1));
fclose(f);
return 0;
	]),
    [dnl ACTION_IF_TRUE
      ax_sizeof_val=`cat conftest.out`
      AC_MSG_RESULT([$ax_sizeof_val])
    ],
    [dnl ACTION_IF_FALSE
      AC_MSG_RESULT([unknown])
      AC_MSG_ERROR([could not determine sizeof($1)])
    ],
    [dnl ACTION_IF_CROSS_COMPILING
      AC_MSG_RESULT([unknown for cross-compiling; using default=$2])
      ax_sizeof_val="$2"
    ])

    AS_TR_SH([SIZEOF_$1])=$[]ax_sizeof_val
])
