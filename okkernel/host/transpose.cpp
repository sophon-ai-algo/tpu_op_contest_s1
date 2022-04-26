#include <assert.h>
#include <iostream>
#include <random>
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
    int order[4];
    unsigned long long output_addr;
    unsigned long long input_addr;
} __attribute__((packed)) param_t;

static inline void transpose_reference(const float *input, float *output,
                                       const param_t &param) {
    const int input_shape[4] = {param.N, param.C, param.H, param.W};
    int trans_shape[4] = {0};
    for (int i = 0; i < 4; i++) {
        trans_shape[i] = input_shape[param.order[i]];
    }
    for (int n = 0; n < param.N; n++) {
        for (int c = 0; c < param.C; c++) {
            for (int h = 0; h < param.H; h++) {
                for (int w = 0; w < param.W; w++) {
                    int nchw[4] = {n, c, h, w};
                    int dst_idx = nchw[param.order[0]] * trans_shape[1] * trans_shape[2] * trans_shape[3] +
                                  nchw[param.order[1]] * trans_shape[2] * trans_shape[3] +
                                  nchw[param.order[2]] * trans_shape[3] + nchw[param.order[3]];
                    int src_idx = n * param.C * param.H * param.W +
                                  c * param.H * param.W +
                                  h * param.W + w;
                    output[dst_idx] = input[src_idx];
                }
            }
        }
    }
}


int transpose(bm_handle_t &handle, param_t &param, const char *device_func_name) {
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real_distribution<float> dist_value{-1.f, 1.f};

    int len = param.N*param.C*param.H*param.W;

    bm_device_mem_t output_dev, input_dev;
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &output_dev, len * sizeof(float)));
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &input_dev, len * sizeof(float)));
    param.output_addr = bm_mem_get_device_addr(output_dev);
    param.input_addr = bm_mem_get_device_addr(input_dev);

    float* input_host = new float[len];
    float* output_host = new float[len];
    float* output_ref = new float[len];

    transpose_reference(input_host, output_ref, param);
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
    for (int i = 0; i < len; ++i) {
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
        {.N=1,      .C=24,      .H=1,       .W=1,       .order={0,2,3,1}},  //0
        {.N=1,      .C=64,      .H=1080,    .W=1920,    .order={0,2,3,1}},  //1
        {.N=576,    .C=1024,    .H=2,       .W=2,       .order={0,2,3,1}},  //2
        {.N=100,    .C=1024,    .H=55,      .W=55,      .order={0,2,3,1}},  //3
        {.N=300,    .C=576,     .H=14,      .W=14,      .order={0,2,3,1}},  //4
        {.N=1,      .C=12,      .H=256,     .W=64,      .order={0,2,1,3}},  //5
        {.N=1,      .C=256,     .H=12,      .W=64,      .order={0,2,1,3}},  //6
        {.N=37,     .C=25,      .H=2,       .W=128,     .order={0,2,1,3}},  //7
        {.N=576,    .C=2,       .H=2,       .W=1024,    .order={0,3,1,2}},  //8
        {.N=1,      .C=24,      .H=24,      .W=128,     .order={0,3,1,2}},  //9
        {.N=1,      .C=1,       .H=60000,   .W=4,       .order={0,3,2,1}},  //10
        {.N=1,      .C=12,      .H=256,     .W=64,      .order={0,1,3,2}},  //11
        {.N=1,      .C=384,     .H=10,      .W=30,      .order={3,0,1,2}},  //12
        {.N=1,      .C=256,     .H=1,       .W=79,      .order={2,3,0,1}},  //13
        {.N=1,      .C=1,       .H=60000,   .W=4,       .order={0,3,2,1}},  //14
        {.N=64,     .C=1,       .H=128,     .W=256,     .order={1,0,2,3}},  //15
    };
    int results[sizeof(params) / sizeof(param_t)];
    for (unsigned int i = 0; i < sizeof(params) / sizeof(param_t); ++i) {
        int res = transpose(handle, params[i], "transpose_contest");
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

