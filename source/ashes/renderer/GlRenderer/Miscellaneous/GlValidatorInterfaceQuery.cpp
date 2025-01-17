/*
This file belongs to Ashes.
See LICENSE file in root folder
*/
#include "Miscellaneous/GlValidatorInterfaceQuery.hpp"

#include "Core/GlDevice.hpp"
#include "Miscellaneous/GlValidator.hpp"
#include "Pipeline/GlPipelineLayout.hpp"
#include "RenderPass/GlRenderPass.hpp"

#include "ashesgl_api.hpp"

#include <algorithm>
#include <sstream>

#if defined( interface )
#	undef interface
#endif

namespace ashes::gl::gl4
{
	namespace
	{
		std::string const ValidationError = "VALIDATION ERROR: ";
		std::string const ValidationWarning = "VALIDATION WARNING: ";

		enum GlslInterface
			: GLenum
		{
			GLSL_INTERFACE_ATOMIC_COUNTER_BUFFER = 0x92C0,
			GLSL_INTERFACE_UNIFORM = 0x92E1,
			GLSL_INTERFACE_UNIFORM_BLOCK = 0x92E2,
			GLSL_INTERFACE_PROGRAM_INPUT = 0x92E3,
			GLSL_INTERFACE_PROGRAM_OUTPUT = 0x92E4,
			GLSL_INTERFACE_BUFFER_VARIABLE = 0x92E5,
			GLSL_INTERFACE_SHADER_STORAGE_BLOCK = 0x92E6,
			GLSL_INTERFACE_VERTEX_SUBROUTINE = 0x92E8,
			GLSL_INTERFACE_TESS_CONTROL_SUBROUTINE = 0x92E9,
			GLSL_INTERFACE_TESS_EVALUATION_SUBROUTINE = 0x92EA,
			GLSL_INTERFACE_GEOMETRY_SUBROUTINE = 0x92EB,
			GLSL_INTERFACE_FRAGMENT_SUBROUTINE = 0x92EC,
			GLSL_INTERFACE_COMPUTE_SUBROUTINE = 0x92ED,
			GLSL_INTERFACE_VERTEX_SUBROUTINE_UNIFORM = 0x92EE,
			GLSL_INTERFACE_TESS_CONTROL_SUBROUTINE_UNIFORM = 0x92EF,
			GLSL_INTERFACE_TESS_EVALUATION_SUBROUTINE_UNIFORM = 0x92F0,
			GLSL_INTERFACE_GEOMETRY_SUBROUTINE_UNIFORM = 0x92F1,
			GLSL_INTERFACE_FRAGMENT_SUBROUTINE_UNIFORM = 0x92F2,
			GLSL_INTERFACE_COMPUTE_SUBROUTINE_UNIFORM = 0x92F3,
		};

		enum GlslDataName
			: GLenum
		{
			GLSL_DATANAME_ACTIVE_RESOURCES = 0x92F5,
			GLSL_DATANAME_MAX_NAME_LENGTH = 0x92F6,
			GLSL_DATANAME_MAX_NUM_ACTIVE_VARIABLES = 0x92F7,
			GLSL_DATANAME_MAX_NUM_COMPATIBLE_SUBROUTINES = 0x92F8,
		};

		enum GlslProperty
			: GLenum
		{
			GLSL_PROPERTY_NUM_COMPATIBLE_SUBROUTINES = 0x8E4A,
			GLSL_PROPERTY_COMPATIBLE_SUBROUTINES = 0x8E4B,
			GLSL_PROPERTY_IS_PER_PATCH = 0x92E7,
			GLSL_PROPERTY_NAME_LENGTH = 0x92F9,
			GLSL_PROPERTY_TYPE = 0x92FA,
			GLSL_PROPERTY_ARRAY_SIZE = 0x92FB,
			GLSL_PROPERTY_OFFSET = 0x92FC,
			GLSL_PROPERTY_BLOCK_INDEX = 0x92FD,
			GLSL_PROPERTY_ARRAY_STRIDE = 0x92FE,
			GLSL_PROPERTY_MATRIX_STRIDE = 0x92FF,
			GLSL_PROPERTY_IS_ROW_MAJOR = 0x9300,
			GLSL_PROPERTY_ATOMIC_COUNTER_BUFFER_INDEX = 0x9301,
			GLSL_PROPERTY_BUFFER_BINDING = 0x9302,
			GLSL_PROPERTY_BUFFER_DATA_SIZE = 0x9303,
			GLSL_PROPERTY_NUM_ACTIVE_VARIABLES = 0x9304,
			GLSL_PROPERTY_ACTIVE_VARIABLES = 0x9305,
			GLSL_PROPERTY_REFERENCED_BY_VERTEX_SHADER = 0x9306,
			GLSL_PROPERTY_REFERENCED_BY_TESS_CONTROL_SHADER = 0x9307,
			GLSL_PROPERTY_REFERENCED_BY_TESS_EVALUATION_SHADER = 0x9308,
			GLSL_PROPERTY_REFERENCED_BY_GEOMETRY_SHADER = 0x9309,
			GLSL_PROPERTY_REFERENCED_BY_FRAGMENT_SHADER = 0x930A,
			GLSL_PROPERTY_REFERENCED_BY_COMPUTE_SHADER = 0x930B,
			GLSL_PROPERTY_TOP_LEVEL_ARRAY_SIZE = 0x930C,
			GLSL_PROPERTY_TOP_LEVEL_ARRAY_STRIDE = 0x930D,
			GLSL_PROPERTY_LOCATION = 0x930E,
			GLSL_PROPERTY_LOCATION_INDEX = 0x930F,
			GLSL_PROPERTY_LOCATION_COMPONENT = 0x934A,
		};

