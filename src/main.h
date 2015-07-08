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

#ifndef __MAIN_H__
#define __MAIN_H__

#include "version.h"
#include "argsConfig.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>
#include <errno.h>
#include <dirent.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include <sys/signal.h>
#include <sys/wait.h>

/* those includes are needed by alarm */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/kd.h>



#define  XPM_NAME      wmlaptop_xpm


typedef unsigned char bool;

#ifndef true
# define true 1
#endif

#ifndef false
# define false 0
#endif

#define    ERROR        1
#define    SUCCESS      0


typedef unsigned char u_int8;
typedef unsigned short u_int16;
typedef unsigned int u_int32;



/****************************************
 * Every global variable of the program *
 * is declared in main.c, but here is   *
 * is declared as 'extern'              *
 ****************************************/
 
extern char ** wmlaptop_xpm;



/* it keeps track of passing time:
 * updated with time(NULL) at every cycle */
extern        time_t secondsCounter;



/********************
 * BATTERIES' STUFF *
 *****************************************************************
 * wmlaptop can read data about battery supporting the kernel    *
 * interface based on APM or the newest ACPI. in the case where  *
 * it is possible to use both of them, only one will be used.    *
 * this last one will be selected at the start.                  *
 *****************************************************************/
 
#define    SUPPORT_ACPI      0;
#define    SUPPORT_APM       1;
typedef    unsigned char     supportType; 

/*****************************************************************
 * For every battery present in the system will be allocated a   *
 * structure containing following informations                   *
 * in case you use APM, only a structure will be allocated,      *
 * because data will be read from /proc/apm, and they'll be      *
 * referring as an average of all the batteries                  *
 *                                                               *
 * present:     true if battery is present, false otherwise      *
 * error:       true if reading the info's file report an error: *
 *              this battery wont be used until present will     *
 *              become false (only ACPI)                         *
 * counter:     the counter for the battery (each battery has    *
 *              unique number ) (only ACPI)                      *
 * stateFile:   file path where to read information about actual *
 *              state of the battery (only ACPI)                 *
 * infoFile:    file path where to read general informations     *
 *              about the battery (only ACPI )                   *
 * capacity:    battery capacity (only ACPI)                     *
 * actualState: remaining battery capacity (only ACPI)           *
 * percentage:  remaining battery percentage (calculated with    *
 *              actualState divided by capacity  with ACPI       *
 *              reading /proc/apm con APM)                       *
 * useLFC:      use 'last full capacity' tag instead of 'design  *
 *              capacity' reading the information from info file *
 *              (only ACPI)                                      *
 * filler:      pointer to the function that will fill the       *
 *              fields (different if you're using APM or ACPI)   *
 *****************************************************************/
typedef struct battery_str * battery;

struct battery_str
{
	bool    present;
	bool    error;
	u_int8  counter;
	char    stateFile[52];
	char    infoFile[52];
	u_int32 capacity;
	u_int32 actualState;
	u_int16 percentage;
	bool    useLFC;
	bool(*filler)(battery);
};

/*****************************************************************
 * this structure is used for keeping track of all batteries and *
 * it calculate estimated remaining time                         *
 *                                                               *
 * type:          it can be SUPPORT_ACPI or SUPPORT_APM. it is   *
 *                set at the start, and it cannot change during  *
 *                program execution                              *
 * nBatt:         it shows the number of elements of             *
 *                BatteryVector; it is also the number of        *
 *                installed batteries, in case you're using ACPI *
 * batteryVector: vector containing all single battery           *
 *                informations (ACPI) or all batteries (APM)     *
 * percentage:    average of all batteries (nBatt) presents in   *
 *                in batteryVector. it's the value that will be  *
 *                showed in the dockapp                          *
 * remainingTime: remaining battery time, estimated calculating  *
 *                how much has power the laptop has used in the  *
 *                last ten minutes (this value could be modified)*
 * isCharging:    true if battery charger is plugged in.         *
 * updater:       pointer to function that is called             *
 *                (periodically) to update battery status.       *
 *                function that points is different if you're    *
 *                using ACPI or APM                              *
 *****************************************************************/
