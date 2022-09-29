#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "cmdline.h"
#include "ax_sys_api.h"
#include "ax_interpreter_external_api.h"
#include "ax_interpreter_external2_api.h"

static int32_t get_file_size(const char *fn) {
    assert(fn != nullptr && "fn is nullptr");
    struct stat st_buf;
    if (stat(fn, &st_buf) != 0) {
        printf("Access file %s failed, please make sure file exist and authority ok\n", fn);
        return -1;
    }
    return st_buf.st_size;
}

static int32_t load_file(const char *fn, void *buf, size_t buf_size) {
    assert(fn != nullptr && buf != nullptr && buf_size > 0 && "invalid params");
    FILE *fp = fopen(fn, "rb");
    if (fp == nullptr) {
        printf("[Errror] load_file: open file %s failed\n", fn);
        return -1;
    }
    assert(fseek(fp, 0, SEEK_END) == 0 && "fseek SEEK_END failed");
    size_t fs = ftell(fp);
    assert(fs > 0 && "empty file");
    assert(fseek(fp, 0, SEEK_SET) == 0 && "fseek SEEK_SET failed");
    assert(buf_size >= fs && "buffer not enough");
    assert(fread(buf, 1, fs, fp) == fs && "fread failed");
    fclose(fp);
    return 0;
}

static int32_t save_file(const char *fn, const void *data, size_t data_size) {
    assert(fn != nullptr && data != nullptr && data_size > 0 && "invalid params");
    FILE *fp = fopen(fn, "wb+");
    if (fp == nullptr) {
        printf("[Errror] save_file: open file %s failed\n", fn);
        return -1;
    }
    assert(fwrite(data, data_size, 1, fp) == 1 && "fwrite failed");
    fclose(fp);
    return 0;
}

static std::vector<std::string> split_string(const std::string& content, const std::string delimiter) {
    if (delimiter.size() == 0) return {content};
    std::vector<std::string> ret;
    int i = 0;
    while (i < (int) content.size()) {
        int j = i;
        while (j < (int) content.size() && content.substr(j, delimiter.size()) != delimiter) ++j;
        ret.emplace_back(content.substr(i, j - i));
        i = j + delimiter.size();
    }
    if (ret.empty()) ret.emplace_back("");
    return ret;
}

static std::unordered_map<std::string, std::string> parse_io_mappings(
    std::string argv,
    std::string unique_tensor_name = "",
    std::string io_delimiter = ";",
    std::string kv_delimiter = ":") {
    std::unordered_map<std::string, std::string> ret;
    if (argv.empty()) return ret;
    auto arr = split_string(argv, io_delimiter);
    for (auto& s: arr) {
        auto i = s.find(kv_delimiter);
        if (i == std::string::npos) {
            if (arr.size() > 1) {
                printf("var name not provided\n");
                assert(0);
            }
            ret[unique_tensor_name] = s;
        } else {
            ret[s.substr(0, i)] = s.substr(i + 1);
        }
    }
    if (arr.size() != ret.size()) {
        printf("duplicated vars provided\n");
        assert(0);
    }
    return ret;
}

static void brief_npu_handle(const AX_NPU_SDK_EX_IO_INFO_T *ioInfo) {
    for (AX_U32 i = 0; i < ioInfo->nInputSize; ++i) {
        auto &tensorMeta = ioInfo->pInputs[i];
        printf("input tensor\n");
        printf("    pName %s\n", (const char*)tensorMeta.pName);
        printf("    shape [");
        for (int j = 0; j < tensorMeta.nShapeNDim; ++j) {
            if (j > 0) printf(", ");
            printf("%d", tensorMeta.pShape[j]);
        }
        printf("]\n");
        printf("    nBit %d\n", tensorMeta.nBit);
        printf("    nSize %d\n", tensorMeta.nSize);
        printf("    nInnerSize %d\n", tensorMeta.nInnerSize);
        printf("    eMemoryType %d\n", tensorMeta.eMemoryType);
        printf("\n");
    }
    for (AX_U32 i = 0; i < ioInfo->nOutputSize; ++i) {
        auto &tensorMeta = ioInfo->pOutputs[i];
        printf("output tensor\n");
        printf("    pName %s\n", (const char*)tensorMeta.pName);
        printf("    shape [");
        for (int j = 0; j < tensorMeta.nShapeNDim; ++j) {
            if (j > 0) printf(", ");
            printf("%d", tensorMeta.pShape[j]);
        }
        printf("]\n");
        printf("    nBit %d\n", tensorMeta.nBit);
        printf("    nSize %d\n", tensorMeta.nSize);
        printf("    nInnerSize %d\n", tensorMeta.nInnerSize);
        printf("    eMemoryType %d\n", tensorMeta.eMemoryType);
        printf("\n");
    }
}

