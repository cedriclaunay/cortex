//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECore/ObjectPool.h"

#include "IECorePython/ObjectPoolBinding.h"
#include "IECorePython/RefCountedBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

ObjectPtr store( ObjectPool &pool, Object* obj )
{
	return const_cast< Object * >( pool.storeReference(obj).get() );
}

ObjectPtr retrieve( const ObjectPool &pool, MurmurHash key )
{
	ConstObjectPtr o = pool.retrieve(key);
	if ( o )
	{
		return o->copy();
	}
	return 0;
}

void bindObjectPool()
{
	RefCountedClass<ObjectPool, RefCounted>( "ObjectPool" )
		.def( init<>() )
		.def( "erase", &ObjectPool::erase )
		.def( "clear", &ObjectPool::clear )
		.def( "retrieve", &retrieve )
		.def( "store",  &store )
		.def( "contains", &ObjectPool::contains )
		.def( "memoryUsage", &ObjectPool::memoryUsage )
		.add_property( "maxMemoryUsage", &ObjectPool::getMaxMemoryUsage, &ObjectPool::setMaxMemoryUsage )
		.def( "defaultObjectPool", &ObjectPool::defaultObjectPool ).staticmethod( "defaultObjectPool" )
	;
}

}
