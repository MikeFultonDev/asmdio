//FULTONM1 JOB  MSGCLASS=A,NOTIFY=&SYSUID
//STEP1    EXEC PGM=IEHLIST
//SYSPRINT DD  SYSOUT=*
//DD1      DD  VOLUME=SER=Z2B049,DISP=OLD
//SYSIN    DD  *
      LISTPDS   DSNAME=FULTONM.ASMXMP.DATA,VOL=SER=Z2B049,FORMAT
/*