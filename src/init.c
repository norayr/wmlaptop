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

#include "init.h"

void init_display(  )
{
	XClassHint       classHint;
	XTextProperty    name;
	XGCValues        gcv;
	unsigned long    gcm;
	int              dummy=0;
	char           * progname = PROGNAME;
	
	Pixel            back_pix;
	Pixel            fore_pix;

	display = XOpenDisplay( args_XDisplayName );
	
	if( display == NULL )
	{
		fprintf(stderr, "Can't open display\n" );
		free_and_exit( ERROR );
	}
	
	screen  = DefaultScreen(display);
	Root    = RootWindow(display, screen);
	d_depth = DefaultDepth(display, screen);
	x_fd    = XConnectionNumber(display);

	
	init_image ( );
	
	/* Create a window to hold the stuff */
	mysizehints.flags = USSize | USPosition;
	mysizehints.x = 0;
	mysizehints.y = 0;
	

	back_pix = WhitePixel( display, screen);
	fore_pix = BlackPixel( display, screen);

	XWMGeometry(display, screen, NULL, NULL, 1, &mysizehints, \
	            &mysizehints.x, &mysizehints.y,&mysizehints.width,&mysizehints.height, &dummy);
	
	mysizehints.width = 64;
	mysizehints.height = 64;
		
	win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y, \
	                          mysizehints.width, mysizehints.height, 1, fore_pix, back_pix);
	
	iconwin = XCreateSimpleWindow(display, win, mysizehints.x, mysizehints.y, \
	                              mysizehints.width, mysizehints.height, 1, fore_pix, back_pix);

	/* Activate hints */
	XSetWMNormalHints(display, win, &mysizehints);
	classHint.res_name = progname;
	classHint.res_class = progname;
	XSetClassHint(display, win, &classHint);

	XSelectInput(display, win, ButtonPressMask | ButtonReleaseMask | ExposureMask | StructureNotifyMask );
	XSelectInput(display, iconwin, ButtonPressMask | ButtonReleaseMask | ExposureMask | StructureNotifyMask );

	if (XStringListToTextProperty(&progname, 1, &name) == 0)
		PRINTQ(stderr, "%s: can't allocate window name\n", PROGNAME);
	

	XSetWMName(display, win, &name);
	
	/* Create GC for drawing */
	
	gcm = GCForeground | GCBackground | GCGraphicsExposures;
	gcv.foreground = fore_pix;
	gcv.background = back_pix;
	gcv.graphics_exposures = 0;
	NormalGC = XCreateGC(display, Root, gcm, &gcv);

	/* ONLYSHAPE ON */
	
	pixmask = XCreateBitmapFromData(display, win, wmlaptop_mask_bits, wmlaptop_mask_width, wmlaptop_mask_height);

	XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
	XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);
	
	/* ONLYSHAPE OFF */

	mywmhints.initial_state = WithdrawnState;
	mywmhints.icon_window = iconwin;
	mywmhints.icon_x = mysizehints.x;
	mywmhints.icon_y = mysizehints.y;
	mywmhints.window_group = win;
	mywmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;

	XSetWMHints(display, win, &mywmhints);
	
	XMapWindow(display, win);
	
	
}


void init_image ()
{
	char *xbm = wmlaptop_mask_bits;
	char ** xpm;
	int sx = wmlaptop_mask_width;
	int sy = wmlaptop_mask_height;
	
	XWindowAttributes      attributes;
	int                    err;
	
	int i,j,k;
	int width, height, numcol, depth;
    int zero=0;
    int bcount;
    int curpixel;
	unsigned char bwrite;


	/* use the command line arg selected pixmap */
	usePixmap( args_skin );
	xpm = XPM_NAME;
	
	sscanf(*xpm, "%d %d %d %d", &width, &height, &numcol, &depth);


    for (k=0; k!=depth; k++)
    {
        zero <<=8;
        zero |= xpm[1][k];
    }
        
	for (i=numcol+1; i < numcol+sy+1; i++)
	{
		bcount = 0;
		bwrite = 0;
		for (j=0; j<sx*depth; j+=depth)
		{
            bwrite >>= 1;

            curpixel=0;
            for (k=0; k!=depth; k++)
            {
                curpixel <<=8;
                curpixel |= xpm[i][j+k];
            }
                
            if ( curpixel != zero )
            {
				bwrite += 128;
			}
			bcount++;
			if (bcount == 8)
			{
				*xbm = bwrite;
				xbm++;
				bcount = 0;
				bwrite = 0;
			}
		}
	}
	
	/* For the colormap */
	XGetWindowAttributes(display, Root, &attributes);
	
	wmgen.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);
	
	err = XpmCreatePixmapFromData(display, Root, XPM_NAME, &(wmgen.pixmap), &(wmgen.mask), &(wmgen.attributes));
	
	if (err != XpmSuccess)
	{
		fprintf(stderr, "Not enough free colorcells.\n");
		free_and_exit( ERROR );
	}
	
}


/* this chech for the string 'userspace' is in scaling_governor file under /sys/.. dir
 * (only in case we are going to use SYS's way to set cpufreq). If 'userspace' is not set
 * then we try to set it by ourself */
