# asmdio
Dataset I/O in assembler

## References

- [SVC99](https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-example-dynamic-allocation-request) [Macro](): DYNALLOC aka SVC99 
- [DCBD](https://www.ibm.com/docs/en/zos/3.1.0?topic=nvmd-dcbdprovide-symbolic-reference-data-control-blocks-bdam-bisam-bpam-bsam-qisam-qsam) [Macro](): Data Control Block symbolic names.
- [OPEN](https://www.ibm.com/docs/en/zos/3.1.0?topic=nvmd-openconnect-program-data-bdam-bisam-interface-vsam-bpam-bsam-qisam-interface-vsam-qsam) [Macro](https://tech.mikefulton.ca/QSAMOPEN), [SVC 19](https://tech.mikefulton.ca/SVC19-OPEN): Open a DCB.
- [BLDL](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-bldl-macro-construct-directory-entry-list) Macro: Read one or more directory entries into virtual storage.
- [FIND](https://www.ibm.com/docs/en/zos/3.1.0?topic=descriptions-findestablish-beginning-data-set-member-bpam) Macro: Establish the beginning of a data set member using a BLDL list or directory.
- [DESERV](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-directory-entry-services) [Macro](https://tech.mikefulton.ca/DESERV): Directory Entry Services for PDS and PDSE data sets.
- [ISITMGD](https://www.ibm.com/docs/en/zos/3.1.0?topic=pmp-using-isitmgd-determine-whether-data-set-is-system-managed) Macro: Determine if data set is SMS managed and info about a PDSE.
- [NOTE](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-note-macro-provide-relative-position) Macro: Return the TTRz that can subsequently be used by POINT.
- [POINT](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-point-macro-position-block) Macro: Cause the next READ or WRITE to be from the TTRz specified.
- [STOW](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-using-stow-macro-update-directory) [Macro](https://tech.mikefulton.ca/STOWMacro): Modify (Add, Delete, Replace, Change) a member in the directory. 
- [READ/WRITE](https://www.ibm.com/docs/en/zos/3.1.0?topic=records-accessing-data-read-write) Macros: Read and Write BLOCKs, not records.
- [GET/PUT](https://www.ibm.com/docs/en/zos/3.1.0?topic=records-accessing-data-get-put) Macros: Get and Put RECORDS, not blocks. 
- [CLOSE](https://www.ibm.com/docs/en/zos/3.1.0?topic=nvmd-openconnect-program-data-bdam-bisam-interface-vsam-bpam-bsam-qisam-interface-vsam-qsam) [Macro](https://tech.mikefulton.ca/QSAMCLOSE), [SVC 20](https://tech.mikefulton.ca/SVC20-CLOSE): Close a DCB.

## Key Points

- I/O
- A _connection_ to a PDSE member provides a [temporary version of that member](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-establishing-connections-members).
- You can _not_ [extend a PDSE member](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-extending-member), although you _can_ [update in place](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-extending-member). 
- A record must be [read before it can be updated](https://www.ibm.com/docs/en/zos/3.1.0?topic=uip-bsam-bpam).
- When a PDSE member is deleted, all aliases to that member are also deleted.


## Terminology

- [PDSE](https://www.ibm.com/docs/en/zos/3.1.0?topic=files-processing-partitioned-data-set-extended-pdse): Partitioned Data set Extended.
- [TTR](https://www.ibm.com/docs/en/zos/3.1.0?topic=pdse-relative-track-addresses-ttr): Token that simulates the track and record location. Similar to a file offset _off_t_. 