extern struct power_str powerState;

struct power_str
{
	supportType   type;
	u_int8        nBatt;
	battery     * batteryVector;
	u_int8        percentage;
	u_int16       remainingTime;
	bool          isCharging;
	void(*updater)();
};


/*******************
 * CPUS' STUFF     *
 *****************************************************************
 * wmlaptop let the user to change cpu frequency on the fly, by  *
 * clicking on two arrows, but it can also handle by itself this *
 * responibility according with cpu load. it can decrease or     *
 * decrease cpu frequency when we're not doing an heavy use of it*
 * or increase it at its maximum when cpu load is at 100%.       *
 * in every case wmlaptop shows on its display the actual cpu    *
 * load, read and stored in the global variable cpuLoad at every *
 * events cycle.                                                 *
 *****************************************************************/
extern        u_int8  cpuLoad;

/*****************************************************************
 * global variable cpuState is a structure of this type, it      *
 * stores cpu status. here are the fields explained:             *
 * auto_freq_state: true if wmlaptop automatically handles cpu   *
 *                  frequency: AUTO-FREQ will be shown in red    *
 *                  color                                        *
 * minFreq:    they value is assigned at start with              *
 * maxFreq:    init_cpuState(), and they keeps the max and the   *
 *             min frequency value (in Khz) for this computer    *
 * setFreqFile:pointer to the path to use for writing/reading    *
 *             cpu frequency                                     *
 * setFreq:    the function to change frequency will use this    *
 *             variable to do its calculations: then it will call*
 *             cpuEchoFreq that will write this value in the     *
 *             right file                                        *
 * stepFreq:   in auto_freq mode, wmlaptop will increase or      *
 *             decrease cpu frequency by this value at every time*
 *             unit. it will be user-settable                    *
 * actualFreq: updated at every events cycle, it takes as value  *
 *             the actual cpu frequency                          *
 *****************************************************************/
struct cpuFreq
{
	bool    auto_freq_state;
	u_int32 actualFreq;
#ifndef LONGRUN
	u_int32 minFreq;
	u_int32 maxFreq;
	char *  setFreqFile;
	u_int32 setFreq;
	u_int32 stepFreq;
#else
        u_int16 *longRunLevels;
        u_int16 nLongRunLevels;
	u_int16 actualLevelIdx;
        u_int16 setLevelIdx;
        u_int16 stepLevelIdx;
#endif
};

extern struct cpuFreq cpuState;



/*******************
 * XLIB' STUFF     *
 *****************************************************************
 * code taken here and there for creating a dockapp with xlib    *
 *****************************************************************/
 
struct XpmIcon
{
	Pixmap			pixmap;
	Pixmap			mask;
	XpmAttributes	 attributes;
	
};

/* the X connection */
extern        Display   * display;
extern        Window      Root;
extern        Window      iconwin;
extern        Window      win;
extern        int         screen;
extern        int         x_fd;
extern        int         d_depth;
extern        XSizeHints  mysizehints;
extern        XWMHints    mywmhints;
extern        GC          NormalGC;
extern        XEvent      e;
extern struct XpmIcon     wmgen;
extern        Pixmap      pixmask;

/* the main image */
extern        char        wmlaptop_mask_bits[64 * 64];
extern        int         wmlaptop_mask_width;
extern        int         wmlaptop_mask_height;

  /*****************/
 /* Mouse Regions */
/*****************/

#define    MAX_MOUSE_REGION    3

#define    MREGION_AUTOFREQ    0
#define    MREGION_MINFREQ     1
#define    MREGION_MAXFREQ     2

struct mouseRegion
{
	u_int8     top;
	u_int8     bottom;
	u_int8     left;
	u_int8     right;
};

extern struct mouseRegion mouse_region[MAX_MOUSE_REGION];


