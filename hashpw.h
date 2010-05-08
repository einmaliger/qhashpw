/*
 * Copyright 2010 (c) Sascha Mueller <mailbox@saschamueller.com>
 *
 * This file is part of qhashpw.
 *
 * qhashpw is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * qhashpw is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with qhashpw.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HASHPW_H
#define HASHPW_H

#define HASHPW_VERSION  2

/****** allowed chars for the password (v1+) ******/

#define FL_LOWER	1		/* lowercase letters */
#define FL_UPPER	2		/* uppercase letters */
#define FL_DIGIT	4		/* digits */
#define FL_SPECIAL	8		/* special chars */

#define FLAGS_LOWER			(FL_LOWER)				/* only lowercase letters */
#define FLAGS_ALPHA			(FL_LOWER|FL_UPPER)		/* any letters */
#define FLAGS_ALNUM			(FLAGS_ALPHA|FL_DIGIT)	/* any letters and digits */
#define FLAGS_PRINT			(FLAGS_ALNUM|FL_SPECIAL)/* any printable characters */
#define FLAGS_DIGIT			(FL_DIGIT)		/* only digit */

#define PARAM_MAX_V1	15		/* maximum value the flags can have in getpw(1)*/

/****** more options (v2+) ******/

/* passwords evenly distributed along the set of possible passwords.
 * Note that there are much more long passwords than short ones, so setting
 * this flag will cause pwgen2 to nearly always generate long passwords. */
#define FL_EVENDIST    16

#define PARAM_MAX_V2   31

#define HASH_RIPEMD160  0
#define HASH_SHA1       1
#define HASH_DSS1       2
#define HASH_MD5        3

#ifdef __cplusplus
extern "C" {
#endif

/* this struct is only needed for v2+ */
struct PasswordOptions
{
    const char *mainPW;       /* main password */
    const char *salt;         /* main salt value ("" for v1) */
    const char *descr;        /* description string (i.e. login) */
    int num;                  /* sequential number of password (normally 0) */
    int min;                  /* minimum length of password */
    int max;                  /* maximum length of password */
    unsigned flags;           /* flags (see above FL_ constants) */
    int hash;                 /* hash algorithm to use (see above HASH_ constants) */
};

/* For upwards compatibility, call this function before filling in the values
 * of the struct. In future versions, it will set new fields to default values
 * to retain downwards compatibility.
 * Right now it sets everything to zero. */
void init_PasswordOptions(struct PasswordOptions *opt);

/*
 * Compatibility function for v1
 *
 * returns:
 * 	-1 - input strings longer than allowed
 * 	-2 - invalid flags
 * 	-3 - we cannot handle (max-min)>255 when FL_EVENDIST is not set
 *      -4 - (v2+) unknown/unsupported hash algorithm
 * 	-9 - not enough memory (i mean, honestly, can this happen these days?)
 */
int getpw(const char *mainPW, const char *descr, int num, int min, int max, unsigned flags, char *result);

int getpw2(const struct PasswordOptions *opt, char *result);

#ifdef __cplusplus
}
#endif

#endif
