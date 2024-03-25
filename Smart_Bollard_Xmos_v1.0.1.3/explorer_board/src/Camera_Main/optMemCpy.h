//====================================================================================================================
// File type: assembly subroutines prototypes
// Author: Kevin Dewar
//
// Description:
//   This contains prototypes of some routines written in assembly but intended to be called from xc
//  
//
// Status:
//   Working.
//====================================================================================================================


#ifndef _OPT_MEMCPY_H_
#define _OPT_MEMCPY_H_

/**
 * DDR to SRAM
 * count is the numnber of words to be copied. Must be a multiple
 * of 32 words. src and dst must be word aligned. For fastest result make
 * them 32-byte aligned.
 */
void OptMemCpy                   (void * unsafe src, void * unsafe dst, unsigned int count);
/**
 * DDR to SRAM
 * count is the numnber of words to be copied. Must be a multiple
 * of 32 words. src and dst must be word aligned. For fastest result make
 * them 32-byte aligned.
 */
void OptMemCpy2                  (void * unsafe src, void * unsafe dst, unsigned int count);

/* SRAM to DDR*/
void OptMemCpy_1line_no_prefetch    (void * unsafe src, void * unsafe dst, unsigned int count);


#endif /* _OPT_MEMCPY_H_ */
