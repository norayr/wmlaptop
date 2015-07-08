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

#include "autoscript.h"

void checkAutoShutdown ( )
{
	static u_int8 lastPercentage = ~0x00;
	
	if( lastPercentage == args_autoShutdown + 1 &&
		powerState.percentage == args_autoShutdown )
	{
		/* exec the shutting down */
		char cmd[ strlen( SHUTDOWN_BIN ) + 24 + strlen( SHUTDOWN_ARGS ) ];
		
		sprintf( cmd, SHUTDOWN_BIN " +%d " SHUTDOWN_ARGS, args_shutdownDelay );
		
#ifndef DEBUG
		strcat( cmd, ">& /dev/null" );
#endif
		strcat( cmd, " &" );
		system( cmd );
	}
	
	lastPercentage = powerState.percentage;
	
}

/* this is declared on event.c: is set to true here,
 * and set to stop by stopFlashingLowBattery(). Making
 * this false will let wmlaptop to exit from the function
 * startFlashingLowBattery() */
extern bool flashingLowBatteryCycle;

void checkAutoAlarm ( )
{
	static u_int8 lastPercentage = ~0x00;
	pid_t pid;
	
	if( lastPercentage == args_autoAlarm + 1 &&
	    powerState.percentage == args_autoAlarm )
	{
		/* as we wanna play the speaker, we fork in two process:
		 * the child will play sounds and then will die,
		 * the father will show "LOW BATTERY" flashing, until the
		 * child is alive */
		flashingLowBatteryCycle = true;
		signal( SIGCHLD, stopFlashingLowBattery );
		pid = fork ();
		
		/* alarm */
		if( pid < 0 )
		{
			fprintf( stderr, "Error forking() to play speaker and flashing the dockapp\n" );
			fprintf( stderr, "Error: %s\n", strerror( errno ));
			fprintf( stderr, "WmLaptop will only play the alarm\n" );
			playAlarm();
		}
		
		/* child */
		if( pid == 0 )
		{
			playAlarm( );
			_exit( SUCCESS );
		}
		
		/* parent */
		if( pid > 0 )
		{
			startFlashingLowBattery( pid );
		}
		
		signal( SIGCHLD, SIG_DFL );
		draw_all( );
	}
	
	lastPercentage = powerState.percentage;
	
	return;
}




void play( int fd, int ms, int tone )
{
	int beep = ( ms << 16 ) + 1190000/tone;
	
	if( ioctl( fd, KDMKTONE, beep ) == -1 )
	{
		perror( "Failed to play a note calling ioctl()" );
		return;
	}
	
	usleep( 1000*ms );
}


void alarm_Simpson( int fd )
{
	play( fd, SEMIMINIMA, MI );
	play( fd, SEMIMINIMA, SOL );
	play( fd, CROMA+SEMIMINIMA, RE_ );
	
	play( fd, SEMICROMA + BISCROMA, DO_ );
	play( fd, CROMA + BISCROMA , RE_ );
	play( fd, SEMICROMA + BISCROMA, DO_ );
	play( fd, CROMA + BISCROMA, RE_ );
	play( fd, SEMICROMA + BISCROMA, MI_ );
	play( fd, SEMIMINIMA, SI );
	
	sleep( 1 );
}


void alarm_Ambulance ( int fd )
{
	int begin = DO_;
	int end = DO__;
	int j;
	
	for( j = begin; j < end; j+=10 )
		play( fd, 10, j );
	for( j = end; j > begin; j-=10 )
		play( fd, 10, j );
	
}


void alarm_Victory ( int fd )
{
	play( fd, SEMICROMA, SOL );
	play( fd, SEMICROMA, DO_ );
	play( fd, SEMICROMA, MI_ );
	play( fd, CROMA, SOL_ );
	play( fd, SEMICROMA, MI_ );
	play( fd, CROMA, SOL_ );
	usleep( 250000 );
}


void alarm_Highbip ( int fd )
{
	play( fd, SEMICROMA, DO__ );
	play( fd, SEMICROMA, DO__*2 );
}


void playAlarm (  )
{
	void (*song)(int) = NULL;
	int fd;
	int i;
		
	if(( fd = open( "/dev/console", O_NOCTTY ) ) == -1 )
	{
		perror( "Failed to open '/dev/console'" );
		return;
	}
	
	switch( args_alarmType )
	{
		case AUTOALARM_OFF:
			PRINTQ( stdout, "playAlarm() called, but autoAlarm is set to 'off'\n" );
			break;
		case 's':
			song = alarm_Simpson; break;
		case 'a':
			song = alarm_Ambulance; break;
		case 'v':
			song = alarm_Victory; break;
		case 'h':
			song = alarm_Highbip; break;
		default:
			PRINTQ( stdout, "playAlarm() called, but uknown alarm type !\n" );
			close( fd );
			return;
	}
	
	for( i = 0; i < args_alarmRepeat; i++ )
		song( fd );
	
	close( fd );
}
