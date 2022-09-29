#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <assert.h>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include "cmdline.h"
#include "ax_sys_api.h"
#include "ax_interpreter_external_api.h"
#include "picojson.h"
#include <mutex>
#include <condition_variable>
#include <atomic>

static std::vector<std::string> split_string(const std::string& content, const std::string delimitter) {
    if (delimitter.size() == 0) return {content};
    std::vector<std::string> ret;
    int i = 0;
    while (i < (int) content.size()) {
        int j = i;
        while (j < (int) content.size() && content.substr(j, delimitter.size()) != delimitter) ++j;
        ret.emplace_back(content.substr(i, j - i));
        i = j + delimitter.size();
    }
    if (ret.empty()) ret.emplace_back("");
    return ret;
}

static std::unordered_map<std::string, std::string> parse_input_mappings(std::string argv, std::string unique_tensor_name="") {
    std::unordered_map<std::string, std::string> ret;
    if (argv.empty()) return ret;
    auto arr = split_string(argv, ";");
    for (auto& s: arr) {
        auto i = s.find(":");
        if (i == std::string::npos) {
            if (arr.size() > 1) {
                fprintf(stderr, "input var name not provided\n");
                exit(1);
            }
            ret[unique_tensor_name] = s;
        } else {
            ret[s.substr(0, i)] = s.substr(i + 1);
        }
    }
    if (arr.size() != ret.size()) {
        fprintf(stderr, "duplicated input vars provided\n");
        exit(1);
    }
    return ret;
}

static std::string loadFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open() || ifs.fail()) return "";
    return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

