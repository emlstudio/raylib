/**********************************************************************************************
*
*   rlvk v0.1 - Vulkan backend for raylib, replacing rlgl
*
*   DESCRIPTION:
*       Provides the same API surface as rlgl.h but backed by Vulkan instead of OpenGL.
*       Drop-in replacement: raylib's upper layers (rshapes, rmodels, rtext, rtextures)
*       call through this API without modification.
*
*   CONFIGURATION:
*       #define GRAPHICS_API_VULKAN
*           Select this backend instead of any GRAPHICS_API_OPENGL_* variant.
*
*       #define RLVK_IMPLEMENTATION
*           Generates the implementation of the library into the included file.
*
*   LICENSE: zlib/libpng (same as raylib)
*
**********************************************************************************************/

#ifndef RLVK_H
#define RLVK_H

#define RLGL_VERSION  "6.0-vulkan"

#include "rlvk_shader.h"

// Function specifiers
#if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
    #define RLAPI __declspec(dllexport)
#elif defined(BUILD_LIBTYPE_SHARED)
    #define RLAPI __attribute__((visibility("default")))
#elif defined(_WIN32) && defined(USE_LIBTYPE_SHARED)
    #define RLAPI __declspec(dllimport)
#endif
#ifndef RLAPI
    #define RLAPI
#endif

// Support TRACELOG macros
#ifndef TRACELOG
    #define TRACELOG(level, ...) (void)0
#endif

// Allow custom memory allocators
#ifndef RL_MALLOC
    #define RL_MALLOC(sz)     malloc(sz)
#endif
#ifndef RL_CALLOC
    #define RL_CALLOC(n,sz)   calloc(n,sz)
#endif
#ifndef RL_REALLOC
    #define RL_REALLOC(n,sz)  realloc(n,sz)
#endif
#ifndef RL_FREE
    #define RL_FREE(p)        free(p)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef RL_DEFAULT_BATCH_BUFFER_ELEMENTS
    #define RL_DEFAULT_BATCH_BUFFER_ELEMENTS  8192
#endif
#ifndef RL_DEFAULT_BATCH_BUFFERS
    #define RL_DEFAULT_BATCH_BUFFERS                 2      // 2 for frames-in-flight
#endif
#ifndef RL_DEFAULT_BATCH_DRAWCALLS
    #define RL_DEFAULT_BATCH_DRAWCALLS             256
#endif
#ifndef RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS
    #define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS       4
#endif
#ifndef RL_MAX_MATRIX_STACK_SIZE
    #define RL_MAX_MATRIX_STACK_SIZE                32
#endif
#ifndef RL_MAX_SHADER_LOCATIONS
    #define RL_MAX_SHADER_LOCATIONS                 32
#endif
#ifndef RL_CULL_DISTANCE_NEAR
    #define RL_CULL_DISTANCE_NEAR                 0.05
#endif
#ifndef RL_CULL_DISTANCE_FAR
    #define RL_CULL_DISTANCE_FAR                4000.0
#endif

// Texture parameters (equivalent to OpenGL defines, kept for API compatibility)
#define RL_TEXTURE_WRAP_S                       0x2802
#define RL_TEXTURE_WRAP_T                       0x2803
#define RL_TEXTURE_MAG_FILTER                   0x2800
#define RL_TEXTURE_MIN_FILTER                   0x2801

#define RL_TEXTURE_FILTER_NEAREST               0x2600
#define RL_TEXTURE_FILTER_LINEAR                0x2601
#define RL_TEXTURE_FILTER_MIP_NEAREST           0x2700
#define RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR    0x2702
#define RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST    0x2701
#define RL_TEXTURE_FILTER_MIP_LINEAR            0x2703
#define RL_TEXTURE_FILTER_ANISOTROPIC           0x3000
#define RL_TEXTURE_MIPMAP_BIAS_RATIO            0x4000

#define RL_TEXTURE_WRAP_REPEAT                  0x2901
#define RL_TEXTURE_WRAP_CLAMP                   0x812F
#define RL_TEXTURE_WRAP_MIRROR_REPEAT           0x8370
#define RL_TEXTURE_WRAP_MIRROR_CLAMP            0x8742

// Matrix modes
#define RL_MODELVIEW                            0x1700
#define RL_PROJECTION                           0x1701
#define RL_TEXTURE                              0x1702

// Primitive assembly draw modes
#define RL_LINES                                0x0001
#define RL_TRIANGLES                            0x0004
#define RL_QUADS                                0x0007

// GL equivalent data types
#define RL_UNSIGNED_BYTE                        0x1401
#define RL_FLOAT                                0x1406

// GL buffer usage hint
#define RL_STREAM_DRAW                          0x88E0
#define RL_STREAM_READ                          0x88E1
#define RL_STREAM_COPY                          0x88E2
#define RL_STATIC_DRAW                          0x88E4
#define RL_STATIC_READ                          0x88E5
#define RL_STATIC_COPY                          0x88E6
#define RL_DYNAMIC_DRAW                         0x88E8
#define RL_DYNAMIC_READ                         0x88E9
#define RL_DYNAMIC_COPY                         0x88EA

// Shader type
#define RL_FRAGMENT_SHADER                      0x8B30
#define RL_VERTEX_SHADER                        0x8B31
#define RL_COMPUTE_SHADER                       0x91B9

// Blending factors
#define RL_ZERO                                 0
#define RL_ONE                                  1
#define RL_SRC_COLOR                            0x0300
#define RL_ONE_MINUS_SRC_COLOR                  0x0301
#define RL_SRC_ALPHA                            0x0302
#define RL_ONE_MINUS_SRC_ALPHA                  0x0303
#define RL_DST_ALPHA                            0x0304
#define RL_ONE_MINUS_DST_ALPHA                  0x0305
#define RL_DST_COLOR                            0x0306
#define RL_ONE_MINUS_DST_COLOR                  0x0307
#define RL_SRC_ALPHA_SATURATE                   0x0308
#define RL_CONSTANT_COLOR                       0x8001
#define RL_ONE_MINUS_CONSTANT_COLOR             0x8002
#define RL_CONSTANT_ALPHA                       0x8003
#define RL_ONE_MINUS_CONSTANT_ALPHA             0x8004

// Blending functions/equations
#define RL_FUNC_ADD                             0x8006
#define RL_MIN                                  0x8007
#define RL_MAX                                  0x8008
#define RL_FUNC_SUBTRACT                        0x800A
#define RL_FUNC_REVERSE_SUBTRACT                0x800B
#define RL_BLEND_EQUATION                       0x8009
#define RL_BLEND_EQUATION_RGB                   0x8009
#define RL_BLEND_EQUATION_ALPHA                 0x883D
#define RL_BLEND_DST_RGB                        0x80C8
#define RL_BLEND_SRC_RGB                        0x80C9
#define RL_BLEND_DST_ALPHA                      0x80CA
#define RL_BLEND_SRC_ALPHA                      0x80CB
#define RL_BLEND_COLOR                          0x8005

#define RL_READ_FRAMEBUFFER                     0x8CA8
#define RL_DRAW_FRAMEBUFFER                     0x8CA9

// Default shader vertex attribute locations
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION    0
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD    1
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL      2
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR       3
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT     4
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2   5
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES     6
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES 7
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS 8
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCETRANSFORM
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCETRANSFORM 9
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if (defined(__STDC__) && __STDC_VERSION__ >= 199901L) || (defined(_MSC_VER) && _MSC_VER >= 1800)
    #include <stdbool.h>
#elif !defined(__cplusplus) && !defined(bool) && !defined(RL_BOOL_TYPE)
typedef enum bool { false = 0, true = !false } bool;
#endif

#if !defined(RL_MATRIX_TYPE)
typedef struct Matrix {
    float m0, m4, m8, m12;
    float m1, m5, m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
} Matrix;
#define RL_MATRIX_TYPE
#endif

// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
typedef struct rlVertexBuffer {
    int elementCount;
    float *vertices;
    float *texcoords;
    float *normals;
    unsigned char *colors;
    unsigned int *indices;
    unsigned int vaoId;         // Unused in Vulkan, kept for API compat
    unsigned int vboId[5];      // Unused in Vulkan, kept for API compat
} rlVertexBuffer;

// Draw call type
typedef struct rlDrawCall {
    int mode;
    int vertexCount;
    int vertexAlignment;
    unsigned int textureId;
} rlDrawCall;

// rlRenderBatch type
typedef struct rlRenderBatch {
    int bufferCount;
    int currentBuffer;
    rlVertexBuffer *vertexBuffer;
    rlDrawCall *draws;
    int drawCounter;
    float currentDepth;
} rlRenderBatch;

// OpenGL version enum (kept for compatibility, Vulkan reports as RL_OPENGL_33)
typedef enum {
    RL_OPENGL_SOFTWARE = 0,
    RL_OPENGL_11,
    RL_OPENGL_21,
    RL_OPENGL_33,
    RL_OPENGL_43,
    RL_OPENGL_ES_20,
    RL_OPENGL_ES_30
} rlGlVersion;

// Trace log level
typedef enum {
    RL_LOG_ALL = 0, RL_LOG_TRACE, RL_LOG_DEBUG, RL_LOG_INFO,
    RL_LOG_WARNING, RL_LOG_ERROR, RL_LOG_FATAL, RL_LOG_NONE
} rlTraceLogLevel;

// Texture pixel formats
typedef enum {
    RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,
    RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
    RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,
    RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,
    RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    RL_PIXELFORMAT_UNCOMPRESSED_R32,
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,
    RL_PIXELFORMAT_UNCOMPRESSED_R16,
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16,
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGB,
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,
    RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,
    RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,
    RL_PIXELFORMAT_COMPRESSED_ETC1_RGB,
    RL_PIXELFORMAT_COMPRESSED_ETC2_RGB,
    RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGB,
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,
    RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,
    RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA
} rlPixelFormat;

// Texture filter modes
typedef enum {
    RL_TEXTURE_FILTER_POINT = 0,
    RL_TEXTURE_FILTER_BILINEAR,
    RL_TEXTURE_FILTER_TRILINEAR,
    RL_TEXTURE_FILTER_ANISOTROPIC_4X,
    RL_TEXTURE_FILTER_ANISOTROPIC_8X,
    RL_TEXTURE_FILTER_ANISOTROPIC_16X,
} rlTextureFilter;

// Color blending modes
typedef enum {
    RL_BLEND_ALPHA = 0,
    RL_BLEND_ADDITIVE,
    RL_BLEND_MULTIPLIED,
    RL_BLEND_ADD_COLORS,
    RL_BLEND_SUBTRACT_COLORS,
    RL_BLEND_ALPHA_PREMULTIPLY,
    RL_BLEND_CUSTOM,
    RL_BLEND_CUSTOM_SEPARATE
} rlBlendMode;

// Shader location point type
typedef enum {
    RL_SHADER_LOC_VERTEX_POSITION = 0,
    RL_SHADER_LOC_VERTEX_TEXCOORD01,
    RL_SHADER_LOC_VERTEX_TEXCOORD02,
    RL_SHADER_LOC_VERTEX_NORMAL,
    RL_SHADER_LOC_VERTEX_TANGENT,
    RL_SHADER_LOC_VERTEX_COLOR,
    RL_SHADER_LOC_MATRIX_MVP,
    RL_SHADER_LOC_MATRIX_VIEW,
    RL_SHADER_LOC_MATRIX_PROJECTION,
    RL_SHADER_LOC_MATRIX_MODEL,
    RL_SHADER_LOC_MATRIX_NORMAL,
    RL_SHADER_LOC_VECTOR_VIEW,
    RL_SHADER_LOC_COLOR_DIFFUSE,
    RL_SHADER_LOC_COLOR_SPECULAR,
    RL_SHADER_LOC_COLOR_AMBIENT,
    RL_SHADER_LOC_MAP_ALBEDO,
    RL_SHADER_LOC_MAP_METALNESS,
    RL_SHADER_LOC_MAP_NORMAL,
    RL_SHADER_LOC_MAP_ROUGHNESS,
    RL_SHADER_LOC_MAP_OCCLUSION,
    RL_SHADER_LOC_MAP_EMISSION,
    RL_SHADER_LOC_MAP_HEIGHT,
    RL_SHADER_LOC_MAP_CUBEMAP,
    RL_SHADER_LOC_MAP_IRRADIANCE,
    RL_SHADER_LOC_MAP_PREFILTER,
    RL_SHADER_LOC_MAP_BRDF,
    RL_SHADER_LOC_VERTEX_BONEIDS,
    RL_SHADER_LOC_VERTEX_BONEWEIGHTS,
    RL_SHADER_LOC_BONE_MATRICES,
    RL_SHADER_LOC_VERTEX_INSTANCETRANSFORM,
} rlShaderLocationIndex;
#define RL_SHADER_LOC_MAP_DIFFUSE       RL_SHADER_LOC_MAP_ALBEDO
#define RL_SHADER_LOC_MAP_SPECULAR      RL_SHADER_LOC_MAP_METALNESS

// Shader uniform data type
typedef enum {
    RL_SHADER_UNIFORM_FLOAT = 0,
    RL_SHADER_UNIFORM_VEC2,
    RL_SHADER_UNIFORM_VEC3,
    RL_SHADER_UNIFORM_VEC4,
    RL_SHADER_UNIFORM_INT,
    RL_SHADER_UNIFORM_IVEC2,
    RL_SHADER_UNIFORM_IVEC3,
    RL_SHADER_UNIFORM_IVEC4,
    RL_SHADER_UNIFORM_UINT,
    RL_SHADER_UNIFORM_UIVEC2,
    RL_SHADER_UNIFORM_UIVEC3,
    RL_SHADER_UNIFORM_UIVEC4,
    RL_SHADER_UNIFORM_SAMPLER2D
} rlShaderUniformDataType;

// Shader attribute data type
typedef enum {
    RL_SHADER_ATTRIB_FLOAT = 0,
    RL_SHADER_ATTRIB_VEC2,
    RL_SHADER_ATTRIB_VEC3,
    RL_SHADER_ATTRIB_VEC4
} rlShaderAttributeDataType;

// Framebuffer attachment type
typedef enum {
    RL_ATTACHMENT_COLOR_CHANNEL0 = 0,
    RL_ATTACHMENT_COLOR_CHANNEL1 = 1,
    RL_ATTACHMENT_COLOR_CHANNEL2 = 2,
    RL_ATTACHMENT_COLOR_CHANNEL3 = 3,
    RL_ATTACHMENT_COLOR_CHANNEL4 = 4,
    RL_ATTACHMENT_COLOR_CHANNEL5 = 5,
    RL_ATTACHMENT_COLOR_CHANNEL6 = 6,
    RL_ATTACHMENT_COLOR_CHANNEL7 = 7,
    RL_ATTACHMENT_DEPTH = 100,
    RL_ATTACHMENT_STENCIL = 200,
} rlFramebufferAttachType;

// Framebuffer texture attachment type
typedef enum {
    RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_X = 1,
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Y = 2,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y = 3,
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Z = 4,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z = 5,
    RL_ATTACHMENT_TEXTURE2D = 100,
    RL_ATTACHMENT_RENDERBUFFER = 200,
} rlFramebufferAttachTextureType;

// Face culling mode
typedef enum {
    RL_CULL_FACE_FRONT = 0,
    RL_CULL_FACE_BACK
} rlCullMode;

// Default shader vertex attribute names to set location points
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEINDICES  "vertexBoneIndices"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS  "vertexBoneWeights"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCETRANSFORM "instanceTransform"

// Default shader uniform names
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_BONEMATRICES "boneMatrices"

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Matrix operations
RLAPI void rlMatrixMode(int mode);
RLAPI void rlPushMatrix(void);
RLAPI void rlPopMatrix(void);
RLAPI void rlLoadIdentity(void);
RLAPI void rlTranslatef(float x, float y, float z);
RLAPI void rlRotatef(float angle, float x, float y, float z);
RLAPI void rlScalef(float x, float y, float z);
RLAPI void rlMultMatrixf(const float *matf);
RLAPI void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlViewport(int x, int y, int width, int height);
RLAPI void rlSetClipPlanes(double nearPlane, double farPlane);
RLAPI double rlGetCullDistanceNear(void);
RLAPI double rlGetCullDistanceFar(void);

