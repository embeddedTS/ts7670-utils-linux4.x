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
	int fd;
	char buf[64];
	int ret;

	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwmchip%d/pwm%d/period",
	  chip, chan);
	fd = open(buf, O_RDWR);
	if(fd >= 0) {
		snprintf(buf, sizeof(buf), "%d", per_ns);
		ret = write(fd, buf, strlen(buf));
		if(ret < strlen(buf)) {
			return -2;
		}
		close(fd);
	} else {
		return -1;
	}

	snprintf(buf, sizeof(buf), "/sys/class/pwm/pwmchip%d/pwm%d/duty_cycle",
	  chip, chan);
	fd = open(buf, O_RDWR);
	if(fd >= 0) {
		snprintf(buf, sizeof(buf), "%d", duty_ns);
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
