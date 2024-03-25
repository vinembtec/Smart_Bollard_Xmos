//====================================================================================================================
// File type: assembly subroutines definitions
// Author: Kevin Dewar
//
// Description:
//   This contains definitiosn of some routines written in assembly but intended to be called from xc
//  
//
// Status:
//   Working.
//====================================================================================================================
#include <xs1.h>


//--------------------------------------------------------------------------------------------------------------------
// OptMemCpy
// Routine to copy efficiently, using VPU,  where source is in Ext Mem (so prefetch necessary for efficiency)
// N.B. Will only work properly on multiples of 4 lines i.e. 32words/128 bytes (size & alignment)
//--------------------------------------------------------------------------------------------------------------------

// r0 - src addr
// r1 - dest addr
// r2 - number of words
.globl OptMemCpy
.globl OptMemCpy.nstackwords
.globl OptMemCpy.maxcores
.set OptMemCpy.maxcores,1
.globl OptMemCpy.maxtimers
.set OptMemCpy.maxtimers,0
.globl OptMemCpy.maxchanends
.set OptMemCpy.maxchanends,0
.linkset OptMemCpy.nstackwords,4
.align 4
.issue_mode dual

OptMemCpy:
  dualentsp 4
  stw  r4,  sp[0]
  stw  r5,  sp[1]
  stw  r6,  sp[2]
  
  shr  r2, r2, 5    // divide number of words by 32 (4 lines) to get correct number of loop iterations
  ldc  r3, 32       // number of bytes address increments, i.e. 1 lines, between vec accesses 
  ldc  r4, 64       // number of bytes address increments, i.e. 2 lines, between 2 stages of loop 
  ldc  r5, 68       // 2 lines + 1word offset to do misaligned prefetch of 2 lines, 2 lines ahead 
  ldc  r6, 128      // 4 lines, the number of bytes to increment addresses per loop

  add r11, r0, 4    // get the first 2 lines, for 1st iteration
  prefetch r11

  add  r11, r0, r5
OMC_cpy_loop:
  { prefetch r11          ; add  r11, r11, r4 }   // prefetch 2 lines for second half of loop
  mov  r11, r0
  
  { vldc r11[0]           ; add  r11, r11, r3 }
  { vldd r11[0]           ; mov  r11, r1      }
  { vstc r11[0]           ; add  r11, r11, r3 }
  { vstd r11[0]           ; add  r11, r0,  r4 }
  
  add r11, r0, r5
  add r11, r11, r4
  prefetch r11
  
  add  r11, r0,  r4 
  { vldc r11[0]           ; add  r11, r11, r3 }
  { vldd r11[0]           ; add  r11, r1,  r4  }
  { vstc r11[0]           ; add  r11, r11, r3 }
  { vstd r11[0]           ; sub  r2,  r2,  1  }

  { add  r0, r0, r6       ; add  r1,  r1, r6  } 
  { bt   r2, OMC_cpy_loop ; add  r11, r0, r5  }

  ldw r4,  sp[0]
  ldw r5,  sp[1]
  ldw r6,  sp[2]
	retsp 4



//--------------------------------------------------------------------------------------------------------------------
// OptMemCpy2
// Routine to copy efficiently, using VPU,  where source is in Ext Mem (so prefetch necessary for efficiency)
// N.B. Will only work properly on multiples of 4 lines i.e. 32words/128 bytes (size & alignment)
//--------------------------------------------------------------------------------------------------------------------

// r0 - src addr
// r1 - dest addr
// r2 - number of words
.globl OptMemCpy2
.globl OptMemCpy2.nstackwords
.globl OptMemCpy2.maxcores
.set OptMemCpy2.maxcores,1
.globl OptMemCpy2.maxtimers
.set OptMemCpy2.maxtimers,0
.globl OptMemCpy2.maxchanends
.set OptMemCpy2.maxchanends,0
.linkset OptMemCpy2.nstackwords,4
.align 4
.issue_mode dual