#include "main.h"
#include "init.h"
#include "event.h"
#include "draw.h"
#include "battery.h"
#include "cpu.h"
#include "autoscript.h"


/*******************
 * alpha and omega *
 *******************/
int   main  ( int , char **  );

/* this one takes as argument the exit's code, but
 * before exiting, it frees any allocated memory */
void  free_and_exit ( int );


/********************
 * HELP and VERSION *
 ********************/

void usage ( char * , char * );

void version ( );



/*******************
 * ARGS STUFF      *
 *****************************************************************
 * wmlaptop arguments                                            *
 *****************************************************************/

void setArgs( int, char ** );

/* this shows the default compiled settings and exit
 * takes as argument argv[0] */
void defaultSettings ( char * );

#define    ARGSDEF_LEAVE              0

/* update frequencies, in milliseconds; cpu must < battery */
#ifndef    ARGSDEF_CPUUPDATE
# define    ARGSDEF_CPUUPDATE         80
#endif
#ifndef    ARGSDEF_BATTERYUPDATE
# define    ARGSDEF_BATTERYUPDATE     5000
#endif

extern u_int32 args_cpuUpdate;
extern u_int32 args_batteryUpdate;

/* name of diplay to open */
#ifndef    ARGSDEF_XDISPLAYNAME
# define    ARGSDEF_XDISPLAYNAME      NULL
#endif
extern char *  args_XDisplayName;

/* activate auto frequency scaling at start */
#define    AUTOFREQ_ON                1
#define    AUTOFREQ_OFF               2

#ifndef    ARGSDEF_AUTOFREQ
# define    ARGSDEF_AUTOFREQ          AUTOFREQ_ON
#endif
extern bool    args_autoFreq;

/* incremental auto-scaling step */
#ifndef    ARGSDEF_INCREMENTALSTEP
# define    ARGSDEF_INCREMENTALSTEP   100000
#endif
extern u_int32 args_incrementalStep;

/* to set at start cpu at max or at min frequency */
#define    STARTINGFREQ_MAX           1
#define    STARTINGFREQ_MIN           2

#ifndef    ARGSDEF_STARTINGFREQ
# define    ARGSDEF_STARTINGFREQ      ARGSDEF_LEAVE
#endif
extern bool    args_startingFreq;

/* maximum usable frequency */
#ifndef    ARGSDEF_MAXFREQ
# define    ARGSDEF_MAXFREQ           ARGSDEF_LEAVE
#endif
extern u_int32 args_maxFreq;


/* minimum usable frequency */
#ifndef    ARGSDEF_MINFREQ
# define    ARGSDEF_MINFREQ           ARGSDEF_LEAVE
#endif
extern u_int32 args_minFreq;


/* stay at max freq when 100% and AC_Adapter is in */
#define    PARADISIAC_ON              1
#define    PARADISIAC_OFF             2

#ifndef    ARGSDEF_PARADISIAC
# define    ARGSDEF_PARADISIAC        PARADISIAC_ON
#endif
extern bool    args_paradisiac;

/* it uses /sys/.. o /proc/.. */
#define    USESYSPROC_SYS             1
#define    USESYSPROC_PROC            2

#ifndef    ARGSDEF_USESYSPROC
# define    ARGSDEF_USESYSPROC        USESYSPROC_SYS
#endif
extern bool    args_useSysProc;

/* it uses a file for max frequency */
#ifndef    ARGSDEF_USEFILEMAX
# define    ARGSDEF_USEFILEMAX        NULL
#endif

extern char *  args_useFileMax;

/* it uses a file for min frequency */
#ifndef    ARGSDEF_USEFILEMIN
# define    ARGSDEF_USEFILEMIN        NULL
#endif

extern char *  args_useFileMin;

/* it uses file to set the frequency */
#ifndef    ARGSDEF_USEFILESET
# define    ARGSDEF_USEFILESET        NULL
#endif

extern char *  args_useFileSet;

