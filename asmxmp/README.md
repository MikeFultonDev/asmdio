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

- rdmccsid.s: This example:
  - opens the PDSE and uses DESERV to get extended attributes
  - returns the CCSID for the member (if set) as a way to validate the CCSID in crtmem works
  - Note that standard utilities DO NOT dump the extended attribute information (the JCLs in this directory)

## Using TSO Test

To build the code for use under TSO TEST:
- make sure to assemble with the TEST option
- make sure to link with the TEST option
- it appears you have to bind into a load module and not a program object?

Here is a link to the docs:
 - [Command Syntax](https://www.ibm.com/docs/en/zos/3.1.0?topic=subcommand-testlist-operands)
 - [Addressing](https://www.ibm.com/docs/en/zos/3.1.0?topic=program-addressing-conventions-associated-test-testauth)

The LIST command does much of what you need:

- `list 0r:15r` # list all the registers
- `list 7r? len(32)` # dump storage from the address in register 7 for 32 bytes
- `list 5r?? len(4)` # double-indirection from R5 instead of just one indirection
- `list 10FE6. len(20)` # dump 20 bytes from address 0x10FE6
- `list +3a i m(20)` # list 20 instructions starting from address +3a

## Using HLASM Toolkit

- Need to build with ADATA and then run ASMLANGX to generate a LANGX file (see buildrd)
- [Using IDF](https://www.ibm.com/docs/en/hla-and-tf/1.6?topic=guide-using-idf)
- [Invoking IDF](https://www.ibm.com/docs/en/hla-and-tf/1.6?topic=tso-invoking-idf)

- [Example](https://www.ibm.com/docs/en/hla-and-tf/1.6?topic=details-address-expressions) to set R6 to 0xEC
  - `R6 X'EC'` 

### Debugging RDMEM with IDF on TSO

- Set up DD for dataset to read:  `ALLOC DD(MYDD) DA(ASMXMP.DATA)`
- Set up DD for ASMLANGX: `ALLOC DD(ASMLANGX) DA(ASMXMP.ASMLANGX)`
- Set up DD for load module: `TSOLIB ACT DA(ASMXMP.LOAD)`
- Run IDF: `ASMIDF RDMEM`
- When done:
  - Free TSOLIB: `TSOLIB DEACT`
  - Free ASMLANGX: `FREE DD(ASMLANGX)`
