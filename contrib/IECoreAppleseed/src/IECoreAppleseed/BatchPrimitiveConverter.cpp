//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#include "boost/filesystem/convenience.hpp"

#include "renderer/api/entity.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAppleseed/private/BatchPrimitiveConverter.h"

#include "IECoreAppleseed/ToAppleseedConverter.h"

using namespace std;
using namespace boost;
using namespace IECore;

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::BatchPrimitiveConverter::BatchPrimitiveConverter( const filesystem::path &projectPath, const asf::SearchPaths &searchPaths ) : PrimitiveConverter( searchPaths )
{
	m_projectPath = projectPath;
	m_meshGeomExtension = ".binarymesh";
}

void IECoreAppleseed::BatchPrimitiveConverter::setOption( const string &name, IECore::ConstDataPtr value )
{
	if( name == "as:mesh_file_format" )
	{
		const string &str = static_cast<const StringData *>( value.get() )->readable();

		if( str == "binarymesh" )
		{
			m_meshGeomExtension = ".binarymesh";
		}
		else if( str == "obj" )
		{
			m_meshGeomExtension = ".obj";
		}
		else
		{
			msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::setOption", format( "Unknown mesh file format \"%s\"." ) % str );
		}
	}
	else
		PrimitiveConverter::setOption( name, value );
}

asf::auto_release_ptr<asr::Object> IECoreAppleseed::BatchPrimitiveConverter::doConvertPrimitive( PrimitivePtr primitive, const MurmurHash &primitiveHash )
{
	if( primitive->typeId() == MeshPrimitiveTypeId )
	{
		return convertAndWriteMeshPrimitive( primitive, primitiveHash );
	}

	return asf::auto_release_ptr<asr::Object>();
}

asf::auto_release_ptr<asr::Object> IECoreAppleseed::BatchPrimitiveConverter::convertAndWriteMeshPrimitive( PrimitivePtr primitive, const MurmurHash &meshHash )
{
	string objectName = meshHash.toString();

	// Check if we already have a mesh saved for this object.
	string fileName = string( "_geometry/" ) + objectName + m_meshGeomExtension;
	filesystem::path p = m_projectPath / fileName;

	if( !filesystem::exists( p ) )
	{
		ToAppleseedConverterPtr converter = ToAppleseedConverter::create( primitive.get() );

		if( !converter )
		{
			msg( Msg::Warning, "IECoreAppleseed::BatchPrimitiveConverter", "Couldn't convert primitive." );
			return asf::auto_release_ptr<asr::Object>();
		}

		asf::auto_release_ptr<asr::Object> entity;
		entity.reset( static_cast<asr::Object*>( converter->convert() ) );

		if( entity.get() == 0 )
		{
			msg( Msg::Warning, "IECoreAppleseed::BatchPrimitiveConverter", "Couldn't convert primitive." );
			return asf::auto_release_ptr<asr::Object>();
		}

		// Write the mesh to a file.
		p = m_projectPath / fileName;
		if( !asr::MeshObjectWriter::write( static_cast<const asr::MeshObject&>( *entity ), objectName.c_str(), p.string().c_str() ) )
		{
			msg( Msg::Warning, "IECoreAppleseed::BatchPrimitiveConverter", "Couldn't save mesh primitive." );
			return asf::auto_release_ptr<asr::Object>();
		}
	}

	asf::auto_release_ptr<asr::MeshObject> meshObj( asr::MeshObjectFactory().create( objectName.c_str(), asr::ParamArray().insert( "filename", fileName.c_str() ) ) );
	return asf::auto_release_ptr<asr::Object>( meshObj.release() );
}
