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
 
#include "main.h"


/* cpu's stuff */
struct cpuFreq cpuState;
       u_int8 cpuLoad;

/* battery's stuff */
struct power_str powerState;

/* time counter */
       time_t secondsCounter;

/* command line's args */
       u_int32 args_cpuUpdate        = ARGSDEF_LEAVE;
       u_int32 args_batteryUpdate    = ARGSDEF_LEAVE;
       char *  args_XDisplayName     = NULL;
       bool    args_autoFreq         = ARGSDEF_LEAVE;
       u_int32 args_incrementalStep  = ARGSDEF_LEAVE;
       bool    args_startingFreq     = ARGSDEF_LEAVE;
       u_int32 args_maxFreq          = ARGSDEF_LEAVE;
       u_int32 args_minFreq          = ARGSDEF_LEAVE;
       bool    args_useSysProc       = ARGSDEF_LEAVE;
       char *  args_useFileMax       = NULL;
       char *  args_useFileMin       = NULL;
       char *  args_useFileSet       = NULL;
       bool    args_useApmAcpi       = ARGSDEF_LEAVE;
       bool    args_paradisiac       = ARGSDEF_LEAVE;
       char *  args_lfc              = NULL;
       char    args_autoShutdown     = AUTOSHUTDOWN_LEAVE;
       short   args_shutdownDelay    = AUTOSHUTDOWN_LEAVE;
       char    args_autoAlarm        = AUTOALARM_LEAVE;
       char    args_alarmRepeat      = AUTOALARM_LEAVE;
       char    args_alarmType        = AUTOALARM_LEAVE;
       bool    args_beQuiet          = ARGSDEF_LEAVE;
       bool    args_dontBlink100     = ARGSDEF_DONTBLINK100;
       bool    args_ttyMode          = ARGSDEF_LEAVE;
	   bool    args_skin             = ARGSDEF_LEAVE;

/* X's stuff */
struct mouseRegion mouse_region[MAX_MOUSE_REGION];

       Display   * display;
       Window      Root;
       Window      iconwin;
       Window      win;
       int         screen;
       int         x_fd;
       int         d_depth;
       XSizeHints  mysizehints;
       XWMHints    mywmhints;
       GC          NormalGC;
       XEvent      e;
struct XpmIcon     wmgen;
       Pixmap      pixmask;

       char        wmlaptop_mask_bits[64 * 64];
       int         wmlaptop_mask_width = 64;
       int         wmlaptop_mask_height = 64;



int main( int argc, char * argv[] )
{
	
	setArgs( argc, argv );
	
    if( args_ttyMode == false )
        init_display  (  );
	
	init_battery  (  );
	
	if( powerState.nBatt == 0 ) {
		fprintf( stderr, "No batteries found !\n");
        fprintf( stderr, "(I only can search in /sys/class/power_supply/* and in /proc/apm)\n");
        /*      free_and_exit( ERROR ); */
	}
	else
    if( args_ttyMode == false )
		PRINTQ( stdout, "Found %d battery's entry\n", powerState.nBatt );
	
	if( args_autoShutdown != AUTOSHUTDOWN_OFF && args_ttyMode == false )
	{
		PRINTQ( stdout, "Enable autoShutdown at %d%% battery state\n", args_autoShutdown );
		if( args_shutdownDelay != 0 )
		{
			PRINTQ( stdout, "Use a shutdown delay of %d minutes\n", args_shutdownDelay );
			PRINTQ( stdout, "The real poweroff will run at %d%% battery state + %d minutes\n",
			                 args_autoShutdown, args_shutdownDelay );
		}
	}
	
	init_cpuState (  );
    powerState.updater();
	
    if( args_ttyMode ) {
        fprintf( stdout, "\n" );
        fprintf( stdout, "Battery's percentage:      %d\n", powerState.percentage );
        if( powerState.isCharging )
            fprintf( stdout, "Battery ac_adapter:        plugged in\n" );
        fprintf( stdout, "Cpu frequency:             %d Khz\n", cpuState.actualFreq );
        fprintf( stdout, "\n" );
    }
    else
    {
        /* auto-freq mouse region */
        AddMouseRegion( MREGION_AUTOFREQ, 7, 36, 55, 46 );
        /* cpu frequency left arrow */
        AddMouseRegion( MREGION_MINFREQ, 4, 52, 7, 59 );
        /* cpu frequency right arrow */
        AddMouseRegion( MREGION_MAXFREQ, 55, 52, 58, 59 );
    }	
	event_handler (  );

	
	return SUCCESS;
}


