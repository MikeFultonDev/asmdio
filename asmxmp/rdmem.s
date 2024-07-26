         START , 
         YREGS ,                  register equates, syslib SYS1.MACLIB 
*
* DSECTs for USING
*
         IGWDES                   DSECTs for DESERV
         IGWSMDE

RDMEM    CSECT , 
RDMEM    AMODE 31 
RDMEM    RMODE ANY
        SYSSTATE AMODE64=NO,ARCHLVL=OSREL,OSREL=SYSSTATE 
        IEABRCX  DEFINE    convert based branches to relative
*------------------------------------------------------------------- 
* Linkage and storage obtain
*------------------------------------------------------------------- 
         BAKR  R14,0                use linkage stack 
         LARL  R12,DATCONST         setup base for CONSTANTS
         USING DATCONST,R12         "baseless" CSECT 
        STORAGE OBTAIN,LENGTH=WALEN,EXECUTABLE=NO,LOC=24,CHECKZERO=YES
         LR    R10,R1               R10 points to Working Storage 
         USING WAREA,R10            BASE FOR DSECT 
*
* Clear storage
*
         CHI   R15,X'0014'           X'14': storage zeroed
         BE    STG_WA_CLEAR
         LR    R2,R1                 system did not clear, do ourselves
         LA    R3,WALEN
         XR    R5,R5
         MVCL  R2,R4                 clear storage (pad byte zero)

STG_WA_CLEAR DS 0H
*
         MVC   SAVEA+4(4),=C'F1SA'  linkage stack convention 
         LAE   R13,SAVEA            ADDRESS OF OUR SA IN R13 

*------------------------------------------------------------------- 
* application logic                                                - 
*------------------------------------------------------------------- 

*
* DCB has to be below the line
*
        STORAGE OBTAIN,LENGTH=DCBLEN,EXECUTABLE=NO,LOC=24,CHECKZERO=YES
         LR R8,R1                   R8 points to Output DCB
         USING DCBAREA,R8
*
* Clear storage
*
         CHI   R15,X'0014'           X'14': storage zeroed
         BE    STG_DCB_CLEAR
         LR    R2,R1                 system did not clear, do ourselves
         LA    R3,DCBLEN
         XR    R5,R5
         MVCL  R2,R4                 clear storage (pad byte zero)

STG_DCB_CLEAR DS 0H
*
*
* Copy the DCB template into 24-bit storage
* The OPEN_PARMS and DCBE is 31-bit to minimize below-line stg
*
LIB_OPEN  DS  0H
         MVC LIB_DCB(DCBLEN),CONST_DCB
         MVC OPEN_PARMS(OPENLEN),CONST_OPEN
        OPEN (LIB_DCB,INPUT),MF=(E,OPEN_PARMS),MODE=31
         CIJE R15,0,OPEN_SUCCESS
*
OPEN_FAIL DS  0H
         LHI R8,OPEN_FAIL_MASK
         LR  R9,R15                put err code in R9
         OR  R9,R8
         B   DONE
*
OPEN_SUCCESS DS 0H

MEM_DESERV  DS 0H
         LA  R6,DESERV_NAME_LEN
         STH R6,MEM_NAME_LEN
         MVC MEM_NAME_VAL(DESERV_NAME_LEN),DESERV_NAME
         LA  R6,MEM_NAME_LIST
         ST  R6,MEM_NAME_PTR
         
         DESERV FUNC=GET,CONN_INTENT=HOLD,DCB=LIB_DCB,                 +
               EXT_ATTR=YES,                                           +
               NAME_LIST=(MEM_NAME_PTR,DESERV_NAME_COUNT),             +
               AREA=(DESERV_AREA,DESERV_AREA_LEN),                     +
               MF=(E,DESP_AREA)
         CIJE R15,0,DESERV_SUCCESS   
         ST 0,0
*
DESERV_FAIL DS  0H
         LHI R8,DESERV_FAIL_MASK
         LR  R9,R15                put err code in R9 (need R0 too)
         OR  R9,R8
         B   DONE