		enum class GlslObject
			: GLenum
		{
			eActiveAttributes,
			eActiveUniforms,
		};

		bool areCompatible( VkFormat lhs, VkFormat rhs )
		{
			if ( lhs == rhs )
			{
				return true;
			}

			switch ( lhs )
			{
			case VK_FORMAT_R16_SFLOAT:
			case VK_FORMAT_R32_SFLOAT:
			case VK_FORMAT_R8_UNORM:
			case VK_FORMAT_R8_SINT:
			case VK_FORMAT_R8_SRGB:
			case VK_FORMAT_R8_SSCALED:
				return rhs == VK_FORMAT_R32_SFLOAT
					|| rhs == VK_FORMAT_R32G32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R8_UNORM
					|| rhs == VK_FORMAT_R8_SINT
					|| rhs == VK_FORMAT_R8_SRGB
					|| rhs == VK_FORMAT_R8_SSCALED;
			case VK_FORMAT_R16G16_SFLOAT:
			case VK_FORMAT_R32G32_SFLOAT:
			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SINT:
			case VK_FORMAT_R8G8_SRGB:
			case VK_FORMAT_R8G8_SSCALED:
				return rhs == VK_FORMAT_R32G32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R8G8_UNORM
					|| rhs == VK_FORMAT_R8G8_SINT
					|| rhs == VK_FORMAT_R8G8_SRGB
					|| rhs == VK_FORMAT_R8G8_SSCALED;
			case VK_FORMAT_R16G16B16_SFLOAT:
			case VK_FORMAT_R32G32B32_SFLOAT:
			case VK_FORMAT_R8G8B8_UNORM:
			case VK_FORMAT_R8G8B8_SINT:
			case VK_FORMAT_R8G8B8_SRGB:
			case VK_FORMAT_R8G8B8_SSCALED:
				return rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R8G8B8_UNORM
					|| rhs == VK_FORMAT_R8G8B8_SINT
					|| rhs == VK_FORMAT_R8G8B8_SRGB
					|| rhs == VK_FORMAT_R8G8B8_SSCALED;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R32G32B32A32_SFLOAT:
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SINT:
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_R8G8B8A8_SSCALED:
				return rhs == VK_FORMAT_R16G16B16A16_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R8G8B8A8_UNORM
					|| rhs == VK_FORMAT_B8G8R8A8_UNORM
					|| rhs == VK_FORMAT_R16G16B16_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R8G8B8_UNORM
					|| rhs == VK_FORMAT_B8G8R8_UNORM
					|| rhs == VK_FORMAT_R8G8B8A8_SINT
					|| rhs == VK_FORMAT_R8G8B8A8_SRGB
					|| rhs == VK_FORMAT_R8G8B8A8_SSCALED;
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return rhs == VK_FORMAT_R32_SFLOAT;
			default:
				assert( false );
				return false;
			}
		}

