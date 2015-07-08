/*
 * longrun.h - longrun module header file
 *
 * Copyright (c) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */

#define LONGRUN_FLAGS_UNKNOWN	 0
#define LONGRUN_FLAGS_PEFORMANCE 1
#define LONGRUN_FLAGS_ECONOMY	 2

/*
 * initializes and determines min/max frequencies and allowed performance levels
 */
void longrun_init(char *cpuid_dev, char *msr_dev);

/*
 * percent:  performance level (0 to 100)
 * flags:    performance/economy/unknown
 * mhz:      frequency
 * volts:    voltage
 */
void longrun_get_stat(int *percent, int *flags, int *mhz, int *voltz);

/* Gets the current low and high limits for CPU frequency.  These will
   be one of a fixed set of values, e.g., 0, 15, 57, 78, 100. */
void get_longrun_range(int *low, int *high);

/* Sets the low and high limits for CPU frequency.  If these are not
   from a fixed set of values, e.g., 0, 15, 57, 78, 100, the closest
   value will be used instead. */
void set_longrun_range(int low, int high);
