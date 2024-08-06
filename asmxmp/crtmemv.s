         START , 
CRTMEMV  CSECT , 
         YREGS ,                   register equates, syslib SYS1.MACLIB 
CRTMEMV  AMODE 31 
CRTMEMV  RMODE ANY
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
         LA    R13,SAVEA            ADDRESS OF OUR SA IN R13 

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
        OPEN (LIB_DCB,OUTPUT),MF=(E,OPEN_PARMS),MODE=31
         CIJE R15,0,OPEN_SUCCESS
*
OPEN_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
OPEN_SUCCESS DS 0H

MEM_WRITE  DS  0H
         MVC   WRITE_DECB(WRITELEN),DECBMODW
         LA   R2,WRITE_BUFFER
         LA   R3,BUFFLEN
         LA   R4,CONST_BUFFER
         LR   R5,R3
         MVCL R2,R4              Copy CONST_BUFFER to WRITE_BUFFER
        WRITE WRITE_DECB,SF,LIB_DCB,WRITE_BUFFER,'S',MF=E

WRITE_SUCCESS DS 0H

MEM_CHECK  DS 0H
         CHECK WRITE_DECB             WAIT UNTIL COMPLETE

MEM_NOTE   DS 0H
         NOTE LIB_DCB                 GET TTR TO 1st (ONLY) BLOCK
         STCM  R1,14,MEM_TTR          SAVE TTR FOR USE BY STOW
*
         MVC   MEM(MEMLEN),CONST_MEM
         STOW  LIB_DCB,MEM,A       ADD TO DIRECTORY

         CIJE R15,0,STOW_SUCCESS   0 is expected
*
STOW_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
STOW_SUCCESS DS 0H
         SR  R9,R9                 clear R9 to 0
*
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
         LA  R9,0                  CRTMEMVsuccessful put 0 in R9
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
CONST_DCB  DCB   DSORG=PO,MACRF=(W),DDNAME=MYDD,DCBE=CONST_DCBE
DCBLEN    EQU   *-CONST_DCB
CONST_DCBE DCBE  RMODE31=BUFF
CONST_OPEN OPEN (*-*,(OUTPUT)),MODE=31,MF=L
OPENLEN   EQU   *-CONST_OPEN
CONST_CLOSE CLOSE (*-*),MODE=31,MF=L
CLOSELEN  EQU   *-CONST_CLOSE
*
WRITE    WRITE  DECBMODW,SF,0,0,'S',MF=L
WRITELEN EQU   *-DECBMODW

CONST_BUFFER DC CL80'Hello world'
             DC CL80'Velo world'
             DC CL80'Cello world'
             DC CL80'Yellow world'
BUFFLEN EQU *-CONST_BUFFER

CONST_MEM          DS 0F
CONST_MEM_NAME     DC CL8'NEWMEM'
CONST_MEM_TTR      DC XL3'0'
MEMLEN EQU *-CONST_MEM

         LTORG ,

*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA       DSECT 
SAVEA       DS    18F 
OPEN_PARMS  DS CL(OPENLEN)
CLOSE_PARMS DS CL(CLOSELEN)
WRITE_DECB  DS CL(WRITELEN)
WRITE_BUFFER DS CL(BUFFLEN)

MEM          DS 0F
MEM_LEN      DS 1H
MEM_FLAGS    DS XL1
MEM_DCB24    DS XL3
MEM_DIRA     DS 1A

MEM_DIR      DS 0F
MEM_NAME     DS CL8
MEM_TTR      DS XL3
WALEN       EQU  *-SAVEA

DCBAREA     DSECT
LIB_DCB     DS   CL(DCBLEN)
         END   CRTMEMV  