void free_and_exit( int code )
{
	
	if( powerState.batteryVector )
	{
		while( powerState.nBatt > 0 )
			freeBatteryVector( 0 );
		free( powerState.batteryVector );
	}
	
	ACPI_lfcArgs_free ( );
	
	if( display )
		XCloseDisplay( display );
	
	exit( code );
}


/* prints help and exits */
void usage ( char * prog, char * unknowOpt )
{
	fprintf( stderr, "%s dockapp. An LdP-Production (version %s, build %d)\n",
	         PROGNAME, VERSION, BUILD_NR );
	fprintf( stderr, "It shows batteries' and cpu's state\n" );
	fprintf( stderr, "\n");
	fprintf( stderr, "[usage]: %s [options]\n", prog );
	fprintf( stderr, "\n");
	fprintf( stderr, "[X-options]:\n");
	fprintf( stderr, "   --display=NAME             set the display to open\n");
	fprintf( stderr, "   --dontblink                if you dont want to see \"CPULOAD\" blinking at 100%%\n");
	fprintf( stderr, "   --blink                    if you do want it and it was compiled off\n");
	fprintf( stderr, "   --skin=N                   possible options are 0 - 1 - 2:\n");
	fprintf( stderr, "                              0 - default wmlaptop skin\n");
	fprintf( stderr, "                              1 - default wmlaptop skin with AUTOFREQ green\n");
	fprintf( stderr, "                              2 - alternative skin\n");
	fprintf( stderr, "[CPU-options]:\n");
	fprintf( stderr, "   --auto-freq=on|off         start with auto-scaling on|off\n");
	fprintf( stderr, "   --incremental-step=N       increment or decrement the cpu frequence by N\n");
	fprintf( stderr, "                              Khz per second when in auto-scaling mode\n");
	fprintf( stderr, "   --starting-freq=min|max    set the cpu frequence to the min|max possible\n");
	fprintf( stderr, "                              when start\n");
	fprintf( stderr, "   --max-freq=N               set manually the max and the min cpu frequence\n");
	fprintf( stderr, "   --min-freq=N               the dockapp must use as limit (in Khz)\n");
	fprintf( stderr, "   --use-sys                  use /sys/devices/system/cpu/cpu0/cpufreq/ to\n");
	fprintf( stderr, "                              control the cpu frequence scaling\n");
	fprintf( stderr, "   --use-proc                 use /proc/sys/cpu/0/ instead (OLD 2.4)\n");
	fprintf( stderr, "   --use-file-maxfreq=FILE    use FILE to read the maximum cpu frequence\n");
	fprintf( stderr, "   --use-file-minfreq=FILE    the minimum cpu frequence\n");
	fprintf( stderr, "   --use-file-setfreq=FILE    and to set the cpu frequence\n");
	fprintf( stderr, "[BATTERY-options]:\n");
	fprintf( stderr, "   --apm                      use APM kernel interface to monitoring battery\n");
	fprintf( stderr, "   --acpi                     use the modern ACPI interface instead\n");
	fprintf( stderr, "   --use-apm                  use APM, or quit if it fails\n");
	fprintf( stderr, "   --use-acpi                 use ACPI, or quit if it fails\n");
	fprintf( stderr, "   --lfc=N                    when using ACPI, use 'last full capacity' instead\n");
	fprintf( stderr, "                              of 'design capacity' tag in the infos' files\n" );
	fprintf( stderr, "                              (N is the number of battery)\n" );
	fprintf( stderr, "[AUTOSCRIPT-options]:\n");
	fprintf( stderr, "   --auto-shutdown=off|N      enable autoshutdown when battery percentage is at\n");
	fprintf( stderr, "                              N%% (and AC adapter is not plugged in), or disable\n");
	fprintf( stderr, "                              it with 'off'. ('/sbin/shutdown' is used)\n");
	fprintf( stderr, "   --shutdown-delay=N         insert a delay in minutes to the shutdown call\n");
	fprintf( stderr, "   --auto-alarm=off|N         enable the speaker to play a alarm when battery\n");
	fprintf( stderr, "                              percentage is at N%%. (or diseable it with 'off')\n");
	fprintf( stderr, "   --alarm-type=s|a|v|h       choose one of this alarm:\n");
	fprintf( stderr, "                                s - simpson's bell\n");
	fprintf( stderr, "                                a - ambulance's siren\n");
	fprintf( stderr, "                                v - victory jingle\n");
	fprintf( stderr, "                                h - high-bell sounds\n");	
	fprintf( stderr, "   --alarm-repeat=N           repeat the choosen alarm N times\n");
	fprintf( stderr, "[GENERAL-options]:\n");
	fprintf( stderr, "   --cpu-update=N             how often, in milliseconds, to update CPU display\n");
	fprintf( stderr, "   --battery-update=N         how often, in milliseconds, update battery state\n");
	fprintf( stderr, "   -p   --play                play alarm and exit\n");
	fprintf( stderr, "   -q   --quiet               do not print messages and warnings\n");
	fprintf( stderr, "   -d   --default             show the default compiled settings and exit\n");
    fprintf( stderr, "   -t   --console             run wmlaptop in console:\n");
    fprintf( stderr, "                              it shows you battery and cpu info and exit\n");
	fprintf( stderr, "   -v   --version             show the version, and exit\n");
	fprintf( stderr, "   -h   --help                show this message, and exit\n");
	fprintf( stderr, "\n");
	
	if( unknowOpt ) {
		fprintf( stderr, "Unkown Options: %s\n", unknowOpt );
		fprintf( stderr, "\n");
		free_and_exit ( ERROR );
	}
	
	free_and_exit( SUCCESS );
}


