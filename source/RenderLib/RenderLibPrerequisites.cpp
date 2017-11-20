#include "RenderLibPrerequisites.h"

#include "FontLoader.h"
#include "Texture.h"
#include "Font.h"

#include <Renderer/StringUtils.hpp>
#include <Renderer/Converter.hpp>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace render
{
	namespace
	{
		char getChar( std::string const & text )
		{
			return char( atoi( text.c_str() ) );
		}

		uint32_t getUInt( std::string const & text )
		{
			return uint32_t( atoi( text.c_str() ) );
		}

		renderer::IVec2 getIVec2( std::string const & text )
		{
			auto infos = renderer::split( text, " ", 3, false );
			renderer::IVec2 ret;

			if ( infos.size() == 2 )
			{
				ret.x = atoi( infos[0].c_str() );
				ret.y = atoi( infos[1].c_str() );
			}

			return ret;
		}
	}

	void loadTexture( ByteArray const & fileContent
		, Texture & texture )
	{
		int x = 0;
		int y = 0;
		int n = 0;
		uint8_t * data = stbi_load_from_memory( fileContent.data()
			, int( fileContent.size() )
			, &x
			, &y
			, &n
			, 0 );

		if ( data )
		{
			renderer::PixelFormat format;

			switch ( n )
			{
			case 1:
				format = renderer::PixelFormat::eL8;
				break;

			case 2:
				format = renderer::PixelFormat::eL8A8;
				break;

			case 3:
				format = renderer::PixelFormat::eR8G8B8;
				break;

			case 4:
				format = renderer::PixelFormat::eR8G8B8A8;
				break;

			default:
				assert( "Unsupported component count" );
				format = renderer::PixelFormat::eR8G8B8A8;
				break;
			}

			texture.image( format
				, renderer::IVec2{ x, y }
				, ByteArray{ data, data + n * x * y } );
			stbi_image_free( data );
		}
	}

	void loadTexture( ByteArray const & fileContent
		, renderer::PixelFormat format
		, Texture & texture )
	{
		int x = 0;
		int y = 0;
		int n = 0;
		uint8_t * data = stbi_load_from_memory( fileContent.data()
			, int( fileContent.size() )
			, &x
			, &y
			, &n
			, 0 );
		size_t bufferSize = x * y * renderer::pixelSize( format );
		ByteArray buffer( bufferSize );

		if ( data )
		{
			switch ( n )
			{
			case 1:
				renderer::convertBuffer< renderer::PixelFormat::eL8 >( data
					, size_t( n * x * y )
					, buffer.data()
					, format
					, bufferSize );
				break;

			case 2:
				renderer::convertBuffer< renderer::PixelFormat::eL8A8 >( data
					, size_t( n * x * y )
					, buffer.data()
					, format
					, bufferSize );
				break;

			case 3:
				renderer::convertBuffer< renderer::PixelFormat::eR8G8B8 >( data
					, size_t( n * x * y )
					, buffer.data()
					, format
					, bufferSize );
				break;

			case 4:
				renderer::convertBuffer< renderer::PixelFormat::eR8G8B8A8 >( data
					, size_t( n * x * y )
					, buffer.data()
					, format
					, bufferSize );
				break;
			}

			texture.image( format
				, renderer::IVec2{ x, y }
			, buffer );
			stbi_image_free( data );
		}
	}

	void loadFont( std::string const & content
		, Font & font )
	{
		auto lines = renderer::split( content, "\n", 0xFFFFFFFF, false );

		for ( auto & line : lines )
		{
			std::clog << line << std::endl;
			auto infos = renderer::split( line, ",", 5, false );

			if ( infos.size() == 4 )
			{
				font.addGlyph( {
					getChar( infos[0] ),
					getIVec2( infos[2] ),
					getIVec2( infos[1] ),
					getUInt( infos[3] ),
					ByteArray{}
				} );
			}
		}
	}

	void loadFont( FontLoader & loader
		, Font & font )
	{
		loader.initialise( font.height() );

		for ( int c = 0; c <= 0xFF; ++c )
		{
			font.addGlyph( loader.loadGlyph( char( c ) ) );
		}
	}

}
