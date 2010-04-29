/* hashpwlib */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/hmac.h>

#include "hashpw.h"

/* maximum length of a string representation of an
 * integer. Used to determine a good size for snprintf
 * and for the maximum allowed size of integers for
 * the tokenizer*/
#define MAX_INT_REP_LEN	(3*65)

/* maximum allowed string length for input strings.
 * Only used for sanity checks */
#define MAX_INPUT_LENGTH	1024

/* A set of special characters */
static const char *specialchars="!\"#$%&'()*+-./:;<=>?@[\\]^_{|}~";

/*******************************************************************/
/** Password generation                                           **/

int getpw(const char *mainPW, const char *descr, int num, int min, int max, unsigned flags, char *result)
{
	/* format of hashed string:
	 * <mainPW><seq><descr><num>
	 * where <seq> is sequential number (if more than one hash value is needed)
	 * where <fl> is 'a'+flags
	 */

	assert(PARAM_MAX < 26);
	assert(flags);

	// check for sane input values
	if(strlen(mainPW) > MAX_INPUT_LENGTH ||
	   strlen(descr) > MAX_INPUT_LENGTH)
		return -1;

	if(flags > PARAM_MAX) return -2;

	if(max-min>255) return -3;

	// Determine bit mask for length byte
	unsigned char mask = 0;
	while(max-min && ((max-min) & (128 >> mask)) == 0) mask++;
	mask = 0xff >> mask;

	// size of hashed string buffer
	int tempLength = MAX_INT_REP_LEN*2+strlen(descr);

	char *tempStr = (char *)malloc(tempLength+1);
	if(tempStr == NULL) return -9;

	unsigned char hmac[EVP_MAX_MD_SIZE];		// hmac output
	unsigned int hmaclen;		// length of current hmac

	int seq = 0;		// seq

	unsigned int i = 0;	// index within hmac output
						// if this is 0, a new
						// hash (with seq++) will be
						// created

	// both state number and counter
	// too complicated to explain ;-)
	int state = max-min?-1:min;

	while(state)
	{
		// retrieve a single byte
		if(i == 0)
		{
			// next hmac
			snprintf(tempStr, tempLength, "%i%s%i",
				seq++,
				descr,
				num);

			HMAC(EVP_ripemd160(),
				mainPW, strlen(mainPW),
				(unsigned char *)tempStr, strlen(tempStr),
				hmac, &hmaclen);
			
			i = hmaclen;
		}

		unsigned char b = hmac[--i];

		// if state == -1, we are determining the length of the password
		if(state == -1)
		{
			b &= mask;
			if(b < max-min) state = ((int)b)+min;
			continue;
		}

		// else we are retrieving the next passwort byte
		// test if b is a valid byte
		if( ((flags&FL_LOWER) && islower(b)) ||
			((flags&FL_UPPER) && isupper(b)) ||
			((flags&FL_DIGIT) && isdigit(b)) ||
			((flags&FL_SPECIAL) && b!= 0 && strchr(specialchars, b)))
		{
			// okay, valid char
			*result++ = (char)b;
			state--;
		}

		// with overwhelming probability, this
		// loop will terminate
	}

	free(tempStr);

	// zero terminate string
	*result=0;

	return 0;
}
