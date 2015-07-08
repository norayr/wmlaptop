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

#include "draw.h"


void draw_area(int sx, int sy, int w, int h, int dx, int dy)
{

	XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, sx, sy, w, h, dx, dy);

}


void draw_all ( )
{
	draw_staticBackGround ( );
	
	draw_ACconnector ( );
	
	draw_batteryPercentage ( );
	
	draw_timeString ( );
	
	draw_cpuload ( );
	
	draw_auto_freq ( );
	
	draw_freq_arrows ( );
	
	draw_actual_freq ( );
	
	while (XCheckTypedWindowEvent(display, iconwin, Expose, &e));
	XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, 0,0,
						wmgen.attributes.width, wmgen.attributes.height, 0,0);
	while (XCheckTypedWindowEvent(display, win, Expose, &e));
	XCopyArea(display, wmgen.pixmap, win, NormalGC, 0,0, \
						wmgen.attributes.width, wmgen.attributes.height, 0,0);
					
	
}


void draw_staticBackGround ( )
{
	/* Mhz */
	draw_area( 46, 119, 15, 9, 38, 51 );
	
	/* two gray lines */
	draw_area( 66, 118, 55, 1, 4, 24 );
	draw_area( 66, 118, 55, 1, 4, 48 );
	
	/* the outline of the battery graphic bar */
	draw_area(  0, 131, 45, 9, 14, 14 );
	return;
}


void draw_ACconnector (  )
{
	draw_area( 47, 64 + (6*powerState.isCharging), 9, 6, 4, 16 );
}

#define   RED_PRINT       ( powerState.percentage < 20 )
#define   YELLOW_PRINT    ( powerState.percentage >= 20 && powerState.percentage < 50 )
#define   GREEN_PRINT     ( powerState.percentage >= 50 )

void draw_batteryPercentage ( )
{
	register u_int8 decine;
	register u_int8 unita;
	register u_int8 baseX;
	register u_int8 baseY;
	
	u_int8 numBars;
	int i;
	int j;
	int battery = 0;
	int batterySlot;
	int presentBatteries = 0;
	
	if( powerState.percentage == 100 )
		/* drawing the green <1> at first position*/
		draw_area( 4, 64, 4, 7, 5, 5 );
	else
		/* drawing "empty" number shadow */
		draw_area( 57, 76, 4, 7, 5, 5 );
	
	/* drawing tens */
	decine = (powerState.percentage / 10)%10;
	baseX = 0;
	baseY = GREEN_PRINT ? 64 : ( YELLOW_PRINT ? 71 : 78 );	
	
	if( decine == 0 && powerState.percentage != 100 )
		draw_area( 57, 76, 4, 7, 10, 5 );
	else
		draw_area( baseX + ( decine * 4 ), baseY, 4, 7, 10, 5 );
	
	/* drawing units */
	unita = powerState.percentage % 10;
	
	draw_area( baseX + ( unita * 4 ), baseY, 4, 7, 15, 5 );
	
	
	/* drawing <%> symbol of the right color */
	draw_area( 40, baseY, 7, 7, 20, 5 );

	
	/* let's see, how many batteries are present */
	for ( batterySlot = 0; batterySlot < powerState.nBatt; batterySlot ++ )
		if (powerState.batteryVector[batterySlot] -> present)
			presentBatteries ++;


	for( batterySlot = 0; batterySlot < powerState.nBatt; batterySlot ++ ) {

		if (powerState.batteryVector[batterySlot] -> present) {
			battery ++;

			/* battery drawing */
			numBars = (22 * powerState.batteryVector[batterySlot] -> percentage) / 100;

			for( i = 0; i < numBars; i++ )
				draw_area( 49 + ( i > 10 ? i - 11 : i ), 95 + ( i > 10 ? 7 : 0), 1, (7 / presentBatteries), 15+(i*2), 15 + (battery - 1) * (7 / presentBatteries + 1 ) );

			/* unlit remaining battery */
			for( j = i; j < 22; j++ )
				draw_area( 60, 95, 1, (7 / presentBatteries), 15 + (j*2), 15 + (battery - 1) * (7 / presentBatteries + 1 ) );

		}
	}

	return;
}


