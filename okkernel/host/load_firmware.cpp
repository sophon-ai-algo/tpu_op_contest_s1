#include "bmlib_runtime.h"
#include <stdio.h>
int main(int argc, char *argv[]) {
    const char *tcm_path = "./firmware_tcm.bin";
    const char *ddr_path = "./firmware_ddr.bin";
    if (argc != 1 && argc != 3) {
        printf("Parameter error!\n");
        printf("Please use: load_firmware path/to/firmware_tcm.bin path/to/firmware_ddr.bin\n");
        return -1;
    }
    if (argc == 3) {
        tcm_path = argv[1];
        ddr_path = argv[2];
    }
    bm_handle_t handle;
    bm_status_t ret = BM_SUCCESS;
    ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS)
        return -1;
    ret = okkernel_load_firmware(handle, tcm_path, ddr_path);
    if (ret != BM_SUCCESS) {
        bm_dev_free(handle);
        return -1;
    }
    bm_dev_free(handle);
    return 0;
}
