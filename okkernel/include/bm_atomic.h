#include <stdbool.h>
#include "okk.h"
#ifndef ATOMIC_H
#define ATOMIC_H
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    BM_INT8,
    BM_UINT8,
    BM_INT16,
    BM_UNT16,
    BM_FP32,
} BmDataType;

typedef enum {
    BM_MUL = 0,
    BM_MAC = 1,
    BM_ADD = 2,
    BM_SUB = 3,
    BM_MAX = 4,
    BM_MIN = 5,
    BM_SHIFT = 6,
    BM_AND = 7,
    BM_OR = 8,
    BM_XOR = 9,
    BM_SG = 10,
    BM_SE = 11,
    BM_DIV = 12,
    BM_TAYLOR = 13,
    BM_FP32_INT32 = 14,
    BM_NORMALIZE_INT32 = 15,
    BM_NORMALIZE_FP32 = 16,
    BM_RSQRT = 17,
    BM_CPY = 19,
    BM_SQR_SUM = 20,
    BM_SQR_DIFF = 21
} BmAtomicOp;

//////////// parameter of atomic ///////////

typedef struct {
    unsigned int input_addr;
    unsigned int weight_addr;
    unsigned int bias_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int output_c;
    unsigned int kernel_h;
    unsigned int kernel_w;
    unsigned int stride_h;
    unsigned int stride_w;
    unsigned int input_stride[4];
    unsigned int kernel_stride[4];
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int dilate_h;
    unsigned int dilate_w;
    bool kernel_is_const;
    float kernel_val;
    unsigned int pad[4];
    bool using_bias;
    bool kernel_flip;
    bool result_add;
} ConvParam;

typedef struct {
    unsigned int input_addr;
    unsigned int weight_addr;
    unsigned int bias_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int output_c;
    unsigned int kernel_h;
    unsigned int kernel_w;
    unsigned int stride_h;
    unsigned int stride_w;
    unsigned int input_stride[4];
    unsigned int kernel_stride[4];
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int dilate_h;
    unsigned int dilate_w;
    bool kernel_is_const;
    signed char kernel_val;
    unsigned int pad[4];
    unsigned int rshift_bit;
    bool using_bias;
    bool kernel_flip;
    bool result_add;
    bool if_relu;
    bool input_sign;
    bool weight_sign;
    bool bias_sign;
} ConvQuantParam;

typedef struct {
    unsigned int input_addr;
    unsigned int weight_addr;
    unsigned int bias_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int output_c;
    unsigned int input_stride[4];
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int pad[4];
    bool using_bias;
    bool result_add;
} WinoParam;

typedef struct {
    unsigned int input_addr;
    unsigned int weight_addr;
    unsigned int bias_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int output_c;
    unsigned int input_stride[4];
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int pad[4];
    unsigned int rshift_bit;
    bool using_bias;
    bool result_add;
    bool if_relu;
    bool input_sign;
    bool bias_sign;
} WinoQuantParam;

typedef struct {
    unsigned int input_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int kernel_h;
    unsigned int kernel_w;
    unsigned int stride_h;
    unsigned int stride_w;
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int pad[4];
    float coeff;
} SumPoolParam;

typedef struct {
    unsigned int input_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int kernel_h;
    unsigned int kernel_w;
    unsigned int stride_h;
    unsigned int stride_w;
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int pad[4];
} PoolParam;

typedef struct {
    unsigned int input_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int kernel_h;
    unsigned int kernel_w;
    unsigned int stride_h;
    unsigned int stride_w;
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int dilate_h;
    unsigned int dilate_w;
    unsigned int pad[4];
    unsigned int rshift_bit;
    bool input_sign;
    bool if_relu;
} PoolQuantParam;

typedef struct {
    unsigned int input_addr;
    unsigned int weight_addr;
    unsigned int bias_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int kernel_h;
    unsigned int kernel_w;
    unsigned int stride_h;
    unsigned int stride_w;
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int dilate_h;
    unsigned int dilate_w;
    unsigned int pad[4];
    bool using_bias;
} DepthwiseParam;

