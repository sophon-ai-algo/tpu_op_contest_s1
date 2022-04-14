#include <stdbool.h>
#ifndef OKK_H
#define OKK_H
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*okk_kernel_func_t)(const void *);
void okk_register_kernel_func(const char *name, okk_kernel_func_t func);
void okk_dump_registered_kernel_funcs();
#define OKKERNEL_FUNC_REGISTER(func)                               \
__attribute__((constructor)) void okk_kernel_register_##func() {   \
    okk_register_kernel_func(#func, func);                         \
}
#ifndef USING_CMODEL
extern void fw_log(int, char *, ...);
#define OKKERNEL_LOG(format, ...) fw_log(0, (char *)format, ##__VA_ARGS__)
#define OKKERNEL_ASSERT(assertion) do {                              \
    if (!(assertion))                                                \
        OKKERNEL_LOG("%s:%d: %s: Assertion \"%s\" failed.\n",        \
                     __FILE__, __LINE__,  __FUNCTION__, #assertion); \
} while(0)
#else
#include <stdio.h>
#define OKKERNEL_LOG(format, ...) printf(format, ##__VA_ARGS__)
#include <assert.h>
#define OKKERNEL_ASSERT(assertion) assert(assertion)
#endif
void okk_set_id_node(void *node);
void okk_get_id_node(void *node);
void okk_enable_check_id_node();
void okk_disable_check_id_node();
typedef unsigned int local_addr_t;
typedef unsigned long long system_addr_t;
typedef unsigned long long global_addr_t;
typedef struct {
    int n, c, h, w;
} dim4;
typedef struct {
    int h, w;
} dim2;
typedef struct {
    int top, bottom, left, right;
} Padding;
typedef char s8x4[4];
typedef unsigned char u8x4[4];
typedef short s16x2[2];
typedef unsigned short u16x2[2];
typedef union {
    float fp32;
    int s32;
    unsigned int u32;
    s8x4 _4N_s8;
    u8x4 _4N_u8;
    s16x2 _2N_s16;
    u16x2 _2N_u16;
} x32;
typedef union {
    short s16;
    unsigned short u16;
} x16;
typedef union {
    char s8;
    unsigned char u8;
} x8;
typedef struct {
    bool is_addr;
    union {
        float val;
        local_addr_t addr;
    } context;
} fp32_or_addr_t;
typedef struct {
    bool is_addr;
    union {
        x32 val;
        local_addr_t addr;
    } context;
} x32_or_addr_t;
/*
 * bit 0: 0 - src0 and src1 are 16-bit, 1 - src0 and src1 are 8-bit
 * bit 1: 0 - src0 is unsigned,         1 - src0 is signed
 * bit 3: 0 - src1 is unsigned,         1 - src1 is signed
 * bit 4: 0 - dst  is 16-bit,           1 - dst  is 8-bit
 * bit 5: 0 - dst  is unsigned,         1 - dst  is signed
 */
#define OP_TYPE(src_bit, src0_sign, src1_sign, dst_bit, dst_sign)    \
    ((src_bit) | ((src0_sign) << 1) | ((src1_sign) << 2) |           \
    ((dst_bit) << 3) | ((dst_sign) << 4))
typedef enum {
    S8_OP_S8_TO_S8     = OP_TYPE(1, 1, 1, 1, 1),
    S8_OP_S8_TO_S16    = OP_TYPE(1, 1, 1, 0, 1),
    S8_OP_U8_TO_S8     = OP_TYPE(1, 1, 0, 1, 1),
    S8_OP_U8_TO_S16    = OP_TYPE(1, 1, 0, 0, 1),
    U8_OP_S8_TO_S8     = OP_TYPE(1, 0, 1, 1, 1),
    U8_OP_S8_TO_S16    = OP_TYPE(1, 0, 1, 0, 1),
    U8_OP_U8_TO_S8     = OP_TYPE(1, 0, 0, 1, 1),
    U8_OP_U8_TO_S16    = OP_TYPE(1, 0, 0, 0, 1),
    U8_OP_U8_TO_U8     = OP_TYPE(1, 0, 0, 1, 0),
    U8_OP_U8_TO_U16    = OP_TYPE(1, 0, 0, 0, 0),
    S16_OP_S16_TO_S8   = OP_TYPE(0, 1, 1, 1, 1),
    S16_OP_S16_TO_S16  = OP_TYPE(0, 1, 1, 0, 1),
    S16_OP_U16_TO_S8   = OP_TYPE(0, 1, 0, 1, 1),
    S16_OP_U16_TO_S16  = OP_TYPE(0, 1, 0, 0, 1),
    U16_OP_S16_TO_S8   = OP_TYPE(0, 0, 1, 1, 1),
    U16_OP_S16_TO_S16  = OP_TYPE(0, 0, 1, 0, 1),
    U16_OP_U16_TO_S8   = OP_TYPE(0, 0, 0, 1, 1),
    U16_OP_U16_TO_S16  = OP_TYPE(0, 0, 0, 0, 1),
    U16_OP_U16_TO_U8   = OP_TYPE(0, 0, 0, 1, 0),
    U16_OP_U16_TO_U16  = OP_TYPE(0, 0, 0, 0, 0)
} op_type_t;
/*
 * bit 0: 0 - src0 is unsigned, 1 - src0 is signed
 * bit 1: 0 - src1 is unsigned, 1 - src1 is signed
 * bit 2: 0 - dst  is unsigned, 1 - dst  is signed
 */