// Vertex level operations
RLAPI void rlBegin(int mode);
RLAPI void rlEnd(void);
RLAPI void rlVertex2i(int x, int y);
RLAPI void rlVertex2f(float x, float y);
RLAPI void rlVertex3f(float x, float y, float z);
RLAPI void rlTexCoord2f(float x, float y);
RLAPI void rlNormal3f(float x, float y, float z);
RLAPI void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
RLAPI void rlColor3f(float x, float y, float z);
RLAPI void rlColor4f(float x, float y, float z, float w);

// Vertex buffer state
RLAPI bool rlEnableVertexArray(unsigned int vaoId);
RLAPI void rlDisableVertexArray(void);
RLAPI void rlEnableVertexBuffer(unsigned int id);
RLAPI void rlDisableVertexBuffer(void);
RLAPI void rlEnableVertexBufferElement(unsigned int id);
RLAPI void rlDisableVertexBufferElement(void);
RLAPI void rlEnableVertexAttribute(unsigned int index);
RLAPI void rlDisableVertexAttribute(unsigned int index);
RLAPI void rlEnableStatePointer(int vertexAttribType, void *buffer);
RLAPI void rlDisableStatePointer(int vertexAttribType);

// Texture state
RLAPI void rlActiveTextureSlot(int slot);
RLAPI void rlEnableTexture(unsigned int id);
RLAPI void rlDisableTexture(void);
RLAPI void rlEnableTextureCubemap(unsigned int id);
RLAPI void rlDisableTextureCubemap(void);
RLAPI void rlTextureParameters(unsigned int id, int param, int value);
RLAPI void rlCubemapParameters(unsigned int id, int param, int value);

// Shader state
RLAPI void rlEnableShader(unsigned int id);
RLAPI void rlDisableShader(void);

// Framebuffer state
RLAPI void rlEnableFramebuffer(unsigned int id);
RLAPI void rlDisableFramebuffer(void);
RLAPI unsigned int rlGetActiveFramebuffer(void);
RLAPI void rlActiveDrawBuffers(int count);
RLAPI void rlBlitFramebuffer(int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight, int bufferMask);
RLAPI void rlBindFramebuffer(unsigned int target, unsigned int framebuffer);

// General render state
RLAPI void rlEnableColorBlend(void);
RLAPI void rlDisableColorBlend(void);
RLAPI void rlEnableDepthTest(void);
RLAPI void rlDisableDepthTest(void);
RLAPI void rlEnableDepthMask(void);
RLAPI void rlDisableDepthMask(void);
RLAPI void rlEnableBackfaceCulling(void);
RLAPI void rlDisableBackfaceCulling(void);
RLAPI void rlColorMask(bool r, bool g, bool b, bool a);
RLAPI void rlSetCullFace(int mode);
RLAPI void rlEnableScissorTest(void);
RLAPI void rlDisableScissorTest(void);
RLAPI void rlScissor(int x, int y, int width, int height);
RLAPI void rlEnablePointMode(void);
RLAPI void rlDisablePointMode(void);
RLAPI void rlSetPointSize(float size);
RLAPI float rlGetPointSize(void);
RLAPI void rlEnableWireMode(void);
RLAPI void rlDisableWireMode(void);
RLAPI void rlSetLineWidth(float width);
RLAPI float rlGetLineWidth(void);
RLAPI void rlEnableSmoothLines(void);
RLAPI void rlDisableSmoothLines(void);
RLAPI void rlEnableStereoRender(void);
RLAPI void rlDisableStereoRender(void);
RLAPI bool rlIsStereoRenderEnabled(void);
RLAPI void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
RLAPI void rlClearScreenBuffers(void);
RLAPI void rlCheckErrors(void);
RLAPI void rlSetBlendMode(int mode);
RLAPI void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation);
RLAPI void rlSetBlendFactorsSeparate(int glSrcRGB, int glDstRGB, int glSrcAlpha, int glDstAlpha, int glEqRGB, int glEqAlpha);

// rlgl initialization
RLAPI void rlglInit(int width, int height);
RLAPI void rlglClose(void);
RLAPI void rlLoadExtensions(void *loader);
RLAPI void *rlGetProcAddress(const char *procName);
RLAPI int rlGetVersion(void);
RLAPI void rlSetFramebufferWidth(int width);
RLAPI int rlGetFramebufferWidth(void);
RLAPI void rlSetFramebufferHeight(int height);
RLAPI int rlGetFramebufferHeight(void);
RLAPI unsigned int rlGetTextureIdDefault(void);
RLAPI unsigned int rlGetShaderIdDefault(void);
RLAPI int *rlGetShaderLocsDefault(void);

// Render batch management
RLAPI rlRenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements);
RLAPI void rlUnloadRenderBatch(rlRenderBatch batch);
RLAPI void rlDrawRenderBatch(rlRenderBatch *batch);
RLAPI void rlSetRenderBatchActive(rlRenderBatch *batch);
RLAPI void rlDrawRenderBatchActive(void);
RLAPI bool rlCheckRenderBatchLimit(int vCount);
RLAPI void rlSetTexture(unsigned int id);

// Vertex buffer management
RLAPI unsigned int rlLoadVertexArray(void);
RLAPI unsigned int rlLoadVertexBuffer(const void *buffer, int size, bool dynamic);
RLAPI unsigned int rlLoadVertexBufferElement(const void *buffer, int size, bool dynamic);
RLAPI void rlUpdateVertexBuffer(unsigned int bufferId, const void *data, int dataSize, int offset);
RLAPI void rlUpdateVertexBufferElements(unsigned int id, const void *data, int dataSize, int offset);
RLAPI void rlUnloadVertexArray(unsigned int vaoId);
RLAPI void rlUnloadVertexBuffer(unsigned int vboId);
RLAPI void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, int offset);
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor);
RLAPI void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count);
RLAPI void rlDrawVertexArray(int offset, int count);
RLAPI void rlDrawVertexArrayElements(int offset, int count, const void *buffer);
RLAPI void rlDrawVertexArrayInstanced(int offset, int count, int instances);
RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, const void *buffer, int instances);

// Texture management
RLAPI unsigned int rlLoadTexture(const void *data, int width, int height, int format, int mipmapCount);
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer);
RLAPI unsigned int rlLoadTextureCubemap(const void *data, int size, int format, int mipmapCount);
RLAPI void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data);
RLAPI void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType);
RLAPI const char *rlGetPixelFormatName(unsigned int format);
RLAPI void rlUnloadTexture(unsigned int id);
RLAPI void rlGenTextureMipmaps(unsigned int id, int width, int height, int format, int *mipmaps);
RLAPI void *rlReadTexturePixels(unsigned int id, int width, int height, int format);
RLAPI unsigned char *rlReadScreenPixels(int width, int height);

// Framebuffer management
RLAPI unsigned int rlLoadFramebuffer(void);
RLAPI void rlFramebufferAttach(unsigned int id, unsigned int texId, int attachType, int texType, int mipLevel);
RLAPI bool rlFramebufferComplete(unsigned int id);
RLAPI void rlUnloadFramebuffer(unsigned int id);
RLAPI void rlCopyFramebuffer(int x, int y, int width, int height, int format, void *pixels);
RLAPI void rlResizeFramebuffer(int width, int height);

// Shader management
RLAPI unsigned int rlLoadShader(const char *code, int type);
RLAPI unsigned int rlLoadShaderProgram(const char *vsCode, const char *fsCode);
RLAPI unsigned int rlLoadShaderProgramEx(unsigned int vsId, unsigned int fsId);
RLAPI unsigned int rlLoadShaderProgramCompute(unsigned int csId);
RLAPI void rlUnloadShader(unsigned int id);
RLAPI void rlUnloadShaderProgram(unsigned int id);
RLAPI int rlGetLocationUniform(unsigned int id, const char *uniformName);
RLAPI int rlGetLocationAttrib(unsigned int id, const char *attribName);
RLAPI void rlSetUniform(int locIndex, const void *value, int uniformType, int count);
RLAPI void rlSetUniformMatrix(int locIndex, Matrix mat);
RLAPI void rlSetUniformMatrices(int locIndex, const Matrix *mat, int count);
RLAPI void rlSetUniformSampler(int locIndex, unsigned int textureId);
RLAPI void rlSetShader(unsigned int id, int *locs);

// Compute shader
RLAPI void rlComputeShaderDispatch(unsigned int groupX, unsigned int groupY, unsigned int groupZ);

// SSBO
RLAPI unsigned int rlLoadShaderBuffer(unsigned int size, const void *data, int usageHint);
RLAPI void rlUnloadShaderBuffer(unsigned int ssboId);
RLAPI void rlUpdateShaderBuffer(unsigned int id, const void *data, unsigned int dataSize, unsigned int offset);
RLAPI void rlBindShaderBuffer(unsigned int id, unsigned int index);
RLAPI void rlReadShaderBuffer(unsigned int id, void *dest, unsigned int count, unsigned int offset);
RLAPI void rlCopyShaderBuffer(unsigned int destId, unsigned int srcId, unsigned int destOffset, unsigned int srcOffset, unsigned int count);
RLAPI unsigned int rlGetShaderBufferSize(unsigned int id);

// Image texture
RLAPI void rlBindImageTexture(unsigned int id, unsigned int index, int format, bool readonly);

// Matrix state
RLAPI Matrix rlGetMatrixModelview(void);
RLAPI Matrix rlGetMatrixProjection(void);
RLAPI Matrix rlGetMatrixTransform(void);
RLAPI Matrix rlGetMatrixProjectionStereo(int eye);
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye);
RLAPI void rlSetMatrixProjection(Matrix proj);
RLAPI void rlSetMatrixModelview(Matrix view);
RLAPI void rlSetMatrixProjectionStereo(Matrix right, Matrix left);
RLAPI void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left);

// Quick helpers
RLAPI void rlLoadDrawCube(void);
RLAPI void rlLoadDrawQuad(void);

#endif // RLVK_H

//----------------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------------
#if defined(RLVK_IMPLEMENTATION)

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GLFW_INCLUDE_VULKAN
#include "external/glfw/include/GLFW/glfw3.h"

//----------------------------------------------------------------------------------
// Embedded SPIR-V shaders (default vertex + fragment)
//----------------------------------------------------------------------------------
// Vertex: layout(location=0) in vec3 pos; (1) vec2 texcoord; (2) vec3 normal; (3) vec4 color
//   push_constant: mat4 mvp (0-63), vec4 colDiffuse (64-79)
//   out: fragTexCoord, fragColor
// Fragment: samples texture0, output = texel * colDiffuse * fragColor

static const uint32_t rlvk_default_vert_spv[] = {
    0x07230203, 0x00010000, 0x000d000b, 0x0000002e, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000c000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x0000000f,
    0x00000011, 0x00000018, 0x00000024, 0x0000002d, 0x00030003, 0x00000002,
    0x000001c2, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70,
    0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004,
    0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572,
    0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005,
    0x00000009, 0x67617266, 0x43786554, 0x64726f6f, 0x00000000, 0x00060005,
    0x0000000b, 0x74726576, 0x65547865, 0x6f6f4378, 0x00006472, 0x00050005,
    0x0000000f, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00050005, 0x00000011,
    0x74726576, 0x6f437865, 0x00726f6c, 0x00060005, 0x00000016, 0x505f6c67,
    0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x00000016, 0x00000000,
    0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x00000016, 0x00000001,
    0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x00000016,
    0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006,
    0x00000016, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e,
    0x00030005, 0x00000018, 0x00000000, 0x00060005, 0x0000001c, 0x68737550,
    0x736e6f43, 0x746e6174, 0x00000073, 0x00040006, 0x0000001c, 0x00000000,
    0x0070766d, 0x00060006, 0x0000001c, 0x00000001, 0x446c6f63, 0x75666669,
    0x00006573, 0x00030005, 0x0000001e, 0x00006370, 0x00060005, 0x00000024,
    0x74726576, 0x6f507865, 0x69746973, 0x00006e6f, 0x00060005, 0x0000002d,
    0x74726576, 0x6f4e7865, 0x6c616d72, 0x00000000, 0x00040047, 0x00000009,
    0x0000001e, 0x00000000, 0x00040047, 0x0000000b, 0x0000001e, 0x00000001,
    0x00040047, 0x0000000f, 0x0000001e, 0x00000001, 0x00040047, 0x00000011,
    0x0000001e, 0x00000003, 0x00030047, 0x00000016, 0x00000002, 0x00050048,
    0x00000016, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x00000016,
    0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x00000016, 0x00000002,
    0x0000000b, 0x00000003, 0x00050048, 0x00000016, 0x00000003, 0x0000000b,
    0x00000004, 0x00030047, 0x0000001c, 0x00000002, 0x00040048, 0x0000001c,
    0x00000000, 0x00000005, 0x00050048, 0x0000001c, 0x00000000, 0x00000007,
    0x00000010, 0x00050048, 0x0000001c, 0x00000000, 0x00000023, 0x00000000,
    0x00050048, 0x0000001c, 0x00000001, 0x00000023, 0x00000040, 0x00040047,
    0x00000024, 0x0000001e, 0x00000000, 0x00040047, 0x0000002d, 0x0000001e,
    0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000002, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b,
    0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a, 0x00000001,
    0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001, 0x00040017,
    0x0000000d, 0x00000006, 0x00000004, 0x00040020, 0x0000000e, 0x00000003,
    0x0000000d, 0x0004003b, 0x0000000e, 0x0000000f, 0x00000003, 0x00040020,
    0x00000010, 0x00000001, 0x0000000d, 0x0004003b, 0x00000010, 0x00000011,
    0x00000001, 0x00040015, 0x00000013, 0x00000020, 0x00000000, 0x0004002b,
    0x00000013, 0x00000014, 0x00000001, 0x0004001c, 0x00000015, 0x00000006,
    0x00000014, 0x0006001e, 0x00000016, 0x0000000d, 0x00000006, 0x00000015,
    0x00000015, 0x00040020, 0x00000017, 0x00000003, 0x00000016, 0x0004003b,
    0x00000017, 0x00000018, 0x00000003, 0x00040015, 0x00000019, 0x00000020,
    0x00000001, 0x0004002b, 0x00000019, 0x0000001a, 0x00000000, 0x00040018,
    0x0000001b, 0x0000000d, 0x00000004, 0x0004001e, 0x0000001c, 0x0000001b,
    0x0000000d, 0x00040020, 0x0000001d, 0x00000009, 0x0000001c, 0x0004003b,
    0x0000001d, 0x0000001e, 0x00000009, 0x00040020, 0x0000001f, 0x00000009,
    0x0000001b, 0x00040017, 0x00000022, 0x00000006, 0x00000003, 0x00040020,
    0x00000023, 0x00000001, 0x00000022, 0x0004003b, 0x00000023, 0x00000024,
    0x00000001, 0x0004002b, 0x00000006, 0x00000026, 0x3f800000, 0x0004003b,
    0x00000023, 0x0000002d, 0x00000001, 0x00050036, 0x00000002, 0x00000004,
    0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007,
    0x0000000c, 0x0000000b, 0x0003003e, 0x00000009, 0x0000000c, 0x0004003d,
    0x0000000d, 0x00000012, 0x00000011, 0x0003003e, 0x0000000f, 0x00000012,
    0x00050041, 0x0000001f, 0x00000020, 0x0000001e, 0x0000001a, 0x0004003d,
    0x0000001b, 0x00000021, 0x00000020, 0x0004003d, 0x00000022, 0x00000025,
    0x00000024, 0x00050051, 0x00000006, 0x00000027, 0x00000025, 0x00000000,
    0x00050051, 0x00000006, 0x00000028, 0x00000025, 0x00000001, 0x00050051,
    0x00000006, 0x00000029, 0x00000025, 0x00000002, 0x00070050, 0x0000000d,
    0x0000002a, 0x00000027, 0x00000028, 0x00000029, 0x00000026, 0x00050091,
    0x0000000d, 0x0000002b, 0x00000021, 0x0000002a, 0x00050041, 0x0000000e,
    0x0000002c, 0x00000018, 0x0000001a, 0x0003003e, 0x0000002c, 0x0000002b,
    0x000100fd, 0x00010038,
};

