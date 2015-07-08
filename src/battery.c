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

#include "battery.h"

bool mallocBatteryVector( )
{
	/* adding a battery to powerState vector */
	powerState.nBatt += 1;
	
	powerState.batteryVector = realloc( powerState.batteryVector, sizeof( battery ) * powerState.nBatt );
	
	/* malloc testing.. */
	if ( powerState.batteryVector == NULL && powerState.nBatt > 0 )
	{
		powerState.nBatt -= 1;
		fprintf( stderr,  "Error calling 'realloc()': %s\n", strerror(errno));
		return ERROR;
	}
	
	powerState.batteryVector[powerState.nBatt-1] = malloc( sizeof( struct battery_str));
	
	/* once more again */
	if ( powerState.batteryVector[ powerState.nBatt-1 ] == NULL )
	{
		powerState.nBatt -= 1;
		powerState.batteryVector = realloc( powerState.batteryVector, sizeof( battery ) * powerState.nBatt );
		fprintf( stderr, "Error calling 'malloc()': %s\n", strerror(errno));
		return ERROR;
	}
	
	memset( powerState.batteryVector[ powerState.nBatt-1 ], 0, sizeof( struct battery_str ) );
	
	return SUCCESS;
}


void freeBatteryVector( int index )
{
	register int i;
	battery ptr = powerState.batteryVector[ index ];
	
	for( i = index; i < powerState.nBatt - 1; i++ )
		powerState.batteryVector[ i ] = powerState.batteryVector[ i+1 ];
	
	powerState.nBatt -= 1;
	
	powerState.batteryVector = realloc( powerState.batteryVector, sizeof( battery ) * powerState.nBatt );
	
	/* we got a real problem */
	if( powerState.batteryVector == NULL && powerState.nBatt > 0 )
	{
		fprintf( stderr, "Error calling 'free()': %s\n", strerror( errno ));
		free_and_exit ( ERROR );
	}
	
	free( ptr );
}



bool ACPI_canSupport( )
{
	if( access( ACPI_ACCESS_TEST, R_OK ) == SUCCESS )
	//if( access( ACPI_ACCESS_TEST, R_OK | X_OK ) == SUCCESS )
		{
		return true;
		}
	else
	    {
	    return false;
		}
}



bool ACPI_init( )
{
	DIR * battdir;
	struct dirent *batt;
	char * name;
	char   path[52];
	battery batPtr;
	u_int8 countBattery = 0;
	
	if ( ACPI_canSupport() == false )
	{
		return ERROR;
	}
	
	powerState.type = SUPPORT_ACPI;
	powerState.updater = ACPI_Update; 
	
	/* counting number of batteries */
	powerState.nBatt = 0;
	powerState.batteryVector = NULL;
	
	if( (battdir = opendir (ACPI_BATTERY_DIR)) == NULL )
	{
		fprintf( stderr,  "Cannot open " ACPI_BATTERY_DIR ": %s\n", strerror(errno) );
		return ERROR;
	}
	
	/* a cicle for creating as many battery_str structures 
	 * as are the batteries on the laptop; everyone will be
	 * filled with static informations */
	while ((batt = readdir (battdir)))
	{
		
		name = batt->d_name;
       /* keeps only BAT* from power_supply */
       if (!strncmp ("BAT", name, 3))
         countBattery++;
       else
         continue;

		/* adding a battery to powerState vector */
		if( mallocBatteryVector( ) == ERROR )
			return ERROR;
		
		batPtr = powerState.batteryVector[ powerState.nBatt - 1 ];
		
		/* we can find 'status' or even 'state' */
		snprintf( path, 52, "%s/%s/status", ACPI_BATTERY_DIR, name );
		if (access(path, R_OK) == SUCCESS )
		    snprintf( batPtr -> stateFile, 52, "%s/%s/uevent",ACPI_BATTERY_DIR, name );
		else
	        snprintf( batPtr -> stateFile, 52, "%s/%s/status", ACPI_BATTERY_DIR, name );	
		
		/* calculating maximum capacity of this battery
		 * reading file infos */
		batPtr -> useLFC = ACPI_lfcArgs_get( countBattery );
		batPtr -> counter = countBattery;
		snprintf( batPtr -> infoFile, 52, "%s/%s/uevent", ACPI_BATTERY_DIR, name );
		batPtr -> present = ACPI_maxCapacity ( batPtr );
		
		/* THIS IS NOT A BATTERY ! set error field to true: don't use it */
		if( batPtr -> capacity == ~0x00 && batPtr -> present == true )
		{
			batPtr -> error = true;
		}
		
		/* filler selection :P */
		batPtr -> filler = ACPI_Filler;
	}
	
	closedir (battdir);
	return SUCCESS;
}