static AX_NPU_SDK_EX_HARD_MODE_T get_npu_mode_by_dot_neu_data(const void *data, size_t data_size) {
    assert(data != nullptr && data_size > 0 && "invalid data");
    AX_NPU_SDK_EX_MODEL_TYPE_T dot_neu_type = AX_NPU_MODEL_TYPE_DEFUALT;
    assert(AX_NPU_SDK_EX_Get_Dot_neu_type(data, data_size, &dot_neu_type) == 0 && "AX_NPU_SDK_EX_Get_Dot_neu_type failed");
    if (dot_neu_type == AX_NPU_MODEL_TYPE_DEFUALT) {
        return AX_NPU_VIRTUAL_DISABLE;
    } else if (dot_neu_type == AX_NPU_MODEL_TYPE_3_1_1 || dot_neu_type == AX_NPU_MODEL_TYPE_3_1_2) {
        return AX_NPU_VIRTUAL_3_1;
    } else if (dot_neu_type == AX_NPU_MODEL_TYPE_2_2_1 || dot_neu_type == AX_NPU_MODEL_TYPE_2_2_2){
        return AX_NPU_VIRTUAL_2_2;
    } else if (dot_neu_type == AX_NPU_MODEL_TYPE_1_1_1 || dot_neu_type == AX_NPU_MODEL_TYPE_1_1_2){
        return AX_NPU_VIRTUAL_1_1;
    } else {
        printf("unknown dot-neu model type %d\n", (int)dot_neu_type);
        assert(0);
    }
}

static const char* get_npu_mode_string(AX_NPU_SDK_EX_HARD_MODE_T npu_mode) {
    if (npu_mode == AX_NPU_VIRTUAL_3_1) {
        return "VIRTUAL_3_1";
    } else if (npu_mode == AX_NPU_VIRTUAL_2_2) {
        return "VIRTUAL_2_2";
    } else if (npu_mode == AX_NPU_VIRTUAL_1_1) {
        return "VIRTUAL_1_1";
    } else if (npu_mode == AX_NPU_VIRTUAL_DISABLE) {
        return "VIRTUAL_DISABLE";
    } else {
        return "Unknown NPU Mode";
    }
}
static void create_npu_io(AX_NPU_SDK_EX_IO_T *npu_io, const AX_NPU_SDK_EX_IO_INFO_T *io_info) {
    assert(npu_io != nullptr && io_info != nullptr && "invalid parameters");

    npu_io->nInputSize = io_info->nInputSize;
    npu_io->pInputs = (AX_NPU_SDK_EX_BUF_T*)malloc(io_info->nInputSize * sizeof(AX_NPU_SDK_EX_BUF_T));
    for(AX_U32 i = 0; i < io_info->nInputSize; ++i) {
        AX_NPU_SDK_EX_TENSOR_META_T *cur_info = io_info->pInputs + i;
        AX_NPU_SDK_EX_BUF_T *cur_input = npu_io->pInputs + i;
        std::string ts_name((const char*)cur_info->pName);
        AX_S32 ret = AX_NPU_SDK_EX_Alloc_buffer(cur_info, cur_input, AX_NPU_ABST_DEFAULT);
        if (ret != AX_NPU_TASK_STATUS_SUCCESS) {
            printf("AX_NPU_SDK_EX_Alloc_buffer for INPUT(%s) failed, error code %x\n",
                    ts_name.c_str(), (AX_U32)ret);
            assert(0);
        }
    }
    npu_io->nOutputSize = io_info->nOutputSize;
    npu_io->pOutputs = (AX_NPU_SDK_EX_BUF_T*)malloc(io_info->nOutputSize * sizeof(AX_NPU_SDK_EX_BUF_T));
    for (AX_U32 i = 0; i < io_info->nOutputSize; ++i) {
        AX_NPU_SDK_EX_TENSOR_META_T *cur_info = io_info->pOutputs + i;
        AX_NPU_SDK_EX_BUF_T *cur_output = npu_io->pOutputs + i;
        std::string ts_name((const char*)cur_info->pName);
        AX_S32 ret = AX_NPU_SDK_EX_Alloc_buffer(cur_info, cur_output, AX_NPU_ABST_DEFAULT);
        if (ret != AX_NPU_TASK_STATUS_SUCCESS) {
            printf("AX_NPU_SDK_EX_Alloc_buffer for OUTPUT(%s) failed, error code %x\n",
                    ts_name.c_str(), (AX_U32)ret);
            assert(0);
        }
        memset(cur_output->pInnerVirAddr, 0, cur_output->nInnerSize);
    }
}

