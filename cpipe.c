/**********************************************************************
  cpipe -- counting pipe

  (C) 1997 Harald Kirsch (kir@iitb.fhg.de)
  $Revision$, $Date$
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>


#include "cmdline.h"

size_t TotalBytes;
double totalTin, totalTout;

double kB = 1.0/(double)1024;

/**********************************************************************/
double
deltaT(struct timeval* tin, struct timeval* tout)
{
  long usec;

  usec = tout->tv_usec - tin->tv_usec;
  if( usec<0 ) {
    usec = (usec + 1000000) +1000000*(tout->tv_sec-tin->tv_sec-1);
  } else {
    usec += 1000000*(tout->tv_sec-tin->tv_sec);
  }
  return (double)usec * 1e-6;
}
/**********************************************************************/
ssize_t
readBuffer(char *buf, size_t length, int show)
{
  size_t totalBytes;
  ssize_t bytes;
  struct timeval tin, tout;
  double dt;

  gettimeofday(&tin, NULL);
  for(totalBytes=0; totalBytes<length; totalBytes+=bytes, buf+=bytes) {
    bytes = read(STDIN_FILENO, buf, length-totalBytes);
    if( 0==bytes ) break;
    if( -1==bytes ) {
      if( errno!=EINTR && errno!=EAGAIN ) {
	fprintf(stderr, "%s: error reading stdin because `%s'\n",
		Program, strerror(errno));
	exit(EXIT_FAILURE);
      } else {
	bytes = 0;
      }
    }
  }
  gettimeofday(&tout, NULL);
  dt =  deltaT(&tin, &tout);
  totalTin += dt;
  TotalBytes += totalBytes;
  if( show ) {
    fprintf(stderr, "  in: %7.3fms at %6.0fkB/s (%6.0fkB/s avg)\n", 
	    1e3*dt, 
	    (double)totalBytes/dt*kB, 
	    (double)TotalBytes/totalTin*kB);
  }
  return totalBytes;
}
/**********************************************************************/
void
writeBuffer(char *buf, size_t length, int show)
{
  size_t totalBytes;
  ssize_t bytes;
  struct timeval tin, tout;
  double dt;
  gettimeofday(&tin, NULL);
  for(totalBytes=0; totalBytes<length; totalBytes+=bytes, buf+=bytes) {
    bytes = write(STDOUT_FILENO, buf, length-totalBytes);
    if( -1==bytes ) {
      if( errno!=EINTR && errno!=EAGAIN ) {
	fprintf(stderr, "%s: error writing stdout because `%s'\n",
		Program, strerror(errno));
	exit(EXIT_FAILURE);
      } else {
	bytes = 0;
      }
    }
  }
  gettimeofday(&tout, NULL);
  dt =  deltaT(&tin, &tout);
  totalTout += dt;
  if( show ) {
    fprintf(stderr, 
	    " out: %7.3fms at %6.0fkB/s (%6.0fkB/s avg)\n", 
	    1e3*dt, 
	    (double)totalBytes/dt*kB, 
	    (double)TotalBytes/totalTout*kB);
  }
}
/**********************************************************************/
int
main(int argc, char **argv)
{
  Cmdline *cmd;
  char *buf;
  int count;
  struct timeval tstart, tin, tnow;

  /***** BEGIN */
  cmd = parseCmdline(argc, argv);
  cmd->bsize *= 1024;
  
  buf = malloc(cmd->bsize);
  if( !buf ) {
    fprintf(stderr, 
    "%s: cannot allocate buffer of length `%d', out of memory\n",
	    Program, cmd->bsize);
    exit(EXIT_FAILURE);
  }

  TotalBytes = 0;
  totalTin = 0.0;
  totalTout = 0.0;
  
  gettimeofday(&tstart, NULL);
  for(count=cmd->bsize; count==cmd->bsize; /**/) {
    double dt, dtAll;
    gettimeofday(&tin, NULL);
    count = readBuffer(buf, cmd->bsize, cmd->vrP);
    writeBuffer(buf, count, cmd->vwP);
    gettimeofday(&tnow, NULL);
    dt = deltaT(&tin, &tnow);
    dtAll = deltaT(&tstart, &tnow);

    if( cmd->vtP ) {
      fprintf(stderr, 
	      "thru: %7.3fms at %6.0fkB/s (%6.0fkB/s avg) %7.0fkB\n",
	      1e3*dt, 
	      (double)count/dt*kB,
	      (double)TotalBytes/dtAll*kB,
	      (double)TotalBytes*kB);
    }
  }
  return 0;
}
