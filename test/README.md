# Test cases

There are currently 3 test cases that show how to use the underlying C interfaces to the low-level dataset I/O services.

- basicalloc.c : This test allocates a hard-coded DD name (DDPASS) to the PDS SYS1.MACLIB as DISP=SHR.
  Note that if you were to use `fopen` of a member directly, the underlying allocation would be `DISP=OLD` so this
  example could be used as a way to establish a DD name that can subsequently be passed to `fopen` using the form: `//DD:<DD name>`,
  with a DD name allocated SHR instead of OLD. An alternate test using a system-generated DD name would be useful to write as well.

- basiccreate.c : This test allocates a hard-coded DD name (MYDD) to the PDSE specified on the command-line. The PDSE needs to already
  exist. It then creates a new member specified on the command-line, with extended attributes including the hard-coded CCSID 819.
  It then writes a block of ASCII 'A' characters to the member, then closes the dataset.

- basicread.c : This test allocates a hard-coded DD name  (MYDD) to the PDSE specified on the command-line. The PDSE needs to already
  exist. It then reads the extended attributes of new member specified on the command-line and prints out the extended attributes.
  It then reads the first block of characters in the member, and dumps the storage to the console, then closes the dataset.