void version ( )
{
	fprintf( stderr, "%s dockapp. An LdP-Production\n", PROGNAME );
	fprintf( stderr, "Version %s, build %d\n", VERSION, BUILD_NR );
	fprintf( stderr, "\n");
	fprintf( stderr, "Author: Giacomo, alias ]Matic[, Galilei:  matic at libero dot it\n");
	fprintf( stderr, "Author: Lorenzo, alias ]StClaus[, Marcon: stclaus at libero dot it\n");
	fprintf( stderr, "\n");
	free_and_exit( SUCCESS );
}




void setArgs( int argc, char ** argv )
{
	register int i;
	char *     ptr;
	bool       playAlarmAndExit = false;
	
	for( i = 1; i < argc; i++ )
	{
		/* X Connection */
		if( !strncmp( argv[i], "--display=", 10 )) {
			EXIT_IF_ALREADY_SET( args_XDisplayName, NULL, "XDisplayName" );
			args_XDisplayName = strchr( argv[i], '=' );
			args_XDisplayName++;
			continue;
		}
		
		if( !strcmp( argv[i], "--dontblink" )) {
			args_dontBlink100 = true;
			continue;
		}
		
		if( !strcmp( argv[i], "--blink" )) {
			args_dontBlink100 = false;
			continue;
		}

		if( !strncmp( argv[i], "--skin=", 7 )) {
			EXIT_IF_ALREADY_SET( args_skin, ARGSDEF_LEAVE, "skin" );
			switch( argv[i][7] ) {
				case '0':
					args_skin = SKIN_DEFAULT;
					break;
				case '1':
					args_skin = SKIN_DEFAULT_GREEN;
					break;
				case '2':
					args_skin = SKIN_OTHER;
					break;
				default:
					fprintf( stderr, "Unknown value for option '--skin'. Try --help\n" );
					free_and_exit ( ERROR );
			}
			continue;
		}

		
		if( !strcmp( argv[i], "--auto-freq=on" )) {
			EXIT_IF_ALREADY_SET( args_autoFreq, ARGSDEF_LEAVE, "autoFreq" );
			args_autoFreq = AUTOFREQ_ON;
			continue;
		}
		
		if( !strcmp( argv[i], "--auto-freq=off" )) {
			EXIT_IF_ALREADY_SET( args_autoFreq, ARGSDEF_LEAVE, "autoFreq" );
			args_autoFreq = AUTOFREQ_OFF;
			continue;
		}
		
		if( !strncmp( argv[i], "--incremental-step=", 19 ) ) {
			EXIT_IF_ALREADY_SET( args_incrementalStep, ARGSDEF_LEAVE, "incrementalStep" );
			ptr = strchr( argv[i], '=' );
			ptr++;
			args_incrementalStep = atoi( ptr );
			WARNING_IS_SET_TO_ZERO( args_incrementalStep, "incrementalStep" );
			continue;
		}
		
		if( !strcmp( argv[i], "--starting-freq=min" ) ) {
			EXIT_IF_ALREADY_SET( args_startingFreq, ARGSDEF_LEAVE, "startingFreq" );
			args_startingFreq = STARTINGFREQ_MIN;
			continue;
		}
		
		if( !strcmp( argv[i], "--starting-freq=max" ) ) {
			EXIT_IF_ALREADY_SET( args_startingFreq, ARGSDEF_LEAVE, "startingFreq" );
			args_startingFreq = STARTINGFREQ_MAX;
			continue;
		}
		
		if( !strncmp( argv[i], "--max-freq=", 11 ) ) {
			EXIT_IF_ALREADY_SET( args_maxFreq, ARGSDEF_LEAVE, "maxFreq" );
			ptr = strchr( argv[i], '=' );
			ptr++;
			args_maxFreq = atoi( ptr );
			WARNING_IS_SET_TO_ZERO( args_maxFreq, "maxFreq" );
			continue;
		}
		
		if( !strncmp( argv[i], "--min-freq=", 11 ) ) {
			EXIT_IF_ALREADY_SET( args_minFreq, ARGSDEF_LEAVE, "minFreq" );
			ptr = strchr( argv[i], '=' );
			ptr++;
			args_minFreq = atoi( ptr );
			WARNING_IS_SET_TO_ZERO( args_minFreq, "minFreq" );
			continue;
		}
		
		if( !strcmp( argv[i], "--use-sys" )) {
			EXIT_IF_ALREADY_SET( args_useSysProc, ARGSDEF_LEAVE, "useSysProc" );
			args_useSysProc = USESYSPROC_SYS;
			continue;
		}
		
		if( !strcmp( argv[i], "--use-proc" )) {
			EXIT_IF_ALREADY_SET( args_useSysProc, ARGSDEF_LEAVE, "useSysProc" );
			args_useSysProc = USESYSPROC_PROC;
			continue;
		}
		
		if( !strncmp( argv[i], "--use-file-maxfreq=", 19 )) {
			EXIT_IF_ALREADY_SET( args_useFileMax, NULL, "useFileMax" );
			args_useFileMax = strchr( argv[i], '=' );
			args_useFileMax++;
			continue;
		}
		
		if( !strncmp( argv[i], "--use-file-minfreq=", 19 )) {
			EXIT_IF_ALREADY_SET( args_useFileMin, NULL, "useFileMin" );
			args_useFileMin = strchr( argv[i], '=' );
			args_useFileMin++;
			continue;
		}
		
		if( !strncmp( argv[i], "--use-file-setfreq=", 19 )) {
			EXIT_IF_ALREADY_SET( args_useFileSet, NULL, "useFileSet" );
			args_useFileSet = strchr( argv[i], '=' );
			args_useFileSet++;
			continue;
		}
		
		if( !strncmp( argv[i], "--paradisiac=", 13 ) ) {
			EXIT_IF_ALREADY_SET( args_paradisiac, ARGSDEF_LEAVE, "paradisiac" );
			if( !strcmp( &argv[i][13], "on" ) ) {
				args_paradisiac = PARADISIAC_ON;
				continue;
			}
			if( !strcmp( &argv[i][13], "off" ) ) {
				args_paradisiac = PARADISIAC_OFF;
				continue;
			}
		}
		
		if( !strcmp( argv[i], "--apm" )) {
			EXIT_IF_ALREADY_SET( args_useApmAcpi, ARGSDEF_LEAVE, "useApmAcpi" );
			args_useApmAcpi = USEAPMACPI_APM;
			continue;
		}
		
		if( !strcmp( argv[i], "--acpi" )) {
			EXIT_IF_ALREADY_SET( args_useApmAcpi, ARGSDEF_LEAVE, "useApmAcpi" );
			args_useApmAcpi = USEAPMACPI_ACPI;
			continue;
		}
		
		if( !strcmp( argv[i], "--use-apm" )) {
			EXIT_IF_ALREADY_SET( args_useApmAcpi, ARGSDEF_LEAVE, "useApmAcpi" );
			args_useApmAcpi = USEAPMACPI_ONLYAPM;
			continue;
		}
		
		if( !strcmp( argv[i], "--use-acpi" )) {
			EXIT_IF_ALREADY_SET( args_useApmAcpi, ARGSDEF_LEAVE, "useApmAcpi" );
			args_useApmAcpi = USEAPMACPI_ONLYACPI;
			continue;
		}
		
		if( !strncmp( argv[i], "--lfc=", 6 ) ) {
			if( ACPI_lfcArgs_add( atoi( &argv[i][6] ) ) ) {
				fprintf( stderr, "null argument or multiple declaration of it in \"-l\" option (try -h).\n" );
				free_and_exit ( ERROR );
			}
			continue;
		}
		
		if( !strncmp( argv[i], "--auto-shutdown=", 16 ) ) {
			EXIT_IF_ALREADY_SET( args_autoShutdown, AUTOSHUTDOWN_LEAVE, "autoShutDown" );
			if( !strcmp( &argv[i][16], "off" ) )
				args_autoShutdown = AUTOSHUTDOWN_OFF;
			else
				args_autoShutdown = atoi( &argv[i][16] );
			continue;
		}
		
		if( !strncmp( argv[i], "--shutdown-delay=", 17 ) ) {
			EXIT_IF_ALREADY_SET( args_shutdownDelay, AUTOSHUTDOWN_LEAVE, "shutDownDelay" );
			args_shutdownDelay = atoi( &argv[i][17] );
			continue;
		}
		
		if( !strncmp( argv[i], "--auto-alarm=", 13 ) ) {
			EXIT_IF_ALREADY_SET( args_autoAlarm, AUTOALARM_LEAVE, "autoAlarm" );
			if( !strcmp( &argv[i][13], "off" ) )
				args_autoAlarm = AUTOALARM_OFF;
			else
				args_autoAlarm = atoi( &argv[i][13] );
			continue;
		}
		
		if( !strncmp( argv[i], "--alarm-type=", 13 ) ) {
			EXIT_IF_ALREADY_SET( args_alarmType, AUTOALARM_LEAVE, "alarmType" );
			args_alarmType = argv[i][13];
			if( argv[i][13] != 's' && argv[i][13] != 'a' && 
			    argv[i][13] != 'v' && argv[i][13] != 'h' ) {
				fprintf( stderr, "Only s a v h are allowed to use with '--alarm-type='\n" );
				fprintf( stderr, "Invalid option: %s (%c)\n", argv[i], argv[i][13] );
				free_and_exit( ERROR );
			}
			continue;
		}
		
		if( !strncmp( argv[i], "--alarm-repeat=", 15 ) ) {
			EXIT_IF_ALREADY_SET( args_alarmRepeat, AUTOALARM_LEAVE, "alarmRepeat" );
			args_alarmRepeat = atoi( &argv[i][15] );
			if( args_alarmRepeat == 0 ) {
				PRINTQ( stderr, "Can't set alarmRepeat to 0: using 1 instead\n" );
				args_alarmRepeat = 1;
			}
			continue;
		}
		
		if( !strncmp( argv[i], "--cpu-update=", 13 ) ) {
			EXIT_IF_ALREADY_SET( args_cpuUpdate, ARGSDEF_LEAVE, "cpuUpdate" );
			ptr = strchr( argv[i], '=' );
			ptr++;
			args_cpuUpdate = atoi( ptr );
			WARNING_IS_SET_TO_ZERO( args_maxFreq, "cpuUpdate" );
			continue;
		}
		
		if( !strncmp( argv[i], "--battery-update=", 17 ) ) {
			EXIT_IF_ALREADY_SET( args_batteryUpdate, ARGSDEF_LEAVE, "batteryUpdate" );
			ptr = strchr( argv[i], '=' );
			ptr++;
			args_batteryUpdate = atoi( ptr );
			WARNING_IS_SET_TO_ZERO( args_maxFreq, "batteryUpdate" );
			continue;
		}
		
		if( !strcmp( argv[i], "-p" ) || !strcmp( argv[i], "--play" )) {
			EXIT_IF_ALREADY_SET( playAlarmAndExit, false, "playAlarmAndExit" );
			playAlarmAndExit = true;
			continue;
		}
		
		if( !strcmp( argv[i], "-q" ) || !strcmp( argv[i], "--quiet" )) {
			EXIT_IF_ALREADY_SET( args_beQuiet, ARGSDEF_LEAVE, "beQuiet" );
			args_beQuiet = !ARGSDEF_BEQUIET;
			continue;
		}
		
		if( !strcmp( argv[i], "-d" ) || !strcmp( argv[i], "--default" ))
			defaultSettings ( argv[0] );
		
        if( !strcmp( argv[i], "-t" ) || !strcmp( argv[i], "--console" )) {
            EXIT_IF_ALREADY_SET( args_ttyMode, ARGSDEF_LEAVE, "ttyMode" );
            args_ttyMode = !ARGSDEF_TTYMODE;
            continue;
        }

		if( !strcmp( argv[i], "-v" ) || !strcmp( argv[i], "--version" ))
			version( );
		
		if( !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "--help" ))
			usage( argv[0], NULL );
		
		usage( argv[0], argv[i] );
	}
	
	/* setting default values not set by command line */
	SET_DEFAULT( args_cpuUpdate,       ARGSDEF_LEAVE,      ARGSDEF_CPUUPDATE );
	SET_DEFAULT( args_batteryUpdate,   ARGSDEF_LEAVE,      ARGSDEF_BATTERYUPDATE );
	SET_DEFAULT( args_XDisplayName,    NULL,               ARGSDEF_XDISPLAYNAME );
	SET_DEFAULT( args_autoFreq,        ARGSDEF_LEAVE,      ARGSDEF_AUTOFREQ );
	SET_DEFAULT( args_incrementalStep, ARGSDEF_LEAVE,      ARGSDEF_INCREMENTALSTEP );
	SET_DEFAULT( args_startingFreq,    ARGSDEF_LEAVE,      ARGSDEF_STARTINGFREQ );
	SET_DEFAULT( args_maxFreq,         ARGSDEF_LEAVE,      ARGSDEF_MAXFREQ );
	SET_DEFAULT( args_minFreq,         ARGSDEF_LEAVE,      ARGSDEF_MINFREQ );
	SET_DEFAULT( args_useSysProc,      ARGSDEF_LEAVE,      ARGSDEF_USESYSPROC );
	SET_DEFAULT( args_useFileMax,      NULL,               ARGSDEF_USEFILEMAX );
	SET_DEFAULT( args_useFileMin,      NULL,               ARGSDEF_USEFILEMIN );
	SET_DEFAULT( args_useFileSet,      NULL,               ARGSDEF_USEFILESET );
	SET_DEFAULT( args_paradisiac,      ARGSDEF_LEAVE,      ARGSDEF_PARADISIAC );
	SET_DEFAULT( args_useApmAcpi,      ARGSDEF_LEAVE,      ARGSDEF_USEAPMACPI );
	SET_DEFAULT( args_lfc,             NULL,               ARGSDEF_LFC );
	SET_DEFAULT( args_autoShutdown,    AUTOSHUTDOWN_LEAVE, ARGSDEF_AUTOSHUTDOWN );
	SET_DEFAULT( args_shutdownDelay,   AUTOSHUTDOWN_LEAVE, ARGSDEF_SHUTDOWNDELAY );
	SET_DEFAULT( args_autoAlarm,       AUTOALARM_LEAVE,    ARGSDEF_AUTOALARM );
	SET_DEFAULT( args_alarmType,       AUTOALARM_LEAVE,    ARGSDEF_ALARMTYPE );
	SET_DEFAULT( args_alarmRepeat,     AUTOALARM_LEAVE,    ARGSDEF_ALARMREPEAT );
	SET_DEFAULT( args_beQuiet,         ARGSDEF_LEAVE,      ARGSDEF_BEQUIET );
    SET_DEFAULT( args_ttyMode,         ARGSDEF_LEAVE,      ARGSDEF_TTYMODE );
	SET_DEFAULT( args_skin,            ARGSDEF_LEAVE,      ARGSDEF_SKIN );
	
	/* add each "--lfc" contained in args_lfc */
	if( args_lfc != NULL )
	{
		u_int16 len = strlen( args_lfc );
		
		for( i = 0; i < len - 6; i++ )
			if( !strncmp( &args_lfc[i], "--lfc=", 6 ) )
				if( ACPI_lfcArgs_add( atoi( &args_lfc[i+6] ) ) ) {
					fprintf( stderr, "null argument or multiple declaration of it in \"-l\" option (try -h).\n" );
					free_and_exit ( ERROR );
				}
	}
	
	/* if user wants to play the alarm and exit, do it */
	if( playAlarmAndExit )
	{
		playAlarm( );
		free_and_exit( ERROR );
	}
}



