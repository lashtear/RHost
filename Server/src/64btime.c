/* pivotal_gmtime_r - a replacement for gmtime/localtime/mktime
                      that works around the 2038 bug on 32-bit
                      systems. (Version 3)

   Copyright (C) 2005  Paul Sheer

   Redistribution and use in source form, with or without modification,
   is permitted provided that the above copyright notice, this list of
   conditions, the following disclaimer, and the following char array
   are retained.

   Redistribution and use in binary form must reproduce an
   acknowledgment: 'With software provided by http://2038bug.com/' in
   the documentation and/or other materials provided with the
   distribution, and wherever such acknowledgments are usually
   accessible in Your program.

   This software is provided "AS IS" and WITHOUT WARRANTY, either
   express or implied, including, without limitation, the warranties of
   NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE. THE ENTIRE RISK AS TO THE QUALITY OF THIS SOFTWARE IS WITH
   YOU. Under no circumstances and under no legal theory, whether in
   tort (including negligence), contract, or otherwise, shall the
   copyright owners be liable for any direct, indirect, special,
   incidental, or consequential damages of any character arising as a
   result of the use of this software including, without limitation,
   damages for loss of goodwill, work stoppage, computer failure or
   malfunction, or any and all other commercial damages or losses. This
   limitation of liability shall not apply to liability for death or
   personal injury resulting from copyright owners' negligence to the
   extent applicable law prohibits such limitation. Some jurisdictions
   do not allow the exclusion or limitation of incidental or
   consequential damages, so this exclusion and limitation may not apply
   to You.

*/

const char pivotal_gmtime_r_stamp[] =
    "pivotal_gmtime_r. Copyright (C) 2005  Paul Sheer. Terms and "
    "conditions apply. Visit http://2038bug.com/ for more info.";

/*

On 32-bit machines, with 'now' passed as NULL, pivotal_gmtime_r() gives
the same result as gmtime() (i.e. gmtime() of the GNU C library 2.2) for
all values of time_t positive and negative. See the gmtime() man page
for more info.

It is intended that you pass 'now' as the current time (as previously
retrieved with a call such as time(&now);). In this case,
pivotal_gmtime_r() returns the correct broken down time in the range of
    now - 2147483648 seconds
through to
    now + 2147483647 seconds

For example, on 10-Jan-2008, pivotal_gmtime_r() will return the correct
broken down time format from 23-Dec-1939 through 29-Jan-2076.

For all values of 'now' before Jan-23-2005 and after Jan-19-2038,
pivotal_gmtime_r() will return the correct broken down time format from
exactly 01-Jan-1970 through to 07-Feb-2106.

In other words, if, for example, pivotal_gmtime_r() is used in a program
that needs to convert time values of 25 years into the future and 68
years in the past, the program will operate as expected until the year
2106-25=2081. This will be true even on 32-bit systems.

Note that "Jan-23-2005" is the date of the authoring of this code.

Programmers who have available to them 64-bit time values as a 'long
long' type can use gmtime64_r() instead, which correctly converts the
time even on 32-bit systems. Whether you have 64-bit time values
will depend on the operating system.

Both functions are 64-bit clean and should work as expected on 64-bit
systems. They have not yet been tested on 64-bit systems however.

The localtime() equivalent functions do both a POSIX localtime_r() and
gmtime_r() and work out the time zone offset from their difference. This
is inefficient but gives the correct timezone offset and daylight
savings time adjustments.

The function prototypes are:

    double pivot_time_t (const time_t * now, double *t);
    double mktime64 (struct tm *t);
    struct tm *localtime64_r (const double *t, struct tm *p);
    struct tm *pivotal_localtime_r (const time_t * now, const time_t * t, struct tm *p);
    struct tm *gmtime64_r (const double *t, struct tm *p);
    struct tm *pivotal_gmtime_r (const time_t * now, const time_t * t, struct tm *p);

pivot_time_t() takes a 64-bit time that may have had its top 32-bits set
to zero, and adjusts it so that it is in the range explained above. You
can use pivot_time_t() to convert any time that may be incorrect.
pivot_time_t() returns its argument unchanged if either now is NULL or
sizeof(time_t) is not 4.

mktime64() is a 64-bit equivalent of mktime().

localtime64_r() is a 64-bit equivalent of localtime_r().

pivotal_localtime_r() is 32-bit equivalent of localtime_r() with
pivoting.

gmtime64_r() is a 64-bit equivalent of gmtime_r().

pivotal_gmtime_r() is a 32-bit equivalent of gmtime_r() with pivoting.

Note that none of these functions handle leap seconds.

RATIONALE: The purpose of pivotal_gmtime_r() is as a replacement for the
functions gmtime(), localtime() and their corresponding reentrant
versions gmtime_r() and localtime_r().

pivotal_gmtime_r() is meant for 32-bit systems that must still correctly
convert 32-bit time into broken down time format through the year 2038.
Since most programs tend to operate within a range of time no more than
68 years in the future or the past, it is possible to determine the
correct interpretation of a 32-bit time value in spite of the wrap that
occurs in the year 2038.

Many databases are likely to store time in 32-bit format and not be
easily upgradeable to 64-bit. By using pivot_time_t(), these time values
can be correctly used.


Changes:
    06-Feb-2005  v3.0:  Some optimizations.
			mktime() no-longer zeros tm struct.

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "autoconf.h"
#include "tprintf.h"
#include "externs.h"
#define MAXSIGNED 2147483648.0
#define MAXUNSIGNED 4294967296.0

static const int days[4][13] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
};

#define LEAP_CHECK(n)	((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)
#define WRAP(a,b,m)	((a) = ((a) <  0  ) ? ((b)--, (a) + (m)) : (a))

double pivot_time_t (const time_t * now, double *_t)
{
    double t;
    t = *_t;
    if (now && sizeof (time_t) == 4) {
        time_t _now;
        _now = *now;
        if (_now < 1106500000 /* Jan 23 2005 - date of writing */ )
            _now = 2147483647;
        if ((double) t + (double)MAXSIGNED < (double) _now)
            t += (double) MAXUNSIGNED;
    }
    return t;
}