static const uint32_t rlvk_default_frag_spv[] = {
    0x07230203, 0x00010000, 0x000d000b, 0x00000025, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000011, 0x00000015, 0x00000022,
    0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2,
    0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
    0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
    0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669,
    0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009,
    0x65786574, 0x6c6f436c, 0x0000726f, 0x00050005, 0x0000000d, 0x74786574,
    0x30657275, 0x00000000, 0x00060005, 0x00000011, 0x67617266, 0x43786554,
    0x64726f6f, 0x00000000, 0x00050005, 0x00000015, 0x616e6966, 0x6c6f436c,
    0x0000726f, 0x00060005, 0x00000018, 0x68737550, 0x736e6f43, 0x746e6174,
    0x00000073, 0x00040006, 0x00000018, 0x00000000, 0x0070766d, 0x00060006,
    0x00000018, 0x00000001, 0x446c6f63, 0x75666669, 0x00006573, 0x00030005,
    0x0000001a, 0x00006370, 0x00050005, 0x00000022, 0x67617266, 0x6f6c6f43,
    0x00000072, 0x00040047, 0x0000000d, 0x00000021, 0x00000000, 0x00040047,
    0x0000000d, 0x00000022, 0x00000000, 0x00040047, 0x00000011, 0x0000001e,
    0x00000000, 0x00040047, 0x00000015, 0x0000001e, 0x00000000, 0x00030047,
    0x00000018, 0x00000002, 0x00040048, 0x00000018, 0x00000000, 0x00000005,
    0x00050048, 0x00000018, 0x00000000, 0x00000007, 0x00000010, 0x00050048,
    0x00000018, 0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x00000018,
    0x00000001, 0x00000023, 0x00000040, 0x00040047, 0x00000022, 0x0000001e,
    0x00000001, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000004, 0x00040020, 0x00000008, 0x00000007, 0x00000007, 0x00090019,
    0x0000000a, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
    0x00000001, 0x00000000, 0x0003001b, 0x0000000b, 0x0000000a, 0x00040020,
    0x0000000c, 0x00000000, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
    0x00000000, 0x00040017, 0x0000000f, 0x00000006, 0x00000002, 0x00040020,
    0x00000010, 0x00000001, 0x0000000f, 0x0004003b, 0x00000010, 0x00000011,
    0x00000001, 0x00040020, 0x00000014, 0x00000003, 0x00000007, 0x0004003b,
    0x00000014, 0x00000015, 0x00000003, 0x00040018, 0x00000017, 0x00000007,
    0x00000004, 0x0004001e, 0x00000018, 0x00000017, 0x00000007, 0x00040020,
    0x00000019, 0x00000009, 0x00000018, 0x0004003b, 0x00000019, 0x0000001a,
    0x00000009, 0x00040015, 0x0000001b, 0x00000020, 0x00000001, 0x0004002b,
    0x0000001b, 0x0000001c, 0x00000001, 0x00040020, 0x0000001d, 0x00000009,
    0x00000007, 0x00040020, 0x00000021, 0x00000001, 0x00000007, 0x0004003b,
    0x00000021, 0x00000022, 0x00000001, 0x00050036, 0x00000002, 0x00000004,
    0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003b, 0x00000008,
    0x00000009, 0x00000007, 0x0004003d, 0x0000000b, 0x0000000e, 0x0000000d,
    0x0004003d, 0x0000000f, 0x00000012, 0x00000011, 0x00050057, 0x00000007,
    0x00000013, 0x0000000e, 0x00000012, 0x0003003e, 0x00000009, 0x00000013,
    0x0004003d, 0x00000007, 0x00000016, 0x00000009, 0x00050041, 0x0000001d,
    0x0000001e, 0x0000001a, 0x0000001c, 0x0004003d, 0x00000007, 0x0000001f,
    0x0000001e, 0x00050085, 0x00000007, 0x00000020, 0x00000016, 0x0000001f,
    0x0004003d, 0x00000007, 0x00000023, 0x00000022, 0x00050085, 0x00000007,
    0x00000024, 0x00000020, 0x00000023, 0x0003003e, 0x00000015, 0x00000024,
    0x000100fd, 0x00010038,
};

//----------------------------------------------------------------------------------
// Vulkan context state
//----------------------------------------------------------------------------------
#define RLVK_MAX_FRAMES_IN_FLIGHT 2

typedef struct rlvkContext {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    uint32_t graphicsFamily;
    uint32_t presentFamily;

    VkSwapchainKHR swapchain;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
    uint32_t swapchainImageCount;
    VkImage *swapchainImages;
    VkImageView *swapchainImageViews;

    VkRenderPass renderPass;
    VkFramebuffer *framebuffers;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[RLVK_MAX_FRAMES_IN_FLIGHT];

    VkSemaphore imageAvailableSemaphores[RLVK_MAX_FRAMES_IN_FLIGHT];
    VkSemaphore *renderFinishedSemaphores;  // One per swapchain image
    VkFence inFlightFences[RLVK_MAX_FRAMES_IN_FLIGHT];

    uint32_t currentFrame;
    uint32_t imageIndex;
    bool framebufferResized;
    bool frameBegun;                // Whether we're between BeginDrawing/EndDrawing

    // Pipeline
    VkPipelineLayout pipelineLayout;
    VkPipeline pipelineTriangles;
    VkPipeline pipelineLines;
    VkDescriptorSetLayout descriptorSetLayout;

    // Default texture
    VkImage defaultTexImage;
    VkDeviceMemory defaultTexMemory;
    VkImageView defaultTexView;
    VkSampler defaultSampler;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet defaultTexDescriptor;

    // Vertex buffers (per frame-in-flight): position, texcoord, normal, color
    VkBuffer vertexBuffers[RLVK_MAX_FRAMES_IN_FLIGHT][4];
    VkDeviceMemory vertexMemory[RLVK_MAX_FRAMES_IN_FLIGHT][4];
    void *vertexMapped[RLVK_MAX_FRAMES_IN_FLIGHT][4];

    // Index buffer (static, shared)
    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;

    VkPhysicalDeviceMemoryProperties memProperties;
} rlvkContext;

#define RLVK_MAX_SHADERS 64

typedef struct rlvkShader {
    int in_use;
    const rlvk_shader_blob *blob;  // borrowed pointer into user-owned blob

    // Vulkan static resources (created in rlvkLoadShaderBlob)
    VkShaderModule vs_module, fs_module;
    VkDescriptorSetLayout set_layouts[RLVK_MAX_DESC_SETS];
    uint8_t set_layout_used[RLVK_MAX_DESC_SETS];  // 1 if layout created
    VkPipelineLayout pipeline_layout;

    // Per-UBO resources
    int n_ubos;
    struct {
        uint8_t  set, binding;
        uint32_t size;
        uint8_t  shared;
        VkBuffer        buffer[RLVK_MAX_FRAMES_IN_FLIGHT];
        VkDeviceMemory  memory[RLVK_MAX_FRAMES_IN_FLIGHT];
        void           *mapped[RLVK_MAX_FRAMES_IN_FLIGHT];
        unsigned char  *shadow;   // CPU shadow, size bytes
        int             dirty;    // 1 = shadow differs from GPU buffer
    } ubos[RLVK_MAX_UBOS_PER_SHADER];

    // Descriptor pool + sets (one per frame-in-flight)
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet  descriptor_sets[RLVK_MAX_FRAMES_IN_FLIGHT][RLVK_MAX_DESC_SETS];

    // Sampler binding shadow
    int bound_sampler_texids[16];
    int sampler_descriptor_dirty[RLVK_MAX_FRAMES_IN_FLIGHT];
} rlvkShader;

// Internal rlgl-compatible state
typedef struct rlvkState {
    int currentMatrixMode;
    Matrix *currentMatrix;
    Matrix modelview;
    Matrix projection;
    Matrix transform;
    bool transformRequired;
    Matrix stack[RL_MAX_MATRIX_STACK_SIZE];
    int stackCounter;

    unsigned int defaultTextureId;
    unsigned int defaultShaderId;
    int *defaultShaderLocs;
    unsigned int currentShaderId;
    int *currentShaderLocs;

    int framebufferWidth;
    int framebufferHeight;

    float clearR, clearG, clearB, clearA;

    // Cull distance
    double cullNear;
    double cullFar;

    // Render batch
    rlRenderBatch *currentBatch;
    rlRenderBatch defaultBatch;

    // Current vertex state (for rlBegin/rlEnd)
    int vertexCounter;
    float texcoordx, texcoordy;
    float normalx, normaly, normalz;
    unsigned char colorr, colorg, colorb, colora;
    unsigned int currentTextureId;

    // Active framebuffer
    unsigned int activeFramebuffer;

    rlvkShader shaders[RLVK_MAX_SHADERS];
} rlvkState;

static rlvkContext RLVK = { 0 };
static rlvkState RLVK_STATE = { 0 };

//----------------------------------------------------------------------------------
// Texture registry
//----------------------------------------------------------------------------------
#define RLVK_MAX_TEXTURES 1024

typedef struct rlvkTexture {
    bool active;
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkSampler sampler;
    VkDescriptorSet descriptorSet;
    int width, height, format;
} rlvkTexture;

static rlvkTexture rlvkTextures[RLVK_MAX_TEXTURES] = { 0 };
static unsigned int rlvkTextureCount = 0;  // Next available slot (0 = unused, 1 = default)

static VkFormat rlvkGetVulkanFormat(int raylibFormat)
{
    switch (raylibFormat) {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:     return VK_FORMAT_R8_UNORM;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:    return VK_FORMAT_R8G8_UNORM;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:        return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8:        return VK_FORMAT_R8G8B8_UNORM;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:      return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:      return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:      return VK_FORMAT_R8G8B8A8_UNORM;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32:            return VK_FORMAT_R32_SFLOAT;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32:      return VK_FORMAT_R32G32B32_SFLOAT;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:   return VK_FORMAT_R32G32B32A32_SFLOAT;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16:            return VK_FORMAT_R16_SFLOAT;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16:      return VK_FORMAT_R16G16B16_SFLOAT;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:   return VK_FORMAT_R16G16B16A16_SFLOAT;
        default: return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

static int rlvkGetPixelSize(int raylibFormat)
{
    switch (raylibFormat) {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:     return 1;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:    return 2;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:        return 2;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8:        return 3;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:      return 2;
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:      return 2;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:      return 4;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32:            return 4;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32:      return 12;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:   return 16;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16:            return 2;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16:      return 6;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:   return 8;
        default: return 4;
    }
}

// Get component swizzle for formats that don't map 1:1 to RGBA
static VkComponentMapping rlvkGetSwizzle(int raylibFormat)
{
    VkComponentMapping swizzle = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
    };

    if (raylibFormat == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) {
        // R8 → (R, R, R, 1) — spread grayscale to RGB, alpha = 1
        swizzle.r = VK_COMPONENT_SWIZZLE_R;
        swizzle.g = VK_COMPONENT_SWIZZLE_R;
        swizzle.b = VK_COMPONENT_SWIZZLE_R;
        swizzle.a = VK_COMPONENT_SWIZZLE_ONE;
    }
    else if (raylibFormat == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) {
        // RG8 → (R, R, R, G) — gray to RGB, alpha from G channel
        swizzle.r = VK_COMPONENT_SWIZZLE_R;
        swizzle.g = VK_COMPONENT_SWIZZLE_R;
        swizzle.b = VK_COMPONENT_SWIZZLE_R;
        swizzle.a = VK_COMPONENT_SWIZZLE_G;
    }

    return swizzle;
}

// Allocate a descriptor set for a texture and update it
static VkDescriptorSet rlvkAllocTextureDescriptor(VkImageView view, VkSampler sampler)
{
    VkDescriptorSet ds;
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = RLVK.descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &RLVK.descriptorSetLayout,
    };
    vkAllocateDescriptorSets(RLVK.device, &allocInfo, &ds);

    VkDescriptorImageInfo imgInfo = {
        .sampler = sampler,
        .imageView = view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ds,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imgInfo,
    };
    vkUpdateDescriptorSets(RLVK.device, 1, &write, 0, NULL);
    return ds;
}

//----------------------------------------------------------------------------------
// Vulkan helpers
//----------------------------------------------------------------------------------
static VKAPI_ATTR VkBool32 VKAPI_CALL rlvkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData)
{
    (void)type; (void)userData;
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        TRACELOG(RL_LOG_WARNING, "VULKAN: %s", callbackData->pMessage);
    return VK_FALSE;
}

//----------------------------------------------------------------------------------
// Vulkan memory/buffer/image helpers
//----------------------------------------------------------------------------------
static uint32_t rlvkFindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < RLVK.memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (RLVK.memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    TRACELOG(RL_LOG_ERROR, "VULKAN: Failed to find suitable memory type");
    return 0;
}

static void rlvkCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps,
                              VkBuffer *buffer, VkDeviceMemory *memory)
{
    VkBufferCreateInfo bufInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    vkCreateBuffer(RLVK.device, &bufInfo, NULL, buffer);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(RLVK.device, *buffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReq.size,
        .memoryTypeIndex = rlvkFindMemoryType(memReq.memoryTypeBits, memProps),
    };
    vkAllocateMemory(RLVK.device, &allocInfo, NULL, memory);
    vkBindBufferMemory(RLVK.device, *buffer, *memory, 0);
}

static VkCommandBuffer rlvkBeginSingleTimeCommands(void)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = RLVK.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(RLVK.device, &allocInfo, &cmd);
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &beginInfo);
    return cmd;
}

static void rlvkEndSingleTimeCommands(VkCommandBuffer cmd)
{
    vkEndCommandBuffer(cmd);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };
    vkQueueSubmit(RLVK.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(RLVK.graphicsQueue);
    vkFreeCommandBuffers(RLVK.device, RLVK.commandPool, 1, &cmd);
}

// Push constant data structure (must match shader layout)
typedef struct rlvkPushConstants {
    float mvp[16];       // mat4 (64 bytes)
    float colDiffuse[4]; // vec4 (16 bytes)
} rlvkPushConstants; // Total: 80 bytes

//----------------------------------------------------------------------------------
// Pipeline and resource creation
//----------------------------------------------------------------------------------
static void rlvkCreateDefaultTexture(void)
{
    // 1x1 white pixel
    unsigned char pixel[4] = { 255, 255, 255, 255 };

    // Create image
    VkImageCreateInfo imgInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = { 1, 1, 1 },
        .mipLevels = 1, .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    vkCreateImage(RLVK.device, &imgInfo, NULL, &RLVK.defaultTexImage);

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(RLVK.device, RLVK.defaultTexImage, &memReq);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReq.size,
        .memoryTypeIndex = rlvkFindMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    vkAllocateMemory(RLVK.device, &allocInfo, NULL, &RLVK.defaultTexMemory);
    vkBindImageMemory(RLVK.device, RLVK.defaultTexImage, RLVK.defaultTexMemory, 0);

    // Upload via staging buffer
    VkBuffer staging; VkDeviceMemory stagingMem;
    rlvkCreateBuffer(4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging, &stagingMem);
    void *mapped;
    vkMapMemory(RLVK.device, stagingMem, 0, 4, 0, &mapped);
    memcpy(mapped, pixel, 4);
    vkUnmapMemory(RLVK.device, stagingMem);

    VkCommandBuffer cmd = rlvkBeginSingleTimeCommands();
    // Transition to transfer dst
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = RLVK.defaultTexImage,
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
        0, NULL, 0, NULL, 1, &barrier);

    VkBufferImageCopy region = {
        .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        .imageExtent = { 1, 1, 1 },
    };
    vkCmdCopyBufferToImage(cmd, staging, RLVK.defaultTexImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition to shader read
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, NULL, 0, NULL, 1, &barrier);

    rlvkEndSingleTimeCommands(cmd);
    vkDestroyBuffer(RLVK.device, staging, NULL);
    vkFreeMemory(RLVK.device, stagingMem, NULL);

    // Image view
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = RLVK.defaultTexImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
    };
    vkCreateImageView(RLVK.device, &viewInfo, NULL, &RLVK.defaultTexView);

    // Sampler
    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    };
    vkCreateSampler(RLVK.device, &samplerInfo, NULL, &RLVK.defaultSampler);

    // Register slot 0 as unused, slot 1 as default texture
    rlvkTextureCount = 2;  // Next free slot is 2
    rlvkTextures[1].active = true;
    rlvkTextures[1].image = RLVK.defaultTexImage;
    rlvkTextures[1].memory = RLVK.defaultTexMemory;
    rlvkTextures[1].view = RLVK.defaultTexView;
    rlvkTextures[1].sampler = RLVK.defaultSampler;
    rlvkTextures[1].width = 1;
    rlvkTextures[1].height = 1;
    rlvkTextures[1].format = RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    // descriptorSet will be set after rlvkCreateDescriptors
}

