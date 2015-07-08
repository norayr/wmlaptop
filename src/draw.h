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

#ifndef __DRAW_H__
#define __DRAW_H__

#include "main.h"


/* function used to redraw visible dockapp's surface.
 * arguments taken are:
 * source X      (x in general xpm )
 * source Y      (y in general xpm )
 * width         (width of the area to copy )
 * height        (height of the area to copy )
 * destination X (x in the dockapp visible area where to copy the image )
 * destination Y (y in the dockapp visible area where to copy the image ) */ 
void draw_area (int, int, int, int, int, int );


void draw_all ( );

/* it draws "Mhz", the two gray lines and the outline of the 
 * battery graphic bar */
void draw_staticBackGround ( );

/* it draws the plug in green if powerState.isCharging is TRUE */
void draw_ACconnector ( );

/* it draws battery percentage value */
void draw_batteryPercentage ( );

/* it draws estimated time if it's possible to know it */
void draw_timeString ( );

/* it draws cpuload value */
void draw_cpuload ( );

/* it draws AUTO-FREQ in red if active, in grey else */
void draw_auto_freq ( );

/* it draws the arrows at left and right of the cpu frequency */
void draw_freq_arrows ( );

/* it draws actua cpu frequency */
void draw_actual_freq ( );


#endif
