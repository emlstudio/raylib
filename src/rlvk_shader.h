/**********************************************************************************************
*
*   rlvk_shader v1.0 - Runtime shader blob types for raylib's Vulkan backend
*
*   DESCRIPTION:
*       Public data types shared between rlvk-shdc-generated shader headers and the
*       raylib Vulkan backend. This header contains types only: no runtime code,
*       no dependency on Vulkan headers. It is safe to include from user code,
*       from generated *.rlvk.h files, and from rlvk.h implementation.
*
*   USAGE:
*       This header is included automatically by rlvk.h and by every generated
*       <name>.rlvk.h. End users typically do not include it directly — they just
*       #include "<name>.rlvk.h" and call Load<Name>Shader() from their code.
*
*       Uniform tables are NULL-terminated: iterate until the `name` field is NULL
*       (uniforms and samplers) or the `size` field is 0 (ubos).
*
*   LIMITS:
*       RLVK_MAX_UBOS_PER_SHADER    8    // AA-level headroom: frame+material+object+etc.
*       RLVK_MAX_DESC_SETS          4    // Vulkan's guaranteed portable minimum
*       RLVK_FRAME_UBO_INDEX     0x7F    // Sentinel in ubo_index for the shared frame UBO
*
*   LICENSE: zlib/libpng (same as raylib)
*
**********************************************************************************************/

#ifndef RLVK_SHADER_H
#define RLVK_SHADER_H

#include <stddef.h>
#include <stdint.h>

// NOTE: `type` in rlvk_uniform_entry holds the numeric value of raylib's
// rlShaderUniformDataType enum (RL_SHADER_UNIFORM_FLOAT=0 .. SAMPLER2D=12).
// rlvk-shdc emits those values as integer literals so generated headers do
// not require including raylib.h or rlgl.h.

// rlvk-shdc emits this synthetic type value for matrix members so runtime
// distinguishes them from float arrays of the same `count`. Runtime should
// treat the entry as `count` columns of vec4 (std140 layout) -> `count * 16`
// bytes total.
#define RLVK_SHADER_UNIFORM_MAT   ((uint8_t)0xFE)

#define RLVK_MAX_UBOS_PER_SHADER  8
#define RLVK_MAX_DESC_SETS        4
#define RLVK_FRAME_UBO_INDEX      0x7F  // sentinel value in ubo_index for shared frame UBO

typedef struct rlvk_uniform_entry {
    const char *name;
    uint16_t    ubo_index;  // 0..7 per-shader; RLVK_FRAME_UBO_INDEX for shared frame UBO
    uint16_t    offset;     // byte offset within the UBO (std140)
    uint8_t     type;       // RL_SHADER_UNIFORM_*
    uint8_t     count;      // array length (1 = scalar)
} rlvk_uniform_entry;

typedef struct rlvk_sampler_entry {
    const char *name;
    uint8_t     set;        // descriptor set index (0..3)
    uint8_t     binding;    // binding within that set
} rlvk_sampler_entry;

typedef struct rlvk_ubo_entry {
    uint8_t  set;
    uint8_t  binding;
    uint16_t size;
    uint8_t  shared;        // 1 = bind rlvk's shared frame UBO; 0 = per-shader UBO
} rlvk_ubo_entry;

typedef struct rlvk_shader_blob {
    const uint32_t           *vs_spv; size_t vs_size;   // size in bytes
    const uint32_t           *fs_spv; size_t fs_size;
    const rlvk_ubo_entry     *ubos;      // terminated by entry with size==0
    const rlvk_uniform_entry *uniforms;  // terminated by entry with name==NULL
    const rlvk_sampler_entry *samplers;  // terminated by entry with name==NULL
} rlvk_shader_blob;

#endif // RLVK_SHADER_H
