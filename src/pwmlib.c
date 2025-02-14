/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2019-2022 Technologic Systems, Inc. dba embeddedTS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h>

#include "pwmlib.h"

int pwm_export(int chip, int chan)
{
	int fd;
	char buf[64];
	int ret;

        /* Quick test if it has already been exported */
        snprintf(buf, sizeof(buf), "/sys/class/pwm/pwmchip%d/pwm%d/enable",
	  chip, chan);
        fd = open(buf, O_WRONLY);
        if(fd >= 0) {
                close(fd);
                return 0;
        }

	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwmchip%d/export", chip);
	fd = open(buf, O_WRONLY);
	if(fd >= 0) {
		snprintf(buf, sizeof(buf), "%d", chan); 
		ret = write(fd, buf, strlen(buf));
		if(ret < strlen(buf)) {
			return -2;
		}
		close(fd);
	} else {
		// If we can't open the export file, we probably
		// dont have proper permissions
		return -1;
	}
	return 0;
}

int pwm_unexport(int chip, int chan)
{
	int fd;
	char buf[64];
	int ret;

	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwmchip%d/unexport", chip);
	fd = open(buf, O_WRONLY);
	if (fd >= 0) {
		sprintf(buf, "%d", chan);
		ret = write(fd, buf, strlen(buf));
		if (ret < strlen(buf)) {
			return -2;
		}
		close(fd);
	} else {
		return -1;
	}

	return 0;
}

int pwm_write(int chip, int chan, int per_ns, int duty_ns)
{
	int period_fd, duty_fd;
	char buf[64];
	int ret;

	/* In order to write any arbitrary values, we have to first clear the
	 * duty cycle and then period, then write period, and then duty cycle.
	 * The reason for this is that if the PWM is already set up with a
	 * specific rate, the duty cycle length cannot exceed the period. By
	 * clearing duty and then period, both are at zero, period can be set
	 * to any arbitrary value, then duty can be set.
	 */

	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwmchip%d/pwm%d/period",
	  chip, chan);
	period_fd = open(buf, O_RDWR);
	if (period_fd < 0)
		return -1;

	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwmchip%d/pwm%d/duty_cycle",
	  chip, chan);
	duty_fd = open(buf, O_RDWR);
	if (duty_fd < 0)
		return -1;


	/* Clear first */
	snprintf(buf, sizeof(buf), "%d", 0);
	ret = write(duty_fd, buf, strlen(buf));
	if(ret < strlen(buf)) {
		return -2;
	}
	lseek(duty_fd, 0, SEEK_SET);

	ret = write(period_fd, buf, strlen(buf));
	if(ret < strlen(buf)) {
		return -2;
	}
	lseek(period_fd, 0, SEEK_SET);
	
	/* Write new values */
	snprintf(buf, sizeof(buf), "%d", per_ns);
	ret = write(period_fd, buf, strlen(buf));
	if(ret < strlen(buf)) {
		return -2;
	}

	snprintf(buf, sizeof(buf), "%d", duty_ns);
	ret = write(duty_fd, buf, strlen(buf));
	if(ret < strlen(buf)) {
		return -2;
	}

	/* Clean up */
	close(period_fd);
	close(duty_fd);

	return 0;
}

int pwm_enable(int chip, int chan, int enable)
{
	int fd;
	char buf[64];
	int ret;

	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwmchip%d/pwm%d/enable",
	  chip, chan);
	fd = open(buf, O_RDWR);
	if(fd >= 0) {
		snprintf(buf, sizeof(buf), "%d", enable);
		ret = write(fd, buf, strlen(buf));
		if(ret < strlen(buf)) {
			return -2;
		}
		close(fd);
	} else {
		return -1;
	}

	return 0;
}
