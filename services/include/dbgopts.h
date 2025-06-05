#ifndef __DBG_OPTS_H__
  #define __DBG_OPTS_H__

  /*
   * The DBG_Opts parameter is passed to most services and
   * can be used to control verbose and debug information 
   * printed out by the services.
   */
  typedef struct {
    int verbose:1;
    int debug:1;
  } DBG_Opts;
#endif
