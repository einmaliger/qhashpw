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

/* Every value of PasswortOptions::hash less than
 * this value will be allowed */
#define MAX_HASH_VALUE          3

static const EVP_MD *get_evp_md_for_hash(int hash)
{
    assert(hash <= MAX_HASH_VALUE);

    switch(hash)
    {
    case HASH_RIPEMD160:  return EVP_ripemd160();
    case HASH_SHA1:       return EVP_sha1();
    case HASH_DSS1:       return EVP_dss1();
    case HASH_MD5:        return EVP_md5();
    default:              return NULL;      /* Will never be reached */
    }
}

/* A set of special characters */
static const char *specialchars="!\"#$%&'()*+-./:;<=>?@[\\]^_{|}~";

void init_PasswordOptions(struct PasswordOptions *opt)
{
    memset((void*)opt, 0, sizeof(struct PasswordOptions));
}

/*******************************************************************/
/** Password generation                                           **/

int getpw(const char *mainPW, const char *descr, int num, int min, int max, unsigned flags, char *result)
{
    struct PasswordOptions opt;

    if(flags > PARAM_MAX_V1) return -2;

    opt.mainPW = mainPW;
    opt.salt = "";
    opt.descr = descr;
    opt.num = num;
    opt.min = min;
    opt.max = max;
    opt.flags = flags;

    opt.hash = HASH_RIPEMD160;

    return getpw2(&opt, result);
}

int getpw2(const struct PasswordOptions *opt, char *result)
{
	/* format of hashed string:
	 * <mainPW><seq><descr><num>
	 * where <seq> is sequential number (if more than one hash value is needed)
	 * where <fl> is 'a'+flags
	 */

        assert(opt->flags);
        assert(opt->hash <= MAX_HASH_VALUE);

	// check for sane input values
        if(strlen(opt->mainPW) > MAX_INPUT_LENGTH ||
           strlen(opt->descr) > MAX_INPUT_LENGTH ||
	   strlen(opt->salt) > MAX_INPUT_LENGTH)
		return -1;

        if(opt->flags > PARAM_MAX_V2) return -2;

        if(opt->hash > MAX_HASH_VALUE) return -4;

        /* If FL_EVENDIST we wait until a character is not a "leading 0" */
        int password_has_started;

        /* number of characters to create or -1 if we don't know that
         * number (i.e., !FL_EVENDIST and min != max) */
        int state;

        /* Needed to determine length value (if !FL_EVENDIST) */
        unsigned char mask;

        if(opt->flags & FL_EVENDIST)
        {
            password_has_started = 0;
            state = opt->max;
        }
         else
        {
            /* Determine password size the "classical" way,
             * by selecting it evenly from min .. max */

            if(opt->max-opt->min>255) return -3;

            /* Do not wait for start of password once we have determined the length */
            password_has_started = 1;

            // Determine bit mask for length byte
            mask = 0;
            while(opt->max-opt->min && ((opt->max-opt->min) & (128 >> mask)) == 0) mask++;
            mask = 0xff >> mask;

            // both state number and counter
            // too complicated to explain ;-)
            state = opt->max-opt->min?-1:opt->min;
        }

	// size of hashed string buffer
        int tempLength = MAX_INT_REP_LEN*2+strlen(opt->salt)+strlen(opt->descr);

	char *tempStr = (char *)malloc(tempLength+1);
	if(tempStr == NULL) return -9;

	unsigned char hmac[EVP_MAX_MD_SIZE];		// hmac output
        unsigned int hmaclen;                           // length of current hmac

	int seq = 0;		// seq

	unsigned int i = 0;	// index within hmac output
						// if this is 0, a new
						// hash (with seq++) will be
						// created

        const EVP_MD *hash_algo = get_evp_md_for_hash(opt->hash);

        if(hash_algo == NULL) return -4;

        while(state)
	{
		// retrieve a single byte
		if(i == 0)
		{
			// next hmac
                        snprintf(tempStr, tempLength, "%i%s%s%i",
				seq++,
                                opt->salt,
                                opt->descr,
                                opt->num);

                        HMAC(hash_algo,
                                opt->mainPW, strlen(opt->mainPW),
				(unsigned char *)tempStr, strlen(tempStr),
				hmac, &hmaclen);
			
			i = hmaclen;
		}

		unsigned char b = hmac[--i];

		// if state == -1, we are determining the length of the password
		if(state == -1)
		{
			b &= mask;
                        if(b < opt->max-opt->min) state = ((int)b)+opt->min;
			continue;
		}

                if(!password_has_started)
                {
                    /* As long as we have more than min character
                     * to create, we can have leading zeros,
                     * If only min chars are left, we must
                     * start */
                    if( b != 0 || state == opt->min)
                    {
                        password_has_started = 1;
                    }
                     else
                    continue;
                }

		// else we are retrieving the next passwort byte
		// test if b is a valid byte
                if( ((opt->flags&FL_LOWER) && islower(b)) ||
                        ((opt->flags&FL_UPPER) && isupper(b)) ||
                        ((opt->flags&FL_DIGIT) && isdigit(b)) ||
                        ((opt->flags&FL_SPECIAL) && b!= 0 && strchr(specialchars, b)))
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
