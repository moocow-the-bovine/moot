/* PACKAGE does not jive well with multiple-program pacakges
   using 'gengetopt'.  'autoheader' doesn't like to undefine
   it, so we include an extra little header file to do that */
#ifdef PACKAGE
# undef PACKAGE
#endif
