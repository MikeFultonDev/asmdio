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

  int thousands = pd_to_d(pdjd[0]);
  int tens = pd_to_d(pdjd[1]);
  int ones = pd_to_d(pdjd[2]);
  int jd = thousands*1000 + tens*10 + ones;

  int I,J,L,N,K;
  if (start_century == 1) {
    jd += 2436310;
  } else if (start_century == 0) {
    return 8; /* need to add support for 20th century dates */
  } else {
    return 4; /* unsupported start century */
  }

  /*
   * Julian date to Gregorian date algorithm from:
   * https://aa.usno.navy.mil/faq/JD_formula
   */
  L = jd+68569;
  N = 4*L/146097;
  L = L-(146097*N+3)/4;
  I = 4000*(L+1)/1461001;
  L = L-1461*I/4+31;
  J = 80*L/2447;
  K = L-2447*J/80;
  L = J/11;
  J = J+2-12*L;
  I = 100*(N-49)+I+L;

  /*
   * For 'struct tm', year starts from 1900, months start from 0, days start from 1
   */
  ltime->tm_year = (I-1900);
  ltime->tm_mon = (J-1);
  ltime->tm_mday = K;

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
  int YEAR,MONTH,DAY,I,J,K;
  int JD;

  YEAR = ltime->tm_year + 1900;
  MONTH = ltime->tm_mon + 1;
  DAY = ltime->tm_mday;

  /*
   * From: https://aa.usno.navy.mil/faq/JD_formula
   */
  I  = YEAR;
  J  = MONTH;
  K  = DAY;
  JD = K-32075+1461*(I+4800+(J-14)/12)/4+367*(J-2-(J-14)/12*12)/12-3*((I+4900+(J-14)/12)/100)/4;

  /*
   * adjust to days from 2000
   */
  *century = 1;   /* msf - add code for dates before 2000 */
  JD -= 2436310;  /* 01/01/2000 Julian                    */

  int thousands = JD/1000;
  int tens = (JD % 1000) / 10;
  int ones = (JD % 10);

  pdjd[0] = d_to_pd(thousands, 0);
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
