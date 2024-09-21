# asmdio

Dataset I/O in assembler

This repository provides core I/O services in C, for use in both 31-bit and 64-bit code.
The services provide the full set of capabilities that an assembler programmer would have at their disposal.
As such, the interfaces tend to have complex structures defining the interface.

The intent of these services are to enable C programmers to use the full capability of the z/OS core I/O services
without having to write assembler code, and that these services are available in both 31-bit and 64-bit mode.

For applications that need to perform common I/O operations, the Language Environment C services may be a better option.

## References

### DSECT Layouts

- [BPAM DCB](https://tech.mikefulton.ca/BPAMDCBLayout)
- [DCBE](https://tech.mikefulton.ca/DCBELayout)  
- [IOB](https://tech.mikefulton.ca/IOBLayout)
- [DECB?]()

### I/O SVCs

- [SVC 99](https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-example-dynamic-allocation-request): Dynamic Allocation of DCB
- [SVC 19](https://tech.mikefulton.ca/SVC19-OPEN): Open a DCB
- [SVC 21](https://tech.mikefulton.ca/SVC21): STOW (update directory entry for member)
- [SVC 20](https://tech.mikefulton.ca/SVC20-CLOSE): Close a DCB.

### I/O Macros

- [DCBD Macro](https://www.ibm.com/docs/en/zos/3.1.0?topic=nvmd-dcbdprovide-symbolic-reference-data-control-blocks-bdam-bisam-bpam-bsam-qisam-qsam): Data Control Block symbolic names.
- [DCB BPAM Macro](https://tech.mikefulton.ca/DCBBPAMMacro): DCB Macro for BPAM usage
- [Dynalloc Macro](https://tech.mikefulton.ca/DynallocMacro): Dynamic Allocation of DCB
- [OPEN](https://www.ibm.com/docs/en/zos/3.1.0?topic=nvmd-openconnect-program-data-bdam-bisam-interface-vsam-bpam-bsam-qisam-interface-vsam-qsam) [Macro](https://tech.mikefulton.ca/QSAMOPEN), 
  - [Macro](https://tech.mikefulton.ca/DynallocMacro): DYNALLOC aka SVC99.
- [BLDL](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-bldl-macro-construct-directory-entry-list) Macro: Read one or more directory entries into virtual storage.
- [FIND](https://www.ibm.com/docs/en/zos/3.1.0?topic=descriptions-findestablish-beginning-data-set-member-bpam) Macro: Establish the beginning of a data set member using a BLDL list or directory.
- [DESERV](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-directory-entry-services) [Macro](https://tech.mikefulton.ca/DESERV): Directory Entry Services for PDS and PDSE data sets.
  - [DESERV Parameters](https://tech.mikefulton.ca/DESERV_GET)
- [SMDE Macro](https://tech.mikefulton.ca/SMDEMacro): Directory Entry information returned from DESERV GET.
- [ISITMGD](https://www.ibm.com/docs/en/zos/3.1.0?topic=pmp-using-isitmgd-determine-whether-data-set-is-system-managed) Macro: Determine if data set is SMS managed and info about a PDSE.
- [NOTE](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-note-macro-provide-relative-position) Macro: Return the TTRz that can subsequently be used by POINT.
- [POINT](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-point-macro-position-block) Macro: Cause the next READ or WRITE to be from the TTRz specified.
- [IHAPDS Macro](https://tech.mikefulton.ca/IHAPDSMacro) PDSD2 Directory Entry for input to STOW.
- [STOW](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-stow-macro-update-directory) [Macro](https://tech.mikefulton.ca/STOWMacro): Modify (Add, Delete, Replace, Change) a member in the directory.
- [DECB Macro](https://tech.mikefulton.ca/DECBMacro): Input/Output for the READ and WRITE services.
- [GETBUF Macro](https://tech.mikefulton.ca/GETBufMacro): Obtain a buffer.
- [READ/WRITE](https://www.ibm.com/docs/en/zos/3.1.0?topic=records-accessing-data-read-write) 
  - [READ Macro](https://tech.mikefulton.ca/READMacro): Read BLOCKs, not records.
  - [WRITE Macro](https://tech.mikefulton.ca/WRITEMacro): Write BLOCKs, not records.
  - [Non-VSAM I/O Exit Routines](https://tech.mikefulton.ca/NonVSAMIOExitRoutines): How to process end of data, synchronous error, etc.
- [GET/PUT](https://www.ibm.com/docs/en/zos/3.1.0?topic=records-accessing-data-get-put) Macros: Get and Put RECORDS, not blocks. 
- [CLOSE](https://www.ibm.com/docs/en/zos/3.1.0?topic=nvmd-openconnect-program-data-bdam-bisam-interface-vsam-bpam-bsam-qisam-interface-vsam-qsam) [Macro](https://tech.mikefulton.ca/QSAMCLOSE)

## Other References

- [Abend Codes](https://tech.mikefulton.ca/ZOSAbendCodes)
- [EBCDIC](https://tech.mikefulton.ca/EBCDICReference)
- [Dataset Record Formats](https://tech.mikefulton.ca/DatasetRecordFormats)

## Key Points

- I/O
- A _connection_ to a PDSE member provides a [temporary version of that member](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-establishing-connections-members).
- You can _not_ [extend a PDSE member](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-extending-member), although you _can_ [update in place](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-extending-member).
- A record must be [read before it can be updated](https://www.ibm.com/docs/en/zos/3.1.0?topic=uip-bsam-bpam).
- When a PDSE member is deleted, all aliases to that member are also deleted.
- When reading a block, the block may not be a complete block (e.g. it is the last block of a dataset). You can [determine the length](https://tech.mikefulton.ca/BlockLengthReadDetermination) after a CHECK macro is issued.
- When writing a block, the block may not be a complete block (e.g. it is the last block of a dataset). You can [specify the length](https://tech.mikefulton.ca/BlockLengthWriteDetermination) on the WRITE DCBBLKSI.

## CCSID for PDSE Members

- The DESERV GET service returns the System Managed Directory Entry (SMDE) for the member requested
- The SMDE includes a section called the SMDE Extended Attributes
- The Extended Attributes has a 2 byte CCSID in it (among other things)
- The STOW service lets you modify member entries, which lets you SET the CCSID (see the IFF function)

## Terminology

- [PDSE](https://www.ibm.com/docs/en/zos/3.1.0?topic=files-processing-partitioned-data-set-extended-pdse): Partitioned Data set Extended.
- [TTR](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-relative-track-addresses-ttr): Token that simulates the track and record location. Similar to a file offset _off_t_.