		VkFormat convertFormat( GlslAttributeType type )
		{
			switch ( type )
			{
			case GLSL_ATTRIBUTE_FLOAT:
				return VK_FORMAT_R32_SFLOAT;
			case GLSL_ATTRIBUTE_FLOAT_VEC2:
				return VK_FORMAT_R32G32_SFLOAT;
			case GLSL_ATTRIBUTE_FLOAT_VEC3:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case GLSL_ATTRIBUTE_FLOAT_VEC4:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			default:
				assert( false );
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		}

		bool areCompatibleInputs( VkFormat lhs, VkFormat rhs )
		{
			if ( lhs == rhs )
			{
				return true;
			}

			switch ( lhs )
			{
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SINT
					|| rhs == VK_FORMAT_R32G32B32A32_UINT
					|| rhs == VK_FORMAT_R8G8B8A8_UNORM;
			case VK_FORMAT_R32G32B32A32_SINT:
				return rhs == VK_FORMAT_R32G32B32_SINT
					|| rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SINT
					|| rhs == VK_FORMAT_R32G32B32A32_UINT
					|| rhs == VK_FORMAT_R8G8B8A8_UNORM;
			case VK_FORMAT_R32G32B32A32_UINT:
				return rhs == VK_FORMAT_R32G32B32_UINT
					|| rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SINT
					|| rhs == VK_FORMAT_R32G32B32A32_UINT
					|| rhs == VK_FORMAT_R8G8B8A8_UNORM;
			case VK_FORMAT_R8G8B8A8_UNORM:
				return rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32A32_SINT
					|| rhs == VK_FORMAT_R32G32B32A32_UINT
					|| rhs == VK_FORMAT_R8G8B8A8_UNORM;
			case VK_FORMAT_R32G32B32_SFLOAT:
				return rhs == VK_FORMAT_R32G32B32A32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32_SINT
					|| rhs == VK_FORMAT_R32G32B32_UINT;
			case VK_FORMAT_R32G32B32_SINT:
				return rhs == VK_FORMAT_R32G32B32A32_SINT
					|| rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32_SINT
					|| rhs == VK_FORMAT_R32G32B32_UINT;
			case VK_FORMAT_R32G32B32_UINT:
				return rhs == VK_FORMAT_R32G32B32A32_UINT
					|| rhs == VK_FORMAT_R32G32B32_SFLOAT
					|| rhs == VK_FORMAT_R32G32B32_SINT
					|| rhs == VK_FORMAT_R32G32B32_UINT;
			case VK_FORMAT_R32G32_SFLOAT:
			case VK_FORMAT_R32G32_SINT:
			case VK_FORMAT_R32G32_UINT:
				return rhs == VK_FORMAT_R32G32_SFLOAT
					|| rhs == VK_FORMAT_R32G32_SINT
					|| rhs == VK_FORMAT_R32G32_UINT;
			case VK_FORMAT_R32_SINT:
			case VK_FORMAT_R32_UINT:
			case VK_FORMAT_R32_SFLOAT:
				return rhs == VK_FORMAT_R32_SINT
					|| rhs == VK_FORMAT_R32_UINT
					|| rhs == VK_FORMAT_R32_SFLOAT;
			default:
				assert( false );
				return false;
			}
		}

		template< typename FuncType >
		void getProgramInterfaceInfos( ContextLock const & context
			, uint32_t program
			, GlslInterface interface
			, std::vector< GlslProperty > const & properties
			, FuncType function )
		{
			int count{};
			glLogCall( context
				, glGetProgramInterfaceiv
				, program
				, interface
				, GLSL_DATANAME_MAX_NAME_LENGTH
				, &count );
			std::vector< char > buffer;
			buffer.resize( size_t( count ) );
			glLogCall( context
				, glGetProgramInterfaceiv
				, program
				, interface
				, GLSL_DATANAME_ACTIVE_RESOURCES
				, &count );
			std::vector< GLint > values;
			values.resize( properties.size() );
			std::vector< GLenum > props;

			for ( auto & prop : properties )
			{
				props.push_back( prop );
			}

			for ( GLuint i = 0; i < GLuint( count ); ++i )
			{
				GLsizei length{};
				glLogCall( context
					, glGetProgramResourceName
					, program
					, interface
					, i
					, GLsizei( buffer.size() )
					, &length
					, buffer.data() );
				std::string name( buffer.data(), size_t( length ) );
				glLogCall( context
					, glGetProgramResourceiv
					, program
					, interface
					, i
					, GLsizei( props.size() )
					, props.data()
					, GLsizei( values.size() )
					, nullptr
					, values.data() );
				function( name, values );
			}
		}

		using BufferFunction = std::function< void( std::string, uint32_t, uint32_t, uint32_t, uint32_t ) >;
		using BufferVariableFunction = std::function< void( std::string, GlslAttributeType, VkDeviceSize, uint32_t ) >;
		void getProgramBufferInfos( ContextLock const & context
			, uint32_t program
			, GlslInterface bufferInterface
			, GlslInterface variableInterface
			, BufferFunction bufferFunction
			, BufferVariableFunction variableFunction = nullptr )
		{
			GLint maxNameLength = 0;
			glLogCall( context
				, glGetProgramInterfaceiv
				, program
				, bufferInterface
				, GLSL_DATANAME_MAX_NAME_LENGTH
				, &maxNameLength );
			std::vector< char > buffer( size_t( std::abs( maxNameLength ) ) );
			GLint numBlocks;
			glLogCall( context
				, glGetProgramInterfaceiv
				, program
				, bufferInterface
				, GLSL_DATANAME_ACTIVE_RESOURCES
				, &numBlocks );
			uint32_t const blockPropertyCount = 2u;
			GLenum const blockProperties[blockPropertyCount] = { GLSL_PROPERTY_BUFFER_BINDING, GLSL_PROPERTY_BUFFER_DATA_SIZE };
			GLenum const activeUniformsCount[1] = { GLSL_PROPERTY_NUM_ACTIVE_VARIABLES };
			GLenum const activeUniforms[1] = { GLSL_PROPERTY_ACTIVE_VARIABLES };
			uint32_t const uniformPropertyCount = 4u;
			GLenum const uniformProperties[uniformPropertyCount] = { GLSL_PROPERTY_NAME_LENGTH, GLSL_PROPERTY_TYPE, GLSL_PROPERTY_OFFSET, GLSL_PROPERTY_ARRAY_SIZE };

			for ( GLuint blockIx = 0; blockIx < GLuint( numBlocks ); ++blockIx )
			{
				GLsizei nameLength = 0;
				glLogCall( context
					, glGetProgramResourceName
					, program
					, bufferInterface
					, blockIx
					, GLsizei( buffer.size() )
					, &nameLength
					, buffer.data() );
				std::string bufferName( buffer.data(), size_t( nameLength ) );
				GLint blockProps[2];
				glLogCall( context
					, glGetProgramResourceiv
					, program
					, bufferInterface
					, blockIx
					, blockPropertyCount
					, blockProperties
					, blockPropertyCount
					, nullptr
					, blockProps );
				GLuint index = glLogNonVoidCall( context
					, glGetProgramResourceIndex
					, program
					, bufferInterface
					, bufferName.c_str() );
				GLint numActiveUnifs = 0;
				glLogCall( context
					, glGetProgramResourceiv
					, program
					, bufferInterface
					, blockIx
					, 1
					, activeUniformsCount
					, 1
					, nullptr
					, &numActiveUnifs );
				bufferFunction( bufferName
					, uint32_t( blockProps[0] )
					, uint32_t( blockProps[1] )
					, index
					, uint32_t( numActiveUnifs ) );

				if ( numActiveUnifs && variableFunction )
				{
					std::vector< GLint > blockUnifs;
					blockUnifs.resize( size_t( numActiveUnifs ) );
					glLogCall( context
						, glGetProgramResourceiv
						, program
						, bufferInterface
						, blockIx
						, 1
						, activeUniforms
						, numActiveUnifs
						, nullptr
						, blockUnifs.data() );

					for ( GLuint unifIx = 0; unifIx < GLuint( numActiveUnifs) ; ++unifIx )
					{
						GLint values[uniformPropertyCount]{};
						glLogCall( context
							, glGetProgramResourceiv
							, program
							, variableInterface
							, GLuint( blockUnifs[unifIx] )
							, GLsizei( uniformPropertyCount )
							, uniformProperties
							, GLsizei( uniformPropertyCount )
							, nullptr
							, values );

						if ( values[0] > 0 )
						{
							std::vector< char > nameData;
							nameData.resize( size_t( values[0] ) );
							glLogCall( context
								, glGetProgramResourceName
								, program
								, variableInterface
								, GLuint( blockUnifs[unifIx] )
								, GLsizei( nameData.size() )
								, nullptr
								, nameData.data() );
							std::string variableName( nameData.begin(), nameData.end() - 1 );
							variableFunction( variableName
								, GlslAttributeType( values[1] )
								, VkDeviceSize( values[2] )
								, uint32_t( values[3] ) );
						}
					}
				}
			}
		}

		template< typename VarFuncType >
		void getVariableInfos( ContextLock const & context
			, uint32_t program
			, GlslInterface variableInterface
			, VarFuncType variableFunction )
		{
			static GLenum constexpr properties[]
			{
				GLSL_PROPERTY_BLOCK_INDEX,
				GLSL_PROPERTY_TYPE,
				GLSL_PROPERTY_NAME_LENGTH,
				GLSL_PROPERTY_LOCATION,
				GLSL_PROPERTY_ARRAY_SIZE,
				GLSL_PROPERTY_OFFSET
			};
			static uint32_t constexpr count = uint32_t( sizeof( properties ) / sizeof( *properties ) );
			GLint numUniforms = 0;
			glLogCall( context
				, glGetProgramInterfaceiv
				, program
				, variableInterface
				, GLSL_DATANAME_ACTIVE_RESOURCES
				, &numUniforms );

			for ( GLuint unif = 0; unif < GLuint( numUniforms ); ++unif )
			{
				GLint values[count];
				glLogCall( context
					, glGetProgramResourceiv
					, program
					, variableInterface
					, unif
					, count
					, properties
					, count
					, nullptr
					, values );

				// Skip any uniforms that are in a block.
				if ( values[0] == -1 )
				{
					std::vector< char > nameData;
					auto bufferSize = size_t( values[2] + 1u );
					nameData.resize( bufferSize, 0 );
					glLogCall( context
						, glGetProgramResourceName
						, program
						, variableInterface
						, unif
						, GLsizei( values[2] )
						, nullptr
						, &nameData[0] );
					std::string variableName = nameData.data();
					variableFunction( variableName
						, GlslAttributeType( values[1] )
						, values[3]
						, values[4]
						, values[5] );
				}
			}
		}

		template< typename FuncType >
		void getUnnamedProgramInterfaceInfos( ContextLock const & context
			, uint32_t program
			, GlslInterface interface
			, GlslProperty property
			, FuncType function )
		{
			uint32_t count{};
			glLogCall( context
				, glGetProgramInterfaceiv
				, program
				, interface
				, GLSL_DATANAME_ACTIVE_RESOURCES
				, reinterpret_cast< GLint * >( &count ) );
			std::vector< int > values( count );
			std::vector< int > lengths( count );

			for ( GLuint i = 0; i < count; ++i )
			{
				GLenum prop = property;
				glLogCall( context
					, glGetProgramResourceiv
					, program
					, interface
					, i
					, 1
					, &prop
					, 1
					, &lengths[i]
					, &values[i] );
			}

			if ( count )
			{
				function( values );
			}
		}
	}