void scalingGovernorHelper( )
{
    char * scaling_governor_path = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    char * scaling_governor_av_path = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors";
    char * error_msg = "Unable to read <%s>\n";
    char * error_msg_w = "Unable to write <%s>\n";
    FILE * sysfp;
    FILE * sysavfp;
    char   littleBuffer[128];
    
    bool   printIt = false;
    int    i;

    if( ( sysfp = fopen( scaling_governor_path, "r" ) ) == NULL ) {
        PRINTQ( stderr, error_msg, scaling_governor_path, strerror(errno) );
        return;
    }

    fgets( littleBuffer, 127, sysfp );
    fclose( sysfp );
    
    if( !strcmp( littleBuffer, "userspace\n" ) )
       {
   cpuState.userspace = true;
        return;
       }
    if( ( sysavfp = fopen( scaling_governor_av_path, "r" ) ) == NULL ) {
        PRINTQ( stderr, error_msg, scaling_governor_av_path, strerror(errno) );
        return;
    }
    
    fgets( littleBuffer, 127, sysavfp );
    fclose( sysavfp );
    
    for( i = 0; i < strlen( littleBuffer ) - 9 ; i++ )
        if( !strncmp( littleBuffer, "userspace", 9 ) )
        {
            /* ok, module is loaded */
            printIt = true;
			cpuState.userspace = true;
            break;
        }


    if( printIt == false ) {
        int s;
        PRINTQ( stderr, "It seems that 'userspace' governor is not set\n");
        PRINTQ( stderr, "wmlaptop may not control CPU frequency\n");       
  cpuState.userspace = false;
    }
    
    /* All ok here */
    if( ( sysfp = fopen( scaling_governor_path, "w" )) == NULL ) 
	    PRINTQ( stderr, error_msg_w, scaling_governor_path, strerror(errno));
    else
    {
        PRINTQ( stderr, "echoing 'userspace' > '%s'\n", scaling_governor_path );
        fprintf( sysfp, "userspace" );
		cpuState.userspace = true;
        fclose( sysfp );
    }

    return;
}
	


/* riempe la struttura cpuState tenendo anche conto
 * degli argomenti passati da linea di comando */