*
DESERV_SUCCESS DS 0H
*
* Need to move to location for member
*
MEM_READ  DS  0H
         MVC   READ_DECB(READLEN),DECBMODW
        READ READ_DECB,SF,LIB_DCB,READ_BUFFER,'S',MF=E

MEM_CHECK  DS 0H
         CHECK READ_DECB           WAIT UNTIL COMPLETE
*
READ_SUCCESS DS 0H

*
LIB_CLOSE  DS 0H
         MVC CLOSE_PARMS(CLOSELEN),CONST_CLOSE
        CLOSE (LIB_DCB),MF=(E,CLOSE_PARMS),MODE=31
         CIJE R15,0,CLOSE_SUCCESS
*
CLOSE_FAIL DS  0H
         LHI R8,CLOSE_FAIL_MASK
         LR  R9,R15                put err code in R9
         OR  R9,R8
         B   DONE
*
CLOSE_SUCCESS DS 0H
         LA  R9,0                  RDMEM  successful put 0 in R9
*
* Free DCB storage
*
RLSE_DCB   DS 0H
        STORAGE RELEASE,ADDR=(R8),LENGTH=DCBLEN,EXECUTABLE=NO 

*------------------------------------------------------------------- 
* Linkage and storage release. set RC (reg 15)                     -
*------------------------------------------------------------------- 
DONE     DS 0H
RLSE_WA  DS 0H
        STORAGE RELEASE,ADDR=(R10),LENGTH=WALEN,EXECUTABLE=NO 
         LR    R15,R9               get saved rc into R15
         PR    ,                    return to caller 

*------------------------------------------------------------------- 
* constants and literal pool                                       - 
*------------------------------------------------------------------- 
DATCONST   DS    0D                 Doubleword alignment for LARL
CONST_DCB  DCB   DSORG=PO,MACRF=(R),DDNAME=MYDD,DCBE=CONST_DCBE
DCBLEN    EQU   *-CONST_DCB
CONST_DCBE DCBE  RMODE31=BUFF
CONST_OPEN OPEN (*-*,(INPUT)),MODE=31,MF=L
OPENLEN   EQU   *-CONST_OPEN
CONST_CLOSE CLOSE (*-*),MODE=31,MF=L
CLOSELEN  EQU   *-CONST_CLOSE
*
READ    READ  DECBMODW,SF,0,0,'S',MF=L
READLEN EQU   *-DECBMODW
*
DESERV_CONN_ID DS 0D
            DC A(0)
*
DESERV_NAME_COUNT EQU 1
DESERV_NAME       DC CL6'NEWMEM'
DESERV_NAME_LEN   EQU *-DESERV_NAME

BUFFLEN   EQU  80

*
* From BPAM DESERV Parameters:
* L'DESB_FIXED + (input_list_entry_count * (SMDE_MAXLEN + gap_size))
*
DESERV_AREA_LEN EQU L'DESB_FIXED+SMDE_MAXLEN

         LTORG ,

*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA       DSECT 
SAVEA       DS    18F 
OPEN_PARMS  DS CL(OPENLEN)
CLOSE_PARMS DS CL(CLOSELEN)
READ_DECB   DS CL(READLEN)
READ_BUFFER DS CL(BUFFLEN)

DESERV_AREA DS CL(DESERV_AREA_LEN)
DESP_AREA   DS CL(DESP_LEN_IV)

MEM_CCSID    DS 1H

MEM_NAME_PTR  DS A
MEM_NAME_LIST DS 0D
MEM_NAME_LEN  DS 1H
MEM_NAME_VAL  DS CL8

WALEN       EQU  *-SAVEA

DCBAREA     DSECT
LIB_DCB     DS   CL(DCBLEN)

*------------------------------------------------------------------- 
* Equates                                                            - 
*------------------------------------------------------------------- 
OPEN_FAIL_MASK EQU   16
DESERV_FAIL_MASK EQU 32
CLOSE_FAIL_MASK EQU  64


         END   RDMEM    
