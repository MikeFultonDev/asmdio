DIOA  TITLE 'DIOA  - Dataset services'
         SPACE 1
DIOA     ASDSECT

DIOA     CSECT
         YREGS

**| OPENA..... SVC 19, return results
**| https://tech.mikefulton.ca/SVC19-OPEN
**| Input:
**|   R1 -> pointer to 8 byte OPT/DCB array
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise
         ENTRY OPENA
OPENA    ASDPRO BASE_REG=3,USR_DSAL=OPENA_DSAL

* Call SVC19 (OPEN) with 24-bit DCB
         USING OPENA_PARMS,R1
         L   R0,OPENA_OPTSANDDCB
         SR  R1,R1
         SR  R15,R15
         SVC 19
*
OPENA_EXIT   DS    0H
         ASDEPI

         DROP
         LTORG

OPENA_PARMS        DSECT
OPENA_OPTSANDDCB   DS AL4
OPENA_DSAL         EQU 0         

**| WRITEA..... Write BLOCK, return results
**| https://tech.mikefulton.ca/WRITEMacro
**| Input:
**|   R1 -> pointer to DECB
**| Output:
**|   R15 -> RC 0 if successful, non-zero otherwise

DIOA     CSECT
         ENTRY WRITEA
WRITEA   ASDPRO BASE_REG=3,USR_DSAL=WRITEA_DSAL
         LR    R7,R1
         USING WRITEA_PARMS,R7

* Call Write function (found in DCB, which is 8(DECB))
         L   R1,WRITEA_DECB
         L   R15,8(,R1)
         ICM R15,B'0111',49(R15)
         BALR R14,R15
*
* Does not seem to be a return code for WRITE?
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
**|   R15 -> RC 0 if successful, non-zero otherwise

DIOA     CSECT
         ENTRY CHECKA
CHECKA   ASDPRO BASE_REG=3,USR_DSAL=CHECKA_DSAL
         LR    R7,R1
         USING CHECKA_PARMS,R7

* Call CHECK function (found in DCB, which is 8(DECB))
         L   R1,CHECKA_DECB
         L   R15,8(,R1)
         ICM R15,B'0111',53(R15)
         BALR R14,R15
*
* Does not seem to be a return code for CHECK?
*
         LA  R15,0
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
         ICM R15,B'0111',85(R1)
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

**| Finish off the CSECT

DIOA     CSECT
         DC    C'Open Source'
         END
