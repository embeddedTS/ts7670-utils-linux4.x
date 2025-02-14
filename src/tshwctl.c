/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2019-2022 Technologic Systems, Inc. dba embeddedTS */

#include <stdio.h>
#include <unistd.h>
#include <dirent.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <gpiod.h>
#include "pwmlib.h"
#include "i2c-rtc.h"

const char copyright[] = "Copyright (c) embeddedTS - " __DATE__ " - "
  GITCOMMIT;

int model = 0;

int get_model()
{
	FILE *proc;
	char mdl[256];
	char *ptr;

	proc = fopen("/proc/device-tree/model", "r");
	if (!proc) {
	    perror("model");
	    return 0;
	}
	fread(mdl, 256, 1, proc);
	ptr = strstr(mdl, "TS-");
	return strtoull(ptr+3, NULL, 16);
}

static void usage(char **argv) {
	fprintf(stderr,
	  "%s\n\n"
	  "Usage: %s [OPTION] ...\n"
	  "embeddedTS Hardware access\n"
	  "\n"
	  "  -i, --info              Print device information\n"
	  "  -o, --rtcinfo           Print RTC power on/off timestamp\n"
	  "  -v, --nvram             Get/Set RTC NVRAM\n"
	  "  -4, --485speed=<baud>   Baud rate, required for RS-485HD to function\n"
	  "  -1, --modbuspoweron     Enable VIN to modbus port\n"
	  "  -Z, --modbuspoweroff    Turn off VIN to modbus port\n"
	  "  -h, --help              This message\n"
	  "\n",
	  copyright, argv[0]
	);
}