void defaultSettings ( char * progname )
{
	char * ptr;
	
	fprintf( stdout, "The default compiled settings are:\n");
	fprintf( stdout, "[X-options]:\n");
	fprintf( stdout, "   --display=%s\n",  ARGSDEF_XDISPLAYNAME == NULL ? "NULL" : ARGSDEF_XDISPLAYNAME );
	fprintf( stdout, "   --skin=%d\n", ARGSDEF_SKIN );
	fprintf( stdout, "[CPU-options]:\n");
	fprintf( stdout, "   --cpu-update=%d\n", ARGSDEF_CPUUPDATE );
	fprintf( stdout, "   --auto-freq=%s\n", ARGSDEF_AUTOFREQ == AUTOFREQ_ON ? "on" : "off" );
	fprintf( stdout, "   --incremental-step=%d\n", ARGSDEF_INCREMENTALSTEP );
	if( ARGSDEF_STARTINGFREQ != ARGSDEF_LEAVE )
		fprintf( stdout, "   --starting-freq=%s\n", ARGSDEF_STARTINGFREQ == STARTINGFREQ_MIN ? "min" : "max" );
	if( ARGSDEF_MAXFREQ != ARGSDEF_LEAVE )
		fprintf( stdout, "   --max-freq=%d\n", ARGSDEF_MAXFREQ );
	if( ARGSDEF_MINFREQ != ARGSDEF_LEAVE )
		fprintf( stdout, "   --min-freq=%d\n", ARGSDEF_MINFREQ );
	if( ARGSDEF_USESYSPROC == USESYSPROC_SYS )
		fprintf( stdout, "   --use-sys\n");
	else
		fprintf( stdout, "   --use-proc\n");
	
	ptr = ARGSDEF_USEFILEMAX;
	if( ptr )
		fprintf( stdout, "   --use-file-maxfreq=%s\n", ptr );
	ptr = ARGSDEF_USEFILEMIN;
	if( ptr )
		fprintf( stdout, "   --use-file-minfreq=%s\n", ptr );
	ptr = ARGSDEF_USEFILESET;
	if( ptr )
		fprintf( stdout, "   --use-file-setfreq=%s\n", ptr );
	fprintf( stdout, "[BATTERY-options]:\n");
	fprintf( stdout, "   --battery-update=%d\n", ARGSDEF_BATTERYUPDATE );
	switch ( ARGSDEF_USEAPMACPI )
	{
		case USEAPMACPI_APM:
			fprintf( stdout, "   --apm\n"); break;
		case USEAPMACPI_ACPI:
			fprintf( stdout, "   --acpi\n"); break;
		case USEAPMACPI_ONLYAPM:
			fprintf( stdout, "   --use-apm\n"); break;
		case USEAPMACPI_ONLYACPI:
			fprintf( stdout, "   --use-acpi\n"); break;
	}
	
	ptr = ARGSDEF_LFC;
	if( ptr )
		fprintf( stdout, "   %s\n", ptr );
	
	if( ARGSDEF_AUTOSHUTDOWN != AUTOSHUTDOWN_OFF )
		fprintf( stdout, "   --auto-shutdown=%d\n", ARGSDEF_AUTOSHUTDOWN );
	else
		fprintf( stdout, "   --auto-shutdown=off\n");
	fprintf( stdout, "   --shutdown-delay=%d\n", ARGSDEF_SHUTDOWNDELAY );
	
	if( ARGSDEF_AUTOALARM != AUTOALARM_OFF )
		fprintf( stdout, "   --auto-alarm=%d\n", ARGSDEF_AUTOALARM );
	else
		fprintf( stdout, "   --auto-alarm=off\n");
	fprintf( stdout, "   --alarm-type=%c\n", ARGSDEF_ALARMTYPE );
	fprintf( stdout, "   --alarm-repeat=%d\n", ARGSDEF_ALARMREPEAT );
	
	free_and_exit( SUCCESS );	
}
