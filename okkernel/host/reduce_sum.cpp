#include <assert.h>
#include <iostream>
#include <random>
#include <cstring>
#include <sys/time.h>
#include "okk_param.h"
#include "bmlib_runtime.h"
#define BMLIB_SAFE_CALL(cmd) assert(cmd == BM_SUCCESS)
#define DIV_UP(a, b) (((a) - 1) / (b) + 1)
#ifdef USING_CMODEL
#define MAXIT (1)
#else
#define MAXIT (100)
#endif
typedef struct {
    int N, C, H, W;
    int axis_list[4];
    int axis_num;
    unsigned long long output_addr;
    unsigned long long input_addr;
} __attribute__((packed)) param_t;

template <typename T>
void transpose(const T *input, T *buffer,
               const int *input_shape,
               const int *trans_order,
               const int *trans_shape,
               const int shape_dims) {
    for (int n = 0; n < input_shape[0]; n++) {
        for (int c = 0; c < input_shape[1]; c++) {
            for (int h = 0; h < input_shape[2]; h++) {
                for (int w = 0; w < input_shape[3]; w++) {
                    int nchw[4] = {n, c, h, w};
                    int dst_idx = nchw[trans_order[0]] * trans_shape[1] * trans_shape[2] * trans_shape[3] +
                                  nchw[trans_order[1]] * trans_shape[2] * trans_shape[3] +
                                  nchw[trans_order[2]] * trans_shape[3] + nchw[trans_order[3]];
                    int src_idx = n * input_shape[1] * input_shape[2] * input_shape[3] +
                                  c * input_shape[2] * input_shape[3] +
                                  h * input_shape[3] + w;
                    buffer[dst_idx] = input[src_idx];
                }
            }
        }
    }
}

template <typename T>
void reduce_sum(T *output, const T *input, const T init_val, const param_t &param) {
    T* buffer = nullptr;
    bool is_reduce[4] = {0};
    for (int i = 0; i < param.axis_num; ++i) {
        is_reduce[param.axis_list[i]] = true;
    }
    int trans_order[4] = {0, 1, 2, 3};
    bool has_trans = false;
    int pos = 0;
    for (int i = 0; i < 4; ++i) {
        if (!is_reduce[i]) trans_order[pos++] = i;
    }
    memset(is_reduce, 0, sizeof(bool)*4);
    for (int i = 0; i < param.axis_num; ++i) {
        is_reduce[pos] = true;
        trans_order[pos++] = param.axis_list[i];
    }
    for (int i = 0; i < 4; ++i) {
        if (trans_order[i] != i) {
            has_trans = true;
            break;
        }
    }

    int input_shape[4] = {param.N, param.C, param.H, param.W};
    int trans_shape[4] = {param.N, param.C, param.H, param.W};
    int axis_list[4] = {0};
    memcpy(axis_list, param.axis_list, sizeof(int)*param.axis_num);

    //int trans_order1[4] = {0, 2, 3, 1};
    if (has_trans) {
        int pos = 0;
        for (int i = 0; i < 4; i++) {
            if (is_reduce[i]) axis_list[pos++] = i;
            trans_shape[i] = input_shape[trans_order[i]];
        }
        int input_size = input_shape[0] * input_shape[1] * input_shape[2] * input_shape[3];
        buffer = new T[input_size];
        //int trans_shape1[4] = {1, 2, 3, 2};
        transpose(input, buffer, input_shape, trans_order,
                  trans_shape, 4);
    }

    int outer_dim = 1;
    int inner_dim = 1;
    for (int i = 0; i < 4 - param.axis_num; ++i) {
        outer_dim *= trans_shape[i];
    }
    for (int i = 4 - param.axis_num; i < 4; ++i) {
        inner_dim *= trans_shape[i];
    }
    const T *in_data = has_trans ? buffer : input;
    for (int i = 0; i < outer_dim; ++i) {
        T tmp = init_val;
        for (int j = 0; j < inner_dim; ++j) {
            tmp = tmp + *in_data;
            ++in_data;
        }
        output[i] = tmp;
    }
    if (has_trans) delete[] buffer;
}

static inline void reduce_sum_reference(float *output, const float *input, const param_t &param) {
    reduce_sum<float>(output, input, 0.0f, param);
}

