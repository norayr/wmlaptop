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

#ifndef __CPU_H__
#define __CPU_H__

#include "main.h"

#ifdef LONGRUN
#include "longrun.h"
#endif


/* thanks to wmfire, returns cpu load */
u_int8 getCpuLoad ( );

/* function invoked from events cycle that updates the dockapp
 * after it has modified the global variable cpuLoad with cpucusage
 * percent. It also do the cpuScaling if auto_freq_state is enabled */
void manageCpuLoad ( );


/* reads from information file cpu actual frequency and stores it in
 * global variable cpuState */
void cpuReadFreq( );


/* invoked by events cycle  e time every 2 seconds, it reads actual
 * frequency (calling readCpuFreq) and it redraws the dockapp if it's
 * changed */
void cpuCheckFreq();


/* it changes processor frequency calling cpuEchoFreq after calculating
 * new desired value.
 * the two arguments are about the way that the frequency should be changed: 
 * 1°: it can be FREQ_INCREMENT, to indicate that the frequency should be
 *     increased or it can be FREQ_DECREMENT to decrease it.
 * 2°: it can be FREQ_STEP, to change the frequency step to a static unit.
 *     (default 100000 Khz), or FREQ_NEXT to change frequency by 50000 Khz
 *     until the nearest value
 */

#define    FREQ_INCREMENT    0
#define    FREQ_DECREMENT    1
#define    FREQ_STEP         0
#define    FREQ_NEXT         1

#define    FREQ_NEXT_DEFAULT 50000


void cpuSetFreq( bool, bool );


/* it is the function that really changes the frequency, writing 
 * cpuState.setFreq value on the right file. */
void cpuEchoFreq( );


/* if the battery is at 100% and the AC_Adapter is plugged in,
 * there is no reason to keep low the frequency, and then.. let's
 * the processor be what it is */
void cpuSetParadisiac( );

#endif