bool ACPI_maxCapacity ( battery p )
{
	FILE *  fp;
	char    buf[512];
	char *  ptr;
	u_int16 saltCounter = 0;
	char *  errorMessage = "wmlaptop will ignore this battery. If you don't want to see\nthis message, use '-q' option as argument.\n";
	
	/* we control 'last full capacity' tag before, if this 
	 * don't exists or it's value is 0, then we use 'design capacity' */
	char *  lfcTagPointer = NULL;
	char *  dcTagPointer  = NULL;
	u_int32 lastFullCapacity = 0;
	u_int32 designCapacity   = 0;
	
	char    presentString[25];
	char    yesNoString[25];
	
	if( (fp = fopen( p -> infoFile, "r" )) == NULL ) {
		PRINTQ( stderr, "Error reading the info file (%s):\n%s\n%s\n", p -> infoFile, strerror(errno), errorMessage );
		p -> capacity =  ~0x00;
		return true;
	}
	
	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);
	
	sscanf( buf, "%s %s", presentString, yesNoString );
	
	if( !strcmp( presentString, "present:" ) && !strcmp( yesNoString, "no" ) )
	{
		/* to avoid division by 0 ? */
		p -> capacity = 1;
		return false;
	}
	
	ptr = buf;
	while( (!dcTagPointer || !lfcTagPointer) && saltCounter < 492 )
	{

     if( !strncmp( ptr, "POWER_SUPPLY_CHARGE_FULL_DESIGN=", 32 ) )
       dcTagPointer = ptr;

     else if( !strncmp( ptr, "POWER_SUPPLY_ENERGY_FULL_DESIGN=", 32 ) )
       dcTagPointer = ptr;


     if( !strncmp( ptr, "POWER_SUPPLY_CHARGE_FULL=", 25 ) )
       lfcTagPointer = ptr;
     else if ( !strncmp( ptr, "POWER_SUPPLY_ENERGY_FULL=", 25 ) )
       lfcTagPointer = ptr;

     saltCounter++;
     ptr++;
    }
	
	
	/* naah.. */
	if( saltCounter >= 492 && !dcTagPointer && !lfcTagPointer )
	{
		PRINTQ( stderr, "The uevent file (%s) has not \n'FULL_DESIGN' and not event 'FULL' tag\n%s\n", p -> infoFile, errorMessage );
		/* we treat 1111.1111.1111.1111 as error here */
		p -> capacity =  ~0x00;
	}
	
	if( lfcTagPointer ) {
		lfcTagPointer += 25;
		sscanf( lfcTagPointer, "%u", &lastFullCapacity );
	}
	
	if( dcTagPointer ) {
		dcTagPointer += 32;
		sscanf( dcTagPointer, "%u", &designCapacity );
	}
	
	if( designCapacity )
		p -> capacity = designCapacity;
	
	if( lastFullCapacity && lastFullCapacity != designCapacity )
	{
		if( p -> useLFC == false )
		{
            PRINTQ( stderr, "'FULL_DESIGN' in <%s>\n", p -> infoFile );
            PRINTQ( stderr, "is different from 'FULL' tag:\n" );
			PRINTQ( stderr, "If you want to use it pass '--lfc=%d' (%d is the number of this battery)\n", p -> counter, p -> counter );
			PRINTQ( stderr, "If you don't want this message to be displayed anymore pass '-q'\n\n" );
		}
		else
		{
			p -> capacity = lastFullCapacity;
		}
	}
	
	if( p -> capacity == 0 )
	{
		p -> capacity = ~0x00;
		PRINTQ( stderr, "The capacity for battery %d is: 0\n%s\n", p -> counter, errorMessage );
	}
	