//----------------------------------------------------------------------------------
// Shader loading (rlvkLoadShaderBlob and helpers)
//----------------------------------------------------------------------------------
static VkShaderModule rlvkCreateShaderModule(const uint32_t *code, size_t size_bytes)
{
    VkShaderModuleCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size_bytes,
        .pCode    = code,
    };
    VkShaderModule m = VK_NULL_HANDLE;
    VkResult r = vkCreateShaderModule(RLVK.device, &ci, NULL, &m);
    if (r != VK_SUCCESS) TRACELOG(RL_LOG_ERROR, "VULKAN: vkCreateShaderModule failed (%d)", r);
    return m;
}

static int rlvkAllocShaderSlot(void)
{
    for (int i = 1; i < RLVK_MAX_SHADERS; i++) {
        if (!RLVK_STATE.shaders[i].in_use) {
            memset(&RLVK_STATE.shaders[i], 0, sizeof(rlvkShader));
            RLVK_STATE.shaders[i].in_use = 1;
            return i;
        }
    }
    return 0;
}

static void rlvkBuildDescriptorSetLayouts(rlvkShader *s)
{
    VkDescriptorSetLayoutBinding bindings[RLVK_MAX_DESC_SETS][16];
    uint32_t count[RLVK_MAX_DESC_SETS] = {0};

    // UBOs
    for (const rlvk_ubo_entry *u = s->blob->ubos; u && u->size; u++) {
        if (u->set >= RLVK_MAX_DESC_SETS) continue;
        VkDescriptorSetLayoutBinding *b = &bindings[u->set][count[u->set]++];
        *b = (VkDescriptorSetLayoutBinding){
            .binding = u->binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        };
    }

    // Samplers
    for (const rlvk_sampler_entry *sa = s->blob->samplers; sa && sa->name; sa++) {
        if (sa->set >= RLVK_MAX_DESC_SETS) continue;
        VkDescriptorSetLayoutBinding *b = &bindings[sa->set][count[sa->set]++];
        *b = (VkDescriptorSetLayoutBinding){
            .binding = sa->binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        };
    }

    for (int set = 0; set < RLVK_MAX_DESC_SETS; set++) {
        if (count[set] == 0) continue;
        VkDescriptorSetLayoutCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = count[set],
            .pBindings    = bindings[set],
        };
        VkResult r = vkCreateDescriptorSetLayout(RLVK.device, &ci, NULL, &s->set_layouts[set]);
        if (r == VK_SUCCESS) s->set_layout_used[set] = 1;
        else TRACELOG(RL_LOG_ERROR, "VULKAN: vkCreateDescriptorSetLayout set=%d failed (%d)", set, r);
    }
}

static void rlvkBuildPipelineLayout(rlvkShader *s)
{
    VkDescriptorSetLayout layouts[RLVK_MAX_DESC_SETS];
    uint32_t n = 0;
    for (int i = 0; i < RLVK_MAX_DESC_SETS; i++)
        if (s->set_layout_used[i]) layouts[n++] = s->set_layouts[i];

    VkPushConstantRange pushRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0, .size = sizeof(rlvkPushConstants),
    };

    VkPipelineLayoutCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = n, .pSetLayouts = layouts,
        .pushConstantRangeCount = 1, .pPushConstantRanges = &pushRange,
    };
    VkResult r = vkCreatePipelineLayout(RLVK.device, &ci, NULL, &s->pipeline_layout);
    if (r != VK_SUCCESS) TRACELOG(RL_LOG_ERROR, "VULKAN: vkCreatePipelineLayout failed (%d)", r);
}

static void rlvkAllocShaderUBOs(rlvkShader *s)
{
    int idx = 0;
    for (const rlvk_ubo_entry *u = s->blob->ubos; u && u->size; u++, idx++) {
        if (idx >= RLVK_MAX_UBOS_PER_SHADER) break;
        s->ubos[idx].set     = u->set;
        s->ubos[idx].binding = u->binding;
        s->ubos[idx].size    = u->size;
        s->ubos[idx].shared  = u->shared;
        if (u->shared) continue;  // shared frame UBO is owned by RLVK_STATE

        s->ubos[idx].shadow = (unsigned char*)RL_CALLOC(1, u->size);
        s->ubos[idx].dirty  = 1;

        for (int f = 0; f < RLVK_MAX_FRAMES_IN_FLIGHT; f++) {
            rlvkCreateBuffer(
                u->size,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &s->ubos[idx].buffer[f], &s->ubos[idx].memory[f]);
            vkMapMemory(RLVK.device, s->ubos[idx].memory[f], 0, u->size, 0, &s->ubos[idx].mapped[f]);
        }
    }
    s->n_ubos = idx;
}

static void rlvkAllocShaderDescriptors(rlvkShader *s)
{
    uint32_t n_ubo = 0, n_smp = 0, n_sets = 0;
    for (int i = 0; i < RLVK_MAX_DESC_SETS; i++) if (s->set_layout_used[i]) n_sets++;
    for (int i = 0; i < s->n_ubos; i++) (void)i, n_ubo++;
    for (const rlvk_sampler_entry *sa = s->blob->samplers; sa && sa->name; sa++) n_smp++;

    VkDescriptorPoolSize sizes[2];
    uint32_t n_sizes = 0;
    if (n_ubo > 0) sizes[n_sizes++] = (VkDescriptorPoolSize){ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, n_ubo * RLVK_MAX_FRAMES_IN_FLIGHT };
    if (n_smp > 0) sizes[n_sizes++] = (VkDescriptorPoolSize){ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, n_smp * RLVK_MAX_FRAMES_IN_FLIGHT };
    if (n_sizes == 0) return;

    VkDescriptorPoolCreateInfo pci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = n_sets * RLVK_MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = n_sizes, .pPoolSizes = sizes,
    };
    vkCreateDescriptorPool(RLVK.device, &pci, NULL, &s->descriptor_pool);

    for (int f = 0; f < RLVK_MAX_FRAMES_IN_FLIGHT; f++) {
        VkDescriptorSetLayout layouts[RLVK_MAX_DESC_SETS];
        int set_map[RLVK_MAX_DESC_SETS]; int n = 0;
        for (int i = 0; i < RLVK_MAX_DESC_SETS; i++) {
            if (s->set_layout_used[i]) { layouts[n] = s->set_layouts[i]; set_map[n] = i; n++; }
        }
        VkDescriptorSetAllocateInfo ai = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = s->descriptor_pool,
            .descriptorSetCount = n, .pSetLayouts = layouts,
        };
        VkDescriptorSet tmp[RLVK_MAX_DESC_SETS];
        vkAllocateDescriptorSets(RLVK.device, &ai, tmp);
        for (int i = 0; i < n; i++) s->descriptor_sets[f][set_map[i]] = tmp[i];
    }
}

static void rlvkWriteShaderDescriptors(rlvkShader *s)
{
    VkWriteDescriptorSet writes[32]; VkDescriptorBufferInfo binfos[32]; VkDescriptorImageInfo iinfos[16];
    for (int f = 0; f < RLVK_MAX_FRAMES_IN_FLIGHT; f++) {
        uint32_t n = 0, bi = 0, ii = 0;
        for (int i = 0; i < s->n_ubos; i++) {
            if (s->ubos[i].shared) continue;   // Task 36 handles shared frame UBO
            binfos[bi] = (VkDescriptorBufferInfo){
                .buffer = s->ubos[i].buffer[f], .offset = 0, .range = s->ubos[i].size
            };
            writes[n++] = (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = s->descriptor_sets[f][s->ubos[i].set],
                .dstBinding = s->ubos[i].binding,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &binfos[bi],
            };
            bi++;
        }
        for (const rlvk_sampler_entry *sa = s->blob->samplers; sa && sa->name; sa++) {
            iinfos[ii] = (VkDescriptorImageInfo){
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView   = RLVK.defaultTexView,
                .sampler     = RLVK.defaultSampler,
            };
            writes[n++] = (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = s->descriptor_sets[f][sa->set],
                .dstBinding = sa->binding,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &iinfos[ii],
            };
            ii++;
        }
        if (n) vkUpdateDescriptorSets(RLVK.device, n, writes, 0, NULL);
    }
}

unsigned int rlvkLoadShaderBlob(const rlvk_shader_blob *blob)
{
    if (!blob) return 0;
    int id = rlvkAllocShaderSlot();
    if (!id) { TRACELOG(RL_LOG_ERROR, "VULKAN: shader registry full"); return 0; }
    rlvkShader *s = &RLVK_STATE.shaders[id];
    s->blob = blob;

    s->vs_module = rlvkCreateShaderModule(blob->vs_spv, blob->vs_size);
    s->fs_module = rlvkCreateShaderModule(blob->fs_spv, blob->fs_size);
    if (!s->vs_module || !s->fs_module) { s->in_use = 0; return 0; }

    rlvkBuildDescriptorSetLayouts(s);
    rlvkBuildPipelineLayout(s);
    rlvkAllocShaderUBOs(s);
    rlvkAllocShaderDescriptors(s);
    rlvkWriteShaderDescriptors(s);

    for (int i = 0; i < 16; i++) s->bound_sampler_texids[i] = -1;
    return (unsigned int)id;
}

static void rlvkCreateDescriptors(void)
{
    // Descriptor set layout: one combined image sampler at binding 0
    VkDescriptorSetLayoutBinding binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1, .pBindings = &binding,
    };
    vkCreateDescriptorSetLayout(RLVK.device, &layoutInfo, NULL, &RLVK.descriptorSetLayout);

    // Descriptor pool
    VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 256 };
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 256,
        .poolSizeCount = 1, .pPoolSizes = &poolSize,
    };
    vkCreateDescriptorPool(RLVK.device, &poolInfo, NULL, &RLVK.descriptorPool);

    // Allocate default texture descriptor set
    VkDescriptorSetAllocateInfo dsAlloc = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = RLVK.descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &RLVK.descriptorSetLayout,
    };
    vkAllocateDescriptorSets(RLVK.device, &dsAlloc, &RLVK.defaultTexDescriptor);

    VkDescriptorImageInfo imgDescInfo = {
        .sampler = RLVK.defaultSampler,
        .imageView = RLVK.defaultTexView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = RLVK.defaultTexDescriptor,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imgDescInfo,
    };
    vkUpdateDescriptorSets(RLVK.device, 1, &write, 0, NULL);

    // Store in texture registry
    rlvkTextures[1].descriptorSet = RLVK.defaultTexDescriptor;
}

static void rlvkCreatePipeline(void)
{
    // Shader modules
    VkShaderModuleCreateInfo vertInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(rlvk_default_vert_spv),
        .pCode = rlvk_default_vert_spv,
    };
    VkShaderModule vertModule;
    vkCreateShaderModule(RLVK.device, &vertInfo, NULL, &vertModule);

    VkShaderModuleCreateInfo fragInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(rlvk_default_frag_spv),
        .pCode = rlvk_default_frag_spv,
    };
    VkShaderModule fragModule;
    vkCreateShaderModule(RLVK.device, &fragInfo, NULL, &fragModule);

    VkPipelineShaderStageCreateInfo stages[2] = {
        { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vertModule, .pName = "main" },
        { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fragModule, .pName = "main" },
    };

    // Vertex input: 4 separate bindings (position, texcoord, normal, color)
    VkVertexInputBindingDescription bindings[4] = {
        { 0, 3*sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX },  // position
        { 1, 2*sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX },  // texcoord
        { 2, 3*sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX },  // normal
        { 3, 4*sizeof(unsigned char), VK_VERTEX_INPUT_RATE_VERTEX },  // color (RGBA8)
    };
    VkVertexInputAttributeDescription attrs[4] = {
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },     // position
        { 1, 1, VK_FORMAT_R32G32_SFLOAT, 0 },         // texcoord
        { 2, 2, VK_FORMAT_R32G32B32_SFLOAT, 0 },      // normal
        { 3, 3, VK_FORMAT_R8G8B8A8_UNORM, 0 },        // color
    };
    VkPipelineVertexInputStateCreateInfo vertexInput = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 4, .pVertexBindingDescriptions = bindings,
        .vertexAttributeDescriptionCount = 4, .pVertexAttributeDescriptions = attrs,
    };

    // Dynamic states
    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2, .pDynamicStates = dynStates,
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1, .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,  // raylib disables culling for 2D by default
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState blendAttachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1, .pAttachments = &blendAttachment,
    };

    // Push constant range: 80 bytes (mat4 mvp + vec4 colDiffuse) for both vert and frag
    VkPushConstantRange pushRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(rlvkPushConstants),
    };

    VkPipelineLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1, .pSetLayouts = &RLVK.descriptorSetLayout,
        .pushConstantRangeCount = 1, .pPushConstantRanges = &pushRange,
    };
    vkCreatePipelineLayout(RLVK.device, &layoutInfo, NULL, &RLVK.pipelineLayout);

    // Triangle pipeline
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyTri = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2, .pStages = stages,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssemblyTri,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynState,
        .layout = RLVK.pipelineLayout,
        .renderPass = RLVK.renderPass,
        .subpass = 0,
    };
    vkCreateGraphicsPipelines(RLVK.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &RLVK.pipelineTriangles);

    // Line pipeline
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyLine = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
    };
    pipelineInfo.pInputAssemblyState = &inputAssemblyLine;
    vkCreateGraphicsPipelines(RLVK.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &RLVK.pipelineLines);

    vkDestroyShaderModule(RLVK.device, vertModule, NULL);
    vkDestroyShaderModule(RLVK.device, fragModule, NULL);

    TRACELOG(RL_LOG_INFO, "VULKAN: Default pipeline created");
}

static void rlvkCreateVertexBuffers(void)
{
    int maxVertices = RL_DEFAULT_BATCH_BUFFER_ELEMENTS * 4; // 4 verts per quad

    for (int f = 0; f < RLVK_MAX_FRAMES_IN_FLIGHT; f++) {
        // Positions: 3 floats per vertex
        VkDeviceSize posSize = maxVertices * 3 * sizeof(float);
        rlvkCreateBuffer(posSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &RLVK.vertexBuffers[f][0], &RLVK.vertexMemory[f][0]);
        vkMapMemory(RLVK.device, RLVK.vertexMemory[f][0], 0, posSize, 0, &RLVK.vertexMapped[f][0]);

        // Texcoords: 2 floats per vertex
        VkDeviceSize tcSize = maxVertices * 2 * sizeof(float);
        rlvkCreateBuffer(tcSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &RLVK.vertexBuffers[f][1], &RLVK.vertexMemory[f][1]);
        vkMapMemory(RLVK.device, RLVK.vertexMemory[f][1], 0, tcSize, 0, &RLVK.vertexMapped[f][1]);

        // Normals: 3 floats per vertex
        VkDeviceSize normSize = maxVertices * 3 * sizeof(float);
        rlvkCreateBuffer(normSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &RLVK.vertexBuffers[f][2], &RLVK.vertexMemory[f][2]);
        vkMapMemory(RLVK.device, RLVK.vertexMemory[f][2], 0, normSize, 0, &RLVK.vertexMapped[f][2]);

        // Colors: 4 bytes per vertex
        VkDeviceSize colSize = maxVertices * 4 * sizeof(unsigned char);
        rlvkCreateBuffer(colSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &RLVK.vertexBuffers[f][3], &RLVK.vertexMemory[f][3]);
        vkMapMemory(RLVK.device, RLVK.vertexMemory[f][3], 0, colSize, 0, &RLVK.vertexMapped[f][3]);
    }

    // Index buffer (static, for quad->triangle conversion)
    int maxIndices = RL_DEFAULT_BATCH_BUFFER_ELEMENTS * 6; // 6 indices per quad
    VkDeviceSize idxSize = maxIndices * sizeof(unsigned int);
    rlvkCreateBuffer(idxSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &RLVK.indexBuffer, &RLVK.indexMemory);

    void *idxMapped;
    vkMapMemory(RLVK.device, RLVK.indexMemory, 0, idxSize, 0, &idxMapped);
    unsigned int *indices = (unsigned int *)idxMapped;
    for (int j = 0; j < RL_DEFAULT_BATCH_BUFFER_ELEMENTS; j++) {
        indices[j*6 + 0] = j*4 + 0;
        indices[j*6 + 1] = j*4 + 1;
        indices[j*6 + 2] = j*4 + 2;
        indices[j*6 + 3] = j*4 + 0;
        indices[j*6 + 4] = j*4 + 2;
        indices[j*6 + 5] = j*4 + 3;
    }
    vkUnmapMemory(RLVK.device, RLVK.indexMemory);

    TRACELOG(RL_LOG_INFO, "VULKAN: Vertex buffers created (%d max vertices)", maxVertices);
}