/* start using APM or ACPI */
#define    USEAPMACPI_APM             1
#define    USEAPMACPI_ACPI            2
#define    USEAPMACPI_ONLYAPM         3
#define    USEAPMACPI_ONLYACPI        4

#ifndef    ARGSDEF_USEAPMACPI
# define    ARGSDEF_USEAPMACPI        USEAPMACPI_ACPI
#endif

extern bool    args_useApmAcpi;


/* for the last full capacity tag */
#ifndef    ARGSDEF_LFC
# define    ARGSDEF_LFC               NULL
#endif

extern char  * args_lfc;


/* enable auto shutdown */
#define    AUTOSHUTDOWN_LEAVE         -1
#define    AUTOSHUTDOWN_OFF           -2

#ifndef    ARGSDEF_AUTOSHUTDOWN
# define    ARGSDEF_AUTOSHUTDOWN      1
#endif

extern char    args_autoShutdown;

#ifndef    ARGSDEF_SHUTDOWNDELAY
# define    ARGSDEF_SHUTDOWNDELAY     1
#endif

extern short   args_shutdownDelay;       


/* play an alamr with the speaker */
#define    AUTOALARM_LEAVE            -1
#define    AUTOALARM_OFF              -2

#ifndef    ARGSDEF_AUTOALARM
# define    ARGSDEF_AUTOALARM         5
#endif

#ifndef    ARGSDEF_ALARMTYPE
# define    ARGSDEF_ALARMTYPE         'h'
#endif

#ifndef    ARGSDEF_ALARMREPEAT
# define    ARGSDEF_ALARMREPEAT       5
#endif

#if ARGSDEF_ALARMREPEAT == 0
# warning  "Set alarmRepeat to 0 is stupid: i will use 1 instead"
# undef    ARGSDEF_ALARMREPEAT
# define   ARGSDEF_ALARMREPEAT        1
#endif

extern char    args_autoAlarm;

/* this could be 's' 'a' 'v' 'h' */
extern char    args_alarmType;

extern char    args_alarmRepeat;


/* don't print messages and warnings */
#ifndef    ARGSDEF_BEQUIET
# define    ARGSDEF_BEQUIET           false
#endif 
extern bool    args_beQuiet;


/* don't blink when we are at 100 cpuload (by Daniel Winkler) */
#ifndef    ARGSDEF_DONTBLINK100
# define    ARGSDEF_DONTBLINK100      false
#endif

extern bool    args_dontBlink100;


/* show info in console and exit */
#ifndef    ARGSDEF_TTYMODE
# define    ARGSDEF_TTYMODE           false
#endif

extern bool    args_ttyMode;


/* three kinds of skin */
#define    SKIN_DEFAULT               0
#define    SKIN_DEFAULT_GREEN         1
#define    SKIN_OTHER                 2

#ifndef    ARGSDEF_SKIN
# define    ARGSDEF_SKIN              SKIN_DEFAULT
#endif

extern bool    args_skin;




/* macros */
#define    EXIT_IF_ALREADY_SET( arg, set, name )                       \
                                                                       \
do {                                                                   \
	if( arg != set ) {                                                 \
		fprintf( stderr, "\nVariable %s set twice or more\n", name);   \
		fprintf( stderr, "Try -h for help\n\n" );                      \
		exit( ERROR );                                                 \
	}                                                                  \
} while( 0 )


#define    WARNING_IS_SET_TO_ZERO( arg, name )                   \
                                                                 \
do {                                                             \
	if( arg == 0 )                                               \
		PRINTQ( stderr, "Warning: %s = 0. Skip.. \n", name);    \
} while( 0 )


#define    SET_DEFAULT( arg, condition, value )                  \
                                                                 \
do {                                                             \
	if( arg == condition )                                       \
		arg = value;                                             \
} while( 0 )


/* the QUIET macro */
#define    PRINTQ( output, frm, args... )                        \
                                                                 \
do {                                                             \
	if( args_beQuiet == false )                                  \
		fprintf( output, frm, ##args );                          \
} while ( 0 )

#endif