#ifdef DEBUG
	printf("%d) lastFullCapacity: %u\n", p -> counter, lastFullCapacity );
	printf("%d) designCapacity: %u\n", p -> counter, designCapacity );
#endif
	
	return true;
}



/* return value set to 'bool', but only for compatibility
 * with APM_Filler; its return value is never read or used */
bool ACPI_Filler ( battery bat )
{
	char *  path = bat -> stateFile;
	FILE *  fp;
	char    buf[512];
	char *  ptr;
	u_int16 saltCounter = 0;
	
	char    presentString[25];
	char    yesNoString[25];
	
	/* it prints message in case of error and then quit.. */
	if( (fp = fopen( path, "r" )) == NULL ) {
		fprintf( stderr, "Error reading the info file (%s):\n%s\n", path, strerror(errno) );
		free_and_exit( ERROR );
	}
	
	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);
	
	/* in the first line there's an info that tells if the battery is present*/
	sscanf( buf, "%s %s", presentString, yesNoString);
	
	/* if not present, set actualState to zero*/
	if( !strcmp( presentString, "present:" ) && !strcmp( yesNoString, "no" ) )
	{
		bat -> actualState = 0;
		
		/* if a battery has been unplugged right now */
		if( bat -> present == true )
			bat -> present = false;
	}
	else
	{
		/* if the battery has been plugged in now, read the information's file */
		if( bat -> present == false )
		{
			bat -> present = ACPI_maxCapacity ( bat );
			
			/* THIS IS NOT A BATTERY ! */
			if( bat -> capacity == ~0x00 && bat -> present == true )
			{
				bat -> error = true;
			}
		}
		
		/* if there is an error on this battery, don't use it */
		if( bat -> error )
		{
			bat -> percentage = 0;
			return ERROR;
		}
		
 		ptr = buf;
        while( strncmp( ptr, "POWER_SUPPLY_CHARGE_NOW=", 24 ) 
               && strncmp( ptr, "POWER_SUPPLY_ENERGY_NOW=", 24 ) 
              && saltCounter < 495)
		{
			saltCounter++;
			*ptr++;
		}
		
		/* naah.. */
		if( saltCounter >= 498 )
		{
			fprintf( stderr, "The uevent file (%s) has not 'NOW' tag", path );
			free_and_exit( ERROR );
		}
		
		ptr += 24;
		
		sscanf( ptr, "%u", &bat -> actualState );
	}
	
	bat -> percentage = (bat -> actualState * 100) / bat -> capacity;
	
	
	return SUCCESS;
}


void ACPI_Update ( )
{
	FILE * fp;
	char *where = NULL;
	char  buf[512];
	
	register int i;
	register u_int32 capacitySum = 0;
	register u_int32 actualStateSum = 0;
	
	for( i = 0; i < powerState.nBatt; i++ )
	{
		powerState.batteryVector[i] -> filler( powerState.batteryVector[i] );
		if (powerState.batteryVector[i] -> present) {
			capacitySum += powerState.batteryVector[i] -> capacity;
			actualStateSum += powerState.batteryVector[i] -> actualState;
		}
	}

	if (capacitySum > 0)
		powerState.percentage = (actualStateSum * 100) / capacitySum;
	else
		powerState.percentage = 0;
	
	/* battery charger information reading
	 * thanks to wmpower */
	
    if (!(fp = fopen ("/sys/class/power_supply/AC/online", "r")))
      if (!(fp = fopen ("/sys/class/power_supply/ADP1/online", "r")))
        return;
	
	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);
	
	if (strncmp(buf, "state:",  6) == 0)
		where = buf;
    if (strncmp(buf, "0", 1) == 0)
        where = buf;
    if (strncmp(buf, "1", 1) == 0)
        where = buf;
 

	
	if (where)
	{
		if (where[0] == '1')
			powerState.isCharging = true;
		if (where[0] == '0')
			powerState.isCharging = false;
	}
	
    if( args_ttyMode )
        return;


	setNewBatteryState();
	
	
	return;
}


