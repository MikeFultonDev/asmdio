         START , 
MACMEM   CSECT , 
MACMEM   AMODE 31 
MACMEM   RMODE ANY
         YREGS ,               register equates, syslib SYS1.MACLIB
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
         LR    R2,R1                 system did not clear,do ourselves
         LA    R3,DCBLEN
         XR    R5,R5
         MVCL  R2,R4                 clear storage (pad byte zero)

STG_DCB_CLEAR DS 0H

*
* Body of Code
*

*------------------------------------------------------------------- 
* Linkage and storage release. set RC (reg 15)                     -
*------------------------------------------------------------------- 
DONE     DS 0H
RLSE_WA  DS 0H
        STORAGE RELEASE,ADDR=(R10),LENGTH=WALEN,EXECUTABLE=NO 
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

CONST_MEM_IFF      DS 0F
CONST_MEM_LEN      DC AL2(MEMIFFLEN)
CONST_MEM_FLAGS    DC XL1'0'
CONST_MEM_DCB24    DC XL3'0'
CONST_MEM_TIME     DC 2F'0'
CONST_MEM_DIRA     DC 1A(0)
CONST_MEM_TYPE     DC XL16'0'
CONST_MEM_CCSID    DC 1H'1047'
MEMIFFLEN EQU *-CONST_MEM_IFF

CONST_MEM_DIR      DS 0F
CONST_MEM_NAME     DC CL8'NEWMEM'
CONST_MEM_TTR      DC XL3'0'
MEMINFOLEN EQU *-CONST_MEM_IFF

         LTORG ,

*
* Addressability DSECTs
*

DCB DCBD DSORG=PO,DEVD=DA
DECB IHADECB
*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA       DSECT 
SAVEA       DS    18F 
OPEN_PARMS  DS CL(OPENLEN)
CLOSE_PARMS DS CL(CLOSELEN)
WRITE_DECB  DS CL(WRITELEN)

MEM_IFF      DS 0F
MEM_LEN      DS 1H
MEM_FLAGS    DS XL1
MEM_DCB24    DS XL3
MEM_TIME     DS 2F
MEM_DIRA     DS 1A
MEM_TYPE     DS XL16
MEM_CCSID    DS 1H

MEM_DIR      DS 0F
MEM_NAME     DS CL8
MEM_TTR      DS XL3
WALEN       EQU  *-SAVEA
DCBAREA     DSECT
LIB_DCB     DS   CL(DCBLEN)
         END   MACMEM 
