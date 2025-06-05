#include <stdlib.h>
#include "ztime.h"

/*
 * ztime: functions for converting to/from various types of Z ISA and z/OS time
 * formats
 *
 * Reference:
 * Packed Decimal Format: https://tech.mikefulton.ca/PackedDecimalFormat
 * ISPF Stats Layout on disk: https://tech.mikefulton.ca/ISPFStatsLayout
 */

static const int days[] = {
  /* Jan */ 31,
  /* Feb */ 28,
  /* Mar */ 31,
  /* Apr */ 30,
  /* May */ 31,
  /* Jun */ 30,
  /* Jul */ 31,
  /* Aug */ 31,
  /* Sep */ 30,
  /* Oct */ 31,
  /* Nov */ 30,
  /* Dec */ 31
};

/*
 * convert julian (ordinal) day to month/day, where month starts from 0
 */
static void j_to_mmdd(int year, int ordinal, int* month, int* day)
{
  *month = 0;
  *day = 0;
  int i;
  int leap;

  int remainder = ordinal;
  if ((year % 4 == 0)) {
    if ((year % 100 != 0) || (year % 400 == 0)) {
      leap = 1;
    } else {
      leap = 0;
    }
  } else {
    leap = 0;
  }
  for (i = 0; i<12; ++i) {
    int days_in_month = days[i];
    if (i == 1 && leap) {
      days_in_month++;
    }
    if (remainder < days_in_month) {
      *month = i;
      *day = remainder;
      break;
    }
    remainder -= days_in_month;
  }
  return;
}

/*
 * convert month/day to julian (ordinal), where month starts from 0
 */

static void mmdd_to_j(int year, int month, int day, int* ordinal)
{
  int i;
  int leap;

  *ordinal = 0;
  if ((year % 4 == 0)) {
    if ((year % 100 != 0) || (year % 400 == 0)) {
      leap = 1;
    } else {
      leap = 0;
    }
  } else {
    leap = 0;
  }
  for (i = 0; i<month; ++i) {
    int days_in_month = days[i];
    if (i == 1 && leap) {
      days_in_month++;
    }
    *ordinal += days_in_month;
  }
  *ordinal += day;
}

/*
 * convert a one byte packed decimal value to decimal
 */
unsigned int pd_to_d(unsigned char pd)
{
  if ((pd & 0x0FU) == 0x0FU) { /* signed value - just grab top nibble */
    return ((pd & 0xF0U) >> 4U);
  } else {
    return (((pd & 0xF0U) >> 4U)*10U) + (pd & 0x0FU);
  }
}

/*
 * convert a decimal value to packed decimal.
 * If 'set_positive_sign' is specified, then the
 * value is expected to only be from 0 to 9 and the low
 * order nibble has the positive sign set.
 */
unsigned char d_to_pd(unsigned int val, int set_positive_sign)
{
  unsigned char pd;
  if (set_positive_sign) {
    pd = 0x0F;
    pd |= (val << 4);
  } else {
    pd = (val % 10);
    pd |= (val / 10) << 4;
  }
  return pd;
}

/*
 * convert a 3 byte Julian Date that is relative to a start century of
 * 0 (20th century) or 1 (21st century), adjusted for subsequent
 * use in a 'struct tm' format, where 0th month is January, 0th year is 1900,
 * and 1st day is the 1st day of the month.
 */
int pdjd_to_tm(const char* pdjd, int start_century, struct tm* ltime)
{
  int year = pd_to_d(pdjd[0]);
  int tens = pd_to_d(pdjd[1]);
  int ones = pd_to_d(pdjd[2]);
  int ordinal = tens*10 + ones;

  year += (1900 + (start_century*100));
  j_to_mmdd(year, ordinal, &ltime->tm_mon, &ltime->tm_mday);

  ltime->tm_year = (year - 1900);

  return 0;
}

/*
 * given a 'struct tm' structure containing the 
 * year, month, and day, set a 3 byte Julian date in packed
 * decimal format, relative to the closest century.
 * Valid century settings are 0 for the 20th century (1900->1999) and
 * 1 for the 21st century (2000->2099).
 */
void tm_to_pdjd(unsigned char* century, char* pdjd, struct tm* ltime)
{
  int year,month,day;
  int ordinal;

  *century = (ltime->tm_year / 100);

  year = ltime->tm_year + 1900;
  month = ltime->tm_mon;
  day = ltime->tm_mday;

  mmdd_to_j(year, month, day, &ordinal);

  int tens = ordinal / 10;
  int ones = ordinal % 10;

  pdjd[0] = d_to_pd(year%100, 0);
  pdjd[1] = d_to_pd(tens, 0);
  pdjd[2] = d_to_pd(ones, 1);
}

time_t tod_to_time(unsigned long long tod)
{
  /*
   * Note that this conversion does not factor in leap seconds.
   * This is 'on purpose' so that it is consistent with other time
   * stamps such as the time of a zFS file, the time returned from time()
   * and other places that a C user would get the time on z/OS.
   *
   * Having consistent relative time for zFS files and PDS members being
   * reported seems more important than providing an 'accurate' time
   * for a PDS member update which is inconsistent with a time a developer
   * would get from a zFS file.
   */
  unsigned long long rawtime = tod;
  unsigned long long raw1970time = rawtime - 9048018124800000000ULL;
  double doubletime = (raw1970time >> 32);
  double doublesecs = doubletime * 1.048576;
  unsigned long rawseconds = (unsigned long) doublesecs;
  time_t ltime = (time_t) rawseconds;

  return ltime;
}