static struct tm *_gmtime64_r (const time_t * now, double *_t, struct tm *p)
{
    int v_tm_sec, v_tm_min, v_tm_hour, v_tm_mon, v_tm_wday, v_tm_tday;
    int leap;
    double t;
    long m;
    t = pivot_time_t (now, _t);
    if ( t > 90000000000000.0 )
       t = 90000000000000.0;
    if ( t < -90000000000000.0 )
       t = -90000000000000.0;
    v_tm_sec = fmod(t,60);
    t /= 60;
    v_tm_min = fmod(t,60);
    t /= 60;
    v_tm_hour = fmod(t,24);
    t /= 24;
    v_tm_tday = t;
    WRAP (v_tm_sec, v_tm_min, 60);
    WRAP (v_tm_min, v_tm_hour, 60);
    WRAP (v_tm_hour, v_tm_tday, 24);
    if ((v_tm_wday = (v_tm_tday + 4) % 7) < 0)
        v_tm_wday += 7;
    m = (long) v_tm_tday;
    if (m >= 0) {
        p->tm_year = 70;
        leap = LEAP_CHECK (p->tm_year);
        while (m >= (long) days[leap + 2][12]) {
            m -= (long) days[leap + 2][12];
            p->tm_year++;
            leap = LEAP_CHECK (p->tm_year);
        }
        v_tm_mon = 0;
        while (m >= (long) days[leap][v_tm_mon]) {
            m -= (long) days[leap][v_tm_mon];
            v_tm_mon++;
        }
    } else {
        p->tm_year = 69;
        leap = LEAP_CHECK (p->tm_year);
        while (m < (long) -days[leap + 2][12]) {
            m += (long) days[leap + 2][12];
            p->tm_year--;
            leap = LEAP_CHECK (p->tm_year);
        }
        v_tm_mon = 11;
        while (m < (long) -days[leap][v_tm_mon]) {
            m += (long) days[leap][v_tm_mon];
            v_tm_mon--;
        }
        m += (long) days[leap][v_tm_mon];
    }
    p->tm_mday = (int) m + 1;
    p->tm_yday = days[leap + 2][v_tm_mon] + m;
    p->tm_sec = v_tm_sec, p->tm_min = v_tm_min, p->tm_hour = v_tm_hour,
        p->tm_mon = v_tm_mon, p->tm_wday = v_tm_wday;
    return p;
}

struct tm *gmtime64_r (const double *_t, struct tm *p)
{
    double t;
    t = *_t;
    return _gmtime64_r (NULL, &t, p);
}

struct tm *pivotal_gmtime_r (const time_t * now, const time_t * _t, struct tm *p)
{
    double t;
    t = *_t;
    return _gmtime64_r (now, &t, p);
}

double mktime64 (struct tm *t)
{
    int i, y;
    long day = 0;
    double r;
    if (t->tm_year < 70) {
        y = 69;
        do {
            day -= 365 + LEAP_CHECK (y);
            y--;
        } while (y >= t->tm_year);
    } else {
        y = 70;
        while (y < t->tm_year) {
            day += 365 + LEAP_CHECK (y);
            y++;
        }
    }
    for (i = 0; i < t->tm_mon; i++)
        day += days[LEAP_CHECK (t->tm_year)][i];
    day += t->tm_mday - 1;
    t->tm_wday = (int) ((day + 4) % 7);
    r = (double) day *86400;
    r += t->tm_hour * 3600;
    r += t->tm_min * 60;
    r += t->tm_sec;
    return r;
}

static struct tm *_localtime64_r (const time_t * now, double *_t, struct tm *p)
{
    double tl;
    time_t t;
    struct tm tm, tm_localtime, tm_gmtime;
    _gmtime64_r (now, _t, &tm);
    if (tm.tm_year > (2037 - 1900))
        tm.tm_year = 2037 - 1900;
    t = mktime64 (&tm);
    localtime_r (&t, &tm_localtime);
    gmtime_r (&t, &tm_gmtime);
    tl = *_t;
    tl += (mktime64 (&tm_localtime) - mktime64 (&tm_gmtime));
    _gmtime64_r (now, &tl, p);
    p->tm_isdst = tm_localtime.tm_isdst;
    return p;
}

struct tm *pivotal_localtime_r (const time_t * now, const time_t * _t, struct tm *p)
{
    double tl;
    tl = *_t;
    return _localtime64_r (now, &tl, p);
}

struct tm *localtime64_r (const double *_t, struct tm *p)
{
    double tl;
    tl = *_t;
    return _localtime64_r (NULL, &tl, p);
}
