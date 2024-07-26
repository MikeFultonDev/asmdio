# Assembler Examples

This directory has 2 examples in assembler:
- brennanxmp.s: This example, courtesy of Tom Brennan:
  - opens the dataset pointed to by DDName PDS for write
  - NOTEs the current location in the data set
  - writes a block (4 records)
  - issues a STOW with the location NOTEd and the member name NEWMEM
  - closes the PDS

- crtmem.s: This example:
  - same general idea as brennanxmp.s but the STOW also specifies IFF to indicate a CCSID of 1047