/* this vector will contain in each position the number of the
 * battery the user wants to be used with 'last full capacity' */
static u_int8 * lfcArgsVector = NULL;
static u_int8   lfcArgsVectorSize = 0;


bool ACPI_lfcArgs_add ( u_int8 n )
{
	register int i;
	
	if( !n )
		return true;
	
	lfcArgsVectorSize++;
	lfcArgsVector = realloc( lfcArgsVector, lfcArgsVectorSize );
	lfcArgsVector[ lfcArgsVectorSize - 1 ] = n;
	
	for( i = 0; i < lfcArgsVectorSize - 1; i++ )
		if( lfcArgsVector[ i ] == n )
			return true;
	return false;
}


bool ACPI_lfcArgs_get ( u_int8 n )
{
	register int i;
	
	for( i = 0; i < lfcArgsVectorSize; i++ )
		if( lfcArgsVector[ i ] == n )
			return true;
	return false;
}


void ACPI_lfcArgs_free ( )
{
	free( lfcArgsVector );	
}




bool APM_canSupport( )
{
	if( access( APM_ACCESS_TEST, R_OK ) == SUCCESS )
		return true;
	return false;
}



bool APM_init ( )
{
	
	if ( APM_canSupport() == false )
	{
		return ERROR;
	}
	
	/* using APM, creating structure for only one battery */
	powerState.type = SUPPORT_APM;
	powerState.updater = APM_Update; 
	
	/* set this to 0, the next function will bring it to 1 */
	powerState.nBatt = 0;
	
	if( mallocBatteryVector() == ERROR )
		return ERROR;
	
	/* for APM, the only valid and updated values for a 'battery' structure
	 * are filler pointer and percent field */
	powerState.batteryVector[0] -> filler = APM_Filler;
	
	
	return SUCCESS;	
}


/* it returns true if battery charger is plugged in */
bool APM_Filler ( battery bat )
{
	FILE * apmFp;
	char   buf[100];
	
	char   useLess[10];
	int    useLessToo;
	
	int    recharging;
	int    percentage;
	
	/* as in case of ACPI, an error in filler make the program exit  */ 
	if( (apmFp = fopen( "/proc/apm", "r" )) == NULL )
	{
		fprintf( stderr, "Cannot open '/proc/apm' to read battery's information: %s\n", strerror(errno));
		free_and_exit( ERROR );
	}
	
	fgets( buf, sizeof(buf) - 1, apmFp );
	buf[sizeof(buf) - 1] = '\0';
	fclose( apmFp );
	
	sscanf( buf, "%s %d.%d %x %x %x %x %d%%",
		useLess, /* driver version */
		&useLessToo, /* apm version major */
		&useLessToo, /* apm version minor */
		&useLessToo, /* apm flags */
		&recharging, /* ac line status */
		&useLessToo, /* battery_status */
		&useLessToo, /* battery_flags */
		&percentage /* battery_percentage */ );
	
	/* -1 means not present */
	if( percentage == -1 )
		percentage = 0;
	bat -> percentage = percentage;
	
	
	return recharging;
}


void APM_Update( )
{
	powerState.isCharging =
	powerState.batteryVector[0] -> filler( powerState.batteryVector[0] );
	powerState.percentage = powerState.batteryVector[0] -> percentage;
	
    setNewBatteryState( );

	return;
}



void setNewBatteryState( )
{
	static bool lastChargingState = false;
	static u_int8 lastPercentage = 0;
	static bool firstRelevation = true;
	
	/* batterySum is used to know if there have been changes 
	 * in the number of plugged in battery: in 95% of cases
	 * if you plug or unplug a battery, batterySum should be
	 * different from lastBatterySum (if you succeed to plug
	 * a battery ad unplug another one in two seconds, you are
	 * in the 5% cases :P) */
	static u_int8 lastBatterySum = 0;
	u_int8 batterySum = 0;
	int i;
	
	for( i = 0; i < powerState.nBatt; i++ )
		batterySum += powerState.batteryVector[i] -> present;
	
	if( lastChargingState != powerState.isCharging || lastBatterySum != batterySum )
		firstRelevation = true;
	
	if( lastChargingState != powerState.isCharging ||
	    lastPercentage != powerState.percentage ||
	    lastBatterySum != batterySum )
	{
#ifdef DEBUG
		printf("setNewBatteryState has been called. firstRelevation = %d\n", firstRelevation );
#endif
		if( firstRelevation )
		{
			powerState.remainingTime = 0;
			firstRelevation = false;
		}
		else
		{
			estimatedTimeClock( );
		}	
		draw_all();
		lastChargingState = powerState.isCharging;
		lastPercentage = powerState.percentage;
		lastBatterySum = batterySum;
	}
}



