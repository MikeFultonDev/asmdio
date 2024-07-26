# Assembler Examples

This directory has 3 examples in assembler:
- brennanxmp.s: This example, courtesy of Tom Brennan:
  - opens the dataset pointed to by DDName PDS for write
  - NOTEs the current location in the data set
  - writes a block (4 records)
  - issues a STOW with the location NOTEd and the member name NEWMEM
  - closes the PDS

- crtmem.s: This example:
  - same general idea as brennanxmp.s but the STOW also specifies IFF to indicate a CCSID of 1047

- rdmem.s: This example:
  - opens the PDSE and uses DESERV to get extended attributes (DESERV failing right now)
  - intent is to return the CCSID for the member (if set) as a way to validate the CCSID in crtmem works
  - Note that standard utilities DO NOT dump the extended attribute information (the JCLs in this directory)