void draw_timeString ( )
{
	u_int8 decineOre;
	u_int8 unitaOre;
	u_int8 decineMin;
	u_int8 unitaMin;
	
	
	if( powerState.remainingTime == 0 )
	{
		
		register int i;
		register int j;
		
		/* unlit numbers */
		for( j = 0; j < 2; j++ )
			for( i = 0; i < 2; i++ )
				draw_area( 57, 65, 6, 10, 30 + (i*7 + j*16), 3 );
		
		/* unlit : */
		draw_area( 61, 87, 1, 6, 44, 5 );
	}
	else
	{
		decineOre = powerState.remainingTime / 600;
		unitaOre  = (powerState.remainingTime / 60) % 10;
		decineMin = (powerState.remainingTime - (decineOre*10*60) - (unitaOre*60) ) / 10;
		unitaMin  = (powerState.remainingTime - (decineOre*10*60) - (unitaOre*60) ) % 10;
		
		
			draw_area( decineOre  * 6, 85, 6, 10, 30, 3 );
			draw_area( unitaOre   * 6, 85, 6, 10, 37, 3 );
			draw_area( 62, 87, 1, 6, 44, 5 ); /* : */
			draw_area( decineMin  * 6, 85, 6, 10, 46, 3 );
			draw_area( unitaMin   * 6, 85, 6, 10, 53, 3 );
		
	}
	
	
	return;
}


void draw_cpuload ( )
{
	static u_int8 lastCpuLoad = 0;
	
	u_int8 decine = cpuLoad == 100 ? 0 : (cpuLoad / 10);
	u_int8 unita = cpuLoad % 10;

	
	/* tens drawing */
	if( decine == 0 && cpuLoad != 100 )
		draw_area( 57, 75, 4, 7, 49, 27 );
	else
		draw_area( decine*4, 71, 4, 7, 49, 27 );
	
	/* units drawing */
	draw_area( unita*4, 71, 4, 7, 54, 27 );	
	
	
	if( cpuLoad == 100 )
	{
		/* blinking CPULOAD */
            draw_area( 0, 117 + (lastCpuLoad == 100 ? 7 : 0), 37, 7, 4, 27 );

 		if( !args_dontBlink100 )
 			lastCpuLoad = ( lastCpuLoad == 100 ? 101 : 100 );
		
		/* drawing number <1> */
		draw_area( 4, 71, 4, 7, 44, 27 );
	}
	else
	{
		/* grey CPULOAD */
		draw_area( 0, 117, 37, 7, 4, 27 );
			
		/* no number <1> at the beginning */
		draw_area( 57, 75, 4, 7, 44, 27 );
		lastCpuLoad = cpuLoad;
	}
	
	
}

void draw_auto_freq ( )
{
	if ( cpuState.auto_freq_state ){
		draw_area( 0, 95, 49, 11, 7, 36);
	} else {
		draw_area( 0, 106, 49, 11, 7, 36);
	}
}

void draw_freq_arrows ( )
{
#ifndef LONGRUN
	if ( cpuState.actualFreq == cpuState.minFreq){
#else
	if ( cpuState.actualLevelIdx == 0){
#endif
		draw_area( 50, 109, 4, 7, 4, 52);;
	} else {
		draw_area( 48, 77, 4, 7, 4, 52);
	}
#ifndef LONGRUN
	if ( cpuState.actualFreq == cpuState.maxFreq){
#else
	if ( cpuState.actualLevelIdx == cpuState.nLongRunLevels-1){
#endif
		draw_area( 54, 109, 4, 7, 55, 52);
	} else {
		draw_area( 52, 77, 4, 7, 55, 52);
	}
}

void draw_actual_freq ( )
{
	u_int8 migliaia_freq  = 0;
	u_int8 centinaia_freq = 0;
	u_int8 decine_freq    = 0;
	u_int8 unita_freq     = 0;
	
	migliaia_freq  = cpuState.actualFreq /  1000000;
	centinaia_freq = ( cpuState.actualFreq / 100000 ) % 10;
	decine_freq    = ( cpuState.actualFreq / 10000  ) % 10;
	unita_freq     = ( cpuState.actualFreq / 1000   ) % 10;
	
		/* drawing actual frequency */
		if ( migliaia_freq )
			draw_area ( (6*migliaia_freq), 85, 6, 10, 10, 50 );
		else
			draw_area ( 57, 75, 6, 10, 10, 50 );
	
	draw_area ( (6*centinaia_freq), 85, 6, 10, 17, 50 );
	draw_area ( (6*decine_freq), 85, 6, 10, 24, 50 );
	draw_area ( (6*unita_freq), 85, 6, 10, 31, 50 );
		
}
