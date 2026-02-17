
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

#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

bool extF80_eq( extFloat80_t a, extFloat80_t b )
{
    union { struct extFloat80M s; extFloat80_t f; } uA;
    uint_fast16_t uiA64;
    uint_fast64_t uiA0;
    union { struct extFloat80M s; extFloat80_t f; } uB;
    uint_fast16_t uiB64;
    uint_fast64_t uiB0;
    int_fast32_t expA, expB;
    struct exp32_sig64 normExpSig;

    uA.f = a;
    uiA64 = uA.s.signExp;
    uiA0  = uA.s.signif;
    uB.f = b;
    uiB64 = uB.s.signExp;
    uiB0  = uB.s.signif;
    if ( isNaNExtF80UI( uiA64, uiA0 ) || isNaNExtF80UI( uiB64, uiB0 ) ) {
        if (
               softfloat_isSigNaNExtF80UI( uiA64, uiA0 )
            || softfloat_isSigNaNExtF80UI( uiB64, uiB0 )
        ) {
            softfloat_raiseFlags( softfloat_flag_invalid );
        }
        return false;
    }
    /*------------------------------------------------------------------------
    | Canonicalize non-canonical encodings so that the bit-pattern comparison
    | below reflects mathematical values:
    |  - unnormal (exp>0, exp<max, J=0): normalize or collapse to zero
    |  - pseudo-denormal (exp=0, J=1): equivalent to exp=1 with same sig
    |  - pseudo-infinity (exp=max, sig=0): canonical infinity sig
    *------------------------------------------------------------------------*/
    expA = uiA64 & 0x7FFF;
    if ( expA == 0x7FFF ) {
        if ( ! (uiA0 & UINT64_C( 0x7FFFFFFFFFFFFFFF )) )
            uiA0 = UINT64_C( 0x8000000000000000 );
    } else if ( expA ) {
        if ( ! (uiA0 & UINT64_C( 0x8000000000000000 )) ) {
            if ( ! uiA0 ) {
                uiA64 &= 0x8000;
            } else {
                normExpSig = softfloat_normSubnormalExtF80Sig( uiA0 );
                if ( expA + normExpSig.exp >= 1 ) {
                    uiA64 = (uiA64 & 0x8000) | (expA + normExpSig.exp);
                    uiA0 = normExpSig.sig;
                } else {
                    if ( expA > 1 ) uiA0 <<= (expA - 1);
                    uiA64 &= 0x8000;
                }
            }
        }
    } else {
        if ( uiA0 & UINT64_C( 0x8000000000000000 ) )
            uiA64 = (uiA64 & 0x8000) | 1;
    }
    expB = uiB64 & 0x7FFF;
    if ( expB == 0x7FFF ) {
        if ( ! (uiB0 & UINT64_C( 0x7FFFFFFFFFFFFFFF )) )
            uiB0 = UINT64_C( 0x8000000000000000 );
    } else if ( expB ) {
        if ( ! (uiB0 & UINT64_C( 0x8000000000000000 )) ) {
            if ( ! uiB0 ) {
                uiB64 &= 0x8000;
            } else {
                normExpSig = softfloat_normSubnormalExtF80Sig( uiB0 );
                if ( expB + normExpSig.exp >= 1 ) {
                    uiB64 = (uiB64 & 0x8000) | (expB + normExpSig.exp);
                    uiB0 = normExpSig.sig;
                } else {
                    if ( expB > 1 ) uiB0 <<= (expB - 1);
                    uiB64 &= 0x8000;
                }
            }
        }
    } else {
        if ( uiB0 & UINT64_C( 0x8000000000000000 ) )
            uiB64 = (uiB64 & 0x8000) | 1;
    }
    return
           (uiA0 == uiB0)
        && ((uiA64 == uiB64) || (! uiA0 && ! ((uiA64 | uiB64) & 0x7FFF)));

}

