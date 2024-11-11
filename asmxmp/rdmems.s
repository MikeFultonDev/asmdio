         START , 
         YREGS ,                  register equates, syslib SYS1.MACLIB 
         IGWDES                   DSECTs for DESERV
         IGWSMDE
RDMEMS   CSECT , 
*
* DSECTs for USING
*

RDMEMS   AMODE 31 
RDMEMS   RMODE ANY
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

EOM       DS  0H
          LA  R2,1
          BR  R14
*
OPEN_SUCCESS DS 0H

MEM_DESERV  DS 0H
         MVC DESP_AREA(DESERV_LEN),CONST_DESP
         
         DESERV FUNC=GET_ALL,CONN_INTENT=NONE,DCB=LIB_DCB,             +
               EXT_ATTR=YES,                                           +
               AREAPTR=DESERV_AREAPTR,                                 +
               MF=(E,DESP_AREA)
         CIJE R15,0,DESERV_SUCCESS   
         CIJE R15,8,DESERV_NOMEMS   
         CIJE R15,12,DESERV_NOPDSE   
*
DESERV_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
DESERV_SUCCESS DS 0H
         LR  R9,R15                put 0 into R9
* See: 
* https://www.ibm.com/docs/en/zos/3.1.0?topic=bpam-deserv-parameters
* https://www.ibm.com/docs/en/zos/3.1.0?topic=smde-funcget-all
*
         B   LIB_CLOSE

DESERV_NOMEMS DS 0H
         LR  R9,R15                put 8 into R9 (no members)
         B   LIB_CLOSE

DESERV_NOPDSE DS 0H
         LR  R9,R15                put 12 into R9 (not PDSE)
         B   LIB_CLOSE

LIB_CLOSE  DS 0H
         MVC CLOSE_PARMS(CLOSELEN),CONST_CLOSE
         CLOSE (LIB_DCB),MF=(E,CLOSE_PARMS),MODE=31
         CIJE R15,0,CLOSE_SUCCESS
*
CLOSE_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
CLOSE_SUCCESS DS 0H
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
CONST_DCBE DCBE  RMODE31=BUFF,EODAD=EOM
CONST_OPEN OPEN (*-*,(INPUT)),MODE=31,MF=L
OPENLEN   EQU   *-CONST_OPEN
CONST_CLOSE CLOSE (*-*),MODE=31,MF=L
CLOSELEN  EQU   *-CONST_CLOSE
*
READ    READ  DECBMODW,SF,0,0,'S',MF=L
READLEN EQU   *-DECBMODW
*
CONST_DESP        DESERV MF=L
DESERV_LEN        EQU *-CONST_DESP
*

         LTORG ,

*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA       DSECT 
SAVEA       DS    18F 
OPEN_PARMS  DS CL(OPENLEN)
CLOSE_PARMS DS CL(CLOSELEN)
DESP_AREA   DS CL(DESERV_LEN)
DESERV_AREAPTR DS 1A

WALEN       EQU  *-SAVEA

DCBAREA     DSECT
LIB_DCB     DS   CL(DCBLEN)

*------------------------------------------------------------------- 
* Equates                                                            - 
*------------------------------------------------------------------- 
OPEN_FAIL_MASK EQU   16
DESERV_FAIL_MASK EQU 32
CLOSE_FAIL_MASK EQU  64
CHECK_FAIL1_MASK EQU 77
CHECK_FAIL2_MASK EQU 99


         END   RDMEMS   
