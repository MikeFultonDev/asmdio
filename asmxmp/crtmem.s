         START , 
         YREGS ,                   register equates, syslib SYS1.MACLIB 
CRTMEM   CSECT , 
CRTMEM   AMODE 31 
CRTMEM   RMODE ANY
        SYSSTATE AMODE64=NO,ARCHLVL=OSREL,OSREL=SYSSTATE 
        IEABRCX  DEFINE    convert based branches to relative
*------------------------------------------------------------------- 
* Linkage and storage obtain
*------------------------------------------------------------------- 
         BAKR  R14,0                use linkage stack 
         LARL  R12,DATCONST         setup base for CONSTANTS
         USING DATCONST,R12         "baseless" CSECT 
        STORAGE OBTAIN,LENGTH=WALEN,EXECUTABLE=NO,LOC=ANY,CHECKZERO=YES
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
         LR R2,R1                   R2 points to Output DCB
         USING IHADCB,R2

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
* Copy the DCB template into IHADCB 24-bit storage
* The OPEN_PARMS and DCBE is 31-bit to minimize below-line stg
*
LIB_OPEN  DS  0H
         MVC IHADCB(DCBLEN),CONST_DCB
        OPEN (IHADCB,OUTPUT),MF=(E,OPEN_PARMS),MODE=31
         CIJE R15,0,OPEN_SUCCESS
*
OPEN_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
OPEN_SUCCESS DS 0H
*
MEM_WRITE DS  0H
        WRITE MEM_DECB,SF,(R2),WRITE_BUFFER
         CIJE R15,0,WRITE_SUCCESS
WRITE_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
WRITE_SUCCESS DS 0H
*

*
LIB_CLOSE  DS 0H
        CLOSE (IHADCB),MF=(E,CLOSE_PARMS),MODE=31
         CIJE R15,0,CLOSE_SUCCESS
*
CLOSE_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
CLOSE_SUCCESS DS 0H
         LA  R9,0                  CRTMEM successful put 0 in R9
*
* Free DCB storage
*
RLSE_DCB   DS 0H
        STORAGE RELEASE,ADDR=(R12),LENGTH=DCBLEN,EXECUTABLE=NO 

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
CONST_DCBE DCBE  RMODE31=BUFF
DCBLEN    EQU   *-CONST_DCB
*
WRITE_BUFFER DC CL80'Hello world'
         LTORG ,

*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA       DSECT 
SAVEA       DS    18F 
OPEN_PARMS  DS 2A
CLOSE_PARMS DS 2A
WALEN       EQU  *-SAVEA

IHADCB      DCBD 
         END   CRTMEM   
