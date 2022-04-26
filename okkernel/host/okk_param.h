#include <vector>
#include <fstream>
typedef struct {
    int N;
    int C;
    int H;
    int W;
    int kh;
    int kw;
    int pad_up_h;
    int pad_down_h;
    int pad_left_w;
    int pad_right_w;
    int stride_h;
    int stride_w;
    int is_avgpool;
    int avgpool_mode;
    int is_gloabl_pool;
    int out_ceil_mode;
} __attribute__((packed)) pool_t;

typedef struct {
    int N;
    int C;
    int H;
    int W;
    int ic;
    int oc;
    int kh;
    int kw;
    int group;
    int pad_up_h;
    int pad_down_h;
    int pad_left_w;
    int pad_right_w;
    int stride_h;
    int stride_w;
    int dh;
    int dw;
    int have_bias;
} __attribute__((packed)) conv_t;

template<class M>
int read_param(const std::string &dir, std::vector<M> &param) {
    std::ifstream f(dir, std::ios::binary | std::ios::in);
    if (!f) {
        std::cout << "Failed to open " << &dir << std::endl;
        exit(-1);
    }
    f.seekg(0, std::ios::end);
    int file_size = f.tellg();
    int param_len = file_size / (sizeof(M));
    M buffer;
    f.seekg(0, std::ios::beg);
    for (int i = 0; i < param_len; ++i) {
        f.read(reinterpret_cast<char*>(&buffer), sizeof(M));
        param.push_back(buffer);
    }
    f.close();
    return 0;
}
