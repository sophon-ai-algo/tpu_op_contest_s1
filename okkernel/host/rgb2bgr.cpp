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
    unsigned int size;
    unsigned long long output_addr;
    unsigned long long input_addr;
} __attribute__((packed)) param_t;


static inline void rgb2bgr_reference(float *output, const float *input, const param_t &param) {
    for(unsigned int i = 0; i < param.size; i++) {
        output[i*3+2] = input[i*3];
        output[i*3+1] = input[i*3+1];
        output[i*3] = input[i*3+2];
    }
}

int rgb2bgr(bm_handle_t &handle, param_t &param, const char *device_func_name) {
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real_distribution<float> dist_value{-1.f, 1.f};

    unsigned long long len = param.size*3;

    bm_device_mem_t output_dev, input_dev;
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &output_dev, len * sizeof(float)));
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &input_dev, len * sizeof(float)));
    param.output_addr = bm_mem_get_device_addr(output_dev);
    param.input_addr = bm_mem_get_device_addr(input_dev);

    float* input_host = new float[len];
    float* output_host = new float[len];
    float* output_ref = new float[len];
    for(unsigned long long i=0; i<len; i++) {
        input_host[i] = dist_value(rng);
    }
    rgb2bgr_reference(output_ref, input_host, param);
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
    for (unsigned long long i = 0; i < len; ++i) {
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
        {.size = 1024      }, // 0
        {.size = 4096      }, // 1
        {.size = 43691     }, // 2
        {.size = 81920     }, // 3
        {.size = 5120      }, // 4
        {.size = 2796202   }, // 5
        {.size = 2796203   }, // 6
        {.size = 5592404   }, // 7
        {.size = 699050    }, // 8
        {.size = 256       }, // 9
        {.size = 11184808  }, // 10
        {.size = 33554432  }, // 11
        {.size = 67108864  }, // 12
        {.size = 135544320 }, // 13
        {.size = 243523506 }, // 14
    };
    int results[sizeof(params) / sizeof(param_t)];
    for (unsigned int i = 0; i < sizeof(params) / sizeof(param_t); ++i) {
        int res = rgb2bgr(handle, params[i], "rgb2bgr_contest");
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

