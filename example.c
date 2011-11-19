/**
 * Copyright 2011  Pierre-Luc Bacon <pierrelucbacon@aqra.ca>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "timedaction.h"
#include <stdio.h>

struct timespec start;
#define MS (1000000)

void hey_cb(void* data)
{
   struct timespec curr;
   if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1) {
       return;
   }

    fprintf(stderr, "Hey %d\n", *(int *) data);

           int secs, nsecs;

           secs = curr.tv_sec - start.tv_sec;
           nsecs = curr.tv_nsec - start.tv_nsec;
           if (nsecs < 0) {
               secs--;
               nsecs += 1000000000;
           }
           fprintf(stderr, "%d.%03d: ", secs, (nsecs + 500000) / 1000000);

    curr = start;
}

int main(void)
{
    timed_action_notifier* notifier = timed_action_mainloop_threaded();
     
   if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
       perror("clock_gettime");
       return;
   }

    int dat = 2;
    timed_action_schedule_periodic(notifier, 1, 0, &hey_cb, &dat);

    int otherdat = 3; 
    timed_action_schedule(notifier, 5, 0, &hey_cb, &otherdat);

    sleep(8);

    return 0;
}