int main(int argc, char **argv)
{
	int i;

	int opt_info = 0;
	
	int opt_rtcinfo = 0;
	char rtcdat[11];

	int opt_nvram = 0;
	int rtcfd = 0;
	uint32_t nvdat[32], x;
	char *cdat = (char *)nvdat;
	char *e, var[8];

	int opt_modbuspoweron = 0;
	int opt_modbuspoweroff = 0;

	struct gpiod_chip *chip1 = NULL;
	struct gpiod_line *modbus_en_3vn = NULL;
	struct gpiod_line *modbus_en_24v = NULL;
	struct gpiod_line *modbus_fault = NULL;

	int opt_485speed = 0;

	static struct option long_options[] = {
	  { "info", 0, 0, 'i' },
	  { "rtcinfo", 0, 0, 'o' },
	  { "nvram", 0, 0, 'v' },
	  { "modbuspoweron", 0, 0, '1'},
	  { "modbuspoweroff", 0, 0, 'Z'},
	  { "485speed", 1, 0, '4'},
	  { "help", 0, 0, 'h' },
	  { 0, 0, 0, 0 }
	};

	if(argc == 1) {
		usage(argv);
		return(1);
	}

	model = get_model();
	switch(model) {
	  case 0x7670:
		break;
	  default:
		fprintf(stderr, "Unsupported model: 0x%X\n", model);
		return 1;
	}

	while((i = getopt_long(argc, argv, 
	  "14:iohvZ",
	  long_options, NULL)) != -1) {
		switch (i) {
		  case 'i':
			opt_info = 1;
			break;
		  case 'o':
			opt_rtcinfo = 1;
			break;
		  case 'v':
			opt_nvram = 1;
			break;
		  case '1':
			opt_modbuspoweron = 1;
			opt_modbuspoweroff = 0;
			break;
		  case 'Z':
			opt_modbuspoweroff = 1;
			opt_modbuspoweron = 0;
			break;
		  case '4':
			opt_485speed = strtoul(optarg, NULL, 0);
			break;
		  case 'h':
		  default:
			usage(argv);
			return 1;
		}
	}

	if (opt_info) {
		printf("model=%X\n", get_model());
	}

	if (opt_nvram) {
		rtcfd = i2c_rtc_init();
		i2c_rtc_read(rtcfd, 0x57, cdat, 0x0, 128);
		for (i = 0; i < 32; i++) {
			sprintf(var, "nvram%d", i);
			e = getenv(var);
			if (e) {
				x = strtoul(e, NULL, 0);
				i2c_rtc_write(rtcfd, 0x57, (char *)&x, i<<2, 4);
			}
		}
		for (i = 0; i < 32; i++) printf("nvram%d=0x%x\n", i, nvdat[i]);
        }

        if (opt_rtcinfo) {
		rtcfd = i2c_rtc_init();

#ifdef RTCTEMP
		/* Enable the TSE bit.
		 * NOTE! The kernel driver is intended to handle the RTC but
		 * does not use the temperature compensation. Enabling this may
		 * cause the RTC driver (or system time) to compensate more than
		 * it should. However, this is needed for RTC temp sensing
		 * results.
		 *
		 * Calibration and adjustment of the temp sensing does not occur
		 * here or in kernel. See ISL1202 datasheet for more info
		 *
		 * To enable this chunk of code, compile with:
		 * make CFLAGS="-DRTCTEMP"
		 */
		i2c_rtc_read(rtcfd, 0x6F, &rtcdat[0], 0xd, 1);
                rtcdat[0] = (0xc0 | (rtcdat[0] & 0x1f));
		i2c_rtc_write(rtcfd, 0x6F, &rtcdat[0], 0xd, 1);

		i2c_rtc_read(rtcfd, 0x6F, &rtcdat[0], 0x28, 2);
		printf("rtctemp_millicelsius=%d\n",
		  ((rtcdat[0]|(rtcdat[1]<<8))*500)-273000);
#endif // RTCTEMP


		i2c_rtc_read(rtcfd, 0x6F, &rtcdat[1], 0x16, 5);
		i2c_rtc_read(rtcfd, 0x6F, &rtcdat[6], 0x1b, 5);

		printf("rtcinfo_firstpoweroff=%02x%02x%02x%02x%02x\n",
		  rtcdat[5], rtcdat[4], (rtcdat[3] & 0x7f), rtcdat[2],
		  rtcdat[1]);
		printf("rtcinfo_lastpoweron=%02x%02x%02x%02x%02x\n",
		  rtcdat[10], rtcdat[9], (rtcdat[8] & 0x7f), rtcdat[7],
		  rtcdat[6]);
		/* Read the current setting of brownout voltages, set by kernel
		 * Then clear the timestamp reg */
		i2c_rtc_read(rtcfd, 0x6F, &rtcdat[0], 0x9, 1);
		rtcdat[0] |= 0x80;
		i2c_rtc_write(rtcfd, 0x6F, &rtcdat[0], 0x9, 1);
        }

	if(opt_485speed) {
		uint32_t speed;

		if (opt_485speed > 1042000) {
			printf("Warning! Auto-TXEN circuit may not function "\
			  "properly above 1,042,000 baud! Setting for %d baud "\
			  "anyway.\n", opt_485speed);
		}

		/* Export the correct PWM channel */
		pwm_export(0, 2);

		/* The clock rate needs to be between 1.25 and 1.3 times the
		 * desired baud rate. Use 1.275 to be in the middle of that.
		 * Integer math is okay for this since its dealing in ns of
		 * time and there is enough resolution even at the high end of
		 * the compatible speed.
		 */
		speed = (1000000000/(opt_485speed*1.275));
		pwm_write(0, 2, speed, speed/2);

		pwm_enable(0, 2, 1);
	}

	if (opt_modbuspoweron || opt_modbuspoweroff) {
		chip1 = gpiod_chip_open_by_number(1);
		if (chip1 == NULL) {
			fprintf(stderr, "Unable to open GPIO chip!\n");
			return 1;
		}

		modbus_en_3vn = gpiod_chip_get_line(chip1, 15);
		if (modbus_en_3vn == NULL) {
			fprintf(stderr, "Unable to get GPIO line MODBUS_3V#_EN!\n");
			return 1;
		}

		modbus_en_24v = gpiod_chip_get_line(chip1, 13);
		if (modbus_en_24v == NULL) {
			fprintf(stderr, "Unable to get GPIO line MODBUS_24V_EN!\n");
			return 1;
		}

		modbus_fault = gpiod_chip_get_line(chip1, 14);
		if (modbus_fault == NULL) {
			fprintf(stderr, "Unable to get GPIO line MODBUS_FAULT!\n");
			return 1;
		}

		if (gpiod_line_request_output(modbus_en_3vn, "tshwctl", 1) < 0) {
			fprintf(stderr, "Unable to req GPIO line MODBUS_3V#_EN!\n");
			return 1;
		}

		if (gpiod_line_request_output(modbus_en_24v, "tshwctl", 0) < 0) {
			fprintf(stderr, "Unable to req GPIO line MODBUS_24V_EN!\n");
			return 1;
		}

		if (gpiod_line_request_input(modbus_fault, "tshwctl") < 0) {
			fprintf(stderr, "Unable to req GPIO line MODBUS_FAULT!\n");
			return 1;
		}
	}

	if(opt_modbuspoweron) {
		/* All pins in a safe state at this point. Turn on 3.3 V, wait
		 * and check for fault.
		 */
		gpiod_line_set_value(modbus_en_3vn, 0);
		usleep(10000);

		if (gpiod_line_get_value(modbus_fault) == 0) {
			/* No fault, disable 3 V and drive 24 V */
			gpiod_line_set_value(modbus_en_24v, 1);
			printf("modbuspoweron=1\n");
		} else {
			/* Fauled for some reason, disable 3.3 V out */
			gpiod_line_set_value(modbus_en_3vn, 1);
			printf("modbuspoweron=0\n");
		}
	}

	/* NOTE:
	 * Since opt_modbuspoweron and opt_modbuspoweroff are mutually exclusive
	 * from optarg settings, and the actual line request process puts the
	 * pins in a safe state, there is no need for any further actions for
	 * opt_modbuspoweroff.
	 */

	if (opt_modbuspoweron || opt_modbuspoweroff) {
		gpiod_line_release(modbus_en_3vn);
		gpiod_line_release(modbus_en_24v);
		gpiod_line_release(modbus_fault);
		gpiod_chip_close(chip1);
	}

	return 0;
}
