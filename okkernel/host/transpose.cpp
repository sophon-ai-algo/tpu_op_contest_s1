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
    int n;
    int c;
    int h;
    int w;
    int order[4];
    unsigned long long output_addr;
    unsigned long long input_addr;
} __attribute__((packed)) param_t;

static inline void transpose_reference(const float *input, float *output,
                                       const param_t &param) {
    const int input_shape[4] = {param.n, param.c, param.h, param.w};
    int trans_shape[4] = {0};
    for (int i = 0; i < 4; i++) {
        trans_shape[i] = input_shape[param.order[i]];
    }
    for (int n = 0; n < param.n; n++) {
        for (int c = 0; c < param.c; c++) {
            for (int h = 0; h < param.h; h++) {
                for (int w = 0; w < param.w; w++) {
                    int nchw[4] = {n, c, h, w};
                    int dst_idx = nchw[param.order[0]] * trans_shape[1] * trans_shape[2] * trans_shape[3] +
                                  nchw[param.order[1]] * trans_shape[2] * trans_shape[3] +
                                  nchw[param.order[2]] * trans_shape[3] + nchw[param.order[3]];
                    int src_idx = n * param.c * param.h * param.w +
                                  c * param.h * param.w +
                                  h * param.w + w;
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

    int len = param.n*param.c*param.h*param.w;

    bm_device_mem_t output_dev, input_dev;
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &output_dev, len * sizeof(float)));
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &input_dev, len * sizeof(float)));
    param.output_addr = bm_mem_get_device_addr(output_dev);
    param.input_addr = bm_mem_get_device_addr(input_dev);

    float* input_host = new float[len];
    float* output_host = new float[len];
    float* output_ref = new float[len];
    for(int i=0; i<len; i++) {
        input_host[i] = dist_value(rng);
        std::cout<<" "<<input_host[i];
    }
    std::cout<<std::endl;
    transpose_reference(input_host, output_ref, param);
    for(int i=0; i<len; i++) {
        std::cout<<" "<<output_ref[i];
    }
    std::cout<<std::endl;
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
        std::cout<<" "<<output_host[i];
    }
    std::cout<<std::endl;
    for (int i = 0; i < len; ++i) {
        if (!std::isfinite(output_host[i]) && !std::isfinite(output_ref[i]))
            continue;
        float max_val = std::max(std::fabs(output_host[i]), std::fabs(output_ref[i]));
        if (!(std::fabs(output_host[i] - output_ref[i]) < 1e-2 * std::max(max_val, 1.f))) {
            pass = false;
            std::cout<<"index: "<<i<<" output: "<<output_host[i]<<" outref: "<<output_ref[i]<<std::endl;
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
    param_t param = {1, 2, 2, 3, {1, 0, 3, 2}};

    transpose(handle, param, "transpose_demo");
    ////////////////////////////////////////////////////////////////////////
    /// CONTEST CASES
    /// ////////////////////////////////////////////////////////////////////
//    param_t params[] = {
//        {.left_rows = 2,      .left_cols = 100352, .right_cols = 2048 }, // 0
//        {.left_rows = 2,      .left_cols = 1280,   .right_cols = 1000 }, // 1
//        {.left_rows = 2,      .left_cols = 25088,  .right_cols = 4096 }, // 2
//        {.left_rows = 4,      .left_cols = 1024,   .right_cols = 25088}, // 3
//        {.left_rows = 32,     .left_cols = 2048,   .right_cols = 36   }, // 4
//        {.left_rows = 64,     .left_cols = 9216,   .right_cols = 4096 }, // 5
//        {.left_rows = 79,     .left_cols = 256,    .right_cols = 4090 }, // 6
//        {.left_rows = 200,    .left_cols = 4096,   .right_cols = 324  }, // 7
//        {.left_rows = 256,    .left_cols = 768,    .right_cols = 3072 }, // 8
//        {.left_rows = 256,    .left_cols = 3072,   .right_cols = 768  }, // 9
//        {.left_rows = 300,    .left_cols = 2048,   .right_cols = 80   }, // 10
//        {.left_rows = 1024,   .left_cols = 1024,   .right_cols = 1024 }, // 11
//        {.left_rows = 2048,   .left_cols = 4,      .right_cols = 1024 }, // 12
//        {.left_rows = 12544,  .left_cols = 2,      .right_cols = 1024 }, // 13
//        {.left_rows = 100352, .left_cols = 1024,   .right_cols = 1    }, // 14
//    };
//    int results[sizeof(params) / sizeof(param_t)];
//    for (unsigned int i = 0; i < sizeof(params) / sizeof(param_t); ++i) {
//        int res = reduce_sum(handle, params[i], "reduce_sum_contest");
//        if (res >= 0)
//            std::cout << "case " << i << " pass" << std::endl;
//        else
//            std::cout << "case " << i << " fail" << std::endl;
//        results[i] = res;
//    }
//    (void)(results);
    // Deinitialize.
    bm_dev_free(handle);
    return 0;
}

