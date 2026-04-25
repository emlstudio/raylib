/**********************************************************************************************
*
*   rlvk-shdc v1.0 - Shader reflection + codegen tool for raylib's Vulkan backend
*
*   DESCRIPTION:
*       Reads a pair of precompiled SPIR-V modules (vertex + fragment) and emits a
*       self-contained C header that the raylib Vulkan backend consumes at runtime.
*       Generated output contains:
*           - Embedded SPIR-V bytecode as static uint32_t arrays
*           - rlvk_ubo_entry table (set / binding / size / shared flag)
*           - rlvk_uniform_entry table (name -> ubo_index + offset + type + count)
*           - rlvk_sampler_entry table (name -> set + binding)
*           - rlvk_shader_blob aggregate + Load<Name>Shader() inline convenience
*
*   USAGE:
*       rlvk-shdc -v <vs.spv> -f <fs.spv> -n <prefix> -o <out.rlvk.h>
*
*   BUILD:
*       Single translation unit, C99, no external dependencies. Any C compiler.
*       NOTE: The SPIR-V input is produced by glslc (Vulkan SDK). rlvk-shdc does
*       NOT invoke glslc — the build system runs glslc first, then this tool.
*
*   DEPENDENCIES:
*       Standard library only. No SPIRV-Reflect, no SPIRV-Cross, no C++.
*
*   LICENSE: zlib/libpng (same as raylib)
*
**********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static void usage(const char *argv0)
{
    fprintf(stderr,
        "usage: %s -v vs.spv -f fs.spv -n <prefix> -o out.rlvk.h\n",
        argv0);
    exit(2);
}

// ---------------------------------------------------------------------------
// SPIR-V loading + walking
// ---------------------------------------------------------------------------

#define SPV_MAGIC 0x07230203u

// Maximum SPIR-V result-ID we can index. Defined here (rather than alongside
// the reflection structures below) so spirv_check_header can validate the
// module's declared id-bound up front and fail loudly instead of silently
// truncating decorations on high-numbered ids.
#define MAX_IDS   8192
#define MAX_MEM   256    // members per struct

static uint32_t *load_file_u32(const char *path, size_t *words_out)
{
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "rlvk-shdc: cannot open %s\n", path); exit(1); }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || (sz % 4) != 0) {
        fclose(f);
        fprintf(stderr, "rlvk-shdc: %s: not a 4-byte-aligned file\n", path);
        exit(1);
    }
    uint32_t *buf = (uint32_t*)malloc((size_t)sz);
    if (!buf) {
        fclose(f);
        fprintf(stderr, "rlvk-shdc: out of memory loading %s\n", path);
        exit(1);
    }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf); fclose(f);
        fprintf(stderr, "rlvk-shdc: short read on %s\n", path);
        exit(1);
    }
    fclose(f);
    *words_out = (size_t)sz / 4;
    return buf;
}

static void spirv_check_header(const uint32_t *w, size_t n, const char *path)
{
    if (n < 5 || w[0] != SPV_MAGIC) {
        fprintf(stderr, "rlvk-shdc: %s: bad SPIR-V magic\n", path);
        exit(1);
    }
    // w[3] is the upper-bound on IDs used in the module (per SPIR-V spec).
    if (w[3] > MAX_IDS) {
        fprintf(stderr, "rlvk-shdc: %s declares %u IDs, exceeds MAX_IDS=%d. "
                "Bump MAX_IDS in rlvk-shdc.c.\n", path, w[3], MAX_IDS);
        exit(1);
    }
    // w[4] = reserved schema (must be 0)
}

typedef void (*spv_visitor)(uint16_t opcode, uint16_t wc, const uint32_t *ins, void *ctx);

static void spirv_walk(const uint32_t *w, size_t n, spv_visitor v, void *ctx)
{
    size_t i = 5;
    while (i < n) {
        uint32_t first = w[i];
        uint16_t wc = (uint16_t)(first >> 16);
        uint16_t op = (uint16_t)(first & 0xFFFFu);
        if (wc == 0 || i + wc > n) {
            fprintf(stderr, "rlvk-shdc: malformed SPIR-V at word %zu\n", i);
            exit(1);
        }
        v(op, wc, &w[i], ctx);
        i += wc;
    }
}

// ---------------------------------------------------------------------------
// Reflection data structures
// ---------------------------------------------------------------------------

typedef enum { T_UNKNOWN, T_INT, T_UINT, T_FLOAT, T_VEC, T_MAT,
               T_STRUCT, T_POINTER, T_ARRAY, T_IMAGE, T_SAMPLED_IMAGE } spv_kind;

typedef struct spv_type {
    spv_kind kind;
    uint32_t elem_id;
    uint32_t cols, rows;
    uint32_t width;
    uint32_t array_len;
    uint32_t storage_class;
    uint32_t member_count;
    uint32_t members[MAX_MEM];
    char     member_names[MAX_MEM][64];
    uint32_t member_offset[MAX_MEM];
} spv_type;

typedef struct spv_var {
    uint32_t type_id;
    uint32_t storage_class;
    char     name[64];
    int      has_set, has_binding;
    uint32_t set, binding;
} spv_var;

typedef struct spv_reflect {
    char      names[MAX_IDS][64];
    spv_type  types[MAX_IDS];
    spv_var   vars[MAX_IDS];
    int       is_var[MAX_IDS];
    int       has_block[MAX_IDS];
} spv_reflect;

// ---------------------------------------------------------------------------
// SPIR-V opcodes / decorations we care about (subset from the core spec)
// ---------------------------------------------------------------------------

enum {
    OpName = 5, OpMemberName = 6,
    OpDecorate = 71, OpMemberDecorate = 72,
    OpTypeInt = 21, OpTypeFloat = 22, OpTypeVector = 23, OpTypeMatrix = 24,
    OpTypeArray = 28, OpTypeStruct = 30, OpTypePointer = 32,
    OpTypeImage = 25, OpTypeSampledImage = 27,
    OpVariable = 59, OpConstant = 43
};
enum {
    DecBlock = 2, DecBufferBlock = 3, DecRowMajor = 4, DecColMajor = 5,
    DecArrayStride = 6, DecMatrixStride = 7, DecBuiltin = 11,
    DecLocation = 30, DecBinding = 33, DecDescriptorSet = 34,
    DecOffset = 35
};
enum {
    SC_UniformConstant = 0,
    SC_Uniform         = 2,
    SC_PushConstant    = 9
};

// Constants stash for array lengths (used by OpTypeArray)
static uint32_t g_const_u32[MAX_IDS] = {0};

static void collect_visitor(uint16_t op, uint16_t wc, const uint32_t *ins, void *ctx)
{
    spv_reflect *R = (spv_reflect*)ctx;
    switch (op) {
    case OpName: {
        uint32_t id = ins[1];
        if (id < MAX_IDS) {
            strncpy(R->names[id], (const char*)&ins[2], 63);
            R->names[id][63] = 0;
        }
    } break;
    case OpMemberName: {
        uint32_t id = ins[1], mi = ins[2];
        if (id < MAX_IDS && mi < MAX_MEM) {
            strncpy(R->types[id].member_names[mi], (const char*)&ins[3], 63);
            R->types[id].member_names[mi][63] = 0;
        }
    } break;
    case OpDecorate: {
        uint32_t id = ins[1], dec = ins[2];
        if (id >= MAX_IDS) break;
        if (dec == DecBlock || dec == DecBufferBlock) R->has_block[id] = 1;
        if (dec == DecDescriptorSet) { R->vars[id].has_set = 1; R->vars[id].set = ins[3]; }
        if (dec == DecBinding)       { R->vars[id].has_binding = 1; R->vars[id].binding = ins[3]; }
    } break;
    case OpMemberDecorate: {
        uint32_t id = ins[1], mi = ins[2], dec = ins[3];
        if (id < MAX_IDS && mi < MAX_MEM && dec == DecOffset) {
            R->types[id].member_offset[mi] = ins[4];
        }
    } break;
    case OpTypeInt: {
        uint32_t id = ins[1];
        R->types[id].kind  = (ins[3] ? T_INT : T_UINT);
        R->types[id].width = ins[2];
    } break;
    case OpTypeFloat: {
        uint32_t id = ins[1];
        R->types[id].kind  = T_FLOAT;
        R->types[id].width = ins[2];
    } break;
    case OpTypeVector: {
        uint32_t id = ins[1];
        R->types[id].kind    = T_VEC;
        R->types[id].elem_id = ins[2];
        R->types[id].cols    = ins[3];
    } break;
    case OpTypeMatrix: {
        uint32_t id = ins[1];
        R->types[id].kind    = T_MAT;
        R->types[id].elem_id = ins[2];
        R->types[id].cols    = ins[3];
    } break;
    case OpTypeArray: {
        uint32_t id = ins[1];
        R->types[id].kind      = T_ARRAY;
        R->types[id].elem_id   = ins[2];
        R->types[id].array_len = g_const_u32[ins[3]];
    } break;
    case OpTypeStruct: {
        uint32_t id = ins[1];
        R->types[id].kind         = T_STRUCT;
        R->types[id].member_count = wc - 2;
        for (int i = 0; i < wc - 2 && i < MAX_MEM; i++)
            R->types[id].members[i] = ins[2 + i];
    } break;
    case OpTypePointer: {
        uint32_t id = ins[1];
        R->types[id].kind          = T_POINTER;
        R->types[id].storage_class = ins[2];
        R->types[id].elem_id       = ins[3];
    } break;
    case OpTypeImage:        (void)wc; R->types[ins[1]].kind = T_IMAGE; break;
    case OpTypeSampledImage: R->types[ins[1]].kind = T_SAMPLED_IMAGE;
                             R->types[ins[1]].elem_id = ins[2]; break;
    case OpConstant: {
        // ins[1]=type_id, ins[2]=result id, ins[3]=literal.
        if (ins[2] < MAX_IDS) g_const_u32[ins[2]] = ins[3];
    } break;
    case OpVariable: {
        uint32_t type_id = ins[1], id = ins[2], sc = ins[3];
        if (id >= MAX_IDS) break;
        R->is_var[id] = 1;
        R->vars[id].type_id       = type_id;
        R->vars[id].storage_class = sc;
    } break;
    default: (void)wc; break;
    }
}

static void finalize_names(spv_reflect *R)
{
    for (uint32_t id = 0; id < MAX_IDS; id++) {
        if (R->is_var[id] && R->names[id][0]) {
            strncpy(R->vars[id].name, R->names[id], 63);
            R->vars[id].name[63] = 0;
        }
    }
}

// ---------------------------------------------------------------------------
// std140 size computation
// ---------------------------------------------------------------------------

static uint32_t type_size_std140(const spv_reflect *R, uint32_t tid);

static uint32_t scalar_size(const spv_type *t) { return t->width / 8; }
static uint32_t vec_size(const spv_type *t, const spv_reflect *R) {
    const spv_type *e = &R->types[t->elem_id];
    return t->cols * scalar_size(e);
}
static uint32_t mat_size_std140(const spv_type *t, const spv_reflect *R) {
    (void)R;
    // std140: each column padded to vec4
    return t->cols * 16;
}

static uint32_t type_size_std140(const spv_reflect *R, uint32_t tid)
{
    const spv_type *t = &R->types[tid];
    switch (t->kind) {
    case T_INT: case T_UINT: case T_FLOAT: return scalar_size(t);
    case T_VEC:    return vec_size(t, R);
    case T_MAT:    return mat_size_std140(t, R);
    case T_ARRAY:  return t->array_len *
        ((type_size_std140(R, t->elem_id) + 15) & ~15u); // 16-byte stride
    case T_STRUCT: {
        uint32_t last = t->member_count - 1;
        return t->member_offset[last] + type_size_std140(R, t->members[last]);
    }
    default: return 0;
    }
}

// ---------------------------------------------------------------------------
// UBO collection (combined across vs + fs)
// ---------------------------------------------------------------------------

#define MAX_UBOS 8

typedef struct {
    uint32_t set, binding;
    uint32_t size;
    uint32_t type_id;
    int      in_vs, in_fs;
} ubo_combined;

static int ubo_combined_add(ubo_combined *out, int *n,
                            uint32_t set, uint32_t binding, uint32_t size,
                            uint32_t type_id, int from_vs)
{
    for (int i = 0; i < *n; i++) {
        if (out[i].set == set && out[i].binding == binding) {
            if (out[i].size != size) return -1; // divergent
            if (from_vs) out[i].in_vs = 1; else out[i].in_fs = 1;
            return i;
        }
    }
    if (*n >= MAX_UBOS) return -2;
    out[*n] = (ubo_combined){ .set = set, .binding = binding, .size = size,
                              .type_id = type_id, .in_vs = from_vs, .in_fs = !from_vs };
    return (*n)++;
}

static int collect_ubos(const spv_reflect *R, ubo_combined *out, int *n, int from_vs)
{
    for (uint32_t id = 0; id < MAX_IDS; id++) {
        if (!R->is_var[id]) continue;
        const spv_var *v = &R->vars[id];
        const spv_type *ptr = &R->types[v->type_id];
        if (ptr->kind != T_POINTER) continue;
        if (ptr->storage_class != SC_Uniform) continue;
        uint32_t struct_id = ptr->elem_id;
        if (!R->has_block[struct_id]) continue;
        if (!v->has_set || !v->has_binding) continue;
        uint32_t size = type_size_std140(R, struct_id);
        int rc = ubo_combined_add(out, n, v->set, v->binding, size, struct_id, from_vs);
        if (rc == -1) {
            fprintf(stderr, "rlvk-shdc: UBO at set=%u binding=%u has divergent size between vs and fs\n",
                    v->set, v->binding);
            return -1;
        }
        if (rc == -2) {
            fprintf(stderr, "rlvk-shdc: too many UBOs (max %d)\n", MAX_UBOS);
            return -1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Sampler collection (combined across vs + fs)
// ---------------------------------------------------------------------------

#define MAX_SAMPLERS 16

typedef struct {
    char name[64];
    uint32_t set, binding;
} sampler_ent;

static int collect_samplers(const spv_reflect *R, sampler_ent *out, int *n)
{
    for (uint32_t id = 0; id < MAX_IDS; id++) {
        if (!R->is_var[id]) continue;
        const spv_var *v = &R->vars[id];
        const spv_type *ptr = &R->types[v->type_id];
        if (ptr->kind != T_POINTER) continue;
        if (ptr->storage_class != SC_UniformConstant) continue;
        const spv_type *elem = &R->types[ptr->elem_id];
        if (elem->kind != T_SAMPLED_IMAGE && elem->kind != T_IMAGE) continue;
        if (!v->has_set || !v->has_binding) continue;
        // Dedup across stages
        int found = 0;
        for (int i = 0; i < *n; i++) if (strcmp(out[i].name, v->name) == 0) { found = 1; break; }
        if (found) continue;
        if (*n >= MAX_SAMPLERS) {
            fprintf(stderr, "rlvk-shdc: too many samplers\n"); return -1;
        }
        strncpy(out[*n].name, v->name, 63); out[*n].name[63] = 0;
        out[*n].set = v->set; out[*n].binding = v->binding;
        (*n)++;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Codegen: emit <prefix>_ubos[]
// ---------------------------------------------------------------------------

static void emit_ubos(FILE *out, const char *prefix, const ubo_combined *u, int n)
{
    fprintf(out, "static const rlvk_ubo_entry %s_ubos[] = {\n", prefix);
    for (int i = 0; i < n; i++) {
        int shared = (u[i].set == 0 && u[i].binding == 0) ? 1 : 0;
        fprintf(out, "    { %u, %u, %u, %d },\n",
                u[i].set, u[i].binding, u[i].size, shared);
    }
    fprintf(out, "    { 0, 0, 0, 0 }  // terminator\n");
    fprintf(out, "};\n");
}

static void emit_samplers(FILE *out, const char *prefix, const sampler_ent *s, int n)
{
    fprintf(out, "static const rlvk_sampler_entry %s_samplers[] = {\n", prefix);
    for (int i = 0; i < n; i++)
        fprintf(out, "    { \"%s\", %u, %u },\n", s[i].name, s[i].set, s[i].binding);
    fprintf(out, "    { 0, 0, 0 }  // terminator\n};\n");
}

// ---------------------------------------------------------------------------
// Uniform member extraction + emission
// ---------------------------------------------------------------------------

static int rl_uniform_type(const spv_reflect *R, uint32_t tid, uint8_t *out_count)
{
    const spv_type *t = &R->types[tid];
    *out_count = 1;
    if (t->kind == T_ARRAY) {
        *out_count = (uint8_t)t->array_len;
        tid = t->elem_id;
        t = &R->types[tid];
    }
    if (t->kind == T_FLOAT)  return 0;     // RL_SHADER_UNIFORM_FLOAT
    if (t->kind == T_INT)    return 4;     // RL_SHADER_UNIFORM_INT
    if (t->kind == T_UINT)   return 8;     // RL_SHADER_UNIFORM_UINT
    if (t->kind == T_VEC) {
        const spv_type *e = &R->types[t->elem_id];
        if (e->kind == T_FLOAT) return 1 + (int)(t->cols - 2);   // VEC2/3/4 = 1..3
        if (e->kind == T_INT)   return 5 + (int)(t->cols - 2);   // IVEC2/3/4 = 5..7
        if (e->kind == T_UINT)  return 9 + (int)(t->cols - 2);   // UIVEC2/3/4 = 9..11
    }
    if (t->kind == T_MAT) return -1; // matrices go through rlSetUniformMatrix
    return -1;
}

static const spv_reflect *pick_reflect_for(uint32_t type_id, const spv_reflect *A, const spv_reflect *B)
{
    return A->types[type_id].kind != T_UNKNOWN ? A : B;
}

static void emit_uniforms(FILE *out, const char *prefix,
                          const ubo_combined *ubos, int n_ubos,
                          const spv_reflect *Rvs, const spv_reflect *Rfs)
{
    fprintf(out, "static const rlvk_uniform_entry %s_uniforms[] = {\n", prefix);
    for (int i = 0; i < n_ubos; i++) {
        const spv_reflect *R = pick_reflect_for(ubos[i].type_id, Rvs, Rfs);
        const spv_type *st = &R->types[ubos[i].type_id];
        for (uint32_t m = 0; m < st->member_count; m++) {
            const char *mn = st->member_names[m];
            uint8_t count = 1;
            int type = rl_uniform_type(R, st->members[m], &count);
            if (type < 0) {
                // Matrix special case: emit synthetic type=0xFE
                // (RLVK_SHADER_UNIFORM_MAT) so the runtime knows this entry is
                // `cols` columns of vec4 (std140 layout) -> `count * 16` bytes
                // total, not a `count`-element float array. Anything else: skip.
                const spv_type *t = &R->types[st->members[m]];
                if (t->kind == T_MAT) {
                    type = 0xFE; // RLVK_SHADER_UNIFORM_MAT - runtime: count cols x 16 bytes
                    count = (uint8_t)t->cols;
                }
                else continue;
            }
            fprintf(out, "    { \"%s\", %d, %u, %d, %u },\n",
                    mn, i, st->member_offset[m], type, count);
        }
    }
    fprintf(out, "    { 0, 0, 0, 0, 0 }  // terminator\n};\n");
}

// ---------------------------------------------------------------------------
// SPIR-V bytecode array + blob emission
// ---------------------------------------------------------------------------

static void emit_spirv(FILE *out, const char *prefix, const char *suffix,
                       const uint32_t *w, size_t n)
{
    fprintf(out, "static const uint32_t %s_%s_spv[] = {", prefix, suffix);
    for (size_t i = 0; i < n; i++) {
        fprintf(out, " 0x%08x", w[i]);
        if (i + 1 < n) fputc(',', out);
    }
    fprintf(out, " };\n");
}

static void capitalize(const char *s, char *out)
{
    // "swirl" -> "Swirl"
    size_t i = 0;
    if (s[0] >= 'a' && s[0] <= 'z') out[i++] = (char)(s[0] - 32);
    else                            out[i++] = s[0];
    strncpy(out + i, s + 1, 62);
    out[63] = 0;
}

static void emit_blob(FILE *out, const char *prefix)
{
    char cap[64]; capitalize(prefix, cap);
    fprintf(out,
        "\nstatic const rlvk_shader_blob %s_blob = {\n"
        "    %s_vs_spv, sizeof(%s_vs_spv),\n"
        "    %s_fs_spv, sizeof(%s_fs_spv),\n"
        "    %s_ubos, %s_uniforms, %s_samplers\n"
        "};\n\n"
        "unsigned int rlvkLoadShaderBlob(const rlvk_shader_blob *blob);\n"
        "static inline unsigned int Load%sShader(void) { return rlvkLoadShaderBlob(&%s_blob); }\n",
        prefix, prefix, prefix, prefix, prefix,
        prefix, prefix, prefix, cap, prefix);
}

int main(int argc, char **argv)
{
    const char *vs_path = NULL, *fs_path = NULL;
    const char *prefix  = NULL, *out_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 && i+1 < argc) vs_path = argv[++i];
        else if (strcmp(argv[i], "-f") == 0 && i+1 < argc) fs_path = argv[++i];
        else if (strcmp(argv[i], "-n") == 0 && i+1 < argc) prefix  = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i+1 < argc) out_path = argv[++i];
        else usage(argv[0]);
    }
    if (!vs_path || !fs_path || !prefix || !out_path) usage(argv[0]);

    size_t vs_n, fs_n;
    uint32_t *vs = load_file_u32(vs_path, &vs_n);
    uint32_t *fs = load_file_u32(fs_path, &fs_n);
    spirv_check_header(vs, vs_n, vs_path);
    spirv_check_header(fs, fs_n, fs_path);

    static spv_reflect Rvs = {0};
    static spv_reflect Rfs = {0};
    spirv_walk(vs, vs_n, collect_visitor, &Rvs); finalize_names(&Rvs);
    spirv_walk(fs, fs_n, collect_visitor, &Rfs); finalize_names(&Rfs);

    fprintf(stderr, "rlvk-shdc: vs=%s fs=%s prefix=%s out=%s\n",
            vs_path, fs_path, prefix, out_path);

    ubo_combined ubos[MAX_UBOS]; int n_ubos = 0;
    if (collect_ubos(&Rvs, ubos, &n_ubos, 1) < 0) return 1;
    if (collect_ubos(&Rfs, ubos, &n_ubos, 0) < 0) return 1;

    sampler_ent samplers[MAX_SAMPLERS]; int n_samplers = 0;
    if (collect_samplers(&Rvs, samplers, &n_samplers) < 0) return 1;
    if (collect_samplers(&Rfs, samplers, &n_samplers) < 0) return 1;

    // Uppercase prefix for the include-guard macro.
    char up[64]; size_t pl = strlen(prefix);
    if (pl > 63) pl = 63;
    for (size_t i = 0; i < pl; i++) {
        char c = prefix[i];
        up[i] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    up[pl] = 0;

    FILE *out = fopen(out_path, "w");
    if (!out) { perror(out_path); return 1; }
    fprintf(out, "// Auto-generated by rlvk-shdc. Do not edit.\n");
    fprintf(out, "#ifndef RLVK_GEN_%s_H\n#define RLVK_GEN_%s_H\n\n", up, up);
    fprintf(out, "#include \"rlvk_shader.h\"\n\n");
    emit_spirv(out, prefix, "vs", vs, vs_n);
    emit_spirv(out, prefix, "fs", fs, fs_n);
    fputc('\n', out);
    emit_ubos(out, prefix, ubos, n_ubos);
    emit_uniforms(out, prefix, ubos, n_ubos, &Rvs, &Rfs);
    emit_samplers(out, prefix, samplers, n_samplers);
    emit_blob(out, prefix);
    fprintf(out, "\n#endif // RLVK_GEN_%s_H\n", up);
    fclose(out);

    free(vs);
    free(fs);
    return 0;
}
