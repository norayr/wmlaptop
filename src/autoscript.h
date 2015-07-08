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

#ifndef __AUTOSCRIPT_H__
#define __AUTOSCRIPT_H__

#include "main.h"


#ifndef SHUTDOWN_BIN
# define SHUTDOWN_BIN      "sudo /sbin/shutdown"
#endif

#ifndef SHUTDOWN_ARGS
# define SHUTDOWN_ARGS     "\"wmlaptop is shutting down the system\" -h"
#endif

/* check each 2 seconds if it's time to run shutdown or 
 * if it's time to play an alarm */
void checkAutoShutdown ( );

void checkAutoAlarm ( );

/* ALARM DEFINITIONS:

Notes:  DO  DO#   RE  RE#   MI   FA  FA#  SOL SOL#   LA  LA#   SI   DO
Hertz: 261  277  294  311  330  349  370  392  415  440  466  494  523
 
 tone = 1190000/frequency
 freq = 1190000/tone
 
*/


#define    VEL                       28

#define    BREVE                     
#define    SEMIBREVE                 VEL*64
#define    MINIMA                    VEL*32
#define    SEMIMINIMA                VEL*16
#define    CROMA                     VEL*8
#define    SEMICROMA                 VEL*4
#define    BISCROMA                  VEL*2
#define    SEMIBISCROMA              VEL


#define    DO                        261
#define    DO_D                      277
#define    RE                        294
#define    RE_D                      311
#define    MI                        330
#define    FA                        349
#define    FA_D                      370
#define    SOL                       392
#define    SOL_D                     415
#define    LA                        440
#define    LA_D                      466
#define    SI                        494
#define    DO_                       ( DO  * 2 )
#define    RE_                       ( RE  * 2 )
#define    MI_                       ( MI  * 2 )
#define    FA_                       ( FA  * 2 )
#define    SOL_                      ( SOL * 2 )
#define    LA_                       ( LA  * 2 )
#define    SI_                       ( SI  * 2 )
#define    DO__                      ( DO_ * 2 )


void play( int , int , int  );

void alarm_Simpson   ( int );
void alarm_Ambulance ( int );
void alarm_Victory   ( int );
void alarm_Highbip   ( int );

void playAlarm (  );

#endif