void estimatedTimeClock ( )
{
	/* we remember last 5 percent values changes.
	 * for every value we record even the exact moment when this
	 * value was recorded (reading global variable secondsCounter) */
	static u_int32 values[5][2];
	static u_int8  valueIndex = ~0;
	
	static bool lastIsCharging = false;
	
	register u_int8  diffPercentage = 0;
	register u_int32 diffTime       = 0;
	register int i;
	
	
	/* if battery charge is plugged in, values fetched before are
	 * reset to zero and return (draw_all() function already called
	 * by updater will draw TIME unlit, to indicate that we haven't
	 * a time estimation */
	if( lastIsCharging != powerState.isCharging )
	{
		lastIsCharging = powerState.isCharging;
		valueIndex = ~0;
		powerState.remainingTime = 0;
		return;
	}
	
	
	for( i = 4; i > 0; i-- )
	{
		values[i][1] = values[i-1][1];
		values[i][0] = values[i-1][0];
	}
	values[i][0] = powerState.percentage;
	values[i][1] = secondsCounter;
	
	
	valueIndex += 1;
	if( valueIndex > 4 )
		valueIndex = 4;
	
	/* if it's the first measure, jump the comparation */
	if( valueIndex > 0 )
	{
		u_int32 remainingSeconds;
		/* we have into values[0] the most recent value,
		 * and in values[ valueIndex ] the less one  */
		
		if( powerState.isCharging )
			diffPercentage = values[0][0] - values[ valueIndex ][0];
		else
			diffPercentage = values[ valueIndex ][0] - values[0][0];
		
		diffTime       = values[0][1] - values[ valueIndex ][1];
		
		/* this is the logic way: if diffTime is the number of seconds for
		 * losing(gaining) diffPercentage percent battery charge, how many
		 * seconds will it takes to lose(gain) remaining points ? in other words:
		 * diffTime : diffPercentage = remainingSeconds : powerState.percentage
		 * (or if we are charging    = remainingSeconds : (100-powerState.percentage))
		 * it shouldn't be possible to have diffPercentage = 0
		 * because this function is called everytime that percent changes */
		if( diffPercentage == 0 )
		{
			/* sometime it happends that 'diffPercentage == 0': i recieved some bug
			 * report about this. I don't know why this happends, and i didn't find
			 * any way to correct this conceptually, so i have to put this condition
			 * here and turn off the estimated remaining time */
			PRINTQ( stderr, "There is no difference of percentage between the first\n");
			PRINTQ( stderr, "relevation and the last one: i have to put remainingTime to 0\n");
			remainingSeconds = 0;
		}
		else
		{
			if( powerState.isCharging )
				remainingSeconds = ( diffTime * (100-powerState.percentage) ) / diffPercentage;
			else
				remainingSeconds = ( diffTime * powerState.percentage ) / diffPercentage;
		}
		
		/* value in minutes */
		powerState.remainingTime = remainingSeconds / 60;
	}
	else
		powerState.remainingTime = 0;
	
	
#ifdef DEBUG
	printf("TEMPO TRASCORSO: %u:%02u   (%u secondi)\n",diffTime/60,diffTime%60,diffTime);
	printf("PERCENTUALI: values[0] = %u,  values[%d] = %u\n", values[0][0], valueIndex, values[valueIndex][0] );
	printf("TEMPIVALORI: values[0] = %u,  values[%d] = %u\n", values[0][1], valueIndex, values[valueIndex][1] );
	printf("DIFFENREZA: %d\n", diffPercentage );
	printf("TEMPOSTIMATO: %d\n\n\n", powerState.remainingTime );
#endif
}