OptMemCpy2:
  dualentsp 4
  stw  r4,  sp[0]
  stw  r5,  sp[1]
  stw  r6,  sp[2]
  
  shr  r2, r2, 5    // divide number of words by 32 (4 lines) to get correct number of loop iterations
  ldc  r3, 32       // number of bytes address increments, i.e. 1 lines, between vec accesses 
  ldc  r4, 64       // number of bytes address increments, i.e. 2 lines, between 2 stages of loop 
  ldc  r5, 132      // 4 lines + 1word offset to do misaligned prefetch of 2 lines, 2 lines ahead 
  ldc  r6, 128      // 4 lines, the number of bytes to increment addresses per loop

  add r11, r0, 4    // get the first 2 lines, for 1st iteration
  prefetch r11
  add r11, r11, r4  // get the next 2 lines, for 1st iteration
  prefetch r11
  add r11, r11, r4

OMC2_cpy_loop:
  { prefetch r11          ; add  r11, r11, r4 }
  { prefetch r11          ; mov  r11, r0      }

  { vldc r11[0]           ; add  r11, r11, r3 }
  { vldd r11[0]           ; mov  r11, r1      }
  { vstc r11[0]           ; add  r11, r11, r3 }
  { vstd r11[0]           ; add  r11, r0,  r4 }

  { vldc r11[0]           ; add  r11, r11, r3 }
  { vldd r11[0]           ; add  r11, r1,  r4  }
  { vstc r11[0]           ; add  r11, r11, r3 }
  { vstd r11[0]           ; sub  r2,  r2,  1  }

  { add  r0, r0, r6       ; add  r1,  r1, r6  } 
  { bt   r2, OMC2_cpy_loop ; add  r11, r0, r5  }

  ldw r4,  sp[0]
  ldw r5,  sp[1]
  ldw r6,  sp[2]
	retsp 4





//--------------------------------------------------------------------------------------------------------------------
// OptMemCpy_1line_no_prefetch
// Routine to copy efficiently, using VPU,  where source is NOT in Ext Mem (so no prefetch required)
// N.B. Will only work properly on multiples of 1 line i.e. 8words/32 bytes (size & alignment)
//--------------------------------------------------------------------------------------------------------------------


// r0 - src addr
// r1 - dest addr
// r2 - number of words
.globl OptMemCpy_1line_no_prefetch
.globl OptMemCpy_1line_no_prefetch.nstackwords
.globl OptMemCpy_1line_no_prefetch.maxcores
.set OptMemCpy_1line_no_prefetch.maxcores,1
.globl OptMemCpy_1line_no_prefetch.maxtimers
.set OptMemCpy_1line_no_prefetch.maxtimers,0
.globl OptMemCpy_1line_no_prefetch.maxchanends
.set OptMemCpy_1line_no_prefetch.maxchanends,0
.linkset OptMemCpy_1line_no_prefetch.nstackwords,2
.align 4
.issue_mode dual

OptMemCpy_1line_no_prefetch:
  dualentsp 2
  stw  r11, sp[1]

  shr  r2, r2, 3    // divide number of words by 8 to get correct number of loop iterations
  ldc  r3, 32       // number of bytes address increments, i.e. 1 lines 
  mov  r11, r0
OMCnp_1line_cpy_loop:
  { vldc r11[0]                   ; mov  r11, r1    }
  { vstc r11[0]                   ; add  r0, r0, r3 }
  { add  r1, r1, r3               ; sub  r2, r2, 1  }
  { bt   r2, OMCnp_1line_cpy_loop ; mov  r11, r0    }

  ldw r11, sp[1]
	retsp 2



//--------------------------------------------------------------------------------------------------------------------
// UnoptMemCpy
// Routine to copy using only scalar instructions (no prefetch done) so just 1 word/loop
//--------------------------------------------------------------------------------------------------------------------


// r0 - src addr
// r1 - dest addr
// r2 - number of words
.globl UnoptMemCpy
.globl UnoptMemCpy.nstackwords
.globl UnoptMemCpy.maxcores
.set UnoptMemCpy.maxcores,1
.globl UnoptMemCpy.maxtimers
.set UnoptMemCpy.maxtimers,0
.globl UnoptMemCpy.maxchanends
.set UnoptMemCpy.maxchanends,0
.linkset UnoptMemCpy.nstackwords,0
.align 4
.issue_mode dual

UnoptMemCpy:
  dualentsp 0

UMC_cpy_loop:
  { ldw  r3, r0[0] ; add  r0, r0, 4 }
  { stw  r3, r1[0] ; add  r1, r1, 4 }
  { sub  r2, r2, 1 ; bt   r2, UMC_cpy_loop }

	retsp 0

