/*
 * longrun_linux.c - module to get LongRun(TM) status, for GNU/Linux
 *
 * Copyright(C) 2001 Transmeta Corporation
 * Copyright(C) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */
#undef USE_PREAD

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef USE_PREAD
#define __USE_UNIX98	     /* for pread/pwrite */
#endif
#include <unistd.h>

#include "longrun.h"
#include "main.h"

#define MSR_DEVICE "/dev/cpu/0/msr"
#define MSR_TMx86_LONGRUN	0x80868010
#define MSR_TMx86_LONGRUN_FLAGS	0x80868011

#define LONGRUN_MASK(x)		((x) & 0x0000007f)
#define LONGRUN_RESERVED(x)	((x) & 0xffffff80)
#define LONGRUN_WRITE(x, y)	(LONGRUN_RESERVED(x) | LONGRUN_MASK(y))

#define CPUID_DEVICE "/dev/cpu/0/cpuid"
#define CPUID_TMx86_VENDOR_ID		0x80860000
#define CPUID_TMx86_PROCESSOR_INFO	0x80860001
#define CPUID_TMx86_LONGRUN_STATUS	0x80860007
#define CPUID_TMx86_FEATURE_LONGRUN(x)	((x) & 0x02)

static int  cpuid_fd;			/* CPUID file descriptor */
static char *cpuid_device;		/* CPUID device name */
static int  msr_fd;			/* MSR file descriptor */
static char *msr_device;		/* MSR device name */


void find_longrun_values();