static void destroy_npu_io(AX_NPU_SDK_EX_IO_T *npu_io) {
    if (npu_io == nullptr) {
        return;
    }
    for (AX_U32 i = 0; i < npu_io->nInputSize; ++i) {
        assert(0 == AX_NPU_SDK_EX_Free_buffer(&npu_io->pInputs[i]) && "AX_NPU_SDK_EX_Free_buffer failed");
    }
    free(npu_io->pInputs);
    npu_io->pInputs = nullptr;
    npu_io->nInputSize = 0;

    for (AX_U32 i = 0; i < npu_io->nOutputSize; ++i) {
        assert(0 == AX_NPU_SDK_EX_Free_buffer(&npu_io->pOutputs[i]) && "AX_NPU_SDK_EX_Free_buffer failed");
    }
    free(npu_io->pOutputs);
    npu_io->pOutputs = nullptr;
    npu_io->nOutputSize = 0;
}

static void load_input_data(
    AX_NPU_SDK_EX_IO_T *npu_io,
    const AX_NPU_SDK_EX_IO_INFO_T *io_info,
    const std::unordered_map<std::string, std::string> &input_mappings) {
    assert(npu_io != nullptr && io_info != nullptr && "invalid parameters");

    for(AX_U32 i = 0; i < io_info->nInputSize; ++i) {
        AX_NPU_SDK_EX_BUF_T *cur_input = npu_io->pInputs + i;
        std::string ts_name((const char*)io_info->pInputs[i].pName);
        if (input_mappings.count(ts_name)) {
            std::string fn = input_mappings.find(ts_name)->second;
            int32_t fs = get_file_size(fn.c_str());
            if (fs <= 0 || (AX_U32)fs != cur_input->nSize) {
                printf("Input file %s size error, file size %d, tensor required size %u\n",
                        fn.c_str(), fs, cur_input->nSize);
                assert(0);
            }
            assert(load_file(fn.c_str(), cur_input->pVirAddr, cur_input->nSize) == 0);
            printf("Init INPUT '%s' with file '%s'\n", ts_name.c_str(), fn.c_str());
        } else {
            srand(time(nullptr));
            AX_U32 rand_count = cur_input->nSize / sizeof(int); // int rand(void);
            int *buf_ptr = (int*)cur_input->pVirAddr;
            for (AX_U32 n = 0; n < rand_count; ++n) {
                buf_ptr[n] = rand();
            }
            printf("Init INPUT '%s' with random data\n", ts_name.c_str()); // TODO random data
        }
    }
}

static void check_npu_output(
    const AX_NPU_SDK_EX_IO_T *io,
    const AX_NPU_SDK_EX_IO_INFO_T *io_info,
    const std::unordered_map<std::string, std::string> &output_mappings) {
    assert(io != nullptr && io_info != nullptr && "Invalid parameters");

    for (AX_U32 i = 0; i < io->nOutputSize; ++i) {
        std::string ts_name((const char*)io_info->pOutputs[i].pName);
        if (output_mappings.count(ts_name) == 0) {
            printf("No GT file for OUTPUT(%s), ignore it's correctness check\n", ts_name.c_str());
            continue;
        }
        std::string fn = output_mappings.find(ts_name)->second;
        int32_t fs = get_file_size(fn.c_str());
        if (fs <= 0 || (AX_U32)fs != io->pOutputs[i].nInnerSize) {
            printf("GT file for OUTPUT(%s) size error, file size %d, tensor required size %u, ignore it's correctness check\n",
                    ts_name.c_str(), fs, io->pOutputs[i].nInnerSize);
            continue;
        }
        void *buf = malloc(fs);
        if (buf == nullptr) {
            printf("malloc buffer for OUTPUT(%s) GT file failed, ignore it's correctness check\n", ts_name.c_str());
            continue;
        }
        if (load_file(fn.c_str(), buf, fs) != 0) {
            printf("Read GT file for OUTPUT(%s) failed, ignore it's correctness check\n", ts_name.c_str());
            free(buf);
            continue;
        }
        if (memcmp(io->pOutputs[i].pInnerVirAddr, buf, fs) == 0) {
            printf("OUTPUT(%s) GT check success\n", ts_name.c_str());
        } else {
            printf("OUTPUT(%s) GT check failed\n", ts_name.c_str());
        }
        free(buf);
    }
}