typedef struct {
    unsigned int input_addr;
    unsigned int weight_addr;
    unsigned int bias_addr;
    unsigned int output_addr;
    unsigned int input_shape[4];
    unsigned int kernel_h;
    unsigned int kernel_w;
    unsigned int stride_h;
    unsigned int stride_w;
    unsigned int ins_h;
    unsigned int ins_w;
    unsigned int dilate_h;
    unsigned int dilate_w;
    unsigned int pad[4];
    unsigned int rshift_bit;
    bool using_bias;
    bool if_relu;
    bool input_sign;
    bool weight_sign;
    bool bias_sign;
} DepthwiseQuantParam;

typedef struct {
    unsigned int input_l_addr;
    unsigned int input_r_addr;
    unsigned int bias_addr;
    unsigned int output_addr;
    unsigned int l_row;
    unsigned int l_col;
    unsigned int l_tensor_w;
    unsigned int l_tensor_c;
    unsigned int r_tensor_w;
    unsigned int r_tensor_c;
    bool l_trans;
    bool using_bias;
    bool result_add;
} MatmulParam;

typedef struct {
    unsigned int input_l_addr;
    unsigned int input_r_addr;
    unsigned int bias_addr;
    unsigned int output_addr;
    unsigned int l_row;
    unsigned int l_col;
    unsigned int l_tensor_w;
    unsigned int l_tensor_c;
    unsigned int r_tensor_w;
    unsigned int r_tensor_c;
    unsigned int lshfit_bit;
    unsigned int rshift_bit;
    bool l_trans;
    bool using_bias;
    bool result_add;
    bool result_add_sign;
    bool if_relu;
    bool input_l_sign;
    bool input_r_sign;
    bool bias_sign;
    bool output_16bit;
} MatmulQuantParam;

typedef struct {
    unsigned int input0_addr;
    unsigned int input1_addr;
    unsigned int output_addr;
    unsigned int shape[4];
    unsigned int input0_stride[4];
    unsigned int input1_stride[4];
    unsigned int output_stride[4];
    BmAtomicOp op;
    bool input0_is_const;
    float input0_const;
    bool input1_is_const;
    float input1_const;
    float select_value;
    int shift_type;
} ArithParam;

typedef struct {
    unsigned int input0_addr;
    unsigned int input1_addr;
    unsigned int output_addr;
    unsigned int shape[4];
    unsigned int input0_stride[4];
    unsigned int input1_stride[4];
    unsigned int output_stride[4];
    BmAtomicOp op;
    bool input0_is_const;
    bool input1_is_const;
    unsigned int rshift_bit;
    unsigned int lshift_bit;
    unsigned int shift_type;
    bool input0_sign;
    bool input1_sign;
    bool input0_8bit;
    bool input1_8bit;
    bool output_8bit;
    bool input1_n_is1;
    bool input1_h_is1;
    bool input1_w_is1;
} ArithQuantParam;

typedef struct {
    unsigned int table_addr;
    unsigned int index_addr;
    unsigned int output_addr;
    unsigned int shape[4];
    unsigned int table_len;
    unsigned int index_prec;
} TableLookupParam;

typedef struct {
    unsigned int input0_addr;
    unsigned int input1_addr;
    unsigned int output_addr;
    unsigned int shape[4];
    BmAtomicOp op;
    bool result_add;
    bool input0_is_const;
    float input0_const;
    bool input1_is_const;
    float input1_const;
} MDScalarParam;

typedef struct {
    unsigned int tensorA_addr;
    unsigned int tensorB_addr;
    unsigned int tensorS_addr;
    unsigned int output_addr;
    unsigned int shape[4];
    BmAtomicOp op;
    bool result_add;
    bool tensorB_is_const;
    float tensorB_const;
    bool tensorS_is_const;
    float tensorS_const;
} MDLinearParam;