// Matrix helper: identity
static Matrix rlvkMatrixIdentity(void)
{
    Matrix m = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return m;
}

// Matrix multiply
static Matrix rlvkMatrixMultiply(Matrix left, Matrix right)
{
    Matrix result = { 0 };
    result.m0 = left.m0*right.m0 + left.m4*right.m1 + left.m8*right.m2 + left.m12*right.m3;
    result.m4 = left.m0*right.m4 + left.m4*right.m5 + left.m8*right.m6 + left.m12*right.m7;
    result.m8 = left.m0*right.m8 + left.m4*right.m9 + left.m8*right.m10 + left.m12*right.m11;
    result.m12 = left.m0*right.m12 + left.m4*right.m13 + left.m8*right.m14 + left.m12*right.m15;
    result.m1 = left.m1*right.m0 + left.m5*right.m1 + left.m9*right.m2 + left.m13*right.m3;
    result.m5 = left.m1*right.m4 + left.m5*right.m5 + left.m9*right.m6 + left.m13*right.m7;
    result.m9 = left.m1*right.m8 + left.m5*right.m9 + left.m9*right.m10 + left.m13*right.m11;
    result.m13 = left.m1*right.m12 + left.m5*right.m13 + left.m9*right.m14 + left.m13*right.m15;
    result.m2 = left.m2*right.m0 + left.m6*right.m1 + left.m10*right.m2 + left.m14*right.m3;
    result.m6 = left.m2*right.m4 + left.m6*right.m5 + left.m10*right.m6 + left.m14*right.m7;
    result.m10 = left.m2*right.m8 + left.m6*right.m9 + left.m10*right.m10 + left.m14*right.m11;
    result.m14 = left.m2*right.m12 + left.m6*right.m13 + left.m10*right.m14 + left.m14*right.m15;
    result.m3 = left.m3*right.m0 + left.m7*right.m1 + left.m11*right.m2 + left.m15*right.m3;
    result.m7 = left.m3*right.m4 + left.m7*right.m5 + left.m11*right.m6 + left.m15*right.m7;
    result.m11 = left.m3*right.m8 + left.m7*right.m9 + left.m11*right.m10 + left.m15*right.m11;
    result.m15 = left.m3*right.m12 + left.m7*right.m13 + left.m11*right.m14 + left.m15*right.m15;
    return result;
}

//----------------------------------------------------------------------------------
// Vulkan initialization functions
//----------------------------------------------------------------------------------

// Called from the platform layer after window creation to pass the GLFW window
// so we can create the Vulkan surface
void rlvkSetWindow(void *glfwWindow)
{
    // Create instance
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "raylib",
        .applicationVersion = VK_MAKE_VERSION(5, 5, 0),
        .pEngineName = "raylib-vulkan",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    uint32_t glfwExtCount = 0;
    const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    uint32_t extCount = glfwExtCount;
    const char *extensions[32];
    for (uint32_t i = 0; i < glfwExtCount; i++) extensions[i] = glfwExts[i];

#if !defined(NDEBUG)
    extensions[extCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    const char *layers[] = { "VK_LAYER_KHRONOS_validation" };

    // Check if validation layer is available
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties *availableLayers = RL_MALLOC(layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    bool validationAvailable = false;
    for (uint32_t i = 0; i < layerCount; i++) {
        if (strcmp(availableLayers[i].layerName, "VK_LAYER_KHRONOS_validation") == 0) {
            validationAvailable = true;
            break;
        }
    }
    RL_FREE(availableLayers);

    uint32_t enabledLayerCount = validationAvailable ? 1 : 0;
    if (!validationAvailable) {
        TRACELOG(RL_LOG_WARNING, "VULKAN: Validation layers not available");
        extCount = glfwExtCount; // Remove debug utils extension
    }
#else
    const char **layers = NULL;
    uint32_t enabledLayerCount = 0;
#endif

#if defined(__APPLE__)
    // MoltenVK requires portability enumeration
    extensions[extCount++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    extensions[extCount++] = "VK_KHR_get_physical_device_properties2";
#endif

    VkInstanceCreateInfo instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if defined(__APPLE__)
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extCount,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = enabledLayerCount,
        .ppEnabledLayerNames = layers,
    };

    VkResult result = vkCreateInstance(&instanceInfo, NULL, &RLVK.instance);
    if (result != VK_SUCCESS) {
        TRACELOG(RL_LOG_FATAL, "VULKAN: Failed to create instance (%d)", result);
        return;
    }
    TRACELOG(RL_LOG_INFO, "VULKAN: Instance created successfully");

    // Debug messenger
#if !defined(NDEBUG)
    if (validationAvailable) {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = rlvkDebugCallback,
        };
        PFN_vkCreateDebugUtilsMessengerEXT createDebug =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(RLVK.instance, "vkCreateDebugUtilsMessengerEXT");
        if (createDebug) createDebug(RLVK.instance, &debugInfo, NULL, &RLVK.debugMessenger);
    }
#endif

    // Surface
    GLFWwindow *window = (GLFWwindow *)glfwWindow;
    result = glfwCreateWindowSurface(RLVK.instance, window, NULL, &RLVK.surface);
    if (result != VK_SUCCESS) {
        TRACELOG(RL_LOG_FATAL, "VULKAN: Failed to create window surface (%d)", result);
        return;
    }

    // Pick physical device (prefer discrete)
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(RLVK.instance, &deviceCount, NULL);
    VkPhysicalDevice *devices = RL_MALLOC(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(RLVK.instance, &deviceCount, devices);

    RLVK.physicalDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);

        uint32_t qfCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &qfCount, NULL);
        VkQueueFamilyProperties *qfProps = RL_MALLOC(qfCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &qfCount, qfProps);

        bool hasGraphics = false, hasPresent = false;
        for (uint32_t j = 0; j < qfCount; j++) {
            if (qfProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) { RLVK.graphicsFamily = j; hasGraphics = true; }
            VkBool32 presentOk = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, RLVK.surface, &presentOk);
            if (presentOk) { RLVK.presentFamily = j; hasPresent = true; }
        }
        RL_FREE(qfProps);

        if (hasGraphics && hasPresent) {
            RLVK.physicalDevice = devices[i];
            TRACELOG(RL_LOG_INFO, "VULKAN: GPU selected: %s", props.deviceName);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) break;
        }
    }
    RL_FREE(devices);

    if (RLVK.physicalDevice == VK_NULL_HANDLE) {
        TRACELOG(RL_LOG_FATAL, "VULKAN: No suitable GPU found");
        return;
    }

    // Create logical device
    float queuePriority = 1.0f;
    uint32_t uniqueFamilies[2] = { RLVK.graphicsFamily, RLVK.presentFamily };
    uint32_t uniqueCount = (RLVK.graphicsFamily == RLVK.presentFamily) ? 1 : 2;

    VkDeviceQueueCreateInfo queueInfos[2];
    for (uint32_t i = 0; i < uniqueCount; i++) {
        queueInfos[i] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
    }

    VkPhysicalDeviceFeatures features = { 0 };

    // Probe for VK_KHR_portability_subset (required on MoltenVK, absent on Linux/Windows)
    uint32_t devExtCount = 0;
    vkEnumerateDeviceExtensionProperties(RLVK.physicalDevice, NULL, &devExtCount, NULL);
    VkExtensionProperties *devExtProps = RL_CALLOC(devExtCount, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(RLVK.physicalDevice, NULL, &devExtCount, devExtProps);

    const char *deviceExtensions[4];
    uint32_t deviceExtCount = 0;
    deviceExtensions[deviceExtCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    for (uint32_t i = 0; i < devExtCount; i++) {
        if (strcmp(devExtProps[i].extensionName, "VK_KHR_portability_subset") == 0) {
            deviceExtensions[deviceExtCount++] = "VK_KHR_portability_subset";
            break;
        }
    }
    RL_FREE(devExtProps);

    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = uniqueCount,
        .pQueueCreateInfos = queueInfos,
        .pEnabledFeatures = &features,
        .enabledExtensionCount = deviceExtCount,
        .ppEnabledExtensionNames = deviceExtensions,
    };

    result = vkCreateDevice(RLVK.physicalDevice, &deviceInfo, NULL, &RLVK.device);
    if (result != VK_SUCCESS) {
        TRACELOG(RL_LOG_FATAL, "VULKAN: Failed to create logical device (%d)", result);
        return;
    }

    vkGetDeviceQueue(RLVK.device, RLVK.graphicsFamily, 0, &RLVK.graphicsQueue);
    vkGetDeviceQueue(RLVK.device, RLVK.presentFamily, 0, &RLVK.presentQueue);
    TRACELOG(RL_LOG_INFO, "VULKAN: Logical device created");
}

// Create swapchain and associated resources
static void rlvkCreateSwapchain(int width, int height)
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(RLVK.physicalDevice, RLVK.surface, &caps);

    // Format
    uint32_t fmtCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(RLVK.physicalDevice, RLVK.surface, &fmtCount, NULL);
    VkSurfaceFormatKHR *fmts = RL_MALLOC(fmtCount * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(RLVK.physicalDevice, RLVK.surface, &fmtCount, fmts);

    VkSurfaceFormatKHR surfFmt = fmts[0];
    for (uint32_t i = 0; i < fmtCount; i++) {
        if (fmts[i].format == VK_FORMAT_B8G8R8A8_SRGB && fmts[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfFmt = fmts[i];
            break;
        }
    }
    RL_FREE(fmts);

    // Present mode
    uint32_t pmCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(RLVK.physicalDevice, RLVK.surface, &pmCount, NULL);
    VkPresentModeKHR *pms = RL_MALLOC(pmCount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(RLVK.physicalDevice, RLVK.surface, &pmCount, pms);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < pmCount; i++) {
        if (pms[i] == VK_PRESENT_MODE_MAILBOX_KHR) { presentMode = VK_PRESENT_MODE_MAILBOX_KHR; break; }
    }
    RL_FREE(pms);

    // Extent
    VkExtent2D extent;
    if (caps.currentExtent.width != UINT32_MAX) {
        extent = caps.currentExtent;
    } else {
        extent.width = (uint32_t)width;
        extent.height = (uint32_t)height;
        if (extent.width < caps.minImageExtent.width) extent.width = caps.minImageExtent.width;
        if (extent.width > caps.maxImageExtent.width) extent.width = caps.maxImageExtent.width;
        if (extent.height < caps.minImageExtent.height) extent.height = caps.minImageExtent.height;
        if (extent.height > caps.maxImageExtent.height) extent.height = caps.maxImageExtent.height;
    }

    uint32_t imgCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imgCount > caps.maxImageCount) imgCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR scInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = RLVK.surface,
        .minImageCount = imgCount,
        .imageFormat = surfFmt.format,
        .imageColorSpace = surfFmt.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
    };

    uint32_t families[] = { RLVK.graphicsFamily, RLVK.presentFamily };
    if (RLVK.graphicsFamily != RLVK.presentFamily) {
        scInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        scInfo.queueFamilyIndexCount = 2;
        scInfo.pQueueFamilyIndices = families;
    } else {
        scInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    vkCreateSwapchainKHR(RLVK.device, &scInfo, NULL, &RLVK.swapchain);

    vkGetSwapchainImagesKHR(RLVK.device, RLVK.swapchain, &RLVK.swapchainImageCount, NULL);
    RLVK.swapchainImages = RL_MALLOC(RLVK.swapchainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(RLVK.device, RLVK.swapchain, &RLVK.swapchainImageCount, RLVK.swapchainImages);
    RLVK.swapchainFormat = surfFmt.format;
    RLVK.swapchainExtent = extent;

    // Image views
    RLVK.swapchainImageViews = RL_MALLOC(RLVK.swapchainImageCount * sizeof(VkImageView));
    for (uint32_t i = 0; i < RLVK.swapchainImageCount; i++) {
        VkImageViewCreateInfo ivInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = RLVK.swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = RLVK.swapchainFormat,
            .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        };
        vkCreateImageView(RLVK.device, &ivInfo, NULL, &RLVK.swapchainImageViews[i]);
    }

    // Render pass
    VkAttachmentDescription colorAtt = {
        .format = RLVK.swapchainFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorRef,
    };
    VkSubpassDependency dep = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    VkRenderPassCreateInfo rpInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1, .pAttachments = &colorAtt,
        .subpassCount = 1, .pSubpasses = &subpass,
        .dependencyCount = 1, .pDependencies = &dep,
    };
    vkCreateRenderPass(RLVK.device, &rpInfo, NULL, &RLVK.renderPass);

    // Framebuffers
    RLVK.framebuffers = RL_MALLOC(RLVK.swapchainImageCount * sizeof(VkFramebuffer));
    for (uint32_t i = 0; i < RLVK.swapchainImageCount; i++) {
        VkFramebufferCreateInfo fbInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = RLVK.renderPass,
            .attachmentCount = 1, .pAttachments = &RLVK.swapchainImageViews[i],
            .width = extent.width, .height = extent.height, .layers = 1,
        };
        vkCreateFramebuffer(RLVK.device, &fbInfo, NULL, &RLVK.framebuffers[i]);
    }

    // Sync: render-finished semaphores (one per swapchain image)
    RLVK.renderFinishedSemaphores = RL_MALLOC(RLVK.swapchainImageCount * sizeof(VkSemaphore));
    VkSemaphoreCreateInfo semInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    for (uint32_t i = 0; i < RLVK.swapchainImageCount; i++) {
        vkCreateSemaphore(RLVK.device, &semInfo, NULL, &RLVK.renderFinishedSemaphores[i]);
    }

    TRACELOG(RL_LOG_INFO, "VULKAN: Swapchain created (%dx%d, %d images)", extent.width, extent.height, RLVK.swapchainImageCount);
}

static void rlvkCleanupSwapchain(void)
{
    for (uint32_t i = 0; i < RLVK.swapchainImageCount; i++) {
        vkDestroyFramebuffer(RLVK.device, RLVK.framebuffers[i], NULL);
        vkDestroyImageView(RLVK.device, RLVK.swapchainImageViews[i], NULL);
        vkDestroySemaphore(RLVK.device, RLVK.renderFinishedSemaphores[i], NULL);
    }
    RL_FREE(RLVK.framebuffers);
    RL_FREE(RLVK.swapchainImageViews);
    RL_FREE(RLVK.swapchainImages);
    RL_FREE(RLVK.renderFinishedSemaphores);
    vkDestroyRenderPass(RLVK.device, RLVK.renderPass, NULL);
    vkDestroySwapchainKHR(RLVK.device, RLVK.swapchain, NULL);
}

static void rlvkRecreateSwapchain(void)
{
    vkDeviceWaitIdle(RLVK.device);
    rlvkCleanupSwapchain();
    rlvkCreateSwapchain(RLVK_STATE.framebufferWidth, RLVK_STATE.framebufferHeight);
}

//----------------------------------------------------------------------------------
// Frame management (called from platform layer)
//----------------------------------------------------------------------------------