	void validateInputs( ContextLock const & context
		, GLuint program
		, VkPipelineVertexInputStateCreateInfo const & vertexInputState )
	{
		struct AttrSpec
		{
			VkFormat format;
			uint32_t location;
		};
		std::vector< AttrSpec > attributes;

		for ( auto it = vertexInputState.pVertexAttributeDescriptions;
			it != vertexInputState.pVertexAttributeDescriptions + vertexInputState.vertexAttributeDescriptionCount;
			++it )
		{
			attributes.push_back( { it->format, it->location } );
		}

		auto findAttribute = [&attributes, &context]( std::string const & name
			, GlslAttributeType glslType
			, uint32_t location )
		{
			auto it = std::find_if( attributes.begin()
				, attributes.end()
				, [&glslType, &location]( AttrSpec const & lookup )
				{
					return areCompatibleInputs( lookup.format, getAttributeFormat( glslType ) )
						&& lookup.location == location;
				} );

			if ( it != attributes.end() )
			{
				attributes.erase( it );
			}
			else if ( name.find( "gl_" ) != 0u )
			{
				std::stringstream stream;
				stream << "Attribute [" << name
					<< "], of type: " << getName( glslType )
					<< ", at location: " << location
					<< " is used in the shader program, but is not listed in the vertex layouts" << std::endl;
				context->reportMessage( VK_DEBUG_REPORT_ERROR_BIT_EXT
					, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT
					, 0ull
					, 0u
					, VK_ERROR_VALIDATION_FAILED_EXT
					, "OpenGL"
					, stream.str().c_str() );
			}
		};

		getProgramInterfaceInfos( context
			, program
			, GLSL_INTERFACE_PROGRAM_INPUT
			, { GLSL_PROPERTY_TYPE, GLSL_PROPERTY_ARRAY_SIZE, GLSL_PROPERTY_LOCATION/*, GLSL_PROPERTY_LOCATION_COMPONENT*/ }
			, [&findAttribute]( std::string const & name, std::vector< GLint > const & values )
			{
				auto glslType = GlslAttributeType( values[0] );
				auto location = uint32_t( values[2] );
				uint32_t locOffset = 0u;

				switch ( glslType )
				{
				case GLSL_ATTRIBUTE_FLOAT_MAT4x2:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC2, location + locOffset++ );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT3x2:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC2, location + locOffset++ );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT2:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC2, location + locOffset++ );
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC2, location + locOffset++ );
					break;
				case GLSL_ATTRIBUTE_FLOAT_MAT4x3:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC3, location + locOffset++ );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT3:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC3, location + locOffset++ );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT2x3:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC3, location + locOffset++ );
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC3, location + locOffset++ );
					break;
				case GLSL_ATTRIBUTE_FLOAT_MAT4:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC4, location + locOffset++ );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT3x4:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC4, location + locOffset++ );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT2x4:
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC4, location + locOffset++ );
					findAttribute( name, GLSL_ATTRIBUTE_FLOAT_VEC4, location + locOffset++ );
					break;
				default:
					findAttribute( name, glslType, location );
					break;
				}
			} );

		for ( auto & attribute : attributes )
		{
			std::stringstream stream;
			stream << "Vertex layout has attribute of type " << ashes::getName( attribute.format )
				<< ", at location " << attribute.location
				<< ", which is not used by the program";
			context->reportMessage( VK_DEBUG_REPORT_WARNING_BIT_EXT
				, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT
				, 0ull
				, 0u
				, VK_ERROR_VALIDATION_FAILED_EXT
				, "OpenGL"
				, stream.str().c_str() );
		}
	}

	void validateOutputs( ContextLock const & context
		, GLuint program
		, VkRenderPass renderPass )
	{
		std::set< VkAttachmentDescription const * > attaches;

		for ( auto & reference : get( renderPass )->getFboAttachable() )
		{
			attaches.insert( &get( renderPass )->getAttachment( reference ) );
		}

		struct GlslOutput
		{
			std::string name;
			GlslAttributeType type;
			uint32_t location;
		};
		std::vector< GlslOutput > outputs;

		getProgramInterfaceInfos( context
			, program
			, GLSL_INTERFACE_PROGRAM_OUTPUT
			, { GLSL_PROPERTY_TYPE, GLSL_PROPERTY_ARRAY_SIZE, GLSL_PROPERTY_LOCATION/*, GLSL_PROPERTY_LOCATION_COMPONENT*/ }
			, [&outputs]( std::string name, std::vector< GLint > const & values )
			{
				outputs.push_back( { name, GlslAttributeType( values[0] ), uint32_t( values[2] ) } );
			} );

		for ( auto & output : outputs )
		{
			bool found = false;

			if ( output.location != ~( 0u ) )
			{
				if ( get( renderPass )->getColourAttaches().size() > output.location )
				{
					auto & attach = get( renderPass )->getColourAttaches()[output.location];

					if ( areCompatible( attach.attach.get().format, convertFormat( output.type ) ) )
					{
						found = true;
						attaches.erase( &attach.attach.get() );
					}
				}

				if ( !found )
				{
					std::stringstream stream;
					stream << "Attachment [" << output.name
						<< "], of type: " << getName( output.type )
						<< ", at location: " << output.location
						<< " is used in the shader program, but is not listed in the render pass attachments";
					context->reportMessage( VK_DEBUG_REPORT_ERROR_BIT_EXT
						, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT
						, 0ull
						, 0u
						, VK_ERROR_VALIDATION_FAILED_EXT
						, "OpenGL"
						, stream.str().c_str() );
				}
			}
			else
			{
				auto it = std::find_if( attaches.begin()
					, attaches.end()
					, [&output]( VkAttachmentDescription const * lookup )
					{
						return areCompatible( lookup->format, convertFormat( output.type ) );
					} );

				if ( it != attaches.end() )
				{
					attaches.erase( it );
				}
			}
		}

		for ( auto & attach : attaches )
		{
			if ( !isDepthOrStencilFormat( attach->format ) )
			{
				std::stringstream stream;
				stream << "Render pass has an attahment of type " << ashes::getName( attach->format )
					<< ", which is not used by the program";
				context->reportMessage( VK_DEBUG_REPORT_WARNING_BIT_EXT
					, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT
					, 0ull
					, 0u
					, VK_ERROR_VALIDATION_FAILED_EXT
					, "OpenGL"
					, stream.str().c_str() );
			}
		}
	}

	void validateUbos( ContextLock const & context
		, GLuint program )
	{
		getProgramBufferInfos( context
			, program
			, GLSL_INTERFACE_UNIFORM_BLOCK
			, GLSL_INTERFACE_UNIFORM
			, []( std::string name
				, uint32_t point
				, uint32_t dataSize
				, uint32_t index
				, uint32_t variables )
			{
				std::clog << "   Uniform block: " << name
					<< ", binding " << point
					<< ", size " << dataSize
					<< ", index " << index
					<< ", active variables " << variables;
			}
			, []( std::string name
				, GlslAttributeType type
				, VkDeviceSize offset
				, uint32_t arraySize )
			{
				std::clog << "      variable: " << name
					<< ", type " << getName( type )
					<< ", arraySize: " << arraySize
					<< ", offset " << offset;
			} );
	}

	void validateSsbos( ContextLock const & context
		, GLuint program )
	{
		getProgramBufferInfos( context
			, program
			, GLSL_INTERFACE_SHADER_STORAGE_BLOCK
			, GLSL_INTERFACE_BUFFER_VARIABLE
			, []( std::string name
				, uint32_t point
				, uint32_t dataSize
				, uint32_t index
				, uint32_t variables )
			{
				std::clog << "   ShaderStorage block: " << name
					<< ", binding " << point
					<< ", size " << dataSize
					<< ", index " << index
					<< ", active variables " << variables;
			}
			, []( std::string name
				, GlslAttributeType type
				, VkDeviceSize offset
				, uint32_t arraySize )
			{
				std::clog << "      variable: " << name
					<< ", type " << getName( type )
					<< ", arraySize: " << arraySize
					<< ", offset " << offset;
			} );
	}

	void validateUniforms( ContextLock const & context
		, GLuint program )
	{
		getVariableInfos( context
			, program
			, GLSL_INTERFACE_UNIFORM
			, []( std::string name
				, GlslAttributeType type
				, GLint location
				, GLint arraySize
				, GLint offset )
			{
				std::clog << "   Uniform variable: " << name
					<< ", type: " << getName( type )
					<< ", location: " << location
					<< ", arraySize: " << arraySize
					<< ", offset: " << offset;
			} );
	}

	InputsLayout getInputs( ContextLock const & context
		, VkShaderStageFlagBits stage
		, GLuint program )
	{
		InputsLayout result;
		getProgramInterfaceInfos( context
			, program
			, GLSL_INTERFACE_PROGRAM_INPUT
			, { GLSL_PROPERTY_TYPE, GLSL_PROPERTY_ARRAY_SIZE, GLSL_PROPERTY_LOCATION }
			, [&result]( std::string const & name, std::vector< GLint > const & values )
			{
				auto glslType = GlslAttributeType( values[0] );
				auto location = uint32_t( values[2] );
				auto offset = 0u;
				auto locOffset = 0u;

				switch ( glslType )
				{
				case GLSL_ATTRIBUTE_FLOAT_MAT4x2:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32_SFLOAT, offset } );
					offset += 2 * sizeof( float );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT3x2:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32_SFLOAT, offset } );
					offset += 2 * sizeof( float );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT2:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32_SFLOAT, offset } );
					offset += 2 * sizeof( float );
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32_SFLOAT, offset } );
					break;
				case GLSL_ATTRIBUTE_FLOAT_MAT4x3:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32B32_SFLOAT, offset } );
					offset += 3 * sizeof( float );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT3:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32B32_SFLOAT, offset } );
					offset += 3 * sizeof( float );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT2x3:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32B32_SFLOAT, offset } );
					offset += 3 * sizeof( float );
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32B32_SFLOAT, offset } );
					break;
				case GLSL_ATTRIBUTE_FLOAT_MAT4:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, offset } );
					offset += 4 * sizeof( float );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT3x4:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, offset } );
					offset += 4 * sizeof( float );
					[[fallthrough]];
				case GLSL_ATTRIBUTE_FLOAT_MAT2x4:
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, offset } );
					offset += 4 * sizeof( float );
					result.vertexAttributeDescriptions.push_back( { location + locOffset++, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, offset } );
					break;
				default:
					result.vertexAttributeDescriptions.push_back( { location + 0u, 0u, getAttributeFormat( glslType ), offset } );
					break;
				}
			} );
		return result;
	}

	ConstantsLayout & getPushConstants( ContextLock const & context
		, ConstantsLayout & constants
		, VkShaderStageFlagBits stage
		, GLuint program )
	{
		getVariableInfos( context
			, program
			, GLSL_INTERFACE_UNIFORM
			, [&constants]( std::string name
				, GlslAttributeType type
				, GLint location
				, GLint arraySize
				, GLint offset )
			{
				if ( !isSampler( type )
					&& !isImage( type )
					&& !isSamplerBuffer( type )
					&& !isImageBuffer( type )
					&& location != -1 )
				{
					auto it = std::find_if( constants.begin()
						, constants.end()
						, [&name]( FormatDescT< ConstantFormat > const & desc )
						{
							return name == desc.name;
						} );
					if ( it != constants.end() )
					{
						it->location = uint32_t( location );
					}
				}
			} );
		return constants;
	}

	InterfaceBlocksLayout getUniformBuffers( ContextLock const & context
		, VkShaderStageFlagBits stage
		, GLuint program )
	{
		InterfaceBlocksLayout result;
		getProgramBufferInfos( context
			, program
			, GLSL_INTERFACE_UNIFORM_BLOCK
			, GLSL_INTERFACE_UNIFORM
			, [&result]( std::string name
				, uint32_t point
				, uint32_t dataSize
				, uint32_t index
				, uint32_t variables )
			{
				result.push_back(
					{
						name,
						point,
						dataSize,
						{},
					} );
			}
			, [&result, &stage, &program]( std::string name
				, GlslAttributeType type
				, VkDeviceSize offset
				, uint32_t arraySize )
			{
				result.back().constants.push_back(
					{
						program,
						stage,
						name,
						0u,
						getConstantFormat( type ),
						getSize( type ),
						( arraySize ? arraySize : 1u ),
						uint32_t( offset ),
					} );
			} );
		return result;
	}

	InterfaceBlocksLayout getStorageBuffers( ContextLock const & context
		, VkShaderStageFlagBits stage
		, GLuint program )
	{
		InterfaceBlocksLayout result;
		getProgramBufferInfos( context
			, program
			, GLSL_INTERFACE_SHADER_STORAGE_BLOCK
			, GLSL_INTERFACE_BUFFER_VARIABLE
			, [&result]( std::string name
				, uint32_t point
				, uint32_t dataSize
				, uint32_t index
				, uint32_t variables )
			{
				result.push_back(
					{
						name,
						point,
						dataSize,
						{},
					} );
			} );
		return result;
	}

	SamplersLayout getSamplerBuffers( ContextLock const & context
		, VkShaderStageFlagBits stage
		, GLuint program )
	{
		SamplersLayout result;
		getVariableInfos( context
			, program
			, GLSL_INTERFACE_UNIFORM
			, [&result, &stage, &program]( std::string name
				, GlslAttributeType type
				, GLint location
				, GLint arraySize
				, GLint offset )
			{
				if ( isSamplerBuffer( type ) )
				{
					result.push_back( { program
						, stage
						, name
						, uint32_t( location )
						, getSamplerFormat( type )
						, 1u
						, uint32_t( arraySize )
						, 0u } );
				}
			} );
		return result;
	}

	SamplersLayout getSamplers( ContextLock const & context
		, VkShaderStageFlagBits stage
		, GLuint program )
	{
		SamplersLayout result;
		getVariableInfos( context
			, program
			, GLSL_INTERFACE_UNIFORM
			, [&result, &stage, &program]( std::string name
				, GlslAttributeType type
				, GLint location
				, GLint arraySize
				, GLint offset )
			{
				if ( isSampler( type ) )
				{
					result.push_back( { program
						, stage
						, name
						, uint32_t( location )
						, getSamplerFormat( type )
						, 1u
						, uint32_t( arraySize )
						, 0u } );
				}
			} );
		return result;
	}

	ImagesLayout getImageBuffers( ContextLock const & context
		, VkShaderStageFlagBits stage
		, GLuint program )
	{
		ImagesLayout result;
		getVariableInfos( context
			, program
			, GLSL_INTERFACE_UNIFORM
			, [&result, &stage, &program]( std::string name
				, GlslAttributeType type
				, GLint location
				, GLint arraySize
				, GLint offset )
			{
				if ( isImageBuffer( type ) )
				{
					result.push_back( { program
						, stage
						, name
						, uint32_t( location )
						, getImageFormat( type )
						, 1u
						, uint32_t( arraySize )
						, 0u } );
				}
			} );
		return result;
	}

	ImagesLayout getImages( ContextLock const & context
		, VkShaderStageFlagBits stage
		, GLuint program )
	{
		ImagesLayout result;
		getVariableInfos( context
			, program
			, GLSL_INTERFACE_UNIFORM
			, [&result, &stage, &program]( std::string name
				, GlslAttributeType type
				, GLint location
				, GLint arraySize
				, GLint offset )
			{
				if ( isImage( type ) )
				{
					result.push_back( { program
						, stage
						, name
						, uint32_t( location )
						, getImageFormat( type )
						, 1u
						, uint32_t( arraySize )
						, 0u } );
				}
			} );
		return result;
	}
}
