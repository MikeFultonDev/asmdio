*
* IBM Confidential
* Licensed Materials - Property of IBM
*
* IBM Rational Developer Traveler
*
* (C) Copyright IBM Corporation 2013, 2014.
*
* The source code for this program is not published or otherwise
* divested of its trade secrets, irrespective of what has been
* deposited with the U.S. Copyright Office
*
DIO   TITLE 'DIO   - Dataset services'
         SPACE 1
DIO      RDTSECT

**| OPENA..... OPEN macro, return results
**| Input:
**|   R1 -> pointer to 4 byte OPT/DCB array
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise

DIO      CSECT
         ENTRY OPENA
OPENA    RDTPRO BASE_REG=12,USR_DSAL=OPENA_DSAL
         LR    R7,R1
         USING OPENA_PARMS,R7

* Call SVC19 (OPEN) with 24-bit DCB

*        SVC 19
*
OPENA_EXIT   DS    0H
         RDTEPI

         DROP
         LTORG

OPENA_PARMS   DSECT
OPENA_OPTSANDDCB   DS AL4
OPENA_DSAL EQU 0         

**| CLOSEA..... CLOSE macro, return results

DIO      CSECT
         ENTRY CLOSEA
CLOSEA   RDTPRO BASE_REG=12,USR_DSAL=CLOSEA_DSAL
         LR    R7,R1

CLOSEA_EXIT    DS    0H
         RDTEPI

         DROP
         LTORG

CLOSEA_PARMS   DSECT
CLOSEA_DCB   DS  AL4
CLOSEA_DSAL  EQU 0

**| MALLOC24.... acquire storage below the line
**| R1 -> Fullword allocation length
**| R15 -> Pointer to allocated storage or 0 if failure

DIO      CSECT
         ENTRY MALLOC24
MALLOC24 RDTPRO BASE_REG=12,USR_DSAL=MALLOC24_DSAL
         LR    R7,R1
         USING MALLOC24_PARMS,R7
         L     R8,MALLOC24_LEN

* Get 24-bit storage

         STORAGE OBTAIN,LENGTH=(8),LOC=24
         CHI R15,0
         BZ  MALLOC24_SUCCESS
MALLOC24_FAILURE DS    0H
         LA  R15,0
         B   MALLOC24_EXIT
MALLOC24_SUCCESS DS    0H
         LR    R15,R1
MALLOC24_EXIT    DS    0H
         RDTEPI

         DROP
         LTORG

MALLOC24_PARMS   DSECT
MALLOC24_LEN   DS  AL4
MALLOC24_DSAL  EQU 0

**| FREE24.... free storage below the line
**| R1 -> Fullword pointer and Fullword allocation length
**| R15 -> 0 if successful, non-zero otherwise

DIO      CSECT
         ENTRY FREE24
FREE24   RDTPRO BASE_REG=12,USR_DSAL=FREE24_DSAL
         LR    R7,R1
         USING FREE24_PARMS,R7
         LA    R8,FREE24_PTR
         L     R9,FREE24_LEN

* Get 24-bit storage

         STORAGE RELEASE,ADDR=(8),LENGTH=(9)
FREE24_EXIT    DS    0H
         RDTEPI

         DROP
         LTORG

FREE24_PARMS   DSECT
FREE24_PTR   DS  AL4
FREE24_LEN   DS  AL4
FREE24_DSAL  EQU 0

**| Common Equates Follow
**| Common Equates Follow

R0         EQU   0
R1         EQU   1
R2         EQU   2
R3         EQU   3
R4         EQU   4
R5         EQU   5
R6         EQU   6
R7         EQU   7
R8         EQU   8
R9         EQU   9
R10        EQU   10
R11        EQU   11
R12        EQU   12
R13        EQU   13
R14        EQU   14
R15        EQU   15

**| Finish off the CSECT

DIO      CSECT
         DC    C'Open Source'
         END