int reduce_sum(bm_handle_t &handle, param_t &param, const char *device_func_name) {
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real_distribution<float> dist_value{-1.f, 1.f};

    int input_len = param.N*param.C*param.H*param.W;
    int output_shape[4] = {param.N, param.C, param.H, param.W};
    int output_len = 1;
    for (int i = 0; i < param.axis_num; ++i) {
        output_shape[param.axis_list[i]] = 1;
    }
    for (int i = 0; i < 4; i++) {
        output_len *= output_shape[i];
    }

    bm_device_mem_t output_dev, input_dev;
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &output_dev, output_len * sizeof(float)));
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &input_dev, input_len * sizeof(float)));
    param.output_addr = bm_mem_get_device_addr(output_dev);
    param.input_addr = bm_mem_get_device_addr(input_dev);

    float* input_host = new float[input_len];
    float* output_host = new float[output_len];
    float* output_ref = new float[output_len];
    for(int i=0; i<input_len; i++) {
        input_host[i] = dist_value(rng);
    }

    reduce_sum_reference(output_ref, input_host, param);
    BMLIB_SAFE_CALL(bm_memcpy_s2d(handle, input_dev, input_host));

    // launch kernel function
    struct timeval start_time, end_time;
    long long elapsed_time = 0;
    for (int i = 0; i < MAXIT; ++i) {
        gettimeofday(&start_time, NULL);
        BMLIB_SAFE_CALL(okkernel_launch_sync(handle, device_func_name, &param, sizeof(param)));
        gettimeofday(&end_time, NULL);
        elapsed_time += (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
    }


    // copy output from device to host
    BMLIB_SAFE_CALL(bm_memcpy_d2s(handle, output_host, output_dev));
    bool pass = true;
    for (int i = 0; i < output_len; ++i) {
        if (!std::isfinite(output_host[i]) && !std::isfinite(output_ref[i]))
            continue;
        float max_val = std::max(std::fabs(output_host[i]), std::fabs(output_ref[i]));
        if (!(std::fabs(output_host[i] - output_ref[i]) < 1e-2 * std::max(max_val, 1.f))) {
            pass = false;
            break;
        }
    }
    int res = -1;
    if (pass) {
        res = std::round(elapsed_time / (double)MAXIT);
        std::cout << "elapsed time: " << res << "(us)" << std::endl;
    }
    // free
    bm_free_device(handle, output_dev);
    bm_free_device(handle, input_dev);
    delete [] input_host;
    delete [] output_host;
    delete [] output_ref;
    return res;
}

int main() {
    bm_handle_t handle;
    // Initialize.
    BMLIB_SAFE_CALL(bm_dev_request(&handle, 0));

    ////////////////////////////////////////////////////////////////////////
    /// CONTEST CASES
    /// ////////////////////////////////////////////////////////////////////
    param_t params[] = {
        {.N=1, .C=68,  .H=4,   .W=18,  .axis_list={2},      .axis_num=1 },  //0
        {.N=1, .C=64,  .H=128, .W=256, .axis_list={3},      .axis_num=1 },  //1
        {.N=1, .C=32,  .H=64,  .W=800, .axis_list={3},      .axis_num=1 },  //2
        {.N=1, .C=64,  .H=32,  .W=400, .axis_list={2},      .axis_num=1 },  //3
        {.N=1, .C=32,  .H=200, .W=200, .axis_list={2},      .axis_num=1 },  //4
        {.N=1, .C=38,  .H=38,  .W=512, .axis_list={3},      .axis_num=1 },  //5
        {.N=1, .C=64,  .H=128, .W=128, .axis_list={1},      .axis_num=1 },  //6
        {.N=1, .C=160, .H=16,  .W=200, .axis_list={2},      .axis_num=1 },  //7
        {.N=1, .C=256, .H=50,  .W=50,  .axis_list={2},      .axis_num=1 },  //8
        {.N=1, .C=1280,.H=7,   .W=7,   .axis_list={3},      .axis_num=1 },  //9
        {.N=1, .C=80,  .H=16,  .W=200, .axis_list={2},      .axis_num=1 },  //10
        {.N=1, .C=160, .H=1,   .W=200, .axis_list={3},      .axis_num=1 },  //11
        {.N=1, .C=1280,.H=7,   .W=7,   .axis_list={0,2},    .axis_num=2 },  //12
        {.N=1, .C=64,  .H=128, .W=256, .axis_list={0,2,3},  .axis_num=3 },  //13
        {.N=1, .C=160, .H=16,  .W=200, .axis_list={0,1,2,3},.axis_num=4 },  //14
    };
    int results[sizeof(params) / sizeof(param_t)];
    for (unsigned int i = 0; i < sizeof(params) / sizeof(param_t); ++i) {
        int res = reduce_sum(handle, params[i], "reduce_sum_contest");
        if (res >= 0)
            std::cout << "case " << i << " pass" << std::endl;
        else
            std::cout << "case " << i << " fail" << std::endl;
        results[i] = res;
    }
    (void)(results);
    // Deinitialize.
    bm_dev_free(handle);
    return 0;
}