static void dump_IO(const AX_NPU_SDK_EX_IO_T *io, const AX_NPU_SDK_EX_IO_INFO_T *io_info, const std::string &out_dir) {
    assert(io != nullptr && io_info != nullptr && "Invalid parameters");
    if (out_dir.empty()) {
        return;
    }

    struct stat info;
    if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        printf("Can not access directory '%s', cancel dump IO\n", out_dir.c_str());
        return;
    }
    for (AX_U32 i = 0; i < io->nInputSize; ++i) {
        std::string fn = out_dir + "/" + (const char*)io_info->pInputs[i].pName + ".bin";
        assert(save_file(fn.c_str(), io->pInputs[i].pVirAddr, io->pInputs[i].nSize) == 0);
    }
    for (AX_U32 i = 0; i < io->nOutputSize; ++i) {
        std::string fn = out_dir + "/" + (const char*)io_info->pOutputs[i].pName + ".bin";
        assert(save_file(fn.c_str(), io->pOutputs[i].pVirAddr, io->pOutputs[i].nSize) == 0);

        std::string inner_fn = out_dir + "/" + (const char*)io_info->pOutputs[i].pName + "_inner.bin";
        assert(save_file(inner_fn.c_str(), io->pOutputs[i].pInnerVirAddr, io->pOutputs[i].nInnerSize) == 0);
    }
    printf("dump IO data to %s done\n", out_dir.c_str());
}

static bool is_io_input_tensor_name_valid(const std::string &tensor_name, const AX_NPU_SDK_EX_IO_INFO_T *io_info) {
    if (io_info == nullptr) {
        printf("Invalid param!\n");
        return false;
    }
    for (size_t i = 0; i < io_info->nInputSize; ++i) {
        if (tensor_name == (const char *)(io_info->pInputs[i].pName)) {
            return true;
        }
    }
    return false;
}

