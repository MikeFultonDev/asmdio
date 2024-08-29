DIOA  TITLE 'DIOA  - Dataset services'
         SPACE 1
DIOA     ASDSECT

DIOA     CSECT
         YREGS

**| OPENA..... SVC 19, return results
**| https://tech.mikefulton.ca/SVC19-OPEN
**| Input:
**|   R1 -> pointer to 8 byte OPT/DCB parmlist
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise
         ENTRY OPENA
OPENA    ASDPRO BASE_REG=3,USR_DSAL=OPENA_DSAL

         USING OPENA_PARMLIST,R1
         L   R7,OPENA_PARMSA

*
* Set up EODAD routine, which will just set R2 to non-zero
* R2 will be checked in the CHECKA code where it is 
* expected to be triggered
*
         LA  R5,EODAD
         USING OPENA_PARMS,R7
         L   R6,OPENA_DCB
         USING IHADCB,R6
         L   R8,DCBDCBE
         USING DCBE,R8
         ST  R5,DCBEEODA

* Call SVC19 (OPEN) with 24-bit DCB in 31-bit MODE
         OPEN MODE=31,MF=(E,(7))

OPENA_EXIT   DS    0H
         ASDEPI

EODAD    DS 0H
*
* Set R2 to 1 indicating 'end of data'
* and then return to caller (CHECKA or READA via SVC19)
*
         LA R2,1
         BR R14

         DROP
         LTORG

OPENA_PARMLIST     DSECT
OPENA_PARMSA       DS AL4
OPENA_DSAL         EQU 0

OPENA_PARMS        DSECT
OPENA_OPTS         DS AL4
OPENA_DCB          DS AL4

**| FINDA..... Find Start of PDS(E) Member
**| https://tech.mikefulton.ca/FINDMacro
**| https://tech.mikefulton.ca/SVC18-FIND
**| Input:
**|   R1 -> pointer to:
**|     Name list Address
**|     DCB Address
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise
**|   High order 2 bytes have reason code, 
**|   Low order 2 bytes have return code

DIOA     CSECT
         ENTRY FINDA
FINDA    ASDPRO BASE_REG=3,USR_DSAL=FINDA_DSAL
         LR    R7,R1
         USING FINDA_PARMS,R7

* Call SVC18 with R0 pointing to PLIST and R1 (complement) 
* containing DCB address
*
* The FIND macro is not used - it generates 'fluff' tests that
* are unnecessary and so a direct call to the SVC is used

*        FIND FINDA_DCB,FINDA_PLIST,D

         L   R0,FINDA_PLIST
         L   R1,FINDA_DCB
         LCR R1,R1
         SVC 18

         SLL R0,16
         OR  R15,R0
*
FINDA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

FINDA_PARMS   DSECT
FINDA_PLIST   DS AL4
FINDA_DCB     DS AL4
FINDA_DSAL    EQU 0

**| READA..... Read BLOCK, return results
**| https://tech.mikefulton.ca/READMacro
**| Input:
**|   R1 -> pointer to DECB
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise

DIOA     CSECT
         ENTRY READA
READA    ASDPRO BASE_REG=3,USR_DSAL=READA_DSAL

* Call Read function

         USING READA_PARMS,R1
         L    R1,READA_DECB
         READ (1),SF,MF=E

*
* No return code for READ. Use CHECK
*
         LA  R15,0
*
READA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

READA_PARMS   DSECT
READA_DECB    DS AL4
READA_DSAL    EQU 0         

**| WRITEA..... Write BLOCK, return results
**| https://tech.mikefulton.ca/WRITEMacro
**| Input:
**|   R1 -> pointer to DECB
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise

DIOA     CSECT
         ENTRY WRITEA
WRITEA   ASDPRO BASE_REG=3,USR_DSAL=WRITEA_DSAL

* Call Write Function

         USING WRITEA_PARMS,R1
         L    R1,WRITEA_DECB
         WRITE (1),SF,MF=E

