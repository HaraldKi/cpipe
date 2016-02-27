#ifndef __cmdline__
#define __cmdline__
/*****
  command line parser -- originally auto-generated, now frozen
*****/

typedef struct s_Cmdline {
  /***** -b: buffer size in kB */
  char bsizeP;
  int bsize;
  int bsizeC;
  /***** -vt: show throughput */
  char vtP;
  /***** -vr: show read-times */
  char vrP;
  /***** -vw: show write-times */
  char vwP;
  /***** -ngr: non-greedy read. Don't enforce a full buffer
on read before starting to write */
  char ngrP;
  /***** -s: throughput speed limit in kB/s */
  char speedP;
  double speed;
  int speedC;
  /***** uninterpreted command line parameters */
  int argc;
  /*@null*/char **argv;
} Cmdline;


extern char *Program;
extern void usage(void);
extern /*@shared*/Cmdline *parseCmdline(int argc, char **argv);

#endif

