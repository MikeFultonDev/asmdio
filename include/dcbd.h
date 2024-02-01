#pragma pack(packed)

/*
 * From: DCBD DSORG=(PO),DEVD=(DA)
 */
struct ihadcb {
  union {
    void          *dcbdcbe;     /* DCBE ADDRESS ASSOCIATED WITH THIS    @L5A */
    unsigned char  dcbrelad[4]; /* --- PARTITIONED ORGANIZATION DATA SET -   */
    };
  char           dcbkeycn;     /* KEYED BLOCK OVERHEAD CONSTANT             */
  union {
    unsigned char  dcbfdad[8]; /* FULL DISK ADDRESS IN THE FORM OF MBBCCHHR */
    struct {
      unsigned char  _filler1[7];
      struct {
        unsigned char  _filler2;      /* LAST BYTE OF DCBFDAD                      */
        unsigned int   dcbdvtba : 24; /* ADDRESS OF ENTRY IN I/O DEVICE            */
        } dcbdvtbl;                   /* SAME AS DCBDVTBA BELOW                    */
      };
    struct {
      unsigned char  _filler3[11];
      struct {
        char           dcbkeyle;    /* KEY LENGTH OF DATA SET                    */
        struct {
          unsigned char  _filler4;
          } dcbdevt;                  /* DEVICE TYPE                               */
        unsigned char  _filler5[2];
        } dcbrelb;                  /* SAME AS DCBREL BELOW                      */
      };
    struct {
      unsigned char  _filler6[11];
      char           _filler7;     /* DCBKEYLE - KEY LENGTH OF DATA SET         */
      unsigned char  _filler8;     /* DCBDEVT - DEVICE TYPE                     */
      short int      dcbtrbal;     /* CODE INDICATING CAPACITY REMAINING ON     */
      };
    struct {
      unsigned char  _filler9[12];
      int            dcbrel : 24;  /* NUMBER OF RELATIVE TRACKS OR BLOCKS IN    */
      struct {
        char           dcbbufno;      /* NUMBER OF BUFFERS REQUIRED FOR THIS DATA  */
        unsigned int   dcbbufca : 24; /* ADDRESS OF BUFFER POOL CONTROL BLOCK      */
        } dcbbufcb;                   /* ADDRESS OF BUFFER POOL CONTROL BLOCK      */
      };
    };
  short int      dcbbufl;      /* LENGTH OF BUFFER.  MAY RANGE FROM 0 TO    */
  union {
    unsigned char  dcbdsorg[2]; /* DATA SET ORGANIZATION BEING USED */
    struct {
      unsigned int   dcbdsgis : 1, /* IS - INDEXED SEQUENTIAL ORGANIZATION      */
                     dcbdsgps : 1, /* PS - PHYSICAL SEQUENTIAL ORGANIZATION     */
                     dcbdsgda : 1, /* DA - DIRECT ORGANIZATION                  */
                     dcbdsgcx : 1, /* CX - BTAM OR QTAM LINE GROUP              */
                              : 2,
                     dcbdsgpo : 1, /* PO - PARTITIONED ORGANIZATION             */
                     dcbdsgu  : 1; /* U  - UNMOVABLE, THE DATA CONTAINS         */
      unsigned int   dcbdsggs : 1, /* GS - GRAPHICS ORGANIZATION                */
                     dcbdsgtx : 1, /* TX - TCAM LINE GROUP                      */
                     dcbdsgtq : 1, /* TQ - TCAM MESSAGE QUEUE                   */
                              : 1,
                     dcbacbm  : 1, /* ALWAYS 0 IN DCB. ALWAYS 1 IN ACB     @L5C */
                     dcbdsgtr : 1, /* TR - TCAM 3705                            */
                              : 2;
      struct {
        struct {
          struct {
            struct {
              unsigned char  dcbqslm; /* QSAM LOCATE MODE LOGICAL RECORD INTERFACE */
              } dcblnp;               /* 3525 PRINTER LINE POSITION COUNTER        */
            unsigned int   dcbodeba : 24; /* ADDRESS OF OLD DEB                        */
            } dcbodeb;                    /* ADDRESS OF OLD DEB                        */
          } dcbicqe;       /* ADDRESS OF ICQE                           */
        } dcbiobad;      /* ADDRESS OF IOB WHEN CHAINED SCHEDULING IS */
      };
    struct {
      unsigned char  _filler10[2];
      struct {
        unsigned char  _filler11;     /* RESERVED                         */
        unsigned int   dcbsvcxa : 24; /* POINTER TO EXIT LIST OF JES      */
        } dcbsvcxl;                   /* SAME AS DCBSVCXA BELOW           */
      };
    struct {
      unsigned char  _filler12[6];
      struct {
        struct {
          struct {
            unsigned char  dcbbfaln; /* BUFFER ALIGNMENT BITS                 */
            } dcbbftek;              /* BUFFERING TECHNIQUE BITS              */
          } dcbhiarc;      /* HIERARCHY BITS                        */
        unsigned int   dcbeoda : 24; /* ADDRESS OF A USER-PROVIDED ROUTINE TO */
        } dcbeodad;                  /* SAME AS DCBEODA BELOW                 */
      };
    struct {
      unsigned char  _filler13[10];
      struct {
        unsigned char  dcbrecfm;      /* RECORD FORMAT                          */
        unsigned int   dcbexlsa : 24; /* ADDRESS OF USER-PROVIDED LIST OF EXITS */
        } dcbexlst;                   /* ADDRESS OF USER-PROVIDED LIST OF EXITS */
      };
    };
  union {
    unsigned char  dcbddnam[8]; /* NAME ON THE DD STATEMENT WHICH DEFINES */
    struct {
      unsigned short dcbtiot; /* OFFSET FROM TIOT ORIGIN TO TIOELNGH FIELD */
      struct {
        unsigned char  dcbmacf1; /* FIRST BYTE OF DCBMACRF                    */
        unsigned char  dcbmacf2; /* SECOND BYTE OF DCBMACRF                   */
        } dcbmacrf;              /* SAME AS DCBMACR BEFORE OPEN               */
      struct {
        unsigned int   dcbifec  : 2, /* ERROR CORRECTION INDICATOR                */
                       dcbifpct : 2, /* PRINTER CARRIAGE TAPE PUNCH INDICATOR     */
                       dcbifioe : 2, /* IOS ERROR ROUTINE USE INDICATOR           */
                       dcbifldt : 1, /* POSSIBLE LOST DATA CONDITION     @42480LP */
                                : 1;
        unsigned int   dcbdeba : 24; /* ADDRESS OF ASSOCIATED DEB                 */
        } dcbdebad;                  /* ADDRESS OF ASSOCIATED DEB                 */
      };
    struct {
      unsigned char  _filler14[8];
      struct {
        struct {
          unsigned char  dcboflg;       /* SAME AS DCBOFLGS BEFORE OPEN     @ZA11086 */
          unsigned int   dcbwrita : 24; /* ADDRESS OF WRITE MODULE          @ZA11086 */
          } dcbwrite;                   /* ADDRESS OF WRITE MODULE          @ZA11086 */
        } dcbread;       /* ADDRESS OF READ MODULE                    */
      };
    struct {
      unsigned char  _filler15[8];
      unsigned char  dcboflgs;     /* FLAGS USED BY OPEN ROUTINE               */
      unsigned char  dcbiflg;      /* FLAGS USED BY IOS IN COMMUNICATING ERROR */
      struct {
        unsigned char  dcbmacr1; /* FIRST BYTE OF DCBMACR                    */
        unsigned char  dcbmacr2; /* SECOND BYTE OF DCBMACR                   */
        } dcbmacr;               /* MACRO INSTRUCTION REFERENCE              */
      };
    struct {
      unsigned char  _filler16[12];
      struct {
        struct {
          struct {
            unsigned char  dcboptcd;      /* OPTION CODES                             */
            unsigned int   dcbchcka : 24; /* ADDRESS OF CHECK MODULE                  */
            } dcbcheck;                   /* ADDRESS OF CHECK MODULE                  */
          } dcbperr;       /* ADDRESS OF SYNCHRONIZING ROUTINE FOR PUT */
        } dcbgerr;       /* ADDRESS OF SYNCHRONIZING ROUTINE FOR GET */
      };
    struct {
      unsigned char  _filler17[16];
      struct {
        char           dcbiobl;      /* IOB LENGTH IN DOUBLE WORDS             */
        unsigned int   dcbsyna : 24; /* ADDRESS OF USER-PROVIDED SYNAD ROUTINE */
        } dcbsynad;                  /* ADDRESS OF USER-PROVIDED SYNAD ROUTINE */
      };
    struct {
      unsigned char  _filler18[20];
      struct {
        unsigned char  dcbcind1; /* CONDITION INDICATORS                   */
        } dcbflag1;              /* TCAM APPLICATION PROGRAM FLAGS         */
      };
    };
  unsigned int   dcbcnsto : 1, /* PARTITIONED DATA SET - STOW HAS BEEN      */
                 dcbcnwr0 : 1, /* DIRECT ORGANIZATION DATA SET - LAST I/O   */
                 dcbcnclo : 1, /* CLOSE IN PROCESS (QSAM)                   */
                 dcbcnioe : 1, /* PERMANENT I/O ERROR (BSAM, BPAM, QSAM)    */
                 dcbcnbfp : 1, /* OPEN ACQUIRED BUFFER POOL                 */
                 dcbcnchs : 1, /* CHAINED SCHEDULING BEING SUPPORTED        */
                 dcbcnfeo : 1, /* FEOV BIT (BSAM, BPAM, QSAM)               */
                 dcbcnqsm : 1; /* ALWAYS ZERO (BSAM, BPAM)                  */
  short int      dcbblksi;     /* MAXIMUM BLOCK SIZE                        */
  unsigned char  dcbwcpo;      /* OFFSET OF WRITE CHANNEL PROGRAM FROM THE  */
  char           dcbwcpl;      /* LENGTH OF WRITE CHANNEL PROGRAM           */
  unsigned char  dcboffsr;     /* OFFSET OF READ CCW FROM BSAM/BPAM PREFIX  */
  unsigned char  dcboffsw;     /* OFFSET OF WRITE CCW FROM BSAM/BPAM PREFIX */
  union {
    void          *dcbcicb; /* SAME AS DCBCICBA BELOW                    */
    void          *dcbioba; /* FOR NORMAL SCHEDULING, ADDRESS OF QSAM OR */
    struct {
      unsigned char  _filler19;    /* @L4A                                      */
      unsigned int   dcbiobb : 24; /* SAME AS DCBIOBA ABOVE                @L4A */
      };
    struct {
      unsigned char  _filler20;     /* INTERNAL ACCESS METHOD USE                */
      unsigned int   dcbcicba : 24; /* POINTER TO JES C.I.                       */
      unsigned char  _filler21[8];
      struct {
        struct {
          unsigned char  dcbusasi; /* FLAG BYTE FOR ASCII TAPES                 */
          } dcbqsws;               /* FLAG BYTE                                 */
        struct {
          char           dcbdircq; /* NUMBER OF BYTES USED IN LAST DIRECTORY    */
          } dcbbufof;              /* BLOCK PREFIX LENGTH (0-99), SPECIFIED BY  */
        } dcbdirct;      /* NUMBER OF BYTES USED IN LAST DIRECTORY    */
      };
    struct {
      unsigned char  _filler22[4];
      struct {
        char           dcbncp;        /* NUMBER OF CHANNEL PROGRAMS.               */
        unsigned int   dcbeobra : 24; /* ADDRESS OF END-OF-BLOCK MODULE FOR READ   */
        } dcbeobr;                    /* ADDRESS OF END-OF-BLOCK MODULE FOR READ   */
      void          *dcbeobw;      /* ADDRESS OF END-OF-BLOCK MODULE FOR WRITE. */
      short int      _filler23;    /* DCBDIRCT - NUMBER OF BYTES USED IN LAST   */
      };
    };
  short int      dcblrecl;     /* LOGICAL RECORD LENGTH                     */
  union {
    void          *dcbcntrl; /* ADDRESS OF CNTRL MODULE      */
    void          *dcbnote;  /* ADDRESS OF NOTE/POINT MODULE */
    void          *dcbpoint; /* ADDRESS OF NOTE/POINT MODULE */
    };
  };

#pragma pack(reset)
