#ifndef DOTEST_H
#define DOTEST_H

#include <sys/time.h>
#include <stdio.h>

timeval tv_started;
timeval tv_stopped;

double tv_elapsed(const timeval &started=tv_started,
		  const timeval &stopped=tv_stopped)
{
  return
    stopped.tv_sec-started.tv_sec
    + (double)(stopped.tv_usec-started.tv_usec)/1000000.0;
};

#define DOTEST_REPORT dotest_report
void dotest_report(const char *label,
		   size_t niters,
		   const timeval &started=tv_started,
		   const timeval &stopped=tv_stopped)
{
  double elapsed = tv_elapsed(started,stopped);
  fprintf(stderr, "%-36s: %u iters / %.2g sec = %.2g iters/sec\n",
	  label, niters, elapsed, ((double)niters)/elapsed);
}

/* DOTEST_START : initialize 'tv_started' */
#define DOTEST_START gettimeofday(&tv_started,NULL);
#define DOTEST_STOP gettimeofday(&tv_stopped,NULL);

/* DOTEST_CODE(NITERS, CODE) */
#define DOTEST_CODE(NITERS, CODE) \
  gettimeofday(&tv_started,NULL); \
  for (size_t i = 0; i < NITERS; i++) { CODE ; } \
  gettimeofday(&tv_stopped,NULL); \
  dotest_report(#CODE,NITERS);

void memprompt(const char *label="none") {
  fprintf(stderr, "Check memory (label=\"%s\") ? ", label);
  fgetc(stdin);
}

#endif // DOTEST_H