#define MUL_TYPE(src0_sign, src1_sign, dst_sign)    \
    ((src0_sign) | ((src1_sign) << 1) | ((dst_sign) << 2))
typedef enum {
    S16_MUL_S8_TO_S16 = MUL_TYPE(1, 1, 1),
    U16_MUL_S8_TO_S16 = MUL_TYPE(0, 1, 1),
    U16_MUL_U8_TO_U16 = MUL_TYPE(0, 0, 0),
    S16_MUL_U8_TO_S16 = MUL_TYPE(1, 0, 1)
} mul_type_t;

/////////// memory info ////////////////
unsigned long long okk_global_mem_start_addr();
unsigned int okk_local_mem_start_addr();
unsigned int okk_local_mem_bank_per_npu();
unsigned int okk_l2_sram_start_addr();
unsigned int okk_dtcm_start_addr();
unsigned int okk_lookup_table_start_addr();
unsigned int okk_lookup_table_size();
unsigned int okk_eu_num();

/////////// get mem ptr by addr //////////
unsigned char* okk_global_mem_addr(unsigned long long addr);
unsigned char* okk_local_mem_addr(unsigned int idx, unsigned int addr);
unsigned char* okk_l2_sram_addr(unsigned int addr);
unsigned char* okk_dtcm_addr(unsigned int addr);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void okk_initialize();

void okk_poll();

void okk_parallel_start();

void okk_parallel_end();

bool okk_is_parallel_state();

unsigned int okk_local_mem_size_per_npu();

unsigned int okk_l2_sram_size();

unsigned int okk_dtcm_size();

int okk_npu_num();

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// UTILS FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int okk_start_npu_index(local_addr_t addr);

int okk_channle_num_per_npu(int start_idx, int num_channels);

void okk_128_byte_aligned_stride_for_32bit(
    dim4         *stride,
    int           start_idx,
    const dim4   *shape);

void okk_128_byte_aligned_stride_for_16bit(
    dim4         *stride,
    int           start_idx,
    const dim4   *shape);

void okk_128_byte_aligned_stride_for_8bit(
    dim4         *stride,
    int           start_idx,
    const dim4   *shape);

void okk_compact_stride(
    dim4         *stride,
    int           start_idx,
    const dim4   *shape);

