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
#ifndef __TIMED_ACTION_H__
#define __TIMED_ACTION_H__

#include <pthread.h>

/**
 * Opaque type. Timed action handle.
 */
typedef struct {
    int tfd;
    void (*timed_action_handler)(void*);
    void* arg;
} timed_action_t;

/**
 * Opaque type. Notifies user by callback when timer expires.
 */
typedef struct {
    int epfd;
    pthread_t th;
} timed_action_notifier;

/**
 * Create a threaded mainloop for even handling. 
 */
timed_action_notifier* timed_action_mainloop_threaded();

/**
 * Schedule a delayed action (callback) at a given time
 */
timed_action_t* timed_action_schedule(timed_action_notifier* notifier, time_t sec, long nsec, void (*timed_action_handler)(void*), void* arg);

/**
 * Schedule a periodic action (callback) at a given time
 */
timed_action_t* timed_action_schedule_periodic(timed_action_notifier* notifier, time_t sec, long nsec, void (*timed_action_handler)(void*), void* arg);

/**
 * Unschedule an action
 */
int timed_action_unschedule(timed_action_notifier* notifier, timed_action_t* action);

#endif