typedef struct {
    unsigned int tensorA_addr;
    unsigned int tensorB_addr;
    unsigned int tensorC_addr;
    unsigned int tensorD_addr;
    unsigned int tensorY_addr;
    unsigned int tensorR_addr;
    unsigned int shape[4];
    bool tensorA_is_const;
    float tensorA_const;
    bool tensorB_is_const;
    float tensorB_const;
    bool tensorC_is_const;
    unsigned int tensorC_const;
    bool tensorD_is_const;
    unsigned int tensorD_const;
    int result_skip;
} MDCmpParam;

typedef struct {
    unsigned int input_addr;
    unsigned int output_addr;
    unsigned int shape[4];
    BmAtomicOp op;
    unsigned int taylor_len;
    unsigned int taylor_coef_addr;
} MDSFUParam;

typedef struct {
    unsigned int input0_addr;
    unsigned int input1_addr;
    unsigned int output_addr;
    BmAtomicOp op;
    unsigned int input0_c;
    unsigned int input0_w;
    unsigned int input0_w_last;
    unsigned int input1_c;
    unsigned int input1_w;
} VecParam;

typedef enum {
    BM_ASCEND,
    BM_DESCEND
} BmSortOrder;

typedef struct {
    unsigned long long input_data_addr;
    unsigned long long input_index_addr;
    unsigned long long output_data_addr;
    unsigned long long output_index_addr;
    unsigned int input_len;
    unsigned int output_len;
    BmSortOrder order;
    bool index_enable;
    bool index_auto;
} SortParam;

typedef enum {
    BM_GD_8BIT = 0,
    BM_GD_32BIT = 2,
    BM_GD_128BIT = 4
} BmGDType;

typedef struct {
    unsigned int data_addr;
    unsigned int index_addr;
    unsigned int output_addr;
    unsigned int length;
    BmGDType type;
} GatherDataParam;

typedef enum {
    BM_RELU,
    BM_ELU,
    BM_PRELU,
    BM_TANH,
    BM_SIGMOID
} BmActType;

typedef struct {
    unsigned long long src_addr;
    unsigned long long dst_addr;
    unsigned long long imm_addr;
    unsigned int shape[4];
    BmActType act_type;
} ActParam;

typedef struct {
    unsigned int src_addr;
    unsigned int imm_addr;
    unsigned int shape[4];
} ExpParam;

typedef struct {
    unsigned int src_addr;
    unsigned int dst_addr;
    unsigned int imm_addr;
    unsigned int shape[4];
} ActiveParam;

typedef struct {
    BmDataType src_dtype;
    BmDataType dst_dtype;
    unsigned long long src_addr;
    unsigned long long dst_addr;
    unsigned int shape[4];
} CastParam;

typedef enum {
    BM_GDMA_DIR_S2L,
    BM_GDMA_DIR_L2S,
    BM_GDMA_DIR_S2S,
    BM_GDMA_DIR_L2L
} BmGDMADir;

typedef enum {
    BM_GDMA_FORMAT_FLOAT32,
    BM_GDMA_FORMAT_INT16,
    BM_GDMA_FORMAT_UINT8,
    BM_GDMA_FORMAT_INT8,
    BM_GDMA_FORMAT_FLOAT16
} BmGDMAFormat;

typedef struct {
    unsigned long long src_addr;
    unsigned long long dst_addr;
    unsigned int length;
    BmGDMAFormat format;
    BmGDMADir direction;
} MemCpy1DParam;

typedef struct {
    unsigned long long src_addr;
    unsigned int src_local_idx;
    unsigned int src_shape[4];
    unsigned int src_stride[4];
    BmGDMAFormat src_format;
    unsigned long long dst_addr;
    unsigned int dst_local_idx;
    unsigned int dst_shape[4];
    unsigned int dst_stride[4];
    BmGDMAFormat dst_format;
    BmGDMADir direction;
    bool transpose;
} MemCpy4DParam;

