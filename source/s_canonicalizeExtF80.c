
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.  

Copyright 2011, 2012, 2013, 2014 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

/* This particular file was written by John Byrd. */

#include <stdint.h>
#include "platform.h"
#include "internals.h"

/*----------------------------------------------------------------------------
| Canonicalizes the encoding of an extFloat80 value so that its bit pattern
| reflects its mathematical value.  Handles three non-canonical cases:
|  - unnormal (exp > 0, exp < 0x7FFF, J = 0): normalize or collapse to zero
|  - pseudo-denormal (exp = 0, J = 1): equivalent to exp = 1
|  - pseudo-infinity (exp = 0x7FFF, sig = 0): set canonical infinity sig
| On return, *expPtr and *sigPtr hold the canonical encoding.  Pseudo-NaN
| inputs (exp = 0x7FFF, sig != 0, J = 0) are left unchanged; callers are
| expected to have already handled NaN before calling this function.
*----------------------------------------------------------------------------*/
void
 softfloat_canonicalizeExtF80(
     int_fast32_t *expPtr, uint_fast64_t *sigPtr )
{
    int_fast32_t exp;
    uint_fast64_t sig;
    struct exp32_sig64 normExpSig;

    exp = *expPtr;
    sig = *sigPtr;
    if ( exp == 0x7FFF ) {
        if ( ! (sig & UINT64_C( 0x7FFFFFFFFFFFFFFF )) )
            *sigPtr = UINT64_C( 0x8000000000000000 );
    } else if ( exp ) {
        if ( ! (sig & UINT64_C( 0x8000000000000000 )) ) {
            if ( ! sig ) {
                *expPtr = 0;
            } else {
                normExpSig = softfloat_normSubnormalExtF80Sig( sig );
                if ( exp + normExpSig.exp >= 1 ) {
                    *expPtr = exp + normExpSig.exp;
                    *sigPtr = normExpSig.sig;
                } else {
                    if ( exp > 1 ) *sigPtr = sig << (exp - 1);
                    *expPtr = 0;
                }
            }
        }
    } else {
        if ( sig & UINT64_C( 0x8000000000000000 ) )
            *expPtr = 1;
    }

}