void okk_continuous_stride(dim4 *stride, const dim4 *shape);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// GDMA ATOMIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void okk_gdma_32bit_cpy_S2L(
    local_addr_t   dst_addr,
    system_addr_t  src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_32bit_cpy_L2S(
    system_addr_t  dst_addr,
    local_addr_t   src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_32bit_cpy_L2L(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_gdma_32bit_cpy_S2S(
    system_addr_t  dst_addr,
    system_addr_t  src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_32bit_matrix_S2L(
    local_addr_t   dst_addr,
    system_addr_t  src_addr,
    int            rows,
    int            cols,
    int            cols_per_channel,
    int            row_stride);

void okk_gdma_32bit_matrix_L2S(
    system_addr_t  dst_addr,
    local_addr_t   src_addr,
    int            rows,
    int            cols,
    int            cols_per_channel,
    int            row_stride);

void okk_gdma_32bit_set_C_system(
    system_addr_t  dst_addr,
    x32            C,
    const dim4    *shape,
    const dim4    *dst_stride);

void okk_gdma_32bit_set_C_local(
    local_addr_t  dst_addr,
    x32           C,
    const dim4   *shape,
    const dim4   *dst_stride);

void okk_gdma_16bit_cpy_S2L(
    local_addr_t   dst_addr,
    system_addr_t  src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_16bit_cpy_L2S(
    system_addr_t  dst_addr,
    local_addr_t   src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_16bit_cpy_L2L(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_gdma_16bit_cpy_S2S(
    system_addr_t  dst_addr,
    system_addr_t  src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_16bit_set_C_system(
    system_addr_t  dst_addr,
    x16            C,
    const dim4    *shape,
    const dim4    *dst_stride);

void okk_gdma_16bit_set_C_local(
    local_addr_t  dst_addr,
    x16           C,
    const dim4   *shape,
    const dim4   *dst_stride);

void okk_gdma_8bit_cpy_S2L(
    local_addr_t   dst_addr,
    system_addr_t  src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_8bit_cpy_L2S(
    system_addr_t  dst_addr,
    local_addr_t   src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_8bit_cpy_L2L(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_gdma_8bit_cpy_S2S(
    system_addr_t  dst_addr,
    system_addr_t  src_addr,
    const dim4    *shape,
    const dim4    *dst_stride,
    const dim4    *src_stride);

void okk_gdma_8bit_set_C_system(
    system_addr_t  dst_addr,
    x8             C,
    const dim4    *shape,
    const dim4    *dst_stride);

void okk_gdma_8bit_set_C_local(
    local_addr_t  dst_addr,
    x8            C,
    const dim4   *shape,
    const dim4   *dst_stride);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// BDC ATOMIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void okk_bdc_32bit_cpy(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_32bit_set_C(
    local_addr_t  dst_addr,
    x32           C,
    const dim4   *shape,
    const dim4   *dst_stride);

void okk_bdc_fp32_to_int32(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape);

void okk_bdc_lookup_int32_to_fp32(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape);

void okk_bdc_rsqrt(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape);

void okk_bdc_sqrt(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape);

void okk_bdc_taylor_exp(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    int           num_series);

void okk_bdc_lookup_exp(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape);

void okk_bdc_exp(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape);

void okk_bdc_exp_tunable(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape,
    int           num_series);

void okk_bdc_sigmoid(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape);

void okk_bdc_sigmoid_tunable(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape,
    int           num_series);

void okk_bdc_tanh(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape);

void okk_bdc_tanh_tunable(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape,
    int           num_series);

void okk_bdc_relu(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_reciprocal(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_neg(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_add(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_add_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_sub(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_sub_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_C_sub(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_mul(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_mul_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_div(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_div_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_C_div(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_mac(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_mac_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_max(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_max_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_min(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_min_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_greater_select_value(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    x32           select_val,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_greater_C_select_value(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    x32           select_val,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_C_greater_select_value(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    x32           select_val,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_equal_select_value(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    x32           select_val,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_equal_C_select_value(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    float         C,
    x32           select_val,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_32bit_and(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_32bit_and_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    x32           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_32bit_or(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_32bit_or_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    x32           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_32bit_xor(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_32bit_xor_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    x32           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_32bit_arithmetic_shift(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_32bit_logical_shift(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_32bit_arithmetic_shift_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_32bit_logical_shift_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_32bit_C_arithmetic_shift(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_32bit_C_logical_shift(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    unsigned int  C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_4N_int8_to_fp32(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape,
    bool          is_signed,
    bool          is_aligned_layout);

void okk_bdc_int8_to_fp32(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape,
    bool          is_signed,
    bool          is_aligned_layout);

void okk_bdc_bias(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  bias_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_scale(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  scale_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_scale_bias(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  scale_addr,
    local_addr_t  bias_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_conv2d(
    local_addr_t    output_addr,
    local_addr_t    input_addr,
    local_addr_t    weight_addr,
    local_addr_t    bias_addr,
    const dim4     *input_shape,
    int             output_c,
    int             kernel_h,
    int             kernel_w,
    const dim4     *input_stride,
    const dim4     *kernel_stride,
    bool            using_bias,
    bool            result_add,
    const Padding  *padding,
    const dim2     *stride,
    const dim2     *dilation);

void okk_bdc_depthwise2d(
    local_addr_t    output_addr,
    local_addr_t    input_addr,
    local_addr_t    weight_addr,
    local_addr_t    bias_addr,
    const dim4     *input_shape,
    int             kernel_h,
    int             kernel_w,
    bool            using_bias,
    const Padding  *padding,
    const dim2     *stride,
    const dim2     *dilation);

void okk_bdc_avg_pool2d(
    local_addr_t    output_addr,
    local_addr_t    input_addr,
    const dim4     *input_shape,
    int             kernel_h,
    int             kernel_w,
    const Padding  *padding,
    const dim2     *stride);

void okk_bdc_max_pool2d(
    local_addr_t    output_addr,
    local_addr_t    input_addr,
    const dim4     *input_shape,
    int             kernel_h,
    int             kernel_w,
    const Padding  *padding,
    const dim2     *stride);

void okk_bdc_matmul(
    local_addr_t  output_addr,
    local_addr_t  left_addr,
    local_addr_t  right_addr,
    local_addr_t  bias_addr,
    int           left_rows,
    int           left_cols,
    int           right_cols,
    int           left_cols_per_channel,
    int           right_cols_per_channel,
    bool          using_bias,
    bool          result_add);

void okk_bdc_greater_select(
    local_addr_t           dst_addr,
    const fp32_or_addr_t  *src0,
    const fp32_or_addr_t  *src1,
    const x32_or_addr_t   *select0,
    const x32_or_addr_t   *select1,
    const dim4            *shape);

void okk_bdc_equal_select(
    local_addr_t           dst_addr,
    const fp32_or_addr_t  *src0,
    const fp32_or_addr_t  *src1,
    const x32_or_addr_t   *select0,
    const x32_or_addr_t   *select1,
    const dim4            *shape);

void okk_bdc_fixed_point_packed_add(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride,
    op_type_t     op_type,
    int           rshift);

void okk_bdc_fixed_point_packed_add_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride,
    op_type_t     op_type,
    int           rshift);

void okk_bdc_fixed_point_packed_sub(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride,
    op_type_t     op_type,
    int           rshift);

void okk_bdc_fixed_point_packed_sub_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride,
    op_type_t     op_type,
    int           rshift);

void okk_bdc_fixed_point_packed_C_sub(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride,
    op_type_t     op_type,
    int           rshift);

void okk_bdc_fixed_point_packed_mul(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride,
    op_type_t     op_type,
    int           rshift);

void okk_bdc_fixed_point_packed_mul_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride,
    op_type_t     op_type,
    int           rshift);

void okk_bdc_fixed_point_packed_mac(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride,
    bool          is_origin_dst_signed,
    op_type_t     op_type,
    int           lshift,
    int           rshift);

void okk_bdc_fixed_point_packed_mac_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride,
    bool          is_origin_dst_signed,
    op_type_t     op_type,
    int           lshift,
    int           rshift);

void okk_bdc_fixed_point_packed_max(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride,
    op_type_t     op_type);

void okk_bdc_fixed_point_packed_max_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride,
    op_type_t     op_type);

void okk_bdc_fixed_point_packed_min(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride,
    op_type_t     op_type);

void okk_bdc_fixed_point_packed_min_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    int           C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride,
    op_type_t     op_type);

void okk_bdc_fixed_point_packed_16bit_arithmetic_shift(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_fixed_point_packed_16bit_logical_shift(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride);

void okk_bdc_fixed_point_packed_16bit_arithmetic_shift_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    short         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_fixed_point_packed_16bit_logical_shift_C(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    short         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_fixed_point_packed_16bit_C_arithmetic_shift(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    short         C,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_fixed_point_packed_16bit_C_logical_shift(
    local_addr_t    dst_addr,
    local_addr_t    src_addr,
    unsigned short  C,
    const dim4     *shape,
    const dim4     *dst_stride,
    const dim4     *src_stride);

void okk_bdc_fixed_point_packed_16bit_split_high_8bit(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride);

void okk_bdc_fixed_point_packed_16bit_split_low_8bit(
    local_addr_t  dst_addr,
    local_addr_t  src_addr,
    local_addr_t  work_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src_stride,
    const dim4   *work_stride);

void okk_bdc_fixed_point_packed_16bit_mul_8bit(
    local_addr_t  dst_addr,
    local_addr_t  src0_high_addr,
    local_addr_t  src0_low_addr,
    local_addr_t  src1_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_high_stride,
    const dim4   *src0_low_stride,
    const dim4   *src1_stride,
    mul_type_t    mul_type,
    int           rshift);

void okk_bdc_fixed_point_packed_s16_mul_u8_to_s16(
    local_addr_t  dst_addr,
    local_addr_t  src0_addr,
    local_addr_t  src1_addr,
    local_addr_t  work_addr,
    local_addr_t  work_high_addr,
    local_addr_t  work_low_addr,
    const dim4   *shape,
    const dim4   *dst_stride,
    const dim4   *src0_stride,
    const dim4   *src1_stride,
    const dim4   *work_stride,
    const dim4   *work_high_stride,
    const dim4   *work_low_stride,
    int           rshift);
#ifdef __cplusplus
}
#endif
#endif