typedef struct {
    unsigned long long global_mem_addr;
    unsigned int local_mem_addr;
    unsigned int local_mem_idx;
    unsigned int row;
    unsigned int col;
    unsigned int row_stride;
    unsigned int sec_size;
    BmGDMAFormat format;
    BmGDMADir direction;
    bool transpose;
    bool result_add;
} MemCpy2DParam;

typedef struct {
    unsigned long long start_addr;
    unsigned int shape[4];
    unsigned int stride[4];
    const void* const_val;
    BmGDMAFormat format;
    bool is_local_mem;
    unsigned int local_idx;
} MemSet4DParam;

typedef struct {
    unsigned int src_addr;
    unsigned int src_local_idx;
    unsigned int dst_addr;
    unsigned int dst_local_idx;
    unsigned int src_c;
    unsigned int src_w;
    BmGDMAFormat format;
    unsigned int src_c_stride;
    unsigned int dst_c_stride;
} CWTransParam;

typedef struct {
    unsigned int data_local_mem_addr;
    unsigned int mask_local_mem_addr;
    unsigned int local_mem_idx;
    unsigned long long dst_global_mem_addr;
    unsigned int shape[4];
} MaskMemCpy4DPram;

typedef struct {
    unsigned int input_addr;
    unsigned int output_addr;
    unsigned int shape[4];
} UnaryParam;

typedef struct {
    unsigned int input0_addr;
    unsigned int input1_addr;
    unsigned int output_addr;
    unsigned int shape[4];
} BinaryParam;

/////////// TPU atomic operation /////////
void bm_atomic_conv(const ConvParam* param);
void bm_atomic_conv_quantized(const ConvQuantParam* param);
void bm_atomic_winograd_quantized(const WinoQuantParam* param);
void bm_atomic_avg_pool(const PoolParam* param);
void bm_atomic_avg_pool_quantized(const PoolQuantParam* param);
void bm_atomic_max_pool(const PoolParam* param);
void bm_atomic_max_pool_quantized(const PoolQuantParam* param);
void bm_atomic_sum_pool(const SumPoolParam* param);
void bm_atomic_depthwise(const DepthwiseParam* param);
void bm_atomic_depthwise_quantized(const DepthwiseQuantParam* param);
void bm_atomic_matmul(const MatmulParam* param);
void bm_atomic_matmul_quantized(const MatmulQuantParam* param);
void bm_atomic_tensor_arithmetic(const ArithParam* param);
void bm_atomic_tensor_arithmetic_quantized(const ArithQuantParam* param);
void bm_atomic_table_lookup(const TableLookupParam* param);
void bm_atomic_md_scalar(const MDScalarParam* param);
void bm_atomic_md_linear(const MDLinearParam* param);
void bm_atomic_md_cmp(const MDCmpParam* param);
void bm_atomic_md_sfu(const MDSFUParam* param);
void bm_atomic_vec_correlation(const VecParam* param);
void bm_atomic_sort(const SortParam* param);
void bm_atomic_gather_data(const GatherDataParam* param);

/////////// local operation (implemented by combining atomic ops)
void bm_local_exp(const ExpParam* param);
void bm_local_act(const ActParam* param);

/////////// GDMA operation /////////
void bm_atomic_memcpy_1D(const MemCpy1DParam* param);
void bm_atomic_memcpy_2D(const MemCpy2DParam* param);
void bm_atomic_memcpy_4D(const MemCpy4DParam* param);
void bm_atomic_memset_4D(const MemSet4DParam* param);
void bm_atomic_cwtrans(const CWTransParam* param);
void bm_atomic_mask_memcpy_4D(const MaskMemCpy4DPram* param);

// To get the result number of the last bm_atomic_mask_memcpy.
// This function will call bm_poll() internal, and
// this function must be called when not use bm_parallel
unsigned int bm_get_mask_memcpy_res_num();

#ifdef __cplusplus
}
#endif
#endif
