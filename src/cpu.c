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

#include "cpu.h"

/* thanks to wmfire */
u_int8 getCpuLoad ( )
{
	static int lastloadstuff = 0, lastidle = 0, lastnice = 0;
	int loadline = 0, loadstuff = 0, nice = 0, idle = 0;

	FILE *stat = fopen("/proc/stat", "r");
	char temp[128], *p;

	if (!stat)
	{
		static bool errorFlag = false;
		if( !errorFlag ) {
			PRINTQ( stderr, "Error reading the file '/proc/stat' for cpu-load info\n" );
			errorFlag = true;
		}
		return 100;
	}
	
	while (fgets(temp, 128, stat)) {
		if (!strncmp(temp, "cpu", 3) && temp[3] == ' ' )
		{
			p = strtok(temp, " \t\n");
			loadstuff = atol(p = strtok(NULL, " \t\n"));
			nice = atol(p = strtok(NULL, " \t\n"));
			loadstuff += atol(p = strtok(NULL, " \t\n"));
			idle = atol(p = strtok(NULL, " \t\n"));
			break;
		}
	}

	fclose(stat);

	if (!lastloadstuff && !lastidle && !lastnice) {
		lastloadstuff = loadstuff; lastidle = idle; lastnice = nice;
		return 0;
	}

	loadline = (loadstuff-lastloadstuff)+(idle-lastidle);
	
	if (loadline)
		loadline = ((loadstuff-lastloadstuff)*100)/loadline;
	else loadline = 100;
	
	lastloadstuff = loadstuff; lastidle = idle; lastnice = nice;
	
	return loadline;
}


void manageCpuLoad( )
{
	static u_int8 lastCpuLoad = 100;
#ifndef LONGRUN
	static u_int32 lastFreqChange = 0;
#endif
	/* update cpu value and redraw */
	cpuLoad = getCpuLoad();

	
	if( cpuLoad != lastCpuLoad || cpuLoad == 100 )
		draw_all();
	
	lastCpuLoad = cpuLoad;
	
	
        /* Longrun manages frequency automatically, so skip all this */
#ifndef LONGRUN	
	if(( cpuState.auto_freq_state == false ) ||
	   ( cpuLoad == 100 && cpuState.actualFreq == cpuState.maxFreq ) ||
	   ( cpuLoad != 100 && cpuState.actualFreq == cpuState.minFreq ))
		return;
#ifdef DEBUG
	printf("AutoFreq = %d, cpuLoad = %d, actualFreq = %d, maxFreq = %d, minFreq = %d\n",
	   cpuState.auto_freq_state, cpuLoad, cpuState.actualFreq, cpuState.maxFreq, cpuState.minFreq );
#endif
		
		/* if auto_freq_state is true, increase a time per second
	 * following cpu frequency by a unit until max frequency possible */
	
	/* a time per second.. */
	if( lastFreqChange == secondsCounter )
		return;
	
	/* first time will wait for a few seconds */
	if( lastFreqChange != secondsCounter-1 )
	{
		lastFreqChange = secondsCounter;
		return;
	}
	
	lastFreqChange = secondsCounter;
	
	/* increase if cpuload is 100 and if it isn't already at its maximum.. */
	if( cpuLoad == 100 && cpuState.actualFreq < cpuState.maxFreq )
		cpuSetFreq( FREQ_INCREMENT, FREQ_STEP );
	
	/* .. decrease if cpuload isn't 100 and if it isn't already at its minimum */
	if( cpuLoad != 100 && cpuState.actualFreq > cpuState.minFreq )
		cpuSetFreq( FREQ_DECREMENT, FREQ_STEP );
	
	
	/* let's read actual frequency to avoid a two seconds wait
	 * dto now it */
	cpuReadFreq();
#endif
	
	return;
}



