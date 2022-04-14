#include <assert.h>
#include <iostream>
#include <random>
#include <sys/time.h>
#include "okk_param.h"
#include "bmlib_runtime.h"
#define BMLIB_SAFE_CALL(cmd) assert(cmd == BM_SUCCESS)
#define DIV_UP(a, b) (((a) - 1) / (b) + 1)
#define MAXIT (10)
typedef struct {
    unsigned long long output_addr;
    unsigned long long input_addr;
    int N, C, H, W;
    int kernel_h, kernel_w;
    int pad_top, pad_bottom, pad_left, pad_right;
    int stride_h, stride_w;
    int ceil_mode;
    int count_include_pad;
} __attribute__((packed)) param_t;

static inline void avg_pool_reference(float *output, const float *input, const param_t &param) {
    int output_h = param.H + param.pad_top + param.pad_bottom - param.kernel_h;
    int output_w = param.W + param.pad_left + param.pad_right - param.kernel_w;
    if (param.ceil_mode) {
        output_h = DIV_UP(output_h, param.stride_h) + 1;
        output_w = DIV_UP(output_w, param.stride_w) + 1;
    } else {
        output_h = output_h / param.stride_h + 1;
        output_w = output_w / param.stride_w + 1;
    }
    for (int n = 0; n < param.N; ++n) {
        for (int c = 0; c < param.C; ++c) {
            for (int oh = 0; oh < output_h; ++oh) {
                for (int ow = 0; ow < output_w; ++ow) {
                    int hstart = oh * param.stride_h;
                    int wstart = ow * param.stride_w;
                    int hend = std::min(hstart + param.kernel_h, param.pad_top + param.pad_bottom + param.H);
                    int wend = std::min(wstart + param.kernel_w, param.pad_left + param.pad_right + param.W);
                    int pool_size = (hend - hstart) * (wend - wstart);
                    hstart = std::max(hstart - param.pad_top, 0);
                    wstart = std::max(wstart - param.pad_left, 0);
                    hend = std::min(hend - param.pad_top, param.H);
                    wend = std::min(wend - param.pad_left, param.W);
                    if (!param.count_include_pad)
                        pool_size = (hend - hstart) * (wend - wstart);
                    output[oh * output_w + ow] = 0.f;
                    for (int h = hstart; h < hend; ++h) {
                        for (int w = wstart; w < wend; ++w)
                            output[oh * output_w + ow] += input[h * param.W + w];
                    }
                    output[oh * output_w + ow] /= pool_size;
                }
            }
            output += output_h * output_w;
            input += param.H * param.W;
        }
    }
}

int avg_pool(bm_handle_t &handle, param_t &param, const char *device_func_name) {
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real_distribution<float> dist_value;
    float *output_host = nullptr, *input_host = nullptr, *output_ref = nullptr;
    int output_h = param.H + param.pad_top + param.pad_bottom - param.kernel_h;
    int output_w = param.W + param.pad_left + param.pad_right - param.kernel_w;
    if (param.ceil_mode) {
        output_h = DIV_UP(output_h, param.stride_h) + 1;
        output_w = DIV_UP(output_w, param.stride_w) + 1;
    } else {
        output_h = output_h / param.stride_h + 1;
        output_w = output_w / param.stride_w + 1;
    }
    int input_len = param.N * param.C * param.H * param.W;
    int output_len = param.N * param.C * output_h * output_w;
    bm_device_mem_t output_dev, input_dev;
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &input_dev, input_len * sizeof(float)));
    BMLIB_SAFE_CALL(bm_malloc_device_byte(handle, &output_dev, output_len * sizeof(float)));
    param.output_addr = bm_mem_get_device_addr(output_dev);
    param.input_addr = bm_mem_get_device_addr(input_dev);
    output_host = new float[output_len];
    output_ref = new float[output_len];
    input_host = new float[input_len];
    for (int i = 0; i < input_len; ++i)
        input_host[i] = dist_value(rng);
    avg_pool_reference(output_ref, input_host, param);
    BMLIB_SAFE_CALL(bm_memcpy_s2d(handle, input_dev, input_host));
    struct timeval start_time, end_time;
    long long elapsed_time = 0;
    for (int i = 0; i < MAXIT; ++i) {
        gettimeofday(&start_time, NULL);
        BMLIB_SAFE_CALL(okkernel_launch_sync(handle, device_func_name, &param, sizeof(param)));
        gettimeofday(&end_time, NULL);
        elapsed_time += (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
    }
    BMLIB_SAFE_CALL(bm_memcpy_d2s(handle, output_host, output_dev));
    bool pass = true;
    for (int i = 0; i < output_len; ++i) {
        if (!std::isfinite(output_host[i]) && !std::isfinite(output_ref[i]))
            continue;
        float max_val = std::max(std::fabs(output_host[i]), std::fabs(output_ref[i]));
        if (!(std::fabs(output_host[i] - output_ref[i]) < 1e-4 * std::max(max_val, 1.f))) {
            pass = false;
            break;
        }
    }
    int res = -1;
    if (pass) {
        res = std::round(elapsed_time / (double)MAXIT);
        std::cout << "elapsed time: " << res << "(us)" << std::endl;
    }
    bm_free_device(handle, output_dev);
    bm_free_device(handle, input_dev);
    delete [] output_host;
    delete [] output_ref;
    delete [] input_host;
    return res;
}
int main() {
    bm_handle_t handle;
    // Initialize.
    BMLIB_SAFE_CALL(bm_dev_request(&handle, 0));
    param_t param;
    std::vector<pool_t> params;
    read_param("./param/pool.dat", params);
    int param_size = params.size();
    pool_t pm;
    for (int i = 0; i < param_size; ++i) {
        pm = params[i];
        if ((pm.is_gloabl_pool != 0) || (pm.is_avgpool == 0))
            continue;
        param.N = 4;
        param.C = pm.C;
        param.H = pm.H;
        param.W = pm.W;
        param.kernel_h = pm.kh;
        param.kernel_w = pm.kw;
        param.pad_top = pm.pad_up_h;
        param.pad_bottom = pm.pad_down_h;
        param.pad_left = pm.pad_left_w;
        param.pad_right = pm.pad_right_w;
        param.stride_h = pm.stride_h;
        param.stride_w = pm.stride_w;
        param.ceil_mode = pm.out_ceil_mode;
        param.count_include_pad = rand() % 2;
        avg_pool(handle, param, "avg_pool_0");
    }
    // Deinitialize.
    bm_dev_free(handle);
    return 0;
}