static bool is_io_output_tensor_name_valid(const std::string &tensor_name, const AX_NPU_SDK_EX_IO_INFO_T *io_info) {
    if (io_info == nullptr) {
        printf("Invalid param!\n");
        return false;
    }
    for (size_t i = 0; i < io_info->nOutputSize; ++i) {
        if (tensor_name == (const char *)(io_info->pOutputs[i].pName)) {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[]) {
    cmdline::parser args;
    args.add<std::string>("neu", '\0', "dot-neu", true);
    args.add<std::string>(
        "data", 'd',
        "The format is \"var0:file0;var1:file1...\" to specify data files for input vars.\n\
\t'file*' would be directly loaded in binary format to tensor 'var*'\n\
\tIf there is only one input var, 'var0' can be omitted.",
        false
    );
    args.add_vector<std::string>(
        "input", 'i',
        "This flag is used to specify input data, format is --input file0 --input file1 --input file2...\n\
\tIf --input_tensor is not set, user should guarantee file order specified by this flag is same to dot-neu's input order, see dot-neu's input order in IO info.\n\
\tIf --input_tensor is set, file specified by --input[i] would be directly loaded in binary format to input tensor specified by --input_tensor[i].\n\
\t'--data' would be ignored when set this flag.",
        false
    );
    args.add_vector<std::string>(
        "input_tensor", '\0',
        "This flag is used to specify input tensor names, format is --input_tensor tensor0_name --input_tensor tensor1_name --input_tensor tensor2_name...\n\
\tcooperating with flag --input to load input data, get more details in the introduction of --input.",
        false
    );
    args.add_vector<std::string>(
        "check", '\0',
        "This flag is used to specify GT files of NPU directly output, format is --check file0 --check file1 --check file2...\n\
\tIf --output_tensor is not set, user should guarantee file order specified by this flag is same to dot-neu's output order, see dot-neu's output order in IO info.\n\
\tIf --output_tensor is set, file specified by --check[i] would be used to do byte correctness check of NPU directly output specified by --output_tensor[i].\n\
\tOnly NPU directly output could do correctness check, also known as \"xxx_inner.bin\" in bin-out-dir.",
        false
    );
    args.add_vector<std::string>(
        "output_tensor", '\0',
        "This flag is used to specify output tensor names, format is --output_tensor tensor0_name --output_tensor tesnor1_name --output_tensor tensor2_name...\n\
\tcooperating with flag --check to load output GT data, get more details in the introduction of --check.",
        false
    );
    args.add<std::string>("rtv", '\0', "RuntimeVars generated by librosetta.", false);
    args.add<int>("repeat", '\0', "Repeat times for inference.", false, 1);
    args.add<std::string>("bin-out-dir", '\0', "Dump input and output tensors in binary format.", false);
    args.add<int>(
        "batch-size", '\0',
        "Inference batchsize, ignored when model not support dynamic batch.\n\
\t0 means default batch size, usually max batch size.",
        false, 0
    );
    args.add<int>("batch-type", '\0', "1 for BLOCK and 2 for GATHER, ignored when model not support dynamic batch", false, 1);
    args.add<int>("show-io-info", '\0', "Enable or disable show dot-neu's IO info", false, 1);
    args.add<int>("enable-npu-cycle", '\0', "Enable or disable calculate and show NPU cycle", false, 1);
    args.add<int>("only-info", '\0', "Only show dot-neu info, no inference", false, 0);
    args.add<int>("wbt-index", '\0', "select WBT for inference", false, 0); 

    args.parse_check(argc, argv);

    // get params
    std::string arg_neu = args.get<std::string>("neu");
    std::string arg_data = args.get<std::string>("data");
    std::vector<std::string> arg_input = args.get_vector<std::string>("input");
    std::vector<std::string> arg_input_tensor = args.get_vector<std::string>("input_tensor");
    std::vector<std::string> arg_check = args.get_vector<std::string>("check");
    std::vector<std::string> arg_output_tensor = args.get_vector<std::string>("output_tensor");
    std::string arg_rtv = args.get<std::string>("rtv");
    const int arg_repeat = args.get<int>("repeat");
    std::string arg_bin_out_dir = args.get<std::string>("bin-out-dir");
    bool show_io_info = (args.get<int>("show-io-info") == 1);
    bool enable_npu_cycle = (args.get<int>("enable-npu-cycle") == 1);
    bool only_info = (args.get<int>("only-info") == 1);
    int wbt_index = args.get<int>("wbt-index");

    if (arg_neu.empty()) {
        printf("[Errror] neu parameter is empty\n");
        return -1;
    }
    if (arg_repeat <= 0) {
        printf("[Errror] invalid repeat parameter %d\n", arg_repeat);
        return -1;
    }
    if (wbt_index < 0) {
        printf("[Errror] invalid WBT index %d\n", wbt_index);
        return -1;
    }

    AX_S32 ret = AX_SYS_Init();
    if (ret != 0) {
        printf("AX_SYS_Init failed, error code %x\n", (AX_U32)ret);
        assert(0);
    }

    int32_t dot_neu_size = get_file_size(arg_neu.c_str());
    if (dot_neu_size <= 0) {
        printf("Invalid dot-neu file '%s'\n", arg_neu.c_str());
        assert(0);
    }
    void *dot_neu_data = malloc(dot_neu_size);
    assert(dot_neu_data != nullptr && "malloc dot_neu buffer failed");
    assert(load_file(arg_neu.c_str(), dot_neu_data, dot_neu_size) == 0);

    AX_NPU_SDK_EX_ATTR_T npu_mode;
    npu_mode.eHardMode = get_npu_mode_by_dot_neu_data(dot_neu_data, dot_neu_size);
    printf("NPU Mode: %s\n", get_npu_mode_string(npu_mode.eHardMode));
    ret = AX_NPU_SDK_EX_Init_with_attr(&npu_mode);
    if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
        printf("AX_NPU_SDK_EX_Init_with_attr failed, error code %x\n", (AX_U32)ret);
        assert(0);
    }

    AX_NPU_SDK_EX_HANDLE_T dot_neu_handle = nullptr;
    ret = AX_NPU_SDK_EX_Create_handle(&dot_neu_handle, dot_neu_data, dot_neu_size);
    if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
        printf("AX_NPU_SDK_EX_Create_handle failed, error code %x\n", (AX_U32)ret);
        assert(0);
    }
    if (dot_neu_data != nullptr) {
        free(dot_neu_data);
        dot_neu_data = nullptr;
    }

    AX_CHAR **wbt_names = nullptr;
    AX_U32 wbt_count = 0;
    ret = AX_NPU_SDK_EX_Get_wbt_params_list(dot_neu_handle, &wbt_names, &wbt_count);
    if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
        printf("AX_NPU_SDK_EX_Get_wbt_params_list failed, error code %x\n", (AX_U32)ret);
        assert(0);
    } else {
        printf("WBT count %u\n", wbt_count);
        // assert(wbt_names != nullptr && "invalid WBT names");
        // for (AX_U32 i = 0; i < wbt_count; ++i) {
        //     printf("WBT[%u] name %s\n", i, (wbt_names[i] == nullptr ? "" : wbt_names[i]));
        // }
        if (wbt_count == 0) {
            printf("[Warning] the model has no wbt-param, wbt-index will be ignored\n");
        } else {
            if ((AX_U32)wbt_index >= wbt_count) {
                printf("Invalid wbt index %d\n", wbt_index);
                assert(0);
            }
        }
    }

    AX_NPU_SDK_EX_ExtraParametersInfo extra_params_info;
    ret = AX_NPU_SDK_EX_Get_extra_parameters_info(dot_neu_handle, &extra_params_info);
    if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
        printf("AX_NPU_SDK_EX_Get_extra_parameters_info failed, error code %x\n", (AX_U32)ret);
        assert(0);
    }
    if (extra_params_info.bHasRuntimeVar == AX_TRUE) {
        printf("RuntimeVar: Yes\n");
    } else {
        printf("RuntimeVar: No\n");
    }

    const AX_NPU_SDK_EX_IO_INFO_T *io_info = nullptr;
    AX_U32 max_batch_size = 0;
    ret = AX_NPU_SDK_EX2_GetMaxBatchSize(dot_neu_handle, &max_batch_size);
    if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
        printf("AX_NPU_SDK_EX2_GetMaxBatchSize failed, error code %x", (AX_U32)ret);
        assert(0);
    }
    printf("Max batch size: %u\n", max_batch_size);
    bool support_db = (AX_NPU_SDK_EX2_IsSupportDynamicBatch(dot_neu_handle) == AX_TRUE);
    printf("Support Dynamic Batch: %s\n", (support_db ? "Yes" : "No"));

    AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T dot_neu_context = nullptr;
    AX_U32 batch_size = max_batch_size;
    if (support_db) {
        const int arg_batch_size = args.get<int>("batch-size");
        if (arg_batch_size < 0 || (AX_U32)arg_batch_size > max_batch_size) {
            printf("Invalid batch size %d, batch size should in range [0, %u]\n",
                    arg_batch_size, max_batch_size);
            assert(0);
        }
        const int arg_batch_type = args.get<int>("batch-type");
        if (arg_batch_type != (int)AX_NPU_BATCH_TYPE_BLOCK &&
            arg_batch_type != (int)AX_NPU_BATCH_TYPE_GATHER) {
            printf("Invalid batch type %d, batch type should be %d (means BLOCK) or %d (means GATHER)\n",
                    arg_batch_type, (int)AX_NPU_BATCH_TYPE_BLOCK, (int)AX_NPU_BATCH_TYPE_GATHER);
            assert(0);
        }

        batch_size = arg_batch_size > 0 ? arg_batch_size : max_batch_size;
        AX_NPU_SDK_EX2_BATCH_INFO_T batch_info;
        memset(&batch_info, 0, sizeof(batch_info));
        batch_info.eBatchType = (AX_NPU_SDK_EX2_BATCH_TYPE_E)arg_batch_type;
        batch_info.nBatchSize = batch_size;

        ret = AX_NPU_SDK_EX2_CreateExecutionContext(dot_neu_handle, &dot_neu_context, &batch_info);
        if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
            printf("AX_NPU_SDK_EX2_CreateExecutionContext failed, error code %X\n", (AX_U32)ret);
            assert(0);
        }
        io_info = AX_NPU_SDK_EX2_GetIOInfo(dot_neu_context);
    } else {
        io_info = AX_NPU_SDK_EX_Get_io_info(dot_neu_handle);
    }

    assert(io_info != nullptr && "AX_NPU_SDK_EX_Get_io_info failed");
    if (show_io_info) {
        printf("\n");
        brief_npu_handle(io_info);
    }

    if (only_info) {
        if (dot_neu_context != nullptr) {
            AX_NPU_SDK_EX2_DestroyExecutionContext(dot_neu_context);
            dot_neu_context = nullptr;
        }
        if (dot_neu_handle != nullptr) {
            AX_NPU_SDK_EX_Destroy_handle(dot_neu_handle);
            dot_neu_handle = nullptr;
        }
        printf("Deinit NPU Driver +++\n");
        AX_NPU_SDK_EX_Deinit();
        printf("NPU Driver Exited ---\n");
        return 0;
    }

    assert(io_info->nInputSize > 0 && "dot-neu input size is invalid");
    assert(io_info->nOutputSize > 0 && "dot-neu output size is invalid");

    void *rtv_data = nullptr;
    AX_NPU_SDK_EX_ExtraParameters extra_params;
    memset(&extra_params, 0, sizeof(extra_params));
    extra_params.nWbtIndex = wbt_index;
    if (extra_params_info.bHasRuntimeVar == AX_TRUE) {
        if (arg_rtv.empty()) {
            printf("Dot-neu '%s' needs RuntimeVar, but not provided\n", arg_neu.c_str());
            assert(0);
        }
        int32_t rtv_fs = get_file_size(arg_rtv.c_str());
        if (rtv_fs <= 0) {
            printf("Invalid RuntimeVar file '%s'\n", arg_rtv.c_str());
            assert(0);
        }
        rtv_data = malloc(rtv_fs);
        if (rtv_data == nullptr) {
            printf("Malloc RuntimeVar buffer failed\n");
            assert(0);
        }
        assert(load_file(arg_rtv.c_str(), rtv_data, rtv_fs) == 0);
        printf("Load RuntimeVar File '%s' success, size %d\n", arg_rtv.c_str(), rtv_fs);
        extra_params.tRuntimeVar = rtv_data;
        extra_params.nRuntimeVarSize = rtv_fs;
    }

    AX_NPU_SDK_EX_IO_T npu_io;
    memset(&npu_io, 0, sizeof(npu_io));
    create_npu_io(&npu_io, io_info);
    std::unordered_map<std::string, std::string> input_mappings;
    if (!arg_input.empty()) { // if set --input, ignore --data
        if (arg_input_tensor.empty()) {
            for (size_t i = 0; i < arg_input.size() && i < io_info->nInputSize; ++i) {
                input_mappings[(const char*)io_info->pInputs[i].pName] = arg_input[i];
            }
        } else {
            if (arg_input_tensor.size() != arg_input.size()) {
                printf("The amount(%u) of input tensor names specified by '--input_tensor'"
                        " and the amount(%u) of input files specified by '--input'"
                        " are mismatch.\n"
                    , arg_input_tensor.size(), arg_input.size()
                    );
                assert(0);
            }

            for (size_t i = 0; i < arg_input.size() && i < io_info->nInputSize; ++i) {
                if (is_io_input_tensor_name_valid(arg_input_tensor[i], io_info)) {
                    input_mappings[arg_input_tensor[i]] = arg_input[i];
                } else {
                    printf("Invalid input tensor name %s\n", arg_input_tensor[i].c_str());
                    assert(0);
                }
            }
        }
    } else {
        input_mappings = parse_io_mappings(arg_data.c_str(), (const char*)io_info->pInputs[0].pName);
    }
    load_input_data(&npu_io, io_info, input_mappings);

    uint64_t total_cycle_sum = 0;
    uint32_t valid_eu_num = 0;
    uint64_t eu_cycle_sum[9];
    memset(eu_cycle_sum, 0, sizeof(eu_cycle_sum));
    uint64_t total_time_cost_us = 0;
    for (int i = 0; i < arg_repeat; ++i) {
        auto time_start = std::chrono::system_clock::now();
        if (support_db) {
            ret = AX_NPU_SDK_EX2_RunTaskSync(dot_neu_context, &npu_io, &extra_params);
        } else {
            ret = AX_NPU_SDK_EX_Run_task_sync_v3(dot_neu_handle, &npu_io, &extra_params);
        }
        auto time_end = std::chrono::system_clock::now();
        total_time_cost_us += std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count();

        if (ret != AX_NPU_TASK_STATUS_SUCCESS) {
            printf("AX_NPU_SDK Inference failed, error code %x\n", (AX_U32)ret);
            assert(0);
        }

        if (enable_npu_cycle) {
            AX_NPU_SDK_EX_WORK_CYCLE_T work_cycle;
            memset(&work_cycle, 0, sizeof(work_cycle));
            ret = AX_NPU_SDK_EX_Get_Work_Cycle(&work_cycle);
            if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
                printf("AX_NPU_SDK_EX_Get_Work_Cycle failed, error code %x\n", (AX_U32)ret);
                assert(0);
            }
            total_cycle_sum += work_cycle.total_cycle;
            valid_eu_num = work_cycle.valid_eu_num;
            for (uint32_t i = 0; i < work_cycle.valid_eu_num; ++i) {
                eu_cycle_sum[i] += work_cycle.eu_cycle[i];
            }
        }
    }

    if (rtv_data != nullptr) {
        free(rtv_data);
        rtv_data = nullptr;
    }

    printf("\nRun task took %llu us (%d rounds for average)\n\n", total_time_cost_us / arg_repeat, arg_repeat);

    if (enable_npu_cycle) {
        printf("Run model took %llu cycle, (%d rounds for average)\n", total_cycle_sum / arg_repeat, arg_repeat);
        for (uint32_t i = 0; i < valid_eu_num; ++i) {
            printf("    EU %d took %llu cycle, (%d rounds for average)\n", i, eu_cycle_sum[i] / arg_repeat, arg_repeat);
        }
        printf("\n");
    }

    // output correctness check
    if (!arg_check.empty()) {
        std::unordered_map<std::string, std::string> output_mappings;
        if (arg_output_tensor.empty()) {
            for (size_t i = 0; i < arg_check.size() && i < io_info->nOutputSize; ++i) {
                output_mappings[(const char*)io_info->pOutputs[i].pName] = arg_check[i];
            }
        } else {
            if (arg_output_tensor.size() != arg_check.size()) {
                printf("The amount(%u) of output tensor names specified by '--output_tensor'"
                        " and the amount(%u) of gt files specified by '--check'"
                        " are mismatch.\n"
                    , arg_output_tensor.size(), arg_check.size()
                    );
                assert(0);
            }

            for (size_t i = 0; i < arg_check.size() && i < io_info->nOutputSize; ++i) {
                if (is_io_output_tensor_name_valid(arg_output_tensor[i], io_info)) {
                    output_mappings[arg_output_tensor[i]] = arg_check[i];
                } else {
                    printf("Invalid output tensor name %s\n", arg_output_tensor[i].c_str());
                    assert(0);
                }
            }
        }
        check_npu_output(&npu_io, io_info, output_mappings);
    }

    // dump IO
    if (!arg_bin_out_dir.empty()) {
        dump_IO(&npu_io, io_info, arg_bin_out_dir);
    }

    // release
    destroy_npu_io(&npu_io);
    if (dot_neu_context != nullptr) {
        AX_NPU_SDK_EX2_DestroyExecutionContext(dot_neu_context);
        dot_neu_context = nullptr;
    }
    if (dot_neu_handle != nullptr) {
        AX_NPU_SDK_EX_Destroy_handle(dot_neu_handle);
        dot_neu_handle = nullptr;
    }
    printf("Deinit NPU Driver +++\n");
    AX_NPU_SDK_EX_Deinit();
    printf("NPU Driver Exited ---\n");

    if (rtv_data != nullptr) {
        free(rtv_data);
        rtv_data = nullptr;
    }
    if (dot_neu_data != nullptr) {
        free(dot_neu_data);
        dot_neu_data = nullptr;
    }

    return 0;
}