*
* No return code for WRITE. Use CHECK
*
         LA  R15,0
*
WRITEA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

WRITEA_PARMS   DSECT
WRITEA_DECB    DS AL4
WRITEA_DSAL    EQU 0         

**| CHECKA..... CHECK DECB, return result
**| https://tech.mikefulton.ca/CHECKMacro
**| Input:
**|   R1 -> pointer to DECB
**| Output:
**|   R15 -> RC 0 if successful, non-zero if end-of-data

DIOA     CSECT
         ENTRY CHECKA
CHECKA   ASDPRO BASE_REG=3,USR_DSAL=CHECKA_DSAL
         LR    R7,R1
         USING CHECKA_PARMS,R7

*
* Set R2 to 0 indicating 'not end of data'
*
         SR  R2,R2

* Call CHECK function 
         L   R1,CHECKA_DECB
         USING DECB,R1
         L   R15,DECDCBAD
         USING IHADCB,R15
         ICM R15,B'0111',DCBCHCKA
         BALR R14,R15

*
* Copy 'end of data' indicator into R15
*
         LR R15,R2
*
CHECKA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

CHECKA_PARMS   DSECT
CHECKA_DECB    DS AL4
CHECKA_DSAL    EQU 0         

**| NOTEA..... NOTE DCB, return results
**| https://tech.mikefulton.ca/NOTEMacro
**| Input:
**|   R1 -> pointer to DCB
**| Output:
**|   R15 -> TTRz returned if successful.

DIOA     CSECT
         ENTRY NOTEA
NOTEA    ASDPRO BASE_REG=3,USR_DSAL=NOTEA_DSAL
         LR    R7,R1
         USING NOTEA_PARMS,R7

* Call NOTE function (found in DCB)
         L   R1,NOTEA_DCB
         XR  R15,R15
         USING IHADCB,R1
         ICM R15,B'0111',DCBNOTE+1
         BASR R14,R15
         LR  R15,R1
*
NOTEA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

NOTEA_PARMS   DSECT
NOTEA_DCB     DS AL4
NOTEA_DSAL    EQU 0

**| DESERVA..... DESERV, return results
**| https://tech.mikefulton.ca/DESERVMacro
**| Input:
**|   R1 -> pointer to DESP
**| Output:
**|   R15 -> Return Code

DIOA     CSECT
         ENTRY DESERVA
DESERVA  ASDPRO BASE_REG=3,USR_DSAL=DESERVA_DSAL
         LR    R7,R1
         USING DESERVA_PARMS,R7

* Do Program Call to DESERV Function
         L   R1,DESERVA_DESP
         L   R15,16(,0)          CVT
         L   R15,1216(,R15)      DFA
         L   R15,44(,R15)
         L   R15,84(,R15)        DESERV PC
         PC  0(R15)              Do Program Call
*
* Return code in R15
*
DESERVA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

DESERVA_PARMS   DSECT
DESERVA_DESP    DS AL4
DESERVA_DSAL    EQU 0

**| CLOSEA..... CLOSE macro, return results
**| https://tech.mikefulton.ca/SVC20-CLOSE 
**| Input:
**|   R1 -> pointer to 8 byte OPT/DCB array
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise

DIOA     CSECT
         ENTRY CLOSEA
CLOSEA   ASDPRO BASE_REG=3,USR_DSAL=CLOSEA_DSAL
         USING CLOSEA_PARMS,R1
         L  R0,CLOSEA_OPTSANDDCB
         SR R1,R1
         SVC 20

CLOSEA_EXIT    DS    0H
         ASDEPI

         DROP
         LTORG

CLOSEA_PARMS         DSECT
CLOSEA_OPTSANDDCB    DS  AL4
CLOSEA_DSAL          EQU 0

**| MALOC24A.... acquire storage below the line
**| https://tech.mikefulton.ca/STORAGE-OBTAINMacro
**| R1 -> Pointer to fullword allocation length
**| R15 -> Pointer to allocated storage or 0 if failure

