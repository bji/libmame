/***************************************************************************

    delegate.c

    Templates and classes to enable delegates for callbacks.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emucore.h"
#include "delegate.h"


//**************************************************************************
//  BINDABLE OBJECT
//**************************************************************************

//-------------------------------------------------
//  bindable_object - constructor
//-------------------------------------------------

bindable_object::bindable_object()
{
}


//-------------------------------------------------
//  ~bindable_object - destructor
//-------------------------------------------------

bindable_object::~bindable_object()
{
}



//**************************************************************************
//  INTERNAL DELEGATE HELPERS
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_INTERNAL)

// NULL structure used in dummy constructor
delegate_gcc_mfp_internal delegate_gcc_mfp_null;


//-------------------------------------------------
//  delegate_convert_raw - given an object and
//  an raw function, adjust the object base and
//  return the actual final code pointer
//-------------------------------------------------

delegate_generic_function delegate_convert_raw(delegate_generic_class *&object, delegate_gcc_mfp_internal &mfp)
{
	// apply the "this" delta to the object first
	object = reinterpret_cast<delegate_generic_class *>(reinterpret_cast<UINT8 *>(object) + mfp.this_delta);

	// if the low bit of the vtable index is clear, then it is just a raw function pointer
	if (!(mfp.u.vtable_index & 1))
		return mfp.u.funcptr;

	// otherwise, it is the byte index into the vtable where the actual function lives
	UINT8 *vtable_base = *reinterpret_cast<UINT8 **>(object);
	return *reinterpret_cast<delegate_generic_function *>(vtable_base + mfp.u.vtable_index - 1);
}

#endif
