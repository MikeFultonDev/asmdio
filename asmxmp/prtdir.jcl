  //PRINTDIR JOB  ...
  //STEP1    EXEC PGM=IEBPTPCH
  //SYSPRINT DD  SYSOUT=A
  //SYSUT1   DD  DSNAME=MAIN.PDS,
  //             DISP=(OLD,KEEP),DCB=(RECFM=U,BLKSIZE=256)
  //SYSUT2   DD  SYSOUT=A
  //SYSIN    DD  *
     PRINT   TYPORG=PS,TOTCONV=XE
     TITLE   ITEM=('PRINT PARTITIONED DIRECTORY OF PDS',10)
     TITLE   ITEM=('FIRST TWO BYTES SHOW NUM OF USED BYTES',10)
    LABELS   DATA=NO
  /*