static void
read_cpuid(long address, int *eax, int *ebx, int *ecx, int *edx)
{
    uint32_t data[4];

#ifdef USE_PREAD
    if (pread(cpuid_fd, &data, 16, address) != 16) {
	perror("pread");
#else
    if (lseek(cpuid_fd, address, SEEK_SET) != address) {
	perror("lseek");
	exit(1);
    }
    if (read(cpuid_fd, &data, 16) != 16) {
	perror("read");
#endif
	exit(1);
    }

    if (eax)
	*eax = data[0];
    if (ebx)
	*ebx = data[1];
    if (ecx)
	*ecx = data[2];
    if (edx)
	*edx = data[3];
}

/* note: if an output is NULL, then don't set it */
static void
read_msr(long address, int *lower, int *upper)
{
    uint32_t data[2];

#ifdef USE_PREAD
    if (pread(msr_fd, &data, 8, address) != 8) {
	perror("pread");
#else
    if (lseek(msr_fd, address, SEEK_SET) != address) {
	perror("lseek");
	exit(1);
    }
    if (read(msr_fd, &data, 8) != 8) {
	perror("read");
#endif
	exit(1);
    }

    if (lower)
	*lower = data[0];
    if (upper)
	*upper = data[1];
}

void
longrun_init(char *cpuid_dev, char *msr_dev)
{
    int eax, ebx, ecx, edx;

    /* set CPUID device */
    cpuid_device = CPUID_DEVICE;
    if (cpuid_dev)
	cpuid_device = cpuid_dev;

    /* set MSR device */
    msr_device = MSR_DEVICE;
    if (msr_dev)
	msr_device = msr_dev;

    /* open CPUID device */
    if ((cpuid_fd = open(cpuid_device, O_RDWR)) < 0) {
	fprintf(stderr, "error opening %s\n", cpuid_device);
	if (errno == ENODEV)
	    fprintf(stderr,
		    "make sure your kernel was compiled with CONFIG_X86_CPUID=y\n");
	exit(1);
    }

    /* open MSR device */
    if ((msr_fd = open(msr_device, O_RDWR)) < 0) {
	fprintf(stderr, "error opening %s\n", msr_device);
	if (errno == ENODEV)
	    fprintf(stderr,
		    "make sure your kernel was compiled with CONFIG_X86_MSR=y\n");
	exit(1);
    }

    /* test for "TransmetaCPU" */
    read_cpuid(CPUID_TMx86_VENDOR_ID, &eax, &ebx, &ecx, &edx);
    if (ebx != 0x6e617254 || ecx != 0x55504361 || edx != 0x74656d73) {
	fprintf(stderr, "not a Transmeta x86 CPU\n");
	exit(1);
    }

    /* test for LongRun feature flag */
    read_cpuid(CPUID_TMx86_PROCESSOR_INFO, &eax, &ebx, &ecx, &edx);
    if (!CPUID_TMx86_FEATURE_LONGRUN(edx)) {
	printf("LongRun: unsupported\n");
	exit(0);
    }

    find_longrun_values();
    get_longrun_range(&eax, &ebx);
    cpuState.auto_freq_state = (eax != ebx);

    /* close device */
    close(cpuid_fd);
    close(msr_fd);

}



/*
 * percent:  performance level (0 to 100)
 * flags:    performance/economy/unknown
 * mhz:      frequency
 * volts:    voltage
 */
void
    longrun_get_stat(int *percent, int *flags, int *mhz, int *voltz)
{
    int eax, ebx, ecx;

    /* open CPUID device */
    if ((cpuid_fd = open(cpuid_device, O_RDWR)) < 0) {
	fprintf(stderr, "error opening %s\n", cpuid_device);
	if (errno == ENODEV)
	    fprintf(stderr,
		    "make sure your kernel was compiled with CONFIG_X86_CPUID=y\n");
	exit(1);
    }

    /* open MSR device */
    if ((msr_fd = open(msr_device, O_RDWR)) < 0) {
	fprintf(stderr, "error opening %s\n", msr_device);
	if (errno == ENODEV)
	    fprintf(stderr,
		    "make sure your kernel was compiled with CONFIG_X86_MSR=y\n");
	exit(1);
    }

    /* frequency, voltage, performance level, */
    read_cpuid(CPUID_TMx86_LONGRUN_STATUS, &eax, &ebx, &ecx, 0);
    *mhz = eax;
    *voltz = ebx;
    *percent = ecx;

    /* flags */
    read_msr(MSR_TMx86_LONGRUN_FLAGS, flags, NULL);
    *flags =
	(*flags & 1) ? LONGRUN_FLAGS_PEFORMANCE : LONGRUN_FLAGS_ECONOMY;

    /* close device */
    close(cpuid_fd);
    close(msr_fd);
}

/* Note: above code was from wmlongrun, which probably got it from the longrun
   command-line utility.  The below is also from that utility, and contains the
   code for writing and setting the longrun parameters. */

void write_msr(long address, int lower, int upper)
{
	uint32_t data[2];

	data[0] = (uint32_t) lower;
	data[1] = (uint32_t) upper;

#ifdef USE_PREAD
	if (pwrite(msr_fd, &data, 8, address) != 8) {
		perror("write");
	exit(1);
	}
#else
    if (lseek(msr_fd, address, SEEK_SET) != address) {
	perror("lseek");
	exit(1);
    }
    if (write(msr_fd, &data, 8) != 8) {
	perror("write");
	exit(1);
    }
#endif
}

void get_longrun_range(int *low, int *high)
{
        /* open MSR device */
        if ((msr_fd = open(msr_device, O_RDWR)) < 0) {
            fprintf(stderr, "error opening %s\n", msr_device);
            if (errno == ENODEV)
                fprintf(stderr,
                        "make sure your kernel was compiled with CONFIG_X86_MSR=y\n");
            exit(1);
        }
	read_msr(MSR_TMx86_LONGRUN, low, high);
	/* only look at desired bits */
	*low = LONGRUN_MASK(*low);
	*high = LONGRUN_MASK(*high);

        close(msr_fd);
}

void set_longrun_range(int low, int high)
{
	int lower, upper;
#ifdef DEBUG
        fprintf(stderr,"Setting performance window: %d to %d\n", low, high);
#endif

        /* open MSR device */
        if ((msr_fd = open(msr_device, O_RDWR)) < 0) {
            fprintf(stderr, "error opening %s\n", msr_device);
            if (errno == ENODEV)
                fprintf(stderr,
                        "make sure your kernel was compiled with CONFIG_X86_MSR=y\n");
            exit(1);
        }

	read_msr(MSR_TMx86_LONGRUN, &lower, &upper);
	write_msr(MSR_TMx86_LONGRUN, LONGRUN_WRITE(lower, low),
		  LONGRUN_WRITE(upper, high));
        close(msr_fd);
}

/* Modified from "old_list_longrun" in Debian longrun_0.9-14.  Puts the
   allowable longrun levels into cpuState.longRunLevels and sets
   cpuState.nLongRunLevels to the number found. */
void find_longrun_values()
{
	int i, save_lower, save_upper, pct_in, pct_last, steps, freq_max;
	int eax, ebx, ecx;
	float power_max, power_ratio;
	int *perf, *mhz, *volts;
        int levels;

	/* save current settings */
	read_msr(MSR_TMx86_LONGRUN, &save_lower, &save_upper);

	/* find freq_max */
	read_cpuid(CPUID_TMx86_PROCESSOR_INFO, 0, 0, &freq_max, 0);

	/* setup for probing */
	write_msr(MSR_TMx86_LONGRUN, LONGRUN_WRITE(save_lower, 0),
		  LONGRUN_WRITE(save_upper, 0));
	read_cpuid(CPUID_TMx86_LONGRUN_STATUS, &eax, 0, 0, 0);
	steps = (((freq_max - eax)) / (100.0 / 3.0) + 0.5); /* 33 MHz steps */
	perf = malloc(sizeof(int) * (steps + 1));
	mhz = malloc(sizeof(int) * (steps + 1));
	volts = malloc(sizeof(int) * (steps + 1));
	for (i = 0; i <= steps; i++) {
		mhz[i] = 0;
	}
        levels = 0;

	/* probe */
	pct_last = -1;
	for (i = 0; i <= steps; i++) {
		pct_in = ((float) i * (100.0 / (float) steps));
		write_msr(MSR_TMx86_LONGRUN, LONGRUN_WRITE(save_lower, pct_in),
			  LONGRUN_WRITE(save_upper, pct_in));
		read_cpuid(CPUID_TMx86_LONGRUN_STATUS, &eax, &ebx, &ecx, 0);
		if (pct_last < ecx) {
			perf[i] = ecx;
			mhz[i] = eax;
			volts[i] = ebx;
                        levels++;
		}
		if (ecx == 100)
			break;
		if (ecx > pct_in)
			i++;
		pct_last = ecx;
	}

        cpuState.longRunLevels = malloc(sizeof(u_int8) * levels);
        cpuState.nLongRunLevels = levels;
        levels = 0;

	/* find power_max */
	power_max = mhz[i] * volts[i] * volts[i];

 	fprintf(stderr,"\nLongRun (Transmeta) mode enabled; available CPU power levels are:\n");
 	fprintf(stderr,"# %%   MHz  Volts  usage\n");
	for (i = 0; i <= steps; i++) {
		if (mhz[i]) {
                        cpuState.longRunLevels[levels++] = perf[i];
			power_ratio = mhz[i] * volts[i] * volts[i] / power_max;
 			fprintf(stderr,"%3d %5d %6.3f %6.3f\n",
 			       perf[i], mhz[i], volts[i] / 1000.0, power_ratio);
                        
		}
	}

	free(perf);
	free(mhz);
	free(volts);

	/* restore current settings */
	write_msr(MSR_TMx86_LONGRUN, save_lower, save_upper);
}