DIOA     CSECT
         ENTRY MALOC24A
MALOC24A ASDPRO BASE_REG=3,USR_DSAL=MALOC24A_DSAL
         LR    R7,R1
         USING MALOC24A_PARMS,R7
         L     R8,MALOC24A_LEN
         L     R8,0(,R8)

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
MALOC24A_LEN     DS  AL4
MALOC24A_DSAL    EQU 0

**| FREE24A.... free storage below the line
**| https://tech.mikefulton.ca/STORAGE-RELEASEMacro
**| R1 -> Fullword pointer and Fullword allocation length
**| R15 -> 0 if successful, non-zero otherwise

DIOA     CSECT
         ENTRY FREE24A
FREE24A  ASDPRO BASE_REG=3,USR_DSAL=FREE24A_DSAL
         LR    R7,R1
         USING FREE24A_PARMS,R7
         L     R8,FREE24A_PTR
         L     R9,FREE24A_LEN
         L     R9,0(,R9)

* Get 24-bit storage

         STORAGE RELEASE,ADDR=(8),LENGTH=(9)
FREE24A_EXIT    DS    0H
         ASDEPI

         DROP
         LTORG

FREE24A_PARMS DSECT
FREE24A_PTR   DS  AL4
FREE24A_LEN   DS  AL4
FREE24A_DSAL  EQU 0

**| S99A..... SVC99
**| https://tech.mikefulton.ca/SVC99
**| Input:
**|   R1 -> pointer to S99RBP
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise

DIOA     CSECT
         ENTRY S99A
S99A     ASDPRO BASE_REG=3,USR_DSAL=S99A_DSAL

* Ensure the High Order Bit is ON for 0(R1)
         USING S99A_PARMS,R1
         L   R2,S99ARBP
         OILH R2,X'8000'
         ST  R2,0(,R1)
* Call SVC99 (DYNALLOC) with S99RBP
         SVC 99
*
S99A_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

S99A_PARMS   DSECT
S99ARBP      DS AL4
S99A_DSAL    EQU 0         

**| STOWA..... SVC 21 massaging input and output
**| https://tech.mikefulton.ca/SVC21
**| Input:
**|   R1 -> pointer to list address and dcb address pointers
**| Output:
**|   R15 -> high order 2 bytes are reason code. 
**|          low order 2 bytes are return code. 

DIOA     CSECT
         ENTRY STOWA
STOWA    ASDPRO BASE_REG=3,USR_DSAL=STOWA_DSAL

* For the STOW (SVC 21) call:
*  R0 is the list address and 
*  R1 is the dcb address
*  R15 is also the list address

         USING STOWA_PARMS,R1
         L   R3,STOWA_LST
         L   R4,STOWA_DCB
         L   R0,0(,R3)
         L   R1,0(,R4)
         LR  R15,R0
         SVC 21
*
* For the return, put low halfword of R0 
* into high halfword of R15 and return R15
*
         SLL  R0,16
         AR   R15,R0
*
STOWA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

STOWA_PARMS   DSECT
STOWA_LST     DS AL4
STOWA_DCB     DS AL4
STOWA_DSAL    EQU 0         

**| S99MSGA..... SVC99MSG
**| https://tech.mikefulton.ca/IEFDB476
**| Input:
**|   R1 -> pointer to em_parms
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise

DIOA     CSECT
         ENTRY S99MSGA
S99MSGA  ASDPRO BASE_REG=3,USR_DSAL=S99MSGA_DSAL
         USING S99MSGA_PARMS,R13

* Call SVC99MSG 
         LINK EP=IEFDB476
*
S99MSGA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

S99MSGA_PARMS   DSECT
S99MSGAP DS     AL4
S99MSGA_DSAL    EQU 0

**|
**| Addressability DSECTs
**|

    DCBD DSORG=PO,DEVD=DA
    IHADCBE
    IHADECB

**| Finish off the CSECT

DIOA     CSECT
         DC    C'Open Source'
         END