void init_cpuState ( )
{
#ifndef LONGRUN
	FILE *  fp;
	char red[10];
	char * paths[4][3] =
	          { /* sys     proc    user */
	/* min */   { "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq",
	              "/proc/sys/cpu/0/speed-min",
	              args_useFileMin },
	/* max */   { "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq",
	              "/proc/sys/cpu/0/speed-max",
	              args_useFileMax },
	/* set */   { "/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed",
	              "/proc/sys/cpu/0/speed",
			      args_useFileSet },
   /* read */  { "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq",
             "/proc/sys/cpu/0/speed",
             args_useFileSet }
	          };
	u_int8 idx[3] = {   /* the second index */
	     args_useFileMin ? 2 : ( args_useSysProc == USESYSPROC_SYS ? 0 : 1 ),
	     args_useFileMax ? 2 : ( args_useSysProc == USESYSPROC_SYS ? 0 : 1 ),
	     args_useFileSet ? 2 : ( args_useSysProc == USESYSPROC_SYS ? 0 : 1 )
	};
	
	
	/* auto_freq_state */
	cpuState.auto_freq_state = ( args_autoFreq == AUTOFREQ_ON );

    /* will be checked later */
    cpuState.userspace = false;
	
	/* min freq */
	if( (fp = fopen( paths[0][idx[0]], "r" )) == NULL ) {
		fprintf( stderr, "Error reading the info file (%s):\n%s\n", paths[0][idx[0]], strerror(errno) );
		free_and_exit( ERROR );
	}
	
	fgets ( red, 10, fp );
	cpuState.minFreq = atoi ( red );
	
	fclose ( fp );
	
	/* max freq */	
	if( (fp = fopen( paths[1][idx[1]], "r" )) == NULL ) {
		fprintf( stderr, "Error reading the info file (%s):\n%s\n", paths[1][idx[1]], strerror(errno) );
		free_and_exit( ERROR );
	}
	
	fgets ( red, 10, fp );
	cpuState.maxFreq = atoi ( red );
	
	fclose ( fp );
	
	/* set freq file */
	cpuState.setFreqFile = paths[2][idx[2]];
    /* read freq file */
    cpuState.readFreqFile = paths[3][idx[2]];
        /* if we have to use the SYS's way to set cpufreq, then ensure that in
         * scaling_governor there has been put 'userspace'; otherwise let's try
         * to put it by ourself */
        if( idx[2] == 0 )
            scalingGovernorHelper( );

	
        cpuReadFreq( );
	
	
        /* set freq */
        cpuState.setFreq = cpuState.actualFreq;


        /* step freq */
        cpuState.stepFreq = args_incrementalStep;
	
#else
        /* init and get first actual frequency */
        /* PENDING: get these files from config */
        longrun_init("/dev/cpu/0/cpuid","/dev/cpu/0/msr");
        cpuReadFreq( );
        cpuState.setLevelIdx = cpuState.actualLevelIdx;
	cpuState.auto_freq_state = ( args_autoFreq == AUTOFREQ_ON );
        cpuState.stepLevelIdx = 1;
        /* did user set a valid value here? */
        if (args_incrementalStep < 10 && args_incrementalStep > 0)
            cpuState.stepLevelIdx = args_incrementalStep;
#endif

    /* ok, if we are in ttyMode we don't need to modify values.. we just want to read them */
    if( args_ttyMode )
        return;

#ifndef LONGRUN	
	/* other options */
	if( args_maxFreq != ARGSDEF_LEAVE ) {
		if( args_maxFreq > cpuState.maxFreq ) {
			PRINTQ( stderr, "Error: args_maxFreq > maxFreq possible (using %d as max)\n", cpuState.maxFreq );
			args_maxFreq = cpuState.maxFreq;
		}
		cpuState.maxFreq = args_maxFreq;
	}
	
	if( args_minFreq != ARGSDEF_LEAVE ) {
		if( args_minFreq < cpuState.minFreq ) {
			PRINTQ( stderr, "Error: args_minFreq < minFreq possible (using %d as min)\n", cpuState.minFreq );
			args_minFreq = cpuState.minFreq;
		}
		cpuState.minFreq = args_minFreq;
	}
	
	/* little control.. */
	if( cpuState.minFreq > cpuState.maxFreq ) {
		fprintf( stderr, "Error: minFreq > maxFreq\n" );
		free_and_exit( ERROR );
	}
#endif
	switch( args_startingFreq )
	{
		case STARTINGFREQ_MIN:
#ifndef LONGRUN	
			/* frequence at min */
			while( cpuState.actualFreq != cpuState.minFreq )
				cpuSetFreq( FREQ_DECREMENT, FREQ_NEXT );
#else
                        cpuState.setLevelIdx = 0;
#endif
			break;
		case STARTINGFREQ_MAX:
			/* frequence at max */
#ifndef LONGRUN	
			while( cpuState.actualFreq != cpuState.maxFreq )
				cpuSetFreq( FREQ_INCREMENT, FREQ_NEXT );
#else
                        cpuState.setLevelIdx = cpuState.nLongRunLevels-1;
#endif
	}
	
#ifndef LONGRUN	
#ifdef DEBUG
	fprintf ( stderr, "min freq: %d\nmax freq: %d\nactual freq: %d\n", cpuState.minFreq, cpuState.maxFreq, cpuState.actualFreq );
#endif
#else
	fprintf ( stderr, "LongRun CPU starts at %d, with autoadjust = %d\n\n", cpuState.setLevelIdx, cpuState.auto_freq_state );
        cpuEchoFreq();  /* set initial range */
#endif
	return;
}


void init_battery ( )
{
	if( args_useApmAcpi == USEAPMACPI_APM || args_useApmAcpi == USEAPMACPI_ONLYAPM )
	{
		if( APM_init() == ERROR )
		{
			PRINTQ( stderr, "Cannot support APM (the file %s don't exists)\n", APM_ACCESS_TEST );
			
			if( args_useApmAcpi == USEAPMACPI_ONLYAPM ) {
				if( args_beQuiet )
					fprintf( stderr, "Cannot support APM (the file %s don't exists)\n", APM_ACCESS_TEST );
				free_and_exit( ERROR );
			}
			
			PRINTQ( stderr, "Switching to ACPI..\n");
			if( ACPI_init() == ERROR )
			{
				if( args_beQuiet )
					fprintf( stderr, "Cannot support APM (the file %s don't exists)\n", APM_ACCESS_TEST );
				fprintf( stderr, "Cannot support ACPI (the file %s don't exists)\n", ACPI_ACCESS_TEST );
				PRINTQ( stderr, ".. and now ?.. i must die\n" );
				free_and_exit( ERROR );
			}
		}
	}
	
	if( args_useApmAcpi == USEAPMACPI_ACPI || args_useApmAcpi == USEAPMACPI_ONLYACPI )
	{
		if( ACPI_init() == ERROR )
		{
			PRINTQ( stderr, "Cannot support ACPI (the file %s don't exists)\n", ACPI_ACCESS_TEST );
			
			if( args_useApmAcpi == USEAPMACPI_ONLYACPI ) {
				if( args_beQuiet )
					fprintf( stderr, "Cannot support ACPI (the file %s don't exists)\n", ACPI_ACCESS_TEST );
				free_and_exit( ERROR );
			}
			
			PRINTQ( stderr, "Switching to APM..\n");
			if( APM_init() == ERROR )
			{
				if( args_beQuiet )
					fprintf( stderr, "Cannot support ACPI (the file %s don't exists)\n", ACPI_ACCESS_TEST );
				fprintf( stderr, "Cannot support APM (the file %s don't exists)\n", APM_ACCESS_TEST );
				PRINTQ( stderr, ".. and now ?.. i must die\n" );
				free_and_exit( ERROR );
			}
		}
	}
	
}
