/**********************************************************************
  cpipe -- counting pipe

  Watch out, here comes the GPL-virus.

  (C) 1997--2001 Harald Kirsch (kirschh@lionbioscience.com)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  $Revision$, $Date$
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include "cmdline.h"

#define ONEk 1024.0
#define ONEkSCALE (1.0/ONEk)
#define ONEM (1024.0*1024.0)
#define ONEMSCALE (1.0/ONEM)
#define ONEG (1024.0*1024.0*1024.0)
#define ONEGSCALE (1.0/ONEG)

double TotalBytes;
double totalTin, totalTout;

/**********************************************************************/
char *
scale(double v, char *buf)
{
  if( v>ONEM ) {
    /***** This is at least a G */
    if( v>ONEG ) {
      sprintf(buf, "%6.1fG", v*ONEGSCALE);
    } else {
      sprintf(buf, "%6.1fM", v*ONEMSCALE);
    }
  } else {
    if( v>ONEk ) {
      sprintf(buf, "%6.1fk", v*ONEkSCALE);
    } else {
      sprintf(buf, "%4.0f", v);
    }
  }
  return buf;
}
/**********************************************************************/
/**
  returns the time delta represented by the given structures as a
  double value in seconds.
*****/
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
  char txt1[40], txt2[40], txt3[40];
  
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
  TotalBytes += (double)totalBytes;
  if( show ) {
    fprintf(stderr, 
	    "  in: %7.3fms at %7sB/s (%7sB/s avg) %7sB\n", 
	    1e3*dt, 
	    scale((double)totalBytes/dt, txt1),
	    scale(TotalBytes/totalTin, txt2),
	    scale(TotalBytes, txt3));
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
  char txt1[40], txt2[40], txt3[40];

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
	    " out: %7.3fms at %7sB/s (%7sB/s avg) %7sB\n", 
	    1e3*dt, 
	    scale((double)totalBytes/dt, txt1),
	    scale(TotalBytes/totalTout, txt2),
	    scale(TotalBytes, txt3));
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
  char txt1[40], txt2[40], txt3[40];
  double targetT=0.0;	     /* used for -s, time one block should take */
  double calib=0.98;	     /* a correcton factor for sleep time */

  /***** BEGIN */
  cmd = parseCmdline(argc, argv);

  cmd->bsize *= ONEk;
  if( cmd->speedP ) {
    cmd->speed *= ONEk;
    targetT = ((double)cmd->bsize)/cmd->speed;
  }

  buf = malloc(cmd->bsize);
  if( !buf ) {
    fprintf(stderr, 
    "%s: cannot allocate buffer of length `%d', out of memory\n",
	    Program, cmd->bsize);
    exit(EXIT_FAILURE);
  }

  TotalBytes = 0.0;
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

    /***** 
      If speed limit is requested, we might have to sleep. On most
      architectures, the usleep will have a fixed minimum delay of
      e.g. 0.01s. Consequently the throughput is severely limited by
      this if the buffer size choosen is too small.
    *****/
    if( cmd->speedP && targetT>dt ) {
      double factor;
      unsigned long sleeptime = (unsigned long)((targetT-dt)*1e6*calib);
      usleep(sleeptime);
      gettimeofday(&tnow, NULL);
      dt = deltaT(&tin, &tnow);

      /***** 
        recalibrate the calibration factor. The value it should have
	had is factor*calib. However we do not jump to this value but
	move only 1/8th of the distance to achive some damping.
      *****/
      factor = ((double)count/dt)/cmd->speed;
      calib += 0.125*(factor*calib-calib);
    }

    dtAll = deltaT(&tstart, &tnow);

    if( cmd->vtP ) {
      fprintf(stderr, 
	      "thru: %7.3fms at %7sB/s (%7sB/s avg) %7sB\n", 
	      1e3*dt, 
	      scale((double)count/dt, txt1),
	      scale(TotalBytes/dtAll, txt2),
	      scale(TotalBytes, txt3));
    }
  }
  return 0;
}