// Begin a new frame — acquire swapchain image, begin command buffer and render pass
void rlvkBeginFrame(void)
{
    vkWaitForFences(RLVK.device, 1, &RLVK.inFlightFences[RLVK.currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(RLVK.device, RLVK.swapchain, UINT64_MAX,
        RLVK.imageAvailableSemaphores[RLVK.currentFrame], VK_NULL_HANDLE, &RLVK.imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        rlvkRecreateSwapchain();
        // Re-acquire after recreation
        vkAcquireNextImageKHR(RLVK.device, RLVK.swapchain, UINT64_MAX,
            RLVK.imageAvailableSemaphores[RLVK.currentFrame], VK_NULL_HANDLE, &RLVK.imageIndex);
    }

    vkResetFences(RLVK.device, 1, &RLVK.inFlightFences[RLVK.currentFrame]);
    vkResetCommandBuffer(RLVK.commandBuffers[RLVK.currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(RLVK.commandBuffers[RLVK.currentFrame], &beginInfo);

    VkClearValue clearColor = { .color = {{ RLVK_STATE.clearR, RLVK_STATE.clearG, RLVK_STATE.clearB, RLVK_STATE.clearA }} };

    VkRenderPassBeginInfo rpBegin = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = RLVK.renderPass,
        .framebuffer = RLVK.framebuffers[RLVK.imageIndex],
        .renderArea = { .offset = {0, 0}, .extent = RLVK.swapchainExtent },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass(RLVK.commandBuffers[RLVK.currentFrame], &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    RLVK.frameBegun = true;
}

// End frame — end render pass, submit command buffer, present
void rlvkEndFrame(void)
{
    if (!RLVK.frameBegun) return;

    VkCommandBuffer cmd = RLVK.commandBuffers[RLVK.currentFrame];

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkSemaphore waitSems[] = { RLVK.imageAvailableSemaphores[RLVK.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSems[] = { RLVK.renderFinishedSemaphores[RLVK.imageIndex] };

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1, .pWaitSemaphores = waitSems, .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1, .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1, .pSignalSemaphores = signalSems,
    };

    vkQueueSubmit(RLVK.graphicsQueue, 1, &submitInfo, RLVK.inFlightFences[RLVK.currentFrame]);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1, .pWaitSemaphores = signalSems,
        .swapchainCount = 1, .pSwapchains = &RLVK.swapchain,
        .pImageIndices = &RLVK.imageIndex,
    };

    VkResult result = vkQueuePresentKHR(RLVK.presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || RLVK.framebufferResized) {
        RLVK.framebufferResized = false;
        rlvkRecreateSwapchain();
    }

    RLVK.currentFrame = (RLVK.currentFrame + 1) % RLVK_MAX_FRAMES_IN_FLIGHT;
    RLVK.frameBegun = false;
}

//----------------------------------------------------------------------------------
// rlgl API implementation — Phase 1: Init, Close, Clear, Matrix, Batch stubs
//----------------------------------------------------------------------------------

RLAPI void rlglInit(int width, int height)
{
    RLVK_STATE.framebufferWidth = width;
    RLVK_STATE.framebufferHeight = height;

    // Create swapchain and rendering resources
    rlvkCreateSwapchain(width, height);

    // Command pool and buffers
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = RLVK.graphicsFamily,
    };
    vkCreateCommandPool(RLVK.device, &poolInfo, NULL, &RLVK.commandPool);

    VkCommandBufferAllocateInfo cbAlloc = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = RLVK.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = RLVK_MAX_FRAMES_IN_FLIGHT,
    };
    vkAllocateCommandBuffers(RLVK.device, &cbAlloc, RLVK.commandBuffers);

    // Sync objects
    VkSemaphoreCreateInfo semInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (int i = 0; i < RLVK_MAX_FRAMES_IN_FLIGHT; i++) {
        vkCreateSemaphore(RLVK.device, &semInfo, NULL, &RLVK.imageAvailableSemaphores[i]);
        vkCreateFence(RLVK.device, &fenceInfo, NULL, &RLVK.inFlightFences[i]);
    }

    // Initialize matrix state
    RLVK_STATE.modelview = rlvkMatrixIdentity();
    RLVK_STATE.projection = rlvkMatrixIdentity();
    RLVK_STATE.transform = rlvkMatrixIdentity();
    RLVK_STATE.currentMatrix = &RLVK_STATE.modelview;
    RLVK_STATE.currentMatrixMode = RL_MODELVIEW;
    RLVK_STATE.stackCounter = 0;

    // Default cull distances
    RLVK_STATE.cullNear = RL_CULL_DISTANCE_NEAR;
    RLVK_STATE.cullFar = RL_CULL_DISTANCE_FAR;

    // Default vertex color (white)
    RLVK_STATE.colorr = 255;
    RLVK_STATE.colorg = 255;
    RLVK_STATE.colorb = 255;
    RLVK_STATE.colora = 255;

    // Get memory properties for buffer allocation
    vkGetPhysicalDeviceMemoryProperties(RLVK.physicalDevice, &RLVK.memProperties);

    // Create Vulkan resources: default texture, descriptors, pipeline, vertex buffers
    rlvkCreateDefaultTexture();
    rlvkCreateDescriptors();
    rlvkCreatePipeline();
    rlvkCreateVertexBuffers();

    // Default texture/shader IDs (placeholder)
    RLVK_STATE.defaultTextureId = 1;
    RLVK_STATE.currentTextureId = 1;
    RLVK_STATE.defaultShaderId = 1;
    RLVK_STATE.currentShaderId = 1;
    RLVK_STATE.defaultShaderLocs = (int *)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));
    RLVK_STATE.currentShaderLocs = RLVK_STATE.defaultShaderLocs;

    // Initialize default render batch
    RLVK_STATE.defaultBatch = rlLoadRenderBatch(RL_DEFAULT_BATCH_BUFFERS, RL_DEFAULT_BATCH_BUFFER_ELEMENTS);
    RLVK_STATE.currentBatch = &RLVK_STATE.defaultBatch;

    TRACELOG(RL_LOG_INFO, "VULKAN: rlgl initialized (%dx%d)", width, height);
}

RLAPI void rlglClose(void)
{
    vkDeviceWaitIdle(RLVK.device);

    // Phase 4: release any shader resources still in flight.
    for (int i = 1; i < RLVK_MAX_SHADERS; i++) {
        if (RLVK_STATE.shaders[i].in_use) rlUnloadShaderProgram((unsigned int)i);
    }

    rlUnloadRenderBatch(RLVK_STATE.defaultBatch);
    RL_FREE(RLVK_STATE.defaultShaderLocs);

    // Destroy vertex buffers
    for (int f = 0; f < RLVK_MAX_FRAMES_IN_FLIGHT; f++) {
        for (int b = 0; b < 4; b++) {
            vkUnmapMemory(RLVK.device, RLVK.vertexMemory[f][b]);
            vkDestroyBuffer(RLVK.device, RLVK.vertexBuffers[f][b], NULL);
            vkFreeMemory(RLVK.device, RLVK.vertexMemory[f][b], NULL);
        }
    }
    vkDestroyBuffer(RLVK.device, RLVK.indexBuffer, NULL);
    vkFreeMemory(RLVK.device, RLVK.indexMemory, NULL);

    // Destroy pipeline
    vkDestroyPipeline(RLVK.device, RLVK.pipelineTriangles, NULL);
    vkDestroyPipeline(RLVK.device, RLVK.pipelineLines, NULL);
    vkDestroyPipelineLayout(RLVK.device, RLVK.pipelineLayout, NULL);

    // Destroy descriptors
    vkDestroyDescriptorPool(RLVK.device, RLVK.descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(RLVK.device, RLVK.descriptorSetLayout, NULL);

    // Destroy default texture
    vkDestroySampler(RLVK.device, RLVK.defaultSampler, NULL);
    vkDestroyImageView(RLVK.device, RLVK.defaultTexView, NULL);
    vkDestroyImage(RLVK.device, RLVK.defaultTexImage, NULL);
    vkFreeMemory(RLVK.device, RLVK.defaultTexMemory, NULL);

    // Destroy sync objects
    for (int i = 0; i < RLVK_MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(RLVK.device, RLVK.imageAvailableSemaphores[i], NULL);
        vkDestroyFence(RLVK.device, RLVK.inFlightFences[i], NULL);
    }

    vkDestroyCommandPool(RLVK.device, RLVK.commandPool, NULL);
    rlvkCleanupSwapchain();
    vkDestroyDevice(RLVK.device, NULL);

#if !defined(NDEBUG)
    if (RLVK.debugMessenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT destroyDebug =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(RLVK.instance, "vkDestroyDebugUtilsMessengerEXT");
        if (destroyDebug) destroyDebug(RLVK.instance, RLVK.debugMessenger, NULL);
    }
#endif

    vkDestroySurfaceKHR(RLVK.instance, RLVK.surface, NULL);
    vkDestroyInstance(RLVK.instance, NULL);

    TRACELOG(RL_LOG_INFO, "VULKAN: rlgl closed");
}

RLAPI void rlLoadExtensions(void *loader) { (void)loader; /* Not needed for Vulkan */ }
RLAPI void *rlGetProcAddress(const char *procName) { (void)procName; return NULL; }
RLAPI int rlGetVersion(void) { return RL_OPENGL_33; } // Report as GL3.3 for compatibility checks

RLAPI void rlSetFramebufferWidth(int width) { RLVK_STATE.framebufferWidth = width; }
RLAPI int rlGetFramebufferWidth(void) { return RLVK_STATE.framebufferWidth; }
RLAPI void rlSetFramebufferHeight(int height) { RLVK_STATE.framebufferHeight = height; }
RLAPI int rlGetFramebufferHeight(void) { return RLVK_STATE.framebufferHeight; }

RLAPI unsigned int rlGetTextureIdDefault(void) { return RLVK_STATE.defaultTextureId; }
RLAPI unsigned int rlGetShaderIdDefault(void) { return RLVK_STATE.defaultShaderId; }
RLAPI int *rlGetShaderLocsDefault(void) { return RLVK_STATE.defaultShaderLocs; }

// Clear color
RLAPI void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    RLVK_STATE.clearR = r / 255.0f;
    RLVK_STATE.clearG = g / 255.0f;
    RLVK_STATE.clearB = b / 255.0f;
    RLVK_STATE.clearA = a / 255.0f;
}

RLAPI void rlClearScreenBuffers(void)
{
    // In Vulkan, clearing happens via the render pass load op.
    // The clear color is set in rlvkBeginFrame() using RLVK_STATE.clear* values.
}

RLAPI void rlCheckErrors(void) { /* Validation layers handle this */ }

//----------------------------------------------------------------------------------
// Matrix operations (pure CPU math, identical to rlgl)
//----------------------------------------------------------------------------------

RLAPI void rlMatrixMode(int mode) {
    if (mode == RL_PROJECTION) RLVK_STATE.currentMatrix = &RLVK_STATE.projection;
    else if (mode == RL_MODELVIEW) RLVK_STATE.currentMatrix = &RLVK_STATE.modelview;
    RLVK_STATE.currentMatrixMode = mode;
}

RLAPI void rlPushMatrix(void) {
    if (RLVK_STATE.stackCounter >= RL_MAX_MATRIX_STACK_SIZE) {
        TRACELOG(RL_LOG_ERROR, "RLVK: Matrix stack overflow (MAX=%d)", RL_MAX_MATRIX_STACK_SIZE);
        return;
    }
    if (RLVK_STATE.currentMatrixMode == RL_MODELVIEW) {
        RLVK_STATE.transformRequired = true;
        RLVK_STATE.currentMatrix = &RLVK_STATE.transform;
    }
    RLVK_STATE.stack[RLVK_STATE.stackCounter] = *RLVK_STATE.currentMatrix;
    RLVK_STATE.stackCounter++;
}

RLAPI void rlPopMatrix(void) {
    if (RLVK_STATE.stackCounter > 0) {
        Matrix mat = RLVK_STATE.stack[RLVK_STATE.stackCounter - 1];
        *RLVK_STATE.currentMatrix = mat;
        RLVK_STATE.stackCounter--;
    }
    if ((RLVK_STATE.stackCounter == 0) && (RLVK_STATE.currentMatrixMode == RL_MODELVIEW)) {
        RLVK_STATE.currentMatrix = &RLVK_STATE.modelview;
        RLVK_STATE.transformRequired = false;
    }
}

RLAPI void rlLoadIdentity(void) { *RLVK_STATE.currentMatrix = rlvkMatrixIdentity(); }

RLAPI void rlTranslatef(float x, float y, float z) {
    Matrix matTranslation = {
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    *RLVK_STATE.currentMatrix = rlvkMatrixMultiply(*RLVK_STATE.currentMatrix, matTranslation);
}

RLAPI void rlRotatef(float angle, float x, float y, float z) {
    Matrix matRotation = rlvkMatrixIdentity();
    float lengthSquared = x*x + y*y + z*z;
    if ((lengthSquared != 1.0f) && (lengthSquared != 0.0f)) {
        float ilength = 1.0f/sqrtf(lengthSquared);
        x *= ilength; y *= ilength; z *= ilength;
    }
    float sinres = sinf(angle*0.0174532925f);
    float cosres = cosf(angle*0.0174532925f);
    float t = 1.0f - cosres;

    matRotation.m0 = x*x*t + cosres;     matRotation.m4 = y*x*t - z*sinres;  matRotation.m8 = z*x*t + y*sinres;
    matRotation.m1 = x*y*t + z*sinres;   matRotation.m5 = y*y*t + cosres;    matRotation.m9 = z*y*t - x*sinres;
    matRotation.m2 = x*z*t - y*sinres;   matRotation.m6 = y*z*t + x*sinres;  matRotation.m10 = z*z*t + cosres;

    *RLVK_STATE.currentMatrix = rlvkMatrixMultiply(*RLVK_STATE.currentMatrix, matRotation);
}

RLAPI void rlScalef(float x, float y, float z) {
    Matrix matScale = {
        x, 0.0f, 0.0f, 0.0f,
        0.0f, y, 0.0f, 0.0f,
        0.0f, 0.0f, z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    *RLVK_STATE.currentMatrix = rlvkMatrixMultiply(*RLVK_STATE.currentMatrix, matScale);
}

RLAPI void rlMultMatrixf(const float *matf) {
    Matrix mat = {
        matf[0], matf[4], matf[8], matf[12],
        matf[1], matf[5], matf[9], matf[13],
        matf[2], matf[6], matf[10], matf[14],
        matf[3], matf[7], matf[11], matf[15]
    };
    *RLVK_STATE.currentMatrix = rlvkMatrixMultiply(*RLVK_STATE.currentMatrix, mat);
}

RLAPI void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar) {
    Matrix matFrustum = { 0 };
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(zfar - znear);
    matFrustum.m0 = ((float)znear*2.0f)/rl;
    matFrustum.m5 = ((float)znear*2.0f)/tb;
    matFrustum.m8 = ((float)right + (float)left)/rl;
    matFrustum.m9 = ((float)top + (float)bottom)/tb;
    matFrustum.m10 = -((float)zfar + (float)znear)/fn;
    matFrustum.m11 = -1.0f;
    matFrustum.m14 = -((float)zfar*(float)znear*2.0f)/fn;
    *RLVK_STATE.currentMatrix = rlvkMatrixMultiply(*RLVK_STATE.currentMatrix, matFrustum);
}

RLAPI void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar) {
    // Vulkan ortho projection:
    //   Y-flip: Vulkan NDC Y goes top-to-bottom (opposite of OpenGL)
    //   Z-remap: Vulkan NDC Z range is [0,1] (OpenGL is [-1,1])
    Matrix matOrtho = { 0 };
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(zfar - znear);
    matOrtho.m0 = 2.0f/rl;
    matOrtho.m5 = -2.0f/tb;                            // Negated for Vulkan Y-flip
    matOrtho.m10 = -1.0f/fn;                            // [0,1] depth range instead of OpenGL [-1,1]
    matOrtho.m12 = -((float)left + (float)right)/rl;
    matOrtho.m13 = ((float)top + (float)bottom)/tb;     // Negated for Vulkan Y-flip
    matOrtho.m14 = -(float)znear/fn;                     // [0,1] depth range offset
    matOrtho.m15 = 1.0f;
    *RLVK_STATE.currentMatrix = rlvkMatrixMultiply(*RLVK_STATE.currentMatrix, matOrtho);
}

RLAPI void rlViewport(int x, int y, int width, int height) { (void)x; (void)y; (void)width; (void)height; }

RLAPI void rlSetClipPlanes(double nearPlane, double farPlane) {
    RLVK_STATE.cullNear = nearPlane;
    RLVK_STATE.cullFar = farPlane;
}
RLAPI double rlGetCullDistanceNear(void) { return RLVK_STATE.cullNear; }
RLAPI double rlGetCullDistanceFar(void) { return RLVK_STATE.cullFar; }

// Matrix getters/setters
RLAPI Matrix rlGetMatrixModelview(void) { return RLVK_STATE.modelview; }
RLAPI Matrix rlGetMatrixProjection(void) { return RLVK_STATE.projection; }
RLAPI Matrix rlGetMatrixTransform(void) { return RLVK_STATE.transform; }
RLAPI Matrix rlGetMatrixProjectionStereo(int eye) { (void)eye; return rlvkMatrixIdentity(); }
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye) { (void)eye; return rlvkMatrixIdentity(); }
RLAPI void rlSetMatrixProjection(Matrix proj) { RLVK_STATE.projection = proj; }
RLAPI void rlSetMatrixModelview(Matrix view) { RLVK_STATE.modelview = view; }
RLAPI void rlSetMatrixProjectionStereo(Matrix right, Matrix left) { (void)right; (void)left; }
RLAPI void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left) { (void)right; (void)left; }

