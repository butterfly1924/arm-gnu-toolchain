/*
 * Copyright (c) 2009 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ARM_ASM__H
#define ARM_ASM__H

#include "acle-compat.h"

#if __ARM_ARCH >= 7 && defined (__ARM_ARCH_ISA_ARM)
# define _ISA_ARM_7
#endif

#if __ARM_ARCH >= 6 && defined (__ARM_ARCH_ISA_ARM)
# define _ISA_ARM_6
#endif

#if __ARM_ARCH >= 5
# define _ISA_ARM_5
#endif

#if __ARM_ARCH >= 4 && __ARM_ARCH_ISA_THUMB >= 1
# define _ISA_ARM_4T
#endif

#if __ARM_ARCH >= 4 && __ARM_ARCH_ISA_THUMB == 0
# define _ISA_ARM_4
#endif


#if __ARM_ARCH_ISA_THUMB >= 2
# define _ISA_THUMB_2
#endif

#if __ARM_ARCH_ISA_THUMB >= 1
# define _ISA_THUMB_1
#endif

/* Check whether leaf function PAC signing has been requested in the
   -mbranch-protect compile-time option.  */
#define LEAF_PROTECT_BIT 2

#ifdef __ARM_FEATURE_PAC_DEFAULT
# define HAVE_PAC_LEAF \
	__ARM_FEATURE_PAC_DEFAULT & (1 << LEAF_PROTECT_BIT)
#else
# define HAVE_PAC_LEAF 0
#endif

/* Provide default parameters for PAC-code handling in leaf-functions.  */
#ifndef PAC_LEAF_PUSH_IP
# define PAC_LEAF_PUSH_IP 0
#endif

#if HAVE_PAC_LEAF
# if PAC_LEAF_PUSH_IP
#   define PAC_CFI_ADJ 4
# else
#  define PAC_CFI_ADJ 0
# endif /* PAC_LEAF_PUSH_IP*/
#else
# undef PAC_LEAF_PUSH_IP
# define PAC_LEAF_PUSH_IP 0
# define PAC_CFI_ADJ 0
#endif /* HAVE_PAC_LEAF */

#ifdef __ASSEMBLER__
/* Emit .cfi_restore directives for a consecutive sequence of registers.  */
	.macro cfirestorelist first, last
	.cfi_restore \last
	.if \last-\first
	cfirestorelist \first, \last-1
	.endif
	.endm

/* Emit .cfi_offset directives for a consecutive sequence of registers.  */
	.macro cfisavelist first, last, index=1
	.cfi_offset \last, -4*(\index) - PAC_CFI_ADJ
	.if \last-\first
	cfisavelist \first, \last-1, \index+1
	.endif
	.endm

/* Create a prologue entry sequence handling PAC/BTI, if required and emitting
   CFI directives for generated PAC code and any pushed registers.  */
	.macro prologue first=-1, last=-1, savepac=PAC_LEAF_PUSH_IP
#if HAVE_PAC_LEAF
#if __ARM_FEATURE_BTI_DEFAULT
	pacbti	ip, lr, sp
#else
	pac	ip, lr, sp
#endif /* __ARM_FEATURE_BTI_DEFAULT */
	.cfi_register 143, 12
#else
#if __ARM_FEATURE_BTI_DEFAULT
	bti
#endif /* __ARM_FEATURE_BTI_DEFAULT */
#endif /* HAVE_PAC_LEAF */
	.if \first != -1
	.if \last != -1
	.if \savepac
	push {r\first-r\last, ip}
	.cfi_adjust_cfa_offset ((\last-\first)+1)*4 + PAC_CFI_ADJ
	.cfi_offset 143, -PAC_CFI_ADJ
	cfisavelist \first, \last
	.else
	push {r\first-r\last}
	.cfi_adjust_cfa_offset ((\last-\first)+1)*4
	cfisavelist \first, \last
	.endif
	.else
	.if \savepac
	push {r\first, ip}
	.cfi_adjust_cfa_offset 4 + PAC_CFI_ADJ
	.cfi_offset 143, -PAC_CFI_ADJ 
	cfisavelist \first, \first
	.else // !\savepac
	push {r\first}
	.cfi_adjust_cfa_offset PAC_CFI_ADJ
	cfisavelist \first, \first
	.endif
	.endif
	.else // \first == -1
	.if \savepac
	push {ip}
	.cfi_adjust_cfa_offset PAC_CFI_ADJ
	.cfi_offset 143, -PAC_CFI_ADJ
	.endif
	.endif
	.endm

/* Create an epilogue exit sequence handling PAC/BTI, if required and emitting
  CFI directives for all restored registers.  */
	.macro epilogue first=-1, last=-1, savepac=PAC_LEAF_PUSH_IP
	.if \first != -1
	.if \last != -1
	.if \savepac
	pop {r\first-r\last, ip}
	.cfi_restore 143
	cfirestorelist \first, \last
	.else
	pop {r\first-r\last}
	cfirestorelist \first, \last
	.endif
	.else
	.if \savepac
	pop {r\first, ip}
	.cfi_restore 143
	cfirestorelist \first, \first
	.else
	pop {r\first}
	cfirestorelist \first, \first
	.endif
	.endif
	.else
	.if \savepac
	pop {ip}
	.cfi_restore 143
	.endif
	.endif
	.cfi_def_cfa_offset 0
#if HAVE_PAC_LEAF
	aut	ip, lr, sp
#endif /* HAVE_PAC_LEAF */
	bx	lr
	.endm
#endif /* __ASSEMBLER__ */

#endif /* ARM_ASM__H */
