         START , 
ENQMEM   CSECT , 
ENQMEM   AMODE 31 
ENQMEM   RMODE ANY
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
         MVC ENQTESTPARM,ENQTEST     INITIALIZE ENQUEUE PARAMETERS
         LA  R3,SPFEDIT
         LA  R4,MEMNAME
         LA  R5,52
         ENQ ((R3),(R4),E,(R5),SYSTEMS),RET=TEST,MF=(E,ENQTESTPARM)
         LR  R9,R15                Save reg for RC
*------------------------------------------------------------------- 
* Linkage and storage release. set RC (reg 15)                     -
*------------------------------------------------------------------- 
DONE     DS 0H
RLSE_WA  DS 0H
        STORAGE RELEASE,ADDR=(R10),LENGTH=WALEN,EXECUTABLE=NO 
         LR R15,R9
         PR    ,                    return to caller 

*------------------------------------------------------------------- 
* constants and literal pool                                       - 
*------------------------------------------------------------------- 
DATCONST   DS    0D                 Doubleword alignment for LARL

SPFEDIT  DC CL8'SPFEDIT '
MEMNAME  DC 0C
PDS      DC CL44'MFULTON.ASMXMP.DATA                        '
MEM      DC CL8'MEMBER3 '
         LTORG ,

*
* Addressability DSECTs
*
ENQTEST     ENQ   (*-*,*-*,E,*-*,SYSTEMS),RET=TEST,MF=L
ENQTESTLEN  EQU *-ENQTEST
ENQDEQ      ISGPEL
*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA       DSECT 
SAVEA       DS   18F
ENQTESTPARM DS   CL(ENQTESTLEN)
WALEN       EQU  *-SAVEA

         END   ENQMEM 
