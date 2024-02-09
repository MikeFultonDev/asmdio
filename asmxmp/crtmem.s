         START , 
         YREGS ,                   register equates, syslib SYS1.MACLIB 
CRTMEM   CSECT , 
CRTMEM   AMODE 31 
CRTMEM   RMODE ANY 
        SYSSTATE AMODE64=NO,ARCHLVL=OSREL,OSREL=SYSSTATE 
        IEABRCX  DEFINE    convert based branches to relative
*------------------------------------------------------------------- 
* fibonacci sequence first 79 results. call BPX1WRT to print       - 
* to stdout in z/OS Unix                                           - 
* Use 64 bit instructions where applicable to get doubleword values-
*------------------------------------------------------------------- 
*------------------------------------------------------------------- 
* Linkage and getmain                                              - 
*------------------------------------------------------------------- 
         BAKR  R14,0                use linkage stack 
         LARL  R12,DATCONST         setup base for CONSTANTS
         USING DATCONST,R12         "baseless" CSECT 
        STORAGE OBTAIN,LENGTH=WALEN1,EXECUTABLE=NO  dynamic storage 
         LR    R10,R1               LOAD ADDRESS OF STORAGE 
         USING WAREA1,R10           BASE FOR DSECT 
         MVC   SAVEA1+4(4),=C'F1SA' linkage stack convention 
         LAE   R13,SAVEA1           ADDRESS OF OUR SA IN R13 
*------------------------------------------------------------------- 
* application logic                                                - 
*------------------------------------------------------------------- 

*------------------------------------------------------------------- 
* Linkage and freemain. set RC (reg 15)                            -
*------------------------------------------------------------------- 
RLSE    STORAGE RELEASE,ADDR=(R10),LENGTH=WALEN1,EXECUTABLE=NO 
         LA    R15,0                load RC=0 
         PR    ,                    return to caller 

*------------------------------------------------------------------- 
* constants and literal pool                                       - 
*------------------------------------------------------------------- 
DATCONST DS     0D                 Doubleword alignment for LARL
LIBOPEN  OPEN (OUTPUT,(OUTPUT)),MODE=24
OUTPUT   DCB   DSORG=PO,MACRF=(W),DDNAME=NAME
NAME     DC    CL8'MYDD    '
DCALL    CALL    ,(0,0,0,0,0,0,0),MF=L 
LDCALL   EQU    *-DCALL 
         LTORG ,                   create literal pool 

*------------------------------------------------------------------- 
* DSECT                                                            - 
*------------------------------------------------------------------- 
WAREA1   DSECT 
SAVEA1   DS    18F 
         DS    0D 
SOMETHING DS   F
WALEN1   EQU   *-SAVEA1 
         END   CRTMEM   
