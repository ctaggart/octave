/*

Copyright (C) 1999 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_UNISTD_H
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <unistd.h>
#endif

#include "systime.h"

#ifdef HAVE_POLL_H
#include <poll.h>
#elif HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

static void
do_octave_usleep (unsigned int useconds)
{
#if defined (HAVE_USLEEP)

  usleep (useconds);

#elif defined (HAVE_SELECT)

  struct timeval delay;

  delay.tv_sec = 0;
  delay.tv_usec = useconds;

  select (0, 0, 0, 0, &delay);

#elif defined (HAVE_POLL)

  struct pollfd pfd;
  int delay = useconds / 1000;

  if (delay > 0)
    poll (&fd, 0, delay);

#endif
}

void
octave_usleep (unsigned int useconds)
{
  unsigned int sec = useconds / 1000000;
  unsigned int usec = useconds % 1000000;

  if (sec > 0)
    sleep (sec);

  do_octave_usleep (usec);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
