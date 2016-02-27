/**********************************************************************
  cpipe -- counting pipe

  Watch out, here comes the GPL-virus.

  (C) 1997--2003 Harald Kirsch (pifpafpuf@gmx.de)

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

  $Revision: 1.7 $, $Date: 2003/07/22 08:16:29 $
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>

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
  double sec;

  usec = tout->tv_usec - tin->tv_usec;
  if( usec<0 ) {
    sec = tout->tv_sec-tin->tv_sec-1 + ((double)(usec + 1000000))*1e-6;
  } else {
    sec = tout->tv_sec-tin->tv_sec + ((double)usec)*1e-6;
  }
  return sec;
}
/**********************************************************************/
ssize_t
readBuffer(char *buf, size_t length, int show, int nonblock, int *eof)
{
  uint64_t totalBytes;
  ssize_t bytes;
  struct timeval tin, tout;
  double dt;
  char txt1[40], txt2[40], txt3[40];
  int flags;

  /***** 
    The first read is always a blocking one, because there is no point
    in returning 0 bytes.
  *****/
  flags = fcntl(STDIN_FILENO, F_GETFL);
  fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);

  gettimeofday(&tin, NULL);
  for(totalBytes=0; totalBytes<length; totalBytes+=bytes, buf+=bytes) {
    bytes = read(STDIN_FILENO, buf, length-totalBytes);
    if( 0==bytes ) {*eof=1; break;}
    if( bytes>0 ) {
      if( nonblock ) fcntl(STDIN_FILENO, F_SETFL, flags|O_NONBLOCK);
      continue;
    }
    if( errno==EAGAIN ) {
      /***** non-blocking operation returned with 0 bytes */
      break;
    }
    if( errno==EINTR ) {
      /****** on interrupt, we try again, even in non-blocking mode */
      bytes = 0;
      continue;
    }

    /***** serious error on read */
    fprintf(stderr, "%s: error reading stdin because `%s'\n",
	    Program, strerror(errno));
    exit(EXIT_FAILURE);
  }

  gettimeofday(&tout, NULL);
  TotalBytes += (double)totalBytes;
  if( show ) {
    dt =  deltaT(&tin, &tout);
    totalTin += dt;
    fprintf(stderr, 
	    "  in: %7.3fms at %7sB/s (%7sB/s avg) %7sB", 
	    1e3*dt, 
	    scale((double)totalBytes/dt, txt1),
	    scale(TotalBytes/totalTin, txt2),
	    scale(TotalBytes, txt3));
    if( totalBytes<length ) {
      fprintf(stderr, "   (bsize=%lu)\n", totalBytes);
    } else {
      fprintf(stderr, "\n");
    }
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
  if( show ) {
    dt =  deltaT(&tin, &tout);
    totalTout += dt;
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
  int eof;
  struct timeval tstart, tin, tnow;
  char txt1[40], txt2[40], txt3[40];
  double calib=0.98;	     /* a correcton factor for sleep time */

  /***** BEGIN */
  cmd = parseCmdline(argc, argv);

  cmd->bsize *= ONEk;
  if( cmd->speedP ) cmd->speed *= ONEk;

  if( cmd->ngrP ) {
    /***** switch input into non-blocking */
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
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
  for(count=cmd->bsize, eof=0; !eof; /**/) {
    double dt, dtAll;
    double delay;

    gettimeofday(&tin, NULL);
    count = readBuffer(buf, cmd->bsize, cmd->vrP, cmd->ngrP, &eof);
    writeBuffer(buf, count, cmd->vwP);
    gettimeofday(&tnow, NULL);
    dt = deltaT(&tin, &tnow);

    /***** 
      If speed limit is requested, we might have to sleep. On most
      architectures, the nanosleep will have a fixed minimum delay of
      e.g. 0.01s. Consequently the throughput is severely limited by
      this if the buffer size choosen is too small.
    *****/
    delay = 
      (double)count/cmd->speed	/* the time it should have taken */
      - dt;			/* the time it took */
    if( cmd->speedP && delay>0 ) {
      struct timespec sleeptime;
      double factor;
      delay *= calib;
      sleeptime.tv_sec = (time_t)delay;
      sleeptime.tv_nsec = 1e9*(delay-floor(delay));
      if( 0==nanosleep(&sleeptime, NULL) ) {
	/***** only recalibrate if we waited the full time */
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