void cpuReadFreq( )
{
#ifndef LONGRUN
	FILE * fp;
	char red[10];
	
	//if( (fp = fopen( cpuState.setFreqFile, "r" )) == NULL )
	//if( (fp = fopen( cpuState.setFreqFile[0], "r" )) == NULL )
	if( (fp = fopen( cpuState.readFreqFile, "r" )) == NULL )
	{
		//fprintf(stderr, "Error reading the info file (%s):\n%s\n", cpuState.setFreqFile, strerror(errno) );
		fprintf(stderr, "Error reading the info file (%s):\n%s\n", cpuState.readFreqFile, strerror(errno) );
		free_and_exit( ERROR );
	}
	
	fgets ( red, 10, fp );
	cpuState.actualFreq = atoi ( red );
	
	fclose ( fp );

#else
        int pct;
        int flags;
        int mhz;
        int voltz;
        int low, high;
        int i;

        longrun_get_stat(&pct, &flags, &mhz, &voltz);
        cpuState.actualFreq = mhz * 1000;

        get_longrun_range(&low, &high);
        for (i=0; i<cpuState.nLongRunLevels; i++)
            if (cpuState.longRunLevels[i] == high) {
                cpuState.actualLevelIdx = i;
                break;
            }
/* #ifdef DEBUG */
/* 	printf("cpu read: MHz %u, pct %u\n", */
/* 	        mhz, pct ); */
/* #endif */
#endif  // LONGRUN
}



void cpuCheckFreq()
{
	static u_int32 lastCpuFreq = 0;
	
	cpuReadFreq();
	
	if( lastCpuFreq != cpuState.actualFreq )
		draw_all();
	
	lastCpuFreq = cpuState.actualFreq;
}


void cpuSetFreq( bool direction, bool speed )
{
	
#ifdef DEBUG
	printf("cpuSetFreq() called: direction %u, speed %u, seconds %ld\n",
	        direction, speed, secondsCounter );
#endif

#ifndef LONGRUN
	/* let's start with a required control */
	if(( direction == FREQ_INCREMENT && cpuState.actualFreq == cpuState.maxFreq ) ||
	   ( direction == FREQ_DECREMENT && cpuState.actualFreq == cpuState.minFreq ))
		return;
    /* if not userspace, forget about it */
    if (! cpuState.userspace)
      return;
    /* check we got the permissions */
    if( (fopen( cpuState.setFreqFile, "w" )) == NULL )
      {
        fprintf(stderr, "Sorry, you may need priviledge to access into (%s):\n%s\n", \
            cpuState.setFreqFile, strerror(errno) );
        cpuState.userspace = false;
        return;
      }
 

	switch( speed )
	{
		case FREQ_STEP:
			switch( direction )
			{
				case FREQ_INCREMENT:
					cpuState.setFreq += cpuState.stepFreq;
					if( cpuState.setFreq > cpuState.maxFreq )
						cpuState.setFreq = cpuState.maxFreq;
					break;
				case FREQ_DECREMENT:
					cpuState.setFreq -= cpuState.stepFreq;
					if( cpuState.setFreq < cpuState.minFreq )
						cpuState.setFreq = cpuState.minFreq;
			}
			
			cpuEchoFreq( );
			break;
		
			
		case FREQ_NEXT:
		{
			/* is speed is FREQ_NEXT we shall increase/decrease frequency
			 * until we find that's really changed when we go to re-read it.
			 * ( it is the behaviour that we want when we clic on frequency
			 * arrows) */
			u_int32 lastCpuFreq = cpuState.actualFreq;
			
			while( lastCpuFreq == cpuState.actualFreq )
			{
				switch( direction )
				{
					case FREQ_INCREMENT:
						cpuState.setFreq += FREQ_NEXT_DEFAULT;
						if( cpuState.setFreq > cpuState.maxFreq )
							cpuState.setFreq = cpuState.maxFreq;
						break;
					case FREQ_DECREMENT:
						cpuState.setFreq -= FREQ_NEXT_DEFAULT;
						if( cpuState.setFreq < cpuState.minFreq )
							cpuState.setFreq = cpuState.minFreq;
				}
				
				cpuEchoFreq();
				cpuReadFreq();
			}
			
			cpuState.setFreq = cpuState.actualFreq;
		}
	} /* switch( speed ) */
#else
	/* let's start with a required control */
	if(( direction == FREQ_INCREMENT && cpuState.actualLevelIdx == cpuState.nLongRunLevels ) ||
	   ( direction == FREQ_DECREMENT && cpuState.actualLevelIdx == cpuState.longRunLevels[0] ))
		return;
	
	switch( speed )
	{
		case FREQ_STEP:
                    // different meaning under longrun: increase/decrease step size; but, never called..
			switch( direction )
			{
				case FREQ_INCREMENT:
                                    cpuState.stepLevelIdx++;
                                    if (cpuState.stepLevelIdx == cpuState.nLongRunLevels)
                                        cpuState.stepLevelIdx =  cpuState.nLongRunLevels - 1;
                                    break;
				case FREQ_DECREMENT:
                                    cpuState.stepLevelIdx--;
                                    if (cpuState.stepLevelIdx == 0)
                                        cpuState.stepLevelIdx =  1;
                                    break;
			}
			break;
			
		case FREQ_NEXT:
		{
                    /* increase / decrease longrun level by stepLevel */
                        switch( direction )
                            {
                            case FREQ_INCREMENT:
                                cpuState.setLevelIdx += cpuState.stepLevelIdx;
                                if (cpuState.setLevelIdx == cpuState.nLongRunLevels)
                                    cpuState.setLevelIdx = cpuState.nLongRunLevels - 1;
                                break;
                            case FREQ_DECREMENT:
                                cpuState.setLevelIdx -= cpuState.stepLevelIdx;
                                if (cpuState.setLevelIdx > 999) // unsigned
                                    cpuState.setLevelIdx = 0;
                                break;
                            }
				
                        cpuEchoFreq();
			
		}
	} /* switch( speed ) */

        return;
#endif // LONGRUN		
}