//----------------------------------------------------------------------------------
// Render batch management
//----------------------------------------------------------------------------------

RLAPI rlRenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements)
{
    rlRenderBatch batch = { 0 };
    batch.bufferCount = numBuffers;
    batch.currentBuffer = 0;
    batch.vertexBuffer = (rlVertexBuffer *)RL_CALLOC(numBuffers, sizeof(rlVertexBuffer));

    for (int i = 0; i < numBuffers; i++) {
        batch.vertexBuffer[i].elementCount = bufferElements;
        batch.vertexBuffer[i].vertices = (float *)RL_CALLOC(bufferElements*4*3, sizeof(float));    // 3 floats per vertex, 4 vertices per quad
        batch.vertexBuffer[i].texcoords = (float *)RL_CALLOC(bufferElements*4*2, sizeof(float));
        batch.vertexBuffer[i].normals = (float *)RL_CALLOC(bufferElements*4*3, sizeof(float));
        batch.vertexBuffer[i].colors = (unsigned char *)RL_CALLOC(bufferElements*4*4, sizeof(unsigned char));
        batch.vertexBuffer[i].indices = (unsigned int *)RL_CALLOC(bufferElements*6, sizeof(unsigned int));

        // Init quad indices
        for (int j = 0; j < bufferElements; j++) {
            batch.vertexBuffer[i].indices[j*6 + 0] = j*4 + 0;
            batch.vertexBuffer[i].indices[j*6 + 1] = j*4 + 1;
            batch.vertexBuffer[i].indices[j*6 + 2] = j*4 + 2;
            batch.vertexBuffer[i].indices[j*6 + 3] = j*4 + 0;
            batch.vertexBuffer[i].indices[j*6 + 4] = j*4 + 2;
            batch.vertexBuffer[i].indices[j*6 + 5] = j*4 + 3;
        }
    }

    batch.draws = (rlDrawCall *)RL_CALLOC(RL_DEFAULT_BATCH_DRAWCALLS, sizeof(rlDrawCall));
    batch.drawCounter = 1;
    batch.draws[0].mode = RL_QUADS;
    batch.draws[0].vertexCount = 0;
    batch.draws[0].textureId = RLVK_STATE.defaultTextureId;
    batch.currentDepth = 0.0f;

    return batch;
}

RLAPI void rlUnloadRenderBatch(rlRenderBatch batch)
{
    for (int i = 0; i < batch.bufferCount; i++) {
        RL_FREE(batch.vertexBuffer[i].vertices);
        RL_FREE(batch.vertexBuffer[i].texcoords);
        RL_FREE(batch.vertexBuffer[i].normals);
        RL_FREE(batch.vertexBuffer[i].colors);
        RL_FREE(batch.vertexBuffer[i].indices);
    }
    RL_FREE(batch.vertexBuffer);
    RL_FREE(batch.draws);
}

RLAPI void rlDrawRenderBatch(rlRenderBatch *batch)
{
    if (RLVK_STATE.vertexCounter > 0 && RLVK.frameBegun)
    {
        VkCommandBuffer cmd = RLVK.commandBuffers[RLVK.currentFrame];
        int frame = (int)RLVK.currentFrame;

        // Upload vertex data to Vulkan buffers
        rlVertexBuffer *vb = &batch->vertexBuffer[batch->currentBuffer];
        int vertexCount = RLVK_STATE.vertexCounter;

        memcpy(RLVK.vertexMapped[frame][0], vb->vertices, vertexCount * 3 * sizeof(float));
        memcpy(RLVK.vertexMapped[frame][1], vb->texcoords, vertexCount * 2 * sizeof(float));
        memcpy(RLVK.vertexMapped[frame][2], vb->normals, vertexCount * 3 * sizeof(float));
        memcpy(RLVK.vertexMapped[frame][3], vb->colors, vertexCount * 4 * sizeof(unsigned char));

        // Set viewport and scissor
        VkViewport viewport = {
            0.0f, 0.0f,
            (float)RLVK.swapchainExtent.width, (float)RLVK.swapchainExtent.height,
            0.0f, 1.0f,
        };
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor = { {0, 0}, RLVK.swapchainExtent };
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        // Bind vertex buffers
        VkBuffer vkBuffers[4] = {
            RLVK.vertexBuffers[frame][0],
            RLVK.vertexBuffers[frame][1],
            RLVK.vertexBuffers[frame][2],
            RLVK.vertexBuffers[frame][3],
        };
        VkDeviceSize offsets[4] = { 0, 0, 0, 0 };
        vkCmdBindVertexBuffers(cmd, 0, 4, vkBuffers, offsets);

        // Compute MVP matrix: projection * modelview
        Matrix matMVP = rlvkMatrixMultiply(RLVK_STATE.modelview, RLVK_STATE.projection);

        // Push constants
        rlvkPushConstants pc;
        // Copy matrix in column-major order (GLSL mat4 convention)
        // Matrix struct: m0,m4,m8,m12 / m1,m5,m9,m13 / ... (row-major in memory)
        // GLSL expects: m0,m1,m2,m3 / m4,m5,m6,m7 / ... (column-major)
        pc.mvp[0]  = matMVP.m0;  pc.mvp[1]  = matMVP.m1;  pc.mvp[2]  = matMVP.m2;  pc.mvp[3]  = matMVP.m3;
        pc.mvp[4]  = matMVP.m4;  pc.mvp[5]  = matMVP.m5;  pc.mvp[6]  = matMVP.m6;  pc.mvp[7]  = matMVP.m7;
        pc.mvp[8]  = matMVP.m8;  pc.mvp[9]  = matMVP.m9;  pc.mvp[10] = matMVP.m10; pc.mvp[11] = matMVP.m11;
        pc.mvp[12] = matMVP.m12; pc.mvp[13] = matMVP.m13; pc.mvp[14] = matMVP.m14; pc.mvp[15] = matMVP.m15;
        // Default diffuse color: white
        pc.colDiffuse[0] = 1.0f;
        pc.colDiffuse[1] = 1.0f;
        pc.colDiffuse[2] = 1.0f;
        pc.colDiffuse[3] = 1.0f;

        vkCmdPushConstants(cmd, RLVK.pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(rlvkPushConstants), &pc);

        // Iterate draw calls
        int vertexOffset = 0;
        unsigned int lastBoundTex = 0;  // Track to avoid redundant descriptor binds
        for (int i = 0; i < batch->drawCounter; i++)
        {
            int mode = batch->draws[i].mode;
            int count = batch->draws[i].vertexCount;
            if (count <= 0) continue;

            // Bind texture descriptor set for this draw call
            unsigned int texId = batch->draws[i].textureId;
            if (texId == 0) texId = RLVK_STATE.defaultTextureId;
            if (texId != lastBoundTex) {
                VkDescriptorSet ds = RLVK.defaultTexDescriptor;
                if (texId < RLVK_MAX_TEXTURES && rlvkTextures[texId].active)
                    ds = rlvkTextures[texId].descriptorSet;
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, RLVK.pipelineLayout,
                    0, 1, &ds, 0, NULL);
                lastBoundTex = texId;
            }

            if (mode == RL_LINES) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, RLVK.pipelineLines);
                vkCmdDraw(cmd, count, 1, vertexOffset, 0);
            }
            else if (mode == RL_TRIANGLES) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, RLVK.pipelineTriangles);
                vkCmdDraw(cmd, count, 1, vertexOffset, 0);
            }
            else if (mode == RL_QUADS) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, RLVK.pipelineTriangles);
                vkCmdBindIndexBuffer(cmd, RLVK.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                // For quads: every 4 vertices become 6 indices (2 triangles)
                // Use firstIndex=0, vertexOffset=base so indices 0,1,2,0,2,3... are offset by base
                int indexCount = (count / 4) * 6;
                vkCmdDrawIndexed(cmd, indexCount, 1, 0, vertexOffset, 0);
            }

            vertexOffset += count;
        }
    }

    // Reset batch — zero EVERY draw call we used this frame, not just slot 0.
    // Otherwise draws[1..drawCounter-1].vertexCount keeps accumulating across
    // frames; eventually the index-count for a quads draw exceeds the index
    // buffer and Vulkan reads past the bound buffer, corrupting glyph quads
    // beyond the first one in any post-shapes text run.
    for (int i = 0; i < batch->drawCounter; i++) {
        batch->draws[i].vertexCount = 0;
        batch->draws[i].vertexAlignment = 0;
    }
    RLVK_STATE.vertexCounter = 0;
    batch->currentDepth = 0.0f;
    batch->drawCounter = 1;
    batch->draws[0].mode = RL_QUADS;
    batch->draws[0].textureId = RLVK_STATE.defaultTextureId;
    batch->currentBuffer++;
    if (batch->currentBuffer >= batch->bufferCount) batch->currentBuffer = 0;
}

RLAPI void rlSetRenderBatchActive(rlRenderBatch *batch) {
    rlDrawRenderBatchActive();
    RLVK_STATE.currentBatch = (batch != NULL) ? batch : &RLVK_STATE.defaultBatch;
}

RLAPI void rlDrawRenderBatchActive(void) {
    rlDrawRenderBatch(RLVK_STATE.currentBatch);
}

RLAPI bool rlCheckRenderBatchLimit(int vCount) {
    bool overflow = false;
    if ((RLVK_STATE.vertexCounter + vCount) >= (RLVK_STATE.currentBatch->vertexBuffer[RLVK_STATE.currentBatch->currentBuffer].elementCount*4)) {
        overflow = true;
        int currentMode = RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].mode;
        int currentTexture = RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].textureId;
        rlDrawRenderBatch(RLVK_STATE.currentBatch);
        RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].mode = currentMode;
        RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].textureId = currentTexture;
    }
    return overflow;
}

RLAPI void rlSetTexture(unsigned int id) {
    if (id == 0) id = RLVK_STATE.defaultTextureId;
    if (RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].textureId != id) {
        if (RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexCount > 0) {
            if (RLVK_STATE.currentBatch->drawCounter >= RL_DEFAULT_BATCH_DRAWCALLS)
                rlDrawRenderBatch(RLVK_STATE.currentBatch);
            RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexAlignment =
                RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexCount;
            RLVK_STATE.currentBatch->drawCounter++;
        }
        if (RLVK_STATE.currentBatch->drawCounter > 0)
            RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].textureId = id;
    }
}

//----------------------------------------------------------------------------------
// Vertex level operations (accumulate into batch CPU buffers)
//----------------------------------------------------------------------------------

RLAPI void rlBegin(int mode) {
    if (RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].mode != mode) {
        if (RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexCount > 0) {
            if (RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].mode == RL_LINES)
                RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexAlignment =
                    RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexCount;
            else if (RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].mode == RL_TRIANGLES)
                RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexAlignment =
                    RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexCount;
            else if (RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].mode == RL_QUADS)
                RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexAlignment =
                    RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexCount;

            if (RLVK_STATE.currentBatch->drawCounter >= RL_DEFAULT_BATCH_DRAWCALLS)
                rlDrawRenderBatch(RLVK_STATE.currentBatch);
            RLVK_STATE.currentBatch->drawCounter++;
        }
        if (RLVK_STATE.currentBatch->drawCounter > 0)
            RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].mode = mode;
    }
}

RLAPI void rlEnd(void) {
    // Vertex alignment for lines/triangles within quads batch
}

RLAPI void rlVertex2i(int x, int y) { rlVertex3f((float)x, (float)y, RLVK_STATE.currentBatch->currentDepth); }
RLAPI void rlVertex2f(float x, float y) { rlVertex3f(x, y, RLVK_STATE.currentBatch->currentDepth); }

RLAPI void rlVertex3f(float x, float y, float z) {
    if (RLVK_STATE.vertexCounter >= (RLVK_STATE.currentBatch->vertexBuffer[RLVK_STATE.currentBatch->currentBuffer].elementCount*4)) return;

    float tx = x, ty = y, tz = z;
    if (RLVK_STATE.transformRequired) {
        tx = RLVK_STATE.transform.m0*x + RLVK_STATE.transform.m4*y + RLVK_STATE.transform.m8*z + RLVK_STATE.transform.m12;
        ty = RLVK_STATE.transform.m1*x + RLVK_STATE.transform.m5*y + RLVK_STATE.transform.m9*z + RLVK_STATE.transform.m13;
        tz = RLVK_STATE.transform.m2*x + RLVK_STATE.transform.m6*y + RLVK_STATE.transform.m10*z + RLVK_STATE.transform.m14;
    }

    int idx = RLVK_STATE.vertexCounter;
    rlVertexBuffer *vb = &RLVK_STATE.currentBatch->vertexBuffer[RLVK_STATE.currentBatch->currentBuffer];

    vb->vertices[3*idx + 0] = tx;
    vb->vertices[3*idx + 1] = ty;
    vb->vertices[3*idx + 2] = tz;

    vb->texcoords[2*idx + 0] = RLVK_STATE.texcoordx;
    vb->texcoords[2*idx + 1] = RLVK_STATE.texcoordy;

    vb->normals[3*idx + 0] = RLVK_STATE.normalx;
    vb->normals[3*idx + 1] = RLVK_STATE.normaly;
    vb->normals[3*idx + 2] = RLVK_STATE.normalz;

    vb->colors[4*idx + 0] = RLVK_STATE.colorr;
    vb->colors[4*idx + 1] = RLVK_STATE.colorg;
    vb->colors[4*idx + 2] = RLVK_STATE.colorb;
    vb->colors[4*idx + 3] = RLVK_STATE.colora;

    RLVK_STATE.vertexCounter++;
    RLVK_STATE.currentBatch->draws[RLVK_STATE.currentBatch->drawCounter - 1].vertexCount++;
}

RLAPI void rlTexCoord2f(float x, float y) { RLVK_STATE.texcoordx = x; RLVK_STATE.texcoordy = y; }
RLAPI void rlNormal3f(float x, float y, float z) { RLVK_STATE.normalx = x; RLVK_STATE.normaly = y; RLVK_STATE.normalz = z; }
RLAPI void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    RLVK_STATE.colorr = r; RLVK_STATE.colorg = g; RLVK_STATE.colorb = b; RLVK_STATE.colora = a;
}
RLAPI void rlColor3f(float x, float y, float z) {
    rlColor4ub((unsigned char)(x*255), (unsigned char)(y*255), (unsigned char)(z*255), 255);
}
RLAPI void rlColor4f(float x, float y, float z, float w) {
    rlColor4ub((unsigned char)(x*255), (unsigned char)(y*255), (unsigned char)(z*255), (unsigned char)(w*255));
}

//----------------------------------------------------------------------------------
// Stub implementations (Phase 2+ will fill these in)
//----------------------------------------------------------------------------------

// Vertex buffer state
RLAPI bool rlEnableVertexArray(unsigned int vaoId) { (void)vaoId; return true; }
RLAPI void rlDisableVertexArray(void) { }
RLAPI void rlEnableVertexBuffer(unsigned int id) { (void)id; }
RLAPI void rlDisableVertexBuffer(void) { }
RLAPI void rlEnableVertexBufferElement(unsigned int id) { (void)id; }
RLAPI void rlDisableVertexBufferElement(void) { }
RLAPI void rlEnableVertexAttribute(unsigned int index) { (void)index; }
RLAPI void rlDisableVertexAttribute(unsigned int index) { (void)index; }
RLAPI void rlEnableStatePointer(int vertexAttribType, void *buffer) { (void)vertexAttribType; (void)buffer; }
RLAPI void rlDisableStatePointer(int vertexAttribType) { (void)vertexAttribType; }

