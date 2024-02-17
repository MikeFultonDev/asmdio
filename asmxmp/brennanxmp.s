         YREGS
PDSWRITE CSECT
         SAVE  (14,12)
         LR    R12,R15
         USING PDSWRITE,R12
         ST    R13,SAVE+4
         LA    R11,SAVE
         ST    R11,8(R13)
         LR    R13,R11
*
***********************************************************************
*           OPEN DATASET AND TRY TO WRITE A NEW MEMBER
*           Example courtesy of Tom Brennan
***********************************************************************
*
         OPEN  (PDS,OUTPUT)           OPEN PDS DIRECTORY
         XR    R5,R5                  CLEAR TTR
*
WRITE    WRITE PDSECB,SF,PDS,PDSAREA,'S'  WRITE THE FIRST BLOCK
CHECK    CHECK PDSECB                 CHECK FOR COMPLETION AND EOF
         LTR   R5,R5                  IS THIS THE FIRST WRITE ?
         BNZ   NONOTE                 NO, THEN DON'T NOTE POSITION
NOTE     NOTE  PDS                    FIND OUT WHERE WE JUST WROTE
         LR    R5,R1                  AND SAVE TTR
         LA    R2,PDS
         USING IHADCB,R2
         MVC   DCBBLKSI,=H'320'
         DROP  R2
NONOTE   DS    0H
*
ALLDONE  STCM  R5,14,TTR              SAVE TTR
         STOW  PDS,DIRLIST,A          ADD TO DIRECTORY
         CLOSE (PDS)
*
         L     R13,SAVE+4
         RETURN (14,12),RC=0
*
PDS      DCB   DDNAME=PDS,MACRF=W,DSORG=PO
*
SAVE     DC    18F'0'
*
DIRLIST  DC    CL8'NEWNAME'
TTR      DS    XL3
K        DC    X'0'
*
         LTORG
*
PDSAREA  DS    0CL6160                BLOCK LENGTH
         DC    CL80'THIS IS RECORD NUMBER ONE'
         DC    CL80'THIS IS RECORD NUMBER 2'
         DC    CL80'THIS IS RECORD NUMBER 3'
         DC    CL80'THIS IS RECORD NUMBER FOUR'
         DS    CL6160
*
         DCBD  DSORG=PO,DEVD=DA
         END
