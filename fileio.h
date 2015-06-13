/**************************************************
 * Project: libkohn_gif
 *  Author: Michael A. Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: Copyright 2004-2012 under LGPL
 **************************************************/

int write_int32(FILE *out, int n);
int write_int16(FILE *out, int n);

int read_int32(FILE *in);
int read_int16(FILE *in);

int read_chars(FILE *in, char *s, int count);
int write_chars(FILE *out, char *s);

