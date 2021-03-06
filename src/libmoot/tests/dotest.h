#ifndef DOTEST_H
#define DOTEST_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

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


size_t dotest_rss = 0;
size_t memrss(void)
{
  pid_t mypid = getpid();
  char statfilename[256];
  sprintf(statfilename, "/proc/%d/status", mypid);
  FILE *statfile = fopen(statfilename, "r");
  if (!statfile) {
    fprintf(stderr, "memrss(): could not open %s\n", statfilename);
    return 0;
  }

  //-- parse
  const char   *FieldName    = "VmRSS:";
  const size_t  FieldNameLen = strlen(FieldName);
  char    *line = NULL;
  size_t  linesize = 0;
  int   linelen = 0;
  size_t rss = 0;
  while (!feof(statfile) && (linelen = getline(&line,&linesize,statfile)) != -1) {
    if (strstr(line,FieldName)==line) {
      sscanf(line+FieldNameLen, "%u", &rss);
      break;
    }
  }

  //-- cleanup
  if (line) free(line);
  fclose(statfile);
  dotest_rss = rss;
  return rss;
}

void showmem(const char *label="none") {
  size_t lastrss = dotest_rss;
  size_t newrss  = memrss();
  fprintf(stderr, "-> rss(label=\"%s\")=%u [last=%u ; diff=%u]\n",
	  label, newrss, lastrss, newrss-lastrss);
}

#endif // DOTEST_H
