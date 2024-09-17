//********************************************************************
//*    LICENSED MATERIALS - PROPERTY OF IBM                          *
//*    5650-ZOS                                                      *
//*    COPYRIGHT IBM CORP. 1988, 2019                                *
//*                                                                  *
//********************************************************************
//*                                                                  *
//*  INVOKE THE ASSEMBLER AND DSECT UTILITY                          *
//*                                                                  *
//*  z/OS XL C/C++                                                   *
//*  STATUS = HLB77C0                                                *
//*                                                                  *
//********************************************************************
//*
//EDCDSECT PROC INFILE=,            < INPUT ASSEMBLER SOURCE...REQUIRED
//   OUTFILE=,                      < OUTPUT C STRUCTURE    ...REQUIRED
//   DPARM=,                        < DSECT UTILITY OPTIONS
//   APARM=,                        < ASSEMBLER OPTIONS
//   LIBPRFX='CEE',                 < PREFIX FOR LIBRARY DSN
//   LNGPRFX='CBC',                 < PREFIX FOR LANGUAGE DSN
//   TUNIT='SYSDA'                  < UNIT FOR TEMPORARY FILES
//*-------------------------------------------------------------
//*  ASSEMBLY STEP
//*-------------------------------------------------------------
//ASSEMBLE EXEC PGM=ASMA90,PARM=(,
//   'ADATA,LIST,NOTERM,NODECK,NOOBJECT,&APARM')
//SYSPRINT DD SYSOUT=*
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR
//SYSADATA DD DSN=&&DSECT,UNIT=&TUNIT.,DISP=(MOD,PASS),
//         SPACE=(32000,(30,30)),DCB=(RECFM=VB,LRECL=8144,BLKSIZE=8192)
//SYSUT1   DD UNIT=&TUNIT.,DISP=(NEW,DELETE),SPACE=(32000,(30,30))
//SYSUT2   DD UNIT=&TUNIT.,DISP=(NEW,DELETE),SPACE=(32000,(30,30))
//SYSUT3   DD UNIT=&TUNIT.,DISP=(NEW,DELETE),SPACE=(32000,(30,30))
//SYSPUNCH DD DUMMY
//SYSLIN   DD DUMMY
//SYSIN    DD DSN=&INFILE,DISP=SHR
//*
//*-------------------------------------------------------------
//* DSECT UTILITY STEP
//*-------------------------------------------------------------
//DSECT    EXEC PGM=CCNEDSCT,PARM='&DPARM',
//         COND=(8,LE,ASSEMBLE)
//STEPLIB  DD  DSNAME=&LIBPRFX..SCEERUN2,DISP=SHR
//         DD  DSNAME=&LNGPRFX..SCCNCMP,DISP=SHR
//         DD  DSNAME=&LIBPRFX..SCEERUN,DISP=SHR
//SYSADATA DD DSN=*.ASSEMBLE.SYSADATA,DISP=(MOD,PASS)
//EDCDSECT DD DSN=&OUTFILE,DISP=OLD
//SYSPRINT DD SYSOUT=*
//SYSOUT   DD SYSOUT=*
