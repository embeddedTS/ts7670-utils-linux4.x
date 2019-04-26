#ifndef _PWMLIB_H_

int pwm_export(int chip, int chan);
int pwm_unexport(int chip, int chan);
int pwm_write(int chip, int chan, int per_ns, int duty_ns);
int pwm_enable(int chip, int chan, int enable);

#endif //_PWMLIB_H_
