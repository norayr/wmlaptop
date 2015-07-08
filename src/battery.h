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

#ifndef  __BATTERY_H__
#define  __BATTERY_H__

#include "main.h"



/*******************
 * ACPI FEATURES   *
 *********************************************************************************/
#define    ACPI_ACCESS_TEST      "/proc/acpi/info"
#define    ACPI_BATTERY_DIR      "/proc/acpi/battery"
//TODO #define    ACPI_BATTERY_DIR      "/home/mtc/hisBattery/battery"


/* it increments, mallocing, the powerState.batteryVector size, to enable
 * it to contain another set of battery's information. */
bool mallocBatteryVector( );

/* it takes the index of the array-slot we want to free, and to it */
void freeBatteryVector( int );


/* checks if it's possible to use ACPI support */
bool ACPI_canSupport( );


/* initializes data structures needed by program to run with ACPI support.
 * it returns SUCCESS if the initialization succeded, else ERROR. */
bool ACPI_init ( );

/* takes as argument a battery pointer and fill the capacity field by
 * reading from "info" file path. It returns false if battery is not
 * present, true if it's present. After the call, if capacity field
 * is ~0x00, there is an error reading the present battery infos */
bool ACPI_maxCapacity ( battery );

/* is the function that periodically read battery data ( battery passed as
 * argument ), and updates its structure. */
bool ACPI_Filler ( battery );


/* if powerState.type is SUPPORT_ACPI, this function is called periodically 
 * to update battery values, percentage and battery charger. */
void ACPI_Update ( );



bool ACPI_lfcArgs_add ( u_int8 );


bool ACPI_lfcArgs_get ( u_int8 );


void ACPI_lfcArgs_free ( );

/***********************************************************************************
 * ACPI FEATURES   *
 *******************


 *******************
 * APM FEATURES    *
 ***********************************************************************************/

#define    APM_ACCESS_TEST      "/proc/apm"

/* checks if it's possible to use APM support */
bool APM_canSupport ( );

/* the same as ACPI, but for APM */
bool APM_init ( );


/* that's for ACPI comes in two distinct steps, in APM, battery status and
 * battery charger status (plugged in or not) is made the same file, so this
 * function fills 'percentage' field of battery structure passe as argument,
 * 'percentage' della struct batteria che gli viene passata, and returns TRUE
 * if battery charger is plugged in. */
bool APM_Filler ( battery );

/* the same for ACPI */
void APM_Update ( );



/*************************************************************************************
 * APM FEATURES    *
 *******************/


/* common function for ACPI and APM support that redraw the dockapp if battery
 * state is changed. */
void setNewBatteryState( );


/* function invoked every time that remaining battery percentage changes
 * and calculates estimated remaining time.
 * (if changed from previous updates the dockapp */
void estimatedTimeClock ( );


#endif
