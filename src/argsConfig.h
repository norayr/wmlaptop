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

#ifndef __ARGSCONFIG_H__
#define __ARGSCONFIG_H__

/* set the number of cpu's */
#define NUMBER_OF_CPUS 2

/* update frequency, in milliseconds; the value for battery MUST be
   greater than that for CPU */
//#define ARGSDEF_CPUUPDATE              500
//#define ARGSDEF_BATTERYUPDATE         5000


/* uncomment following lines to open by default wmlaptop
 * on another X-Server */
//#define    ARGSDEF_XDISPLAYNAME       "192.168.0.2:0.0"


/* uncomment to get the auto-frequency-scaling
 * activated or deactivated at start */
//#define     ARGSDEF_AUTOFREQ          AUTOFREQ_ON
//#define     ARGSDEF_AUTOFREQ          AUTOFREQ_OFF


/* uncomment or modify this value to change autoscaling 
 * incremental step; under speedstep this should be in KHz,
   under longrun it is in units of the CPU power level setting
   ('1' is usually the best value; it is ignored if over 10,
   which it is by default) */
//#define    ARGSDEF_INCREMENTALSTEP    100000


/* to set cpu frequency at its max or min at start */
//#define    ARGSDEF_STARTINGFREQ       STARTINGFREQ_MIN
//#define    ARGSDEF_STARTINGFREQ       STARTINGFREQ_MAX


/* max usable frequency; this is ignored on longrun systems */
//#define    ARGSDEF_MAXFREQ            1400000


/* min usable frequency; this is ignored on longrun systems */
//#define    ARGSDEF_MINFREQ            600000


/* using /sys/.. or /proc/.. for cpu scaling; ignored on longrun
   systems */
//#define    ARGSDEF_USESYSPROC         USESYSPROC_SYS
//#define    ARGSDEF_USESYSPROC         USESYSPROC_PROC

/* using a file for reading max, min, and for setting cpu frequency;
   ignored on longrun systems */
//#define    ARGSDEF_USEFILEMAX         "/home/foo/bar/maxFreq"
//#define    ARGSDEF_USEFILEMIN         "/home/foo/bar/minFreq"
//#define    ARGSDEF_USEFILESET         "/proc/my/strange/place"

/* when battery is at 100 and AC_Adapter is plugged in, set the max freq
 * let the processor be what is designed to be */
//#define    ARGSDEF_PARADISIAC         PARADISIAC_OFF
//#define    ARGSDEF_PARADISIAC         PARADISIAC_ON


/* start using APM or ACPI, trying one before the other one,
 * or only the one selected */
//#define    ARGSDEF_USEAPMACPI         USEAPMACPI_APM
//#define    ARGSDEF_USEAPMACPI         USEAPMACPI_ACPI
//#define    ARGSDEF_USEAPMACPI         USEAPMACPI_ONLYAPM
//#define    ARGSDEF_USEAPMACPI         USEAPMACPI_ONLYACPI

/* use 'last full capacity' tag for battery 1 2 and 4 instead of 
 *'design capacity' in the info battery file */
//#define    ARGSDEF_LFC                "--lfc=1 --lfc=2 --lfc=4"


/* set percentage for autoshutdown, or disable it */
//#define    ARGSDEF_AUTOSHUTDOWN        0
//#define    ARGSDEF_AUTOSHUTDOWN       SHUTDOWN_OFF

/* set a delay for a shutdown (in minutes), 0 == now */
//#define    ARGSDEF_SHUTDOWNDELAY      1

/* be quiet and don't display simple messages or warnings */
//#define    ARSGDEF_BEQUIET            true


/* if you have the shutdown binary in a place different from "/sbin/shutdown"
 * you have to change this definition */
//#define    SHUTDOWN_BIN               "/sbin/shutdown"

/* those are the options given to the shutdown binary: they make shutdown
 * to display a general warning message and make it halt the system (-h) */
//#define    SHUTDOWN_ARGS              "\"wmlaptop is shutting down the system\" -h"

/* start alarm at a perarranged battery's percentage or disable it */
//#define    ARGSDEF_AUTOALARM          5
//#define    ARGSDEF_AUTOALARM          AUTOALARM_OFF

/* choose one of 4 funny jingles as alarm default sounds */
//#define    ARGSDEF_ALARMTYPE          's' /* simpson's bell */
//#define    ARGSDEF_ALARMTYPE          'a' /* ambulance siren */
//#define    ARGSDEF_ALARMTYPE          'v' /* victory jingle */
//#define    ARGSDEF_ALARMTYPE          'h' /* high beeps */

/* choose how many times the alarm have to play the jingle (it varies if
 * you want to use 's' or 'h' (type 1 or 2 with 's', 5 or 10 with 'h') */
//#define   ARGSDEF_ALARMREPEAT         10


/* uncomment this line if you dont want to see "CPULOAD" blinking at 100% */
//#define   ARGSDEF_DONTBLINK100        true

/* uncomment this line if you want wmlaptop print info in console and
 *  * quit as normal behaviour (it has no sense!?) */
//#define   ARGSDEF_TTYMODE             true

/* set another default skin for wmlaptop. The available options are
 *  - SKIN_DEFAULT          default wmlaptop skin
 *  - SKIN_DEFAULT_GREEN    default wmlaptop skin with AUTOFREQ written in green [Thanks to: Adrian Robert]
 *  - SKIN_OTHER            alternative skin [Thanks to: Dino Ghilardi ] */
//#define   ARGSDEF_SKIN                SKIN_DEFAULT_GREEN
//#define   ARGSDEF_SKIN                SKIN_OTHER


#endif
