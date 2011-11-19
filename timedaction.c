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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#define MAX_TIMED_ACTIONS 1024

#undef MIN
#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))

static void* timed_action_watcher(void *ptr)
{
    timed_action_notifier* notifier = (timed_action_notifier*) ptr;

    struct epoll_event* events = (struct epoll_event*) malloc(sizeof(struct epoll_event) * MAX_TIMED_ACTIONS);

    while(1) { // FIXME Might want to have proper exit condition

        int nfds = epoll_wait(notifier->epfd, events, MAX_TIMED_ACTIONS, -1);
        if (nfds < 0 && errno != EINTR) {
            return;
        }
       
        if (nfds == EINTR) {
            continue;
        }

        int i;
        for (i = 0; i < nfds; i++) {   
            timed_action_t* action = (timed_action_t*) events[i].data.ptr;

            // Consume the timer data in fd
            uint64_t exp;
            ssize_t s = read(action->tfd, &exp, sizeof(uint64_t));
            if (s != sizeof(uint64_t)) {
                perror("read() timerfd:");
                continue;
            }

            // Call action handler
            action->timed_action_handler(action->arg);
        }
    }

    free(events); // FIXME : Will never reach this
}

timed_action_notifier* timed_action_mainloop_threaded()
{
    timed_action_notifier* notifier = (timed_action_notifier*) malloc(sizeof(timed_action_notifier));

    notifier->epfd = epoll_create1(0);
    if (notifier->epfd < 0) {
        return NULL;
    }

    pthread_create(&notifier->th, NULL, timed_action_watcher, notifier);

    return notifier;
}

static int timer_set_expiry(int timer, time_t sec, long nsec, time_t intsec, long intnsec)
{
    struct itimerspec timerSpec;
    memset(&timerSpec, 0, sizeof(timerSpec));

    timerSpec.it_value.tv_sec = sec;
    timerSpec.it_value.tv_nsec = nsec;

    timerSpec.it_interval.tv_sec = intsec;
    timerSpec.it_interval.tv_nsec = intnsec;

    struct itimerspec oldSpec;
    return timerfd_settime(timer, 0, &timerSpec, &oldSpec);
}

static timed_action_t* schedule_timer(timed_action_notifier* notifier, time_t sec, long nsec, time_t intsec, long intnsec, void (*timed_action_handler)(void*), void* arg)
{
    timed_action_t* action = (timed_action_t*) malloc(sizeof(timed_action_t));
    action->tfd = timerfd_create(CLOCK_REALTIME, 0);
    action->timed_action_handler = timed_action_handler;
    action->arg = arg;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = action;
    epoll_ctl(notifier->epfd, EPOLL_CTL_ADD, action->tfd, &ev);

    timer_set_expiry(action->tfd, sec, nsec, sec, nsec);

    return action;
}

timed_action_t* timed_action_schedule(timed_action_notifier* notifier, time_t sec, long nsec, void (*timed_action_handler)(void*), void* arg)
{
    return schedule_timer(notifier, sec, nsec, 0, 0, timed_action_handler, arg);
}

timed_action_t* timed_action_schedule_periodic(timed_action_notifier* notifier, time_t sec, long nsec, void (*timed_action_handler)(void*), void* arg)
{
    return schedule_timer(notifier, sec, nsec, sec, nsec, timed_action_handler, arg);
}

int timed_action_unschedule(timed_action_notifier* notifier, timed_action_t* action)
{
    int ret = epoll_ctl(notifier->epfd, EPOLL_CTL_DEL, action->tfd, NULL);
    ret = MIN(timer_set_expiry(action->tfd, 0, 0, 0, 0), ret);
    ret = MIN(close(action->tfd), ret);

    return ret;
}
