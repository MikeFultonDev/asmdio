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
        STORAGE OBTAIN,LENGTH=WALEN,EXECUTABLE=NO
         LR    R10,R1               R10 points to Working Storage 
         USING WAREA,R10            BASE FOR DSECT 
         MVC   SAVEA+4(4),=C'F1SA'  linkage stack convention 
         LAE   R13,SAVEA            ADDRESS OF OUR SA IN R13 
*------------------------------------------------------------------- 
* application logic                                                - 
*------------------------------------------------------------------- 

        STORAGE OBTAIN,LENGTH=DCB_LEN,EXECUTABLE=NO,LOC=24
         LR R2,R1                   R2 points to Output DCB

LIB_OPEN  DS  0H
         MVC 0(DCB_LEN,R2),CONST_DCB
        OPEN ((R2),OUTPUT),MF=(E,OPEN_PARMS),MODE=24
         CIJE R15,0,OPEN_SUCCESS
OPEN_FAIL DS  0H
         LR  R9,R15                put err code in R9
         B   DONE
*
OPEN_SUCCESS DS 0H
*
*MEM_WRITE DS  0H
*        WRITE MEM_DECB,SF,(R2),WRITE_BUFFER
*         CIJE R15,0,WRITE_SUCCESS
*WRITE_FAIL DS  0H
*         LR  R9,R15                put err code in R9
*         B   DONE
*
WRITE_SUCCESS DS 0H
*
         LA  R9,0                  CRTMEM successful put 0 in R9
*------------------------------------------------------------------- 
* Linkage and storage release. set RC (reg 15)                     -
*------------------------------------------------------------------- 
DONE     DS 0H
RLSE    STORAGE RELEASE,ADDR=(R10),LENGTH=WALEN,EXECUTABLE=NO 
         LR    R15,R9               get saved rc into R15
         PR    ,                    return to caller 

*------------------------------------------------------------------- 
* constants and literal pool                                       - 
*------------------------------------------------------------------- 
DATCONST DS     0D                 Doubleword alignment for LARL
CONST_DCB  DCB   DSORG=PO,MACRF=(W),DDNAME=MYDD
DCB_LEN   EQU    *-CONST_DCB
WRITE_BUFFER DC CL80'Hello world'
         LTORG ,                   create literal pool 

*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA    DSECT 
SAVEA    DS    18F 
OPEN_PARMS DS 1A
WALEN  EQU   *-SAVEA

IHADCB   DCBD 
DCBLEN   EQU   *-IHADCB
         END   CRTMEM   
