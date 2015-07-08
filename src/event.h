/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include "main.h"


/* wmlaptop's main cicle. it handles expose and mouse_clicking events
 * it also periodically recalls some monitoring functions:
 * every clock cycle: cpu load
 * every two seconds: battery and frequency states
 * every time that changes battery state: estimation of remaining time */
void event_handler ( );


/* it adds a sensible-to-mouse-clic region */
void AddMouseRegion(u_int8, u_int8, u_int8, u_int8, u_int8);

/* called every clic, it returns the clicked region or -1 if no region
 * has been clicked. */
u_int8 CheckMouseRegion( u_int8, u_int8 );


/* this function will let startFlashingLowBattery to return */
void stopFlashingLowBattery ( int );

/* this function will display "LOW BATTERY" flashing on the screen 
 * until a SIGCHLD is sent to the dockapp (handled by stopFlashingLowBattery) */
void startFlashingLowBattery ( pid_t );

#endif
