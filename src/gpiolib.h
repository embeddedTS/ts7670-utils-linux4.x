/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2019-2022 Technologic Systems, Inc. dba embeddedTS */

#ifndef _GPIOLIB_H_

int gpio_direction(int gpio, int dir);
int gpio_export(int gpio);
void gpio_unexport(int gpio);
int gpio_read(int gpio);
int gpio_write(int gpio, int val);
int gpio_setedge(int gpio, int rising, int falling);
int gpio_select(int gpio);

#endif //_GPIOLIB_H_