void cpuEchoFreq()
{
#ifndef LONGRUN
	FILE * fp;
#ifdef DEBUG
	printf("cpuEchoFreq() called: %u\n", cpuState.setFreq );
#endif
#else
        u_int16 setLevel;
#ifdef DEBUG
	printf("cpuEchoFreq() called: %u\n", cpuState.setLevelIdx );
#endif
#endif

#ifndef LONGRUN	
	if( (fp = fopen( cpuState.setFreqFile, "w" )) == NULL )
	{
		fprintf(stderr, "Error, cannot write the new freq (%d) into (%s):\n%s\n", \
		                cpuState.setFreq, cpuState.setFreqFile, strerror(errno) );
		free_and_exit( ERROR );
		return;
	}
	
	fprintf( fp, "%u", cpuState.setFreq );
	
	fclose ( fp );

#else
        setLevel = cpuState.longRunLevels[cpuState.setLevelIdx];
        if (cpuState.auto_freq_state)
            set_longrun_range(0, setLevel);
        else
            set_longrun_range(setLevel, setLevel);
#endif // LONGRUN		
        return;
}


/* it performs two checks:
 * - passing from 99% to 100% battery state (AC is plugged in) will mean
 *   to set the max cpu freq and disabling the autofreq (this works even
 *   if we are at 100% battery percentage while wmlaptop starts)
 * - passing from 100% to 99% after the pass 1 has been performed will
 *   mean to enable the autofreq
*/
void cpuSetParadisiac( )
{
	static bool pass1done = false;
	static u_int8 lastPercentage = 101;
	static bool saved_autoFreq = false;

	if( pass1done && powerState.isCharging == false )
	{
		pass1done = false;
		cpuState.auto_freq_state = saved_autoFreq;
	}
	
	if( ( lastPercentage == 99 || lastPercentage == 101 ) && powerState.percentage == 100 && powerState.isCharging )
	{
#ifndef LONGRUN
		/* set cpu freq to max */
		while( cpuState.actualFreq < cpuState.maxFreq )
		{
			cpuSetFreq( FREQ_INCREMENT, FREQ_NEXT );
			cpuReadFreq( );
		}
#endif		
                saved_autoFreq = cpuState.auto_freq_state;
		cpuState.auto_freq_state = false;
		pass1done = true;
#ifdef LONGRUN
                /* must do after autofreq state set false */
                cpuState.setLevelIdx = cpuState.nLongRunLevels-1;
                cpuEchoFreq();
#endif
	}
	
	lastPercentage = powerState.percentage;

        return;

}
