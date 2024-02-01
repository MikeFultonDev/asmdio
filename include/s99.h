#pragma pack(packed)

struct s99rbp {
  unsigned int   s99rbpnd  : 1, /* LAST POINTER INDICATOR */
                          : 31;
  };

struct s99rb {
  unsigned char  s99rbln;  /* LENGTH OF REQUEST BLOCK         */
  unsigned char  s99verb;  /* VERB CODE                       */
  union {
    unsigned char  s99flag1[2]; /* FLAGS */
    struct {
      unsigned int   s99oncnv : 1, /* ALLOC FUNCTION-DO NOT USE AN     */
                     s99nocnv : 1, /* ALLOC FUNCTION-DO NOT USE AN     */
                     s99nomnt : 1, /* ALLOC FUNCTION-DO NOT MOUNT      */
                     s99jbsys : 1, /* ALLOC FUNC-JOB RELATED SYSOUT    */
                     s99cnenq : 1, /* ALL FUNCTIONS-ISSUE A   @ZA32641 */
                     s99gdgnt : 1, /* ALLOC FUNCTION - IGNORE @YA10531 */
                     s99msgl0 : 1, /* All functions - ignore the  @01A */
                     s99nomig : 1; /* ALLOC function - do not     @03A */
      unsigned int   s99nosym : 1, /* Allocate, unallocate, info       */
                     s99acucb : 1, /* Alloc function-use Actual        */
                     s99dsaba : 1, /* Request that the DSAB for        */
                     s99dxacu : 1, /* Request above-the-line DSABs,    */
                              : 4;
      struct {
        unsigned char  s99error[2]; /* ERROR REASON CODE                */
        unsigned char  s99info[2];  /* INFORMATION REASON CODE          */
        } s99rsc;                   /* REASON CODE FIELDS               */
      };
    };
  int            s99txtpp; /* ADDR OF LIST OF TEXT UNIT PTRS  */
  int            s99s99x;  /* ADDR OF REQ BLK EXTENSION  @L1C */
  union {
    unsigned char  s99flag2[4]; /* FLAGS FOR AUTHORIZED FUNCTIONS */
    struct {
      unsigned int   s99wtvol : 1, /* ALLOC FUNCTION-WAIT FOR          */
                     s99wtdsn : 1, /* ALLOC FUNCTION-WAIT FOR DSNAME   */
                     s99nores : 1, /* ALLOC FUNCTION-DO NOT DO         */
                     s99wtunt : 1, /* ALLOC FUNCTION-WAIT FOR UNITS    */
                     s99offln : 1, /* ALLOC FUNCTION-CONSIDER OFFLINE  */
                     s99tionq : 1, /* ALL FUNCTIONS-TIOT ENQ ALREADY   */
                     s99catlg : 1, /* ALLOC FUNCTION-SET SPECIAL       */
                     s99mount : 1; /* ALLOC FUNCTION-MAY MOUNT VOLUME  */
      unsigned int   s99udevt : 1, /* ALLOCATION FUNCTION-UNIT NAME    */
                     s99pcint : 1, /* ALLOC FUNCTION-ALLOC    @Y30QPPB */
                     s99dyndi : 1, /* ALLOC FUNCTION-NO JES3  @ZA63125 */
                     s99tioex : 1, /* ALLOC FUNCTION - XTIOT           */
                     s99aserr : 1, /* Unit Allocation / Unallocation   */
                     s99igncl : 1, /* Alloc function - ignore          */
                     s99dasup : 1, /* Alloc function - suppress        */
                              : 1;
      unsigned char  s99flg23;     /* THIRD BYTE OF FLAGS              */
      unsigned char  s99flg24;     /* FOURTH BYTE OF FLAGS             */
      };
    };
  };

struct s99tupl {
  unsigned int   s99tupln  : 1, /* LAST TEXT UNIT POINTER IN LIST */
                          : 31;
  };

struct s99tunit {
  unsigned char  s99tukey[2]; /* KEY                             */
  unsigned char  s99tunum[2]; /* N0. OF LENGTH+PARAMETER ENTRIES */
  union {
    unsigned char  s99tuent;    /* ENTRY OF LENGTH+PARAMETER        */
    unsigned char  s99tulng[2]; /* LENGH OF 1ST (OR ONLY) PARAMETER */
    };
  unsigned char  s99tupar;    /* 1ST (OR ONLY) PARAMETER         */
  };

struct s99tufld {
  unsigned char  s99tulen[2]; /* LENGTH OF PARAMETER */
  unsigned char  s99tuprm;    /* PARAMETER           */
  };

struct s99rbx {
  unsigned char  s99eid[6];    /* CONTROL BLOCK ID ='S99RBX'  @L1A */
  unsigned int            : 7,
                 s99rbxvr : 1; /* CURRENT VERSION NUMBER      @L2A */
  unsigned int   s99eimsg : 1, /* ISSUE MSG BEFORE RETURNING  @L1A */
                 s99ermsg : 1, /* RETURN MSG TO CALLER        @L1A */
                 s99elsto : 1, /* USER STORAGE SHOULD BE      @L1A */
                 s99emkey : 1, /* USER SPECIFIED STORAGE KEY  @L1A */
                 s99emsub : 1, /* USER SPECIFIED SUBPOOL FOR  @L1A */
                 s99ewtp  : 1, /* USE WTO FOR MESSAGE OUTPUT  @L2A */
                          : 2;
  unsigned char  s99esubp;     /* SUBPOOL FOR MESSAGE BLOCKS  @L1A */
  unsigned char  s99ekey;      /* STORAGE KEY FOR MESSAGE     @L1A */
  unsigned char  s99emgsv;     /* SEVERITY LEVEL FOR MESSAGES @L1A */
  unsigned char  s99enmsg;     /* NUMBER OF MESSAGE BLOCKS         */
  int            s99ecppl;     /* ADDRESS OF CPPL             @L1A */
  union {
    unsigned char  s99emrc[4]; /* MESSAGE SERVICE RETURN CODE @L1A */
    struct {
      unsigned char  s99ercr; /* RESERVED                    @L1A */
      unsigned char  s99ercm; /* RESERVED                    @D1C */
      unsigned char  s99erco; /* RETURN CODE DEALING WITH    @L1A */
      unsigned char  s99ercf; /* RETURN CODE DEALING WITH    @L1A */
      };
    };
  int            s99ewrc;      /* PUTLINE/WTO RETURN CODE     @L1A */
  int            s99emsgp;     /* MESSAGE BLOCK POINTER       @L1A */
  union {
    int            s99esirc; /* INFORMATION RETRIEVAL       @L1A */
    struct {
      unsigned char  s99eerr[2];  /* ERROR REASON CODE                */
      unsigned char  s99einfo[2]; /* INFORMATION REASON CODE          */
      };
    };
  unsigned char  s99ersn[4];   /* SMS REASON CODE             @02C */
  };

#pragma pack(reset)