static AX_VOID brief_npu_handle(const AX_NPU_SDK_EX_IO_INFO_T *ioInfo) {
    for (AX_U32 i = 0; i < ioInfo->nInputSize; ++i) {
        auto &tensorMeta = ioInfo->pInputs[i];
        printf("input tensor\n");
        printf("    pName %s\n", tensorMeta.pName);
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
        printf("    pName %s\n", tensorMeta.pName);
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

static void brief_rtv_info(const AX_NPU_SDK_EX_RTV_INFO_T *rtvInfo) {
    if (rtvInfo == nullptr) {
        return;
    }
    printf("RTV info:\n");
    for (AX_U32 i = 0; i < rtvInfo->nMetaSize; ++i) {
        auto &meta = rtvInfo->pMeta[i];
        printf("    RTV(type %d): {name:%s, index:%d}\n", (int)meta.eType, (const char*)meta.pName, meta.nIndex);
    }
}


static bool load_param(
    const std::string& json_path,
    std::vector<std::string>& model_fns,
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& data_fns
) {
    picojson::value v;
    auto err = picojson::parse(v, loadFile(json_path));
    if (!err.empty() || !v.is<picojson::object>()) {
        printf("invalid config json, error:%s\n", err.c_str());
        return false;
    }

    auto models = v.get("model");
    if (models.is<picojson::array>()) {
        int model_num = models.get<picojson::array>().end() - models.get<picojson::array>().begin();
        for (int i = 0; i < model_num; i++) {
            auto model_fn = models.get(i).get("neu");
            if(!model_fn.is<picojson::null>()) {
                model_fns.emplace_back(model_fn.get<std::string>());
            } else {
                printf("Model file is empty!\n");
                printf("Please refer to params.json for help.\n");
                return -1;
            }
            auto model_data_fns = models.get(i).get("data");
            if (model_data_fns.is<picojson::null>()) {
                printf("Input data is empty!\n");
                continue;
            }

            std::unordered_map<std::string, std::string> input_data;
            const picojson::value::object& obj = model_data_fns.get<picojson::object>();
            for (picojson::value::object::const_iterator j = obj.begin(); j!=obj.end(); ++j) {
                if (j->second.get<std::string>().empty()) {
                    printf("Input data is empty!\n");
                    continue;
                } else {
                    input_data[j->first] =  j->second.get<std::string>();
                }
            }
            data_fns[model_fn.get<std::string>()] = input_data;
        }
    }
    return true;
}

std::chrono::high_resolution_clock::time_point g_vnpu_start_point; //g_vnpu_start_point
std::atomic<uint64_t> g_vnpu_1_task_time{0}; // task run time in v_npu_1
std::atomic<uint64_t> g_vnpu_2_task_time{0}; // task run time in v_npu_2
std::atomic<int> g_vnpu_1_count{0};
std::atomic<int> g_vnpu_2_count{0};
typedef enum {
    VNPU_DISABLE = 0,
    VNPU_TYPE_1 = 1,
    VNPU_TYPE_2 = 2
} VNPU_TYPE;

struct UserData{
    bool finish_flag;
    std::condition_variable cv;
    std::mutex mtx;
    std::chrono::high_resolution_clock::time_point time_start;
    VNPU_TYPE vnpu_type;
    uint64_t task_time;
};

AX_VOID on_npu_task_finish (AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_TASK_ID_T taskId, AX_U32 error_code, AX_VOID *data) {
    auto ud = (UserData*) data;
    auto task_start = ud->time_start;
    auto task_end = std::chrono::system_clock::now();
    ud->task_time += std::chrono::duration_cast<std::chrono::microseconds>(task_end  - task_start).count();
    switch (ud->vnpu_type) {
        case VNPU_TYPE_1: {
            g_vnpu_1_count--;
            if (g_vnpu_1_count == 0) {
                g_vnpu_1_task_time += std::chrono::duration_cast<std::chrono::microseconds>(task_end - g_vnpu_start_point).count();
            }
            break;
        }
        case VNPU_TYPE_2: {
            g_vnpu_2_count--;
            if (g_vnpu_2_count == 0) {
                g_vnpu_2_task_time += std::chrono::duration_cast<std::chrono::microseconds>(task_end - g_vnpu_start_point).count();
            }
            break;
        }
        default:
            break;
    }
    std::unique_lock<std::mutex> grd((ud->mtx));
    (ud->finish_flag) = true;
    (ud->cv).notify_all();
}

int main(int argc, char* argv[]) {
    cmdline::parser args;
    args.add<std::string> ("mode", '\0',
        "NPU mode, disable for no virtual npu; 3_1 for AX_NPU_VIRTUAL_3_1; 2_2 for AX_NPU_VIRTUAL_2_2; 1_1 for AX_NPU_VIRTUAL_1_1",
        false, "disable");
    args.add<std::string>("neu", '\0', "dot-neu", false);
    args.add<std::string>(
        "data", 'd',
        "The format is var0:file0;var1:file1... to specify data files for input vars.\n\
\t'file*' would be directly loaded in binary format to tensor 'var*'\n\
\tIf there is only one input var, 'var0' can be omitted.",
        false
    );
    args.add<int>("repeat", '\0', "Repeat times for inference", false, 1);
    args.add<std::string>("params", '\0', "Param json for multi model and inputs", false);
    args.add<std::string>("bin-out-dir", '\0', "Dump output tensors in binary format", false);
    if (argc < 2) {
        printf("%s\n", args.usage().c_str());
        exit(0);
    }
    args.parse_check(argc, argv);

    // choose load param from json or from cmd
    std::string model_fn = args.get<std::string>("neu");
    std::string param_fn = args.get<std::string>("params");
    std::string data_fn = args.get<std::string>("data");
    std::vector<std::string> model_pathes;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data_pathes;
    int model_num = 1;
    if (!model_fn.empty()) {
        model_pathes = {model_fn};
    } else if (!param_fn.empty()){
        assert(load_param(param_fn, model_pathes, data_pathes));
        model_num = model_pathes.size();
    } else {
        printf("Model file is empty!\n");
        printf("Usage: run_neu --neu model.neu\n");
        return -1;
    }
    printf("Model num: %d\n", model_num);

    assert(AX_SYS_Init() == 0);
    AX_NPU_SDK_EX_ATTR_T npuMode;
    if (args.get<std::string>("mode") == "3_1") {
        printf("\nVirtual npu mode is 3_1\n\n");
        npuMode.eHardMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_3_1;
    } else if (args.get<std::string>("mode") == "2_2") {
        printf("\nVirtual npu mode is 2_2\n\n");
        npuMode.eHardMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_2_2;
    } else if (args.get<std::string>("mode") == "1_1") {
        printf("\nVirtual npu mode is 1_1\n\n");
        npuMode.eHardMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_1_1;
    } else {
        printf("\nVirtual npu disable!\n\n");
        npuMode.eHardMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_DISABLE;
    }
    assert(AX_NPU_SDK_EX_Init_with_attr(&npuMode) == 0);

    // Load model and data cyclically
    AX_NPU_SDK_EX_HANDLE_T* npuHandles = new AX_NPU_SDK_EX_HANDLE_T[model_num];
    const AX_NPU_SDK_EX_IO_INFO_T** ioInfos = new  const AX_NPU_SDK_EX_IO_INFO_T*[model_num];
    AX_NPU_SDK_EX_IO_T* ios = new  AX_NPU_SDK_EX_IO_T[model_num];
    AX_NPU_SDK_EX_RESOURCE_T* resources = new AX_NPU_SDK_EX_RESOURCE_T[model_num];
    UserData ud[model_num] = {0};

    for(int j = 0; j < model_num; j++) {
        model_fn = model_pathes[j];
        auto modelData = loadFile(model_fn);
        assert(!modelData.empty());
        AX_NPU_SDK_EX_HANDLE_T npuHandle;
        int ret = AX_NPU_SDK_EX_Create_handle(&npuHandle, modelData.data(), modelData.size());
        if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
            printf("AX_NPU_SDK_EX_Create_handle failed, error code %d\n", ret);
            for (int i = 0; i < j; i++) {
                AX_NPU_SDK_EX_IO_T io = ios[i];
                for (AX_U32 i = 0; i < io.nInputSize; ++i) {
                    assert(0 == AX_NPU_SDK_EX_Free_buffer(&io.pInputs[i]));
                }
                delete[] io.pInputs;
                for (AX_U32 i = 0; i < io.nOutputSize; ++i) {
                    assert(0 == AX_NPU_SDK_EX_Free_buffer(&io.pOutputs[i]));
                }
                delete[] io.pOutputs;
                AX_NPU_SDK_EX_Destroy_handle(npuHandles[i]);
            }
            delete[] ios;
            delete[] ioInfos;
            delete[] npuHandles;
            delete[] resources;
            AX_NPU_SDK_EX_Deinit();
            return -1;
        }
        npuHandles[j] = npuHandle;

        auto ioInfo = AX_NPU_SDK_EX_Get_io_info(npuHandle);
        assert(ioInfo != nullptr);
        printf("\nioInfo of model: %d\n\n", j);
        brief_npu_handle(ioInfo);
        assert(ioInfo->nInputSize > 0);
        if (param_fn.empty()) {
            data_pathes[model_fn] = parse_input_mappings(
                data_fn,
                std::string((char *)ioInfo->pInputs[0].pName));
        }
        ioInfos[j] = ioInfo;

        auto rtv_info = AX_NPU_SDK_EX_Get_RTV_info(npuHandle);
        if (rtv_info != nullptr) {
            brief_rtv_info(rtv_info);
        }

        AX_NPU_SDK_EX_IO_T io;
        memset(&io, 0, sizeof(io));
        io.nInputSize = ioInfo->nInputSize;
        io.pInputs = new AX_NPU_SDK_EX_BUF_T[ioInfo->nInputSize];
        auto input_mappings = data_pathes[model_fn];
        for (uint32_t i = 0; i < ioInfo->nInputSize; ++i) {
            assert(0 == AX_NPU_SDK_EX_Alloc_buffer(&ioInfo->pInputs[i], &io.pInputs[i], AX_NPU_ABST_DEFAULT));
            std::string name((char *)ioInfo->pInputs[i].pName);
            if (input_mappings.count(name)) {
                auto fn = input_mappings[name];
                printf("init '%s' with '%s'\n", name.c_str(), fn.c_str());
                auto data = loadFile(fn);
                assert(data.size() == io.pInputs[i].nSize);
                memcpy(io.pInputs[i].pVirAddr, data.data(), data.size());
            } else {
                printf("init '%s' with zero data\n", name.c_str());
                memset(io.pInputs[i].pVirAddr, 0, io.pInputs[i].nSize);
            }
        }
        io.nOutputSize = ioInfo->nOutputSize;
        io.pOutputs = new AX_NPU_SDK_EX_BUF_T[ioInfo->nOutputSize];
        for (uint32_t i = 0; i < ioInfo->nOutputSize; ++i) {
            assert(0 == AX_NPU_SDK_EX_Alloc_buffer(&ioInfo->pOutputs[i], &io.pOutputs[i], AX_NPU_ABST_DEFAULT));
        }
        ios[j] = io;

        // vnpu type count
        AX_NPU_SDK_EX_MODEL_TYPE_T pModelType;
        assert (AX_NPU_DEV_STATUS_SUCCESS == AX_NPU_SDK_EX_Get_Model_type(npuHandle, &pModelType));
        if (pModelType == AX_NPU_SDK_EX_MODEL_TYPE_T::AX_NPU_MODEL_TYPE_3_1_1
            || pModelType == AX_NPU_SDK_EX_MODEL_TYPE_T::AX_NPU_MODEL_TYPE_2_2_1
            || pModelType == AX_NPU_SDK_EX_MODEL_TYPE_T::AX_NPU_MODEL_TYPE_1_1_1
            ) {
            g_vnpu_1_count++;
            ud[j].vnpu_type = VNPU_TYPE_1;
        } else if (pModelType == AX_NPU_SDK_EX_MODEL_TYPE_T::AX_NPU_MODEL_TYPE_3_1_2
            || pModelType == AX_NPU_SDK_EX_MODEL_TYPE_T::AX_NPU_MODEL_TYPE_2_2_2
            || pModelType == AX_NPU_SDK_EX_MODEL_TYPE_T::AX_NPU_MODEL_TYPE_1_1_2
            ) {
            g_vnpu_2_count++;
            ud[j].vnpu_type = VNPU_TYPE_2;
        } else if (pModelType == AX_NPU_SDK_EX_MODEL_TYPE_T::AX_NPU_MODEL_TYPE_DEFUALT) {
            ud[j].vnpu_type = VNPU_DISABLE;
        } else {
            printf("Model type is %d\n", pModelType);
            assert(0);
        }

        // Init resource
        AX_NPU_SDK_EX_RESOURCE_T resource;
        ud[j].finish_flag = false;
        resource.tIo = io;
        resource.pUserData = (void*) (ud + j);
        resource.fnFinishFunc = on_npu_task_finish;
        resources[j] = resource;
    }

    // run npu_handle
    AX_NPU_SDK_EX_TASK_ID_T task_id[model_num];
    const int test_count = args.get<int>("repeat");
    auto task_start= std::chrono::system_clock::now();
    int npu_1_count = g_vnpu_1_count;
    int npu_2_count = g_vnpu_2_count;
    AX_NPU_SDK_EX_WORK_CYCLE_T work_cycle_sum;
    memset(&work_cycle_sum, 0x0, sizeof(work_cycle_sum));

    for (int j = 0; j < test_count; ++j) {
        g_vnpu_start_point = std::chrono::system_clock::now();
        for (int i = 0; i < model_num; i++) {
            ud[i].finish_flag = false;
            task_id[i] = i;
            ud[i].time_start = std::chrono::system_clock::now();
            assert(AX_NPU_TASK_STATUS_SUCCESS == AX_NPU_SDK_EX_Run_task_async(npuHandles[i], resources+i, task_id+i));
        }
        for (int i = 0; i < model_num; i++) {
            std::unique_lock<std::mutex> grd(ud[i].mtx);
            ud[i].cv.wait(grd, [&]()->bool { return ud[i].finish_flag; });
        }
        g_vnpu_1_count = npu_1_count;
        g_vnpu_2_count = npu_2_count;
        AX_NPU_SDK_EX_WORK_CYCLE_T work_cycle;
        memset(&work_cycle, 0x0, sizeof(work_cycle));
        assert(AX_NPU_DEV_STATUS_SUCCESS == AX_NPU_SDK_EX_Get_Work_Cycle(&work_cycle));

        work_cycle_sum.total_cycle += work_cycle.total_cycle;
        work_cycle_sum.valid_eu_num = work_cycle.valid_eu_num;
        for (uint32_t i = 0; i < work_cycle.valid_eu_num; i++)
            work_cycle_sum.eu_cycle[i] += work_cycle.eu_cycle[i];

    }
    auto time_end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - task_start) / test_count;

    for (int i = 0; i < model_num; i++) {
        printf("Model %d task took %llu us (%d rounds for average)\n\n", i, ud[i].task_time / test_count, test_count);
    }

    printf("Run model took %llu cycle, (%d rounds for average)\n", work_cycle_sum.total_cycle / test_count, test_count);
    for (uint32_t i = 0; i < work_cycle_sum.valid_eu_num; i++) {
        printf("\teu %d took %d cycle, (%d rounds for average)\n", i, work_cycle_sum.eu_cycle[i] / test_count, test_count);
    }
    printf("\n");


    if (g_vnpu_1_count || g_vnpu_2_count) {
        printf("vnpu_1 took %lld us (%d rounds for average)\n\n", g_vnpu_1_task_time / test_count, test_count);
        printf("vnpu_2 took %lld us (%d rounds for average)\n\n", g_vnpu_2_task_time / test_count, test_count);
    }
    printf("Run tasks took %lld us (%d rounds for average)\n\n", duration.count(), test_count);

    auto out_dir = args.get<std::string>("bin-out-dir");
    if (!out_dir.empty()) {
        for (int j = 0; j < model_num; j++) {
            AX_NPU_SDK_EX_IO_T io = ios[j];
            const AX_NPU_SDK_EX_IO_INFO_T* ioInfo = ioInfos[j];
            struct stat info;
            if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
                fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
                return 1;
            }
            model_fn = model_pathes[j];
            for (AX_U32 i = 0; i < io.nOutputSize; ++i) {
                // load model_name
                auto name_start = model_fn.find_last_of("/");
                auto name_end = model_fn.find_last_of(".");
                if (name_start == std::string::npos)
                    name_start = 0;
                else
                    name_start++;
                std::string fn = out_dir + "/" + model_fn.substr(name_start, name_end - name_start) + "_" + (char *)ioInfo->pOutputs[i].pName + ".bin";
                FILE* fout = fopen(fn.c_str(), "wb");
                if (!fout) {
                    fprintf(stderr, "error openning '%s'\n", fn.c_str());
                    return 1;
                }
                fwrite(io.pOutputs[i].pVirAddr, 1, io.pOutputs[i].nSize, fout);
                fclose(fout);

                fn = out_dir + "/" +  model_fn.substr(name_start, name_end - name_start) + "_" + (char *)ioInfo->pOutputs[i].pName + "_inner.bin";
                fout = fopen(fn.c_str(), "wb");
                if (!fout) {
                    fprintf(stderr, "error openning '%s'\n", fn.c_str());
                    return 1;
                }
                fwrite(io.pOutputs[i].pInnerVirAddr, 1, io.pOutputs[i].nInnerSize, fout);
                fclose(fout);
            }
        }
    }

    for (int i = 0; i < model_num; i++) {
        AX_NPU_SDK_EX_IO_T io = ios[i];
        for (AX_U32 i = 0; i < io.nInputSize; ++i) {
            assert(0 == AX_NPU_SDK_EX_Free_buffer(&io.pInputs[i]));
        }
        delete[] io.pInputs;
        for (AX_U32 i = 0; i < io.nOutputSize; ++i) {
            assert(0 == AX_NPU_SDK_EX_Free_buffer(&io.pOutputs[i]));
        }
        delete[] io.pOutputs;
        AX_NPU_SDK_EX_Destroy_handle(npuHandles[i]);
    }

    delete[] ios;
    delete[] ioInfos;
    delete[] npuHandles;
    delete[] resources;

    printf("Deinit NPU Driver +++\n");
    AX_NPU_SDK_EX_Deinit();
    printf("NPU Driver Exited ---\n");

    return 0;
}
