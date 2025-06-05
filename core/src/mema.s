MEMA  TITLE 'MEMA  - Memory management services'
         SPACE 1
MEMA     ASDSECT

MEMA     CSECT
         YREGS

**| MALOC24A.... acquire storage below the line
**| https://tech.mikefulton.ca/STORAGE-OBTAINMacro
**| R1 -> Pointer to fullword allocation length
**| R15 -> Pointer to allocated storage or 0 if failure

MEMA     CSECT
         ENTRY MALOC24A
MALOC24A ASDPRO BASE_REG=3,USR_DSAL=MALOC24A_DSAL
         LR    R7,R1
         USING MALOC24A_PARMS,R7
         L     R8,MALOC24A_LEN
* Get 24-bit storage

         STORAGE OBTAIN,LENGTH=(8),LOC=24,COND=YES
         CHI R15,0
         BZ  MALOC24A_SUCCESS
MALOC24A_FAILURE DS    0H
         LA  R15,0
         B   MALOC24A_EXIT
MALOC24A_SUCCESS DS    0H
         LR    R15,R1
MALOC24A_EXIT    DS    0H
         ASDEPI

         DROP
         LTORG

MALOC24A_PARMS   DSECT
MALOC24A_LEN     DS  F
MALOC24A_DSAL    EQU 0

**| FREE24A.... free storage below the line
**| https://tech.mikefulton.ca/STORAGE-RELEASEMacro
**| R1 -> Fullword pointer and Fullword allocation length
**| R15 -> 0 if successful, non-zero otherwise

MEMA     CSECT
         ENTRY FREE24A
FREE24A  ASDPRO BASE_REG=3,USR_DSAL=FREE24A_DSAL
         LR    R7,R1
         USING FREE24A_PARMS,R7
         L     R8,FREE24A_PTR
         L     R9,FREE24A_LEN

* Get 24-bit storage

         STORAGE RELEASE,ADDR=(8),LENGTH=(9)
FREE24A_EXIT    DS    0H
         ASDEPI

         DROP
         LTORG

FREE24A_PARMS DSECT
FREE24A_PTR   DS  AL4
FREE24A_LEN   DS  F
FREE24A_DSAL  EQU 0

**| Finish off the CSECT

MEMA     CSECT
         DC    C'Open Source'
         END
