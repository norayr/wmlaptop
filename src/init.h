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

#ifndef __INIT_H__
#define __INIT_H__

#include "main.h"
#include "pixmap.h"

#ifdef LONGRUN
#include "longrun.h"
#endif


/* it starts connection to X and sets dockapps features */
void init_display(  );

/* it initializes dockapp's image */
void init_image  ( );

/* try to put 'userspace' as cpu scaling governor when needed (Kernel 2.6.*) */
void scalingGovernorHelper ( );

/* it reads cpu informations */
void init_cpuState ( );

/* it initializes battery if is selected APM or ACPI support */
void init_battery ( );

#endif