// Texture state
RLAPI void rlActiveTextureSlot(int slot) { (void)slot; }
RLAPI void rlEnableTexture(unsigned int id) { (void)id; }
RLAPI void rlDisableTexture(void) { }
RLAPI void rlEnableTextureCubemap(unsigned int id) { (void)id; }
RLAPI void rlDisableTextureCubemap(void) { }
RLAPI void rlTextureParameters(unsigned int id, int param, int value) { (void)id; (void)param; (void)value; }
RLAPI void rlCubemapParameters(unsigned int id, int param, int value) { (void)id; (void)param; (void)value; }

// Shader state
RLAPI void rlEnableShader(unsigned int id) { (void)id; }
RLAPI void rlDisableShader(void) { }

// Framebuffer state
RLAPI void rlEnableFramebuffer(unsigned int id) { RLVK_STATE.activeFramebuffer = id; }
RLAPI void rlDisableFramebuffer(void) { RLVK_STATE.activeFramebuffer = 0; }
RLAPI unsigned int rlGetActiveFramebuffer(void) { return RLVK_STATE.activeFramebuffer; }
RLAPI void rlActiveDrawBuffers(int count) { (void)count; }
RLAPI void rlBlitFramebuffer(int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight, int bufferMask)
    { (void)srcX; (void)srcY; (void)srcWidth; (void)srcHeight; (void)dstX; (void)dstY; (void)dstWidth; (void)dstHeight; (void)bufferMask; }
RLAPI void rlBindFramebuffer(unsigned int target, unsigned int framebuffer) { (void)target; (void)framebuffer; }

// Render state
RLAPI void rlEnableColorBlend(void) { }
RLAPI void rlDisableColorBlend(void) { }
RLAPI void rlEnableDepthTest(void) { }
RLAPI void rlDisableDepthTest(void) { }
RLAPI void rlEnableDepthMask(void) { }
RLAPI void rlDisableDepthMask(void) { }
RLAPI void rlEnableBackfaceCulling(void) { }
RLAPI void rlDisableBackfaceCulling(void) { }
RLAPI void rlColorMask(bool r, bool g, bool b, bool a) { (void)r; (void)g; (void)b; (void)a; }
RLAPI void rlSetCullFace(int mode) { (void)mode; }
RLAPI void rlEnableScissorTest(void) { }
RLAPI void rlDisableScissorTest(void) { }
RLAPI void rlScissor(int x, int y, int width, int height) { (void)x; (void)y; (void)width; (void)height; }
RLAPI void rlEnablePointMode(void) { }
RLAPI void rlDisablePointMode(void) { }
RLAPI void rlSetPointSize(float size) { (void)size; }
RLAPI float rlGetPointSize(void) { return 1.0f; }
RLAPI void rlEnableWireMode(void) { }
RLAPI void rlDisableWireMode(void) { }
RLAPI void rlSetLineWidth(float width) { (void)width; }
RLAPI float rlGetLineWidth(void) { return 1.0f; }
RLAPI void rlEnableSmoothLines(void) { }
RLAPI void rlDisableSmoothLines(void) { }
RLAPI void rlEnableStereoRender(void) { }
RLAPI void rlDisableStereoRender(void) { }
RLAPI bool rlIsStereoRenderEnabled(void) { return false; }
RLAPI void rlSetBlendMode(int mode) { (void)mode; }
RLAPI void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation) { (void)glSrcFactor; (void)glDstFactor; (void)glEquation; }
RLAPI void rlSetBlendFactorsSeparate(int glSrcRGB, int glDstRGB, int glSrcAlpha, int glDstAlpha, int glEqRGB, int glEqAlpha)
    { (void)glSrcRGB; (void)glDstRGB; (void)glSrcAlpha; (void)glDstAlpha; (void)glEqRGB; (void)glEqAlpha; }

// Vertex buffer management
RLAPI unsigned int rlLoadVertexArray(void) { return 0; }
RLAPI unsigned int rlLoadVertexBuffer(const void *buffer, int size, bool dynamic) { (void)buffer; (void)size; (void)dynamic; return 0; }
RLAPI unsigned int rlLoadVertexBufferElement(const void *buffer, int size, bool dynamic) { (void)buffer; (void)size; (void)dynamic; return 0; }
RLAPI void rlUpdateVertexBuffer(unsigned int bufferId, const void *data, int dataSize, int offset) { (void)bufferId; (void)data; (void)dataSize; (void)offset; }
RLAPI void rlUpdateVertexBufferElements(unsigned int id, const void *data, int dataSize, int offset) { (void)id; (void)data; (void)dataSize; (void)offset; }
RLAPI void rlUnloadVertexArray(unsigned int vaoId) { (void)vaoId; }
RLAPI void rlUnloadVertexBuffer(unsigned int vboId) { (void)vboId; }
RLAPI void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, int offset) { (void)index; (void)compSize; (void)type; (void)normalized; (void)stride; (void)offset; }
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor) { (void)index; (void)divisor; }
RLAPI void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count) { (void)locIndex; (void)value; (void)attribType; (void)count; }
RLAPI void rlDrawVertexArray(int offset, int count) { (void)offset; (void)count; }
RLAPI void rlDrawVertexArrayElements(int offset, int count, const void *buffer) { (void)offset; (void)count; (void)buffer; }
RLAPI void rlDrawVertexArrayInstanced(int offset, int count, int instances) { (void)offset; (void)count; (void)instances; }
RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, const void *buffer, int instances) { (void)offset; (void)count; (void)buffer; (void)instances; }

// Texture management
RLAPI unsigned int rlLoadTexture(const void *data, int width, int height, int format, int mipmapCount)
{
    (void)mipmapCount;

    if (data == NULL || width <= 0 || height <= 0) return 0;
    if (rlvkTextureCount >= RLVK_MAX_TEXTURES) {
        TRACELOG(RL_LOG_ERROR, "VULKAN: Texture registry full (max=%d)", RLVK_MAX_TEXTURES);
        return RLVK_STATE.defaultTextureId;
    }

    unsigned int id = rlvkTextureCount++;
    rlvkTexture *tex = &rlvkTextures[id];
    VkFormat vkFormat = rlvkGetVulkanFormat(format);
    int pixelSize = rlvkGetPixelSize(format);
    VkDeviceSize dataSize = (VkDeviceSize)width * height * pixelSize;

    // Create image
    VkImageCreateInfo imgInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = vkFormat,
        .extent = { (uint32_t)width, (uint32_t)height, 1 },
        .mipLevels = 1, .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VkResult result = vkCreateImage(RLVK.device, &imgInfo, NULL, &tex->image);
    if (result != VK_SUCCESS) {
        TRACELOG(RL_LOG_ERROR, "VULKAN: Failed to create image (%d) format=%d %dx%d", result, format, width, height);
        rlvkTextureCount--;
        return RLVK_STATE.defaultTextureId;
    }

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(RLVK.device, tex->image, &memReq);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReq.size,
        .memoryTypeIndex = rlvkFindMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    vkAllocateMemory(RLVK.device, &allocInfo, NULL, &tex->memory);
    vkBindImageMemory(RLVK.device, tex->image, tex->memory, 0);

    // Upload via staging buffer
    VkBuffer staging; VkDeviceMemory stagingMem;
    rlvkCreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging, &stagingMem);
    void *mapped;
    vkMapMemory(RLVK.device, stagingMem, 0, dataSize, 0, &mapped);
    memcpy(mapped, data, dataSize);
    vkUnmapMemory(RLVK.device, stagingMem);

    VkCommandBuffer cmd = rlvkBeginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = tex->image,
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
        0, NULL, 0, NULL, 1, &barrier);

    VkBufferImageCopy region = {
        .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        .imageExtent = { (uint32_t)width, (uint32_t)height, 1 },
    };
    vkCmdCopyBufferToImage(cmd, staging, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, NULL, 0, NULL, 1, &barrier);

    rlvkEndSingleTimeCommands(cmd);
    vkDestroyBuffer(RLVK.device, staging, NULL);
    vkFreeMemory(RLVK.device, stagingMem, NULL);

    // Image view with swizzle for special formats
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = tex->image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = vkFormat,
        .components = rlvkGetSwizzle(format),
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
    };
    vkCreateImageView(RLVK.device, &viewInfo, NULL, &tex->view);

    // Sampler — default to NEAREST to match raylib's OpenGL backend (bitmap
    // fonts and pixel-art textures rely on point sampling; users opt into
    // bilinear via SetTextureFilter()).
    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    };
    vkCreateSampler(RLVK.device, &samplerInfo, NULL, &tex->sampler);

    // Descriptor set
    tex->descriptorSet = rlvkAllocTextureDescriptor(tex->view, tex->sampler);

    tex->active = true;
    tex->width = width;
    tex->height = height;
    tex->format = format;

    TRACELOG(RL_LOG_INFO, "VULKAN: Texture loaded id=%u (%dx%d fmt=%d)", id, width, height, format);
    return id;
}
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer) { (void)width; (void)height; (void)useRenderBuffer; return 0; }
RLAPI unsigned int rlLoadTextureCubemap(const void *data, int size, int format, int mipmapCount) { (void)data; (void)size; (void)format; (void)mipmapCount; return 0; }
RLAPI void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data) { (void)id; (void)offsetX; (void)offsetY; (void)width; (void)height; (void)format; (void)data; }
RLAPI void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType) {
    (void)format; if (glInternalFormat) *glInternalFormat = 0; if (glFormat) *glFormat = 0; if (glType) *glType = 0;
}
RLAPI const char *rlGetPixelFormatName(unsigned int format) { (void)format; return "UNKNOWN"; }
RLAPI void rlUnloadTexture(unsigned int id) {
    if (id == 0 || id >= RLVK_MAX_TEXTURES || !rlvkTextures[id].active) return;
    if (id == RLVK_STATE.defaultTextureId) return;  // Don't unload default texture

    vkDeviceWaitIdle(RLVK.device);
    rlvkTexture *tex = &rlvkTextures[id];
    vkFreeDescriptorSets(RLVK.device, RLVK.descriptorPool, 1, &tex->descriptorSet);
    vkDestroySampler(RLVK.device, tex->sampler, NULL);
    vkDestroyImageView(RLVK.device, tex->view, NULL);
    vkDestroyImage(RLVK.device, tex->image, NULL);
    vkFreeMemory(RLVK.device, tex->memory, NULL);
    memset(tex, 0, sizeof(rlvkTexture));
}
RLAPI void rlGenTextureMipmaps(unsigned int id, int width, int height, int format, int *mipmaps) { (void)id; (void)width; (void)height; (void)format; (void)mipmaps; }
RLAPI void *rlReadTexturePixels(unsigned int id, int width, int height, int format) { (void)id; (void)width; (void)height; (void)format; return NULL; }
RLAPI unsigned char *rlReadScreenPixels(int width, int height) { (void)width; (void)height; return NULL; }

// Framebuffer management
RLAPI unsigned int rlLoadFramebuffer(void) { return 0; }
RLAPI void rlFramebufferAttach(unsigned int id, unsigned int texId, int attachType, int texType, int mipLevel) { (void)id; (void)texId; (void)attachType; (void)texType; (void)mipLevel; }
RLAPI bool rlFramebufferComplete(unsigned int id) { (void)id; return true; }
RLAPI void rlUnloadFramebuffer(unsigned int id) { (void)id; }
RLAPI void rlCopyFramebuffer(int x, int y, int width, int height, int format, void *pixels) { (void)x; (void)y; (void)width; (void)height; (void)format; (void)pixels; }
RLAPI void rlResizeFramebuffer(int width, int height) { (void)width; (void)height; }

// Shader management
RLAPI unsigned int rlLoadShader(const char *code, int type) { (void)code; (void)type; return 0; }
RLAPI unsigned int rlLoadShaderProgram(const char *vsCode, const char *fsCode) { (void)vsCode; (void)fsCode; return RLVK_STATE.defaultShaderId; }
RLAPI unsigned int rlLoadShaderProgramEx(unsigned int vsId, unsigned int fsId) { (void)vsId; (void)fsId; return RLVK_STATE.defaultShaderId; }
RLAPI unsigned int rlLoadShaderProgramCompute(unsigned int csId) { (void)csId; return 0; }
RLAPI void rlUnloadShader(unsigned int id) { (void)id; }
RLAPI void rlUnloadShaderProgram(unsigned int id)
{
    if (id == 0 || id >= RLVK_MAX_SHADERS) return;
    rlvkShader *s = &RLVK_STATE.shaders[id];
    if (!s->in_use) return;

    vkDeviceWaitIdle(RLVK.device);

    for (int i = 0; i < s->n_ubos; i++) {
        if (s->ubos[i].shared) continue;
        for (int f = 0; f < RLVK_MAX_FRAMES_IN_FLIGHT; f++) {
            if (s->ubos[i].mapped[f]) vkUnmapMemory(RLVK.device, s->ubos[i].memory[f]);
            if (s->ubos[i].buffer[f]) vkDestroyBuffer(RLVK.device, s->ubos[i].buffer[f], NULL);
            if (s->ubos[i].memory[f]) vkFreeMemory(RLVK.device, s->ubos[i].memory[f], NULL);
        }
        RL_FREE(s->ubos[i].shadow);
    }

    if (s->descriptor_pool) vkDestroyDescriptorPool(RLVK.device, s->descriptor_pool, NULL);
    for (int i = 0; i < RLVK_MAX_DESC_SETS; i++)
        if (s->set_layout_used[i]) vkDestroyDescriptorSetLayout(RLVK.device, s->set_layouts[i], NULL);
    if (s->pipeline_layout) vkDestroyPipelineLayout(RLVK.device, s->pipeline_layout, NULL);
    if (s->vs_module) vkDestroyShaderModule(RLVK.device, s->vs_module, NULL);
    if (s->fs_module) vkDestroyShaderModule(RLVK.device, s->fs_module, NULL);

    memset(s, 0, sizeof(*s));
}
RLAPI int rlGetLocationUniform(unsigned int id, const char *uniformName) { (void)id; (void)uniformName; return -1; }
RLAPI int rlGetLocationAttrib(unsigned int id, const char *attribName) { (void)id; (void)attribName; return -1; }
RLAPI void rlSetUniform(int locIndex, const void *value, int uniformType, int count) { (void)locIndex; (void)value; (void)uniformType; (void)count; }
RLAPI void rlSetUniformMatrix(int locIndex, Matrix mat) { (void)locIndex; (void)mat; }
RLAPI void rlSetUniformMatrices(int locIndex, const Matrix *mat, int count) { (void)locIndex; (void)mat; (void)count; }
RLAPI void rlSetUniformSampler(int locIndex, unsigned int textureId) { (void)locIndex; (void)textureId; }
RLAPI void rlSetShader(unsigned int id, int *locs) {
    if (id != RLVK_STATE.currentShaderId) {
        rlDrawRenderBatchActive();
        RLVK_STATE.currentShaderId = id;
        RLVK_STATE.currentShaderLocs = locs;
    }
}

// Compute
RLAPI void rlComputeShaderDispatch(unsigned int groupX, unsigned int groupY, unsigned int groupZ) { (void)groupX; (void)groupY; (void)groupZ; }

// SSBO
RLAPI unsigned int rlLoadShaderBuffer(unsigned int size, const void *data, int usageHint) { (void)size; (void)data; (void)usageHint; return 0; }
RLAPI void rlUnloadShaderBuffer(unsigned int ssboId) { (void)ssboId; }
RLAPI void rlUpdateShaderBuffer(unsigned int id, const void *data, unsigned int dataSize, unsigned int offset) { (void)id; (void)data; (void)dataSize; (void)offset; }
RLAPI void rlBindShaderBuffer(unsigned int id, unsigned int index) { (void)id; (void)index; }
RLAPI void rlReadShaderBuffer(unsigned int id, void *dest, unsigned int count, unsigned int offset) { (void)id; (void)dest; (void)count; (void)offset; }
RLAPI void rlCopyShaderBuffer(unsigned int destId, unsigned int srcId, unsigned int destOffset, unsigned int srcOffset, unsigned int count) { (void)destId; (void)srcId; (void)destOffset; (void)srcOffset; (void)count; }
RLAPI unsigned int rlGetShaderBufferSize(unsigned int id) { (void)id; return 0; }

// Image texture
RLAPI void rlBindImageTexture(unsigned int id, unsigned int index, int format, bool readonly) { (void)id; (void)index; (void)format; (void)readonly; }

// Quick helpers
RLAPI void rlLoadDrawCube(void) { }
RLAPI void rlLoadDrawQuad(void) { }

#endif // RLVK_IMPLEMENTATION
