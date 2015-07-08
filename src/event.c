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

#include "event.h"



void event_handler ( )
{
	u_int32  update_seconds;
	u_int8   lastPercentage;
	int j;

#ifdef LONGRUN
        int low, high;
#endif
	
	secondsCounter = time(NULL);
	powerState.updater();
	update_seconds = time(NULL);
	lastPercentage = powerState.percentage;
	
	if( powerState.percentage == 100 && powerState.isCharging && args_paradisiac == PARADISIAC_ON )
    {
        if( args_ttyMode && args_beQuiet == false
#ifndef LONGRUN
 && cpuState.actualFreq != cpuState.maxFreq
#endif
  ) {
            int a;
            fprintf( stdout, "battery at max.. ac_adapter plugged in..\n");
            fprintf( stdout, "d'u wanna set cpu in paradisiac mode ? (y/^y): ");
            a = getchar();
            if( a != 'y' )
                return;
            else
                fprintf( stdout, "Good choice !\n" );
        }
            
		cpuSetParadisiac();
    }

    if( args_ttyMode )
        return;
	
	while( 1 )
	{
		secondsCounter = time(NULL);
		
		if( XPending( display )  )
		{
			XNextEvent( display, &e);
			
			switch( e.type)
			{
				case Expose:
					
					draw_all ( );
					
					break;
				
				case ButtonPress:
					
					j = CheckMouseRegion(e.xbutton.x, e.xbutton.y);
				
					switch ( j )
					{
						/* auto-freq */
						case MREGION_AUTOFREQ:
							/* toggle auto-freqency "(c)2003 LdP" scaling mode */
							cpuState.auto_freq_state = ( cpuState.auto_freq_state ? false: true );
							/* auto_freq_state is read from manageCpuLoad() function:
                                                         * if this is active, this function increase cpu frequency
														 * if it is at 100%, else decrease it.
                                                         *
                                                         * note that on longrun systems this is unneeded; longrun does
                                                         * this automatically, but we need to give it the range to do so. */
#ifdef LONGRUN
                                                        get_longrun_range(&low, &high);
                                                        if (cpuState.auto_freq_state)
                                                            set_longrun_range(0, high);
                                                        else
                                                            set_longrun_range(high, high);
#endif
							break;
						/* < freq */
						case MREGION_MINFREQ:
							/* decrease cpu frequency */
							cpuSetFreq( FREQ_DECREMENT, FREQ_NEXT );
#ifndef LONGRUN
							cpuState.auto_freq_state = false;
#endif
							break;
						/* > freq */
						case MREGION_MAXFREQ:
							/* increase cpu frequency */
							cpuSetFreq( FREQ_INCREMENT, FREQ_NEXT );
#ifndef LONGRUN
							cpuState.auto_freq_state = false;
#endif
							break;
							
					}
					
					/* if a cliccable region was clicked, the dockapp is redrawn */
					if( j != -1 )
						draw_all ( );
					
					break;
					
				case ButtonRelease:
					
					/* activate screen saver on right click mouse */
					if( e.xbutton.state & Button3Mask )
					{
						XForceScreenSaver( display, ScreenSaverActive );
					}
					break;
					
			}
		}
		
		
		/* update battery percentage and cpuFreq
		 * once every batteryUpdate milliseconds; check for autoscripts
		 * and cpuParadisiac too */
		if( update_seconds <= secondsCounter - (args_batteryUpdate/1000) )
		{
			update_seconds = secondsCounter;
                        /* longrun adjusts so quickly that if you put the check
                           after the succeeding line it consistently reads a
                           higher MHz (in ACPI) */
			cpuCheckFreq();
			powerState.updater();
			
			if( args_autoShutdown != AUTOSHUTDOWN_OFF )
				checkAutoShutdown ( );
			
			if( args_autoAlarm != AUTOALARM_OFF )
				checkAutoAlarm ( );

			if( args_paradisiac == PARADISIAC_ON )
				cpuSetParadisiac ( );
		}
		
		/* check and redraw cpu load */
		manageCpuLoad();
		usleep( 1000*args_cpuUpdate /*150000*/ );
		
	}
}


void AddMouseRegion( u_int8 index, u_int8 left, u_int8 top, u_int8 right, u_int8 bottom)
{
	if (index < MAX_MOUSE_REGION) {
		mouse_region[index].top = top;
		mouse_region[index].left = left;
		mouse_region[index].bottom = bottom;
		mouse_region[index].right = right;
	}
}

u_int8 CheckMouseRegion( u_int8 x, u_int8 y)
{
	register int i;
	register bool found;
	
	found = 0;
	
	for (i=0; i<MAX_MOUSE_REGION && !found; i++) {
		if (x <= mouse_region[i].right &&
			x >= mouse_region[i].left &&
			y <= mouse_region[i].bottom &&
			y >= mouse_region[i].top)
			found = 1;
	}
	if (!found) return -1;
	return (i-1);
}


bool flashingLowBatteryCycle;

void stopFlashingLowBattery ( int useless )
{   
	flashingLowBatteryCycle = false;
	wait( NULL );
	return;
}

void startFlashingLowBattery ( pid_t childPid )
{
	bool alternate = false;
	int i;
	
	while ( flashingLowBatteryCycle )
	{
		/* we make the child die if the user click on the dockapp */
		if( XPending( display )  )
		{
			XNextEvent( display, &e);
			switch( e.type)
			{
				case ButtonPress:
					/* kill the child */
					kill( childPid, SIGKILL );
			}
		}
		
		if( alternate )
			draw_area( 64, 0, 59, 59, 2, 2 );
		else
			draw_area( 64, 59, 59, 59, 2, 2 );
		
		alternate = !alternate;
		
		while (XCheckTypedWindowEvent(display, iconwin, Expose, &e));
		XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, 0,0,
							wmgen.attributes.width, wmgen.attributes.height, 0,0);
		while (XCheckTypedWindowEvent(display, win, Expose, &e));
		XCopyArea(display, wmgen.pixmap, win, NormalGC, 0,0, \
							wmgen.attributes.width, wmgen.attributes.height, 0,0);
		
		usleep( 80000 );
	}
	
	/* now i have to fill the dockapp background with black.. ugly job */
	for( i = 2; i < 60; i += 20 )
		draw_area( 64, 119, 59, 20, 2,i );
	
	while (XCheckTypedWindowEvent(display, iconwin, Expose, &e));
	XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, 0,0,
						wmgen.attributes.width, wmgen.attributes.height, 0,0);
	while (XCheckTypedWindowEvent(display, win, Expose, &e));
	XCopyArea(display, wmgen.pixmap, win, NormalGC, 0,0, \
						wmgen.attributes.width, wmgen.attributes.height, 0,0);
		
	
}
