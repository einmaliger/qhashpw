#ifndef HASHPW_H
#define HASHPW_H

/* allowed chars for the password */

#define FL_LOWER	1		/* lowercase letters */
#define FL_UPPER	2		/* uppercase letters */
#define FL_DIGIT	4		/* digits */
#define FL_SPECIAL	8		/* special chars */

#define FLAGS_LOWER			(FL_LOWER)				/* only lowercase letters */
#define FLAGS_ALPHA			(FL_LOWER|FL_UPPER)		/* any letters */
#define FLAGS_ALNUM			(FLAGS_ALPHA|FL_DIGIT)	/* any letters and digits */
#define FLAGS_PRINT			(FLAGS_ALNUM|FL_SPECIAL)/* any printable characters */
#define FLAGS_DIGIT			(FL_DIGIT)		/* only digit */

#define PARAM_MAX	15		/* maximum value the flags can have */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * mainPW: main password
 * descr: description string (i.e. login)
 * num: number of password
 * min: minimum length of password
 * max: maximum length of password
 * flags: (see above)
 * 
 * returns:
 * 	-1 - input strings longer than allowed
 * 	-2 - invalid flags
 * 	-3 - we cannot handle (max-min)>255
 * 	-9 - not enough memory (i mean, honestly, can this happen these days?)
 */
int getpw(const char *mainPW, const char *descr, int num, int min, int max, unsigned flags, char *result);

#ifdef __cplusplus
}
#endif

#endif
