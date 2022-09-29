#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_set>
#include <vector>

#include "ax_interpreter_external_api.h"
#include "ax_sys_api.h"
#include "cmdline.h"
#include "joint.h"
#include "joint_adv.h"

static const AX_S32 SYS_MEM_ALIGN = 128;

static void report_memory_usage(const std::string& msg) {
  static bool enable_inspect_mem =
      (std::getenv("AX_JOINT_MEMORY_DETAIL") != nullptr &&
       strcmp(std::getenv("AX_JOINT_MEMORY_DETAIL"), "ON") == 0);
  if (enable_inspect_mem) {
    if (!msg.empty()) printf("%s\n", msg.c_str());
    raise(SIGUSR2);  // print the memory usage
    printf("\n");
  } else {
    static int print_cnt = 0;
    if (print_cnt == 0) {
      printf("Not set environment variable to report memory usage!\n\n");
      print_cnt++;
    }
  }
}

bool read_file(const char* fn, std::vector<char>& data) {
  FILE* fp = fopen(fn, "r");
  if (fp != nullptr) {
    fseek(fp, 0L, SEEK_END);
    auto len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data.clear();
    size_t read_size = 0;
    if (len > 0) {
      data.resize(len);
      read_size = fread(data.data(), 1, len, fp);
    }
    fclose(fp);
    return read_size == (size_t)len;
  }
  return false;
}

bool save_file(const char* fn, const void* data, size_t size) {
  FILE* fp = fopen(fn, "wb+");
  if (fp != nullptr) {
    auto save_size = fwrite(data, size, 1, fp);
    fclose(fp);
    return save_size == 1;
  }
  return false;
}

std::vector<std::string> split_string(const std::string& content,
                                      const std::string delimitter) {
  if (delimitter.size() == 0) return {content};
  if (content.size() == 0) return {};
  std::vector<std::string> ret;
  int i = 0;
  while (i < (int)content.size()) {
    int j = i;
    while (j < (int)content.size() &&
           content.substr(j, delimitter.size()) != delimitter)
      ++j;
    ret.emplace_back(content.substr(i, j - i));
    i = j + delimitter.size();
  }
  if (ret.empty()) ret.emplace_back("");
  return ret;
}

void brief_io_info(const AX_JOINT_IO_INFO_T* io_info) {
  auto describe_shape_type = [](AX_JOINT_TENSOR_LAYOUT_T type) -> const char* {
    switch (type) {
      case AX_JOINT_TENSOR_LAYOUT_T::JOINT_TENSOR_LAYOUT_NHWC:
        return "NHWC";
      case AX_JOINT_TENSOR_LAYOUT_T::JOINT_TENSOR_LAYOUT_NCHW:
        return "NCHW";
      default:
        return "unknown";
    }
  };
  auto describe_data_type = [](AX_JOINT_DATA_TYPE_T type) -> const char* {
    switch (type) {
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_UINT8:
        return "uint8";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_UINT16:
        return "uint16";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_FLOAT32:
        return "float32";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_SINT16:
        return "sint16";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_SINT8:
        return "sint8";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_SINT32:
        return "sint32";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_UINT32:
        return "uint32";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_FLOAT64:
        return "float64";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_UINT10_PACKED:
        return "uint10_packed";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_UINT12_PACKED:
        return "uint12_packed";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_UINT14_PACKED:
        return "uint14_packed";
      case AX_JOINT_DATA_TYPE_T::AX_JOINT_DT_UINT16_PACKED:
        return "uint16_packed";
      default:
        return "unknown";
    }
  };
  auto describe_memory_type = [](AX_JOINT_MEMORY_TYPE_T type) -> const char* {
    switch (type) {
      case AX_JOINT_MEMORY_TYPE_T::AX_JOINT_MT_PHYSICAL:
        return "Physical";
      case AX_JOINT_MEMORY_TYPE_T::AX_JOINT_MT_VIRTUAL:
        return "Virtual";
      default:
        return "unknown";
    }
  };
  auto describe_color_space = [](AX_JOINT_COLOR_SPACE_T cs) -> const char* {
    switch (cs) {
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_FEATUREMAP:
        return "FeatureMap";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_BGR:
        return "BGR";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_RGB:
        return "RGB";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_GRAY:
        return "GRAY";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_NV12:
        return "NV12";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_NV21:
        return "NV21";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_YUV444:
        return "YUV444";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_RAW8:
        return "RAW8";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_RAW10:
        return "RAW10";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_RAW12:
        return "RAW12";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_RAW14:
        return "RAW14";
      case AX_JOINT_COLOR_SPACE_T::AX_JOINT_CS_RAW16:
        return "RAW16";
      default:
        return "unknown";
    }
  };
  printf("Max Batch Size %d\n", io_info->nMaxBatchSize);
  printf("Support Dynamic Batch? %s\n",
         io_info->bDynamicBatchSize == AX_TRUE ? "Yes" : "No");
  printf("Is FilterMode? %s\n\n",
         io_info->bFilterMode == AX_TRUE ? "Yes" : "No");
  if (io_info->eQuantizationType ==
      AX_JOINT_QUANTIZATION_TYPE_T::AX_JOINT_QT_BIT_MIX) {
    printf("Quantization Type is mix\n\n");
  } else if (io_info->eQuantizationType ==
             AX_JOINT_QUANTIZATION_TYPE_T::AX_JOINT_QT_BIT_8) {
    printf("Quantization Type is 8 bit\n\n");
  } else if (io_info->eQuantizationType ==
             AX_JOINT_QUANTIZATION_TYPE_T::AX_JOINT_QT_BIT_16) {
    printf("Quantization Type is 16 bit\n\n");
  } else if (io_info->eQuantizationType ==
             AX_JOINT_QUANTIZATION_TYPE_T::AX_JOINT_QT_BIT_32) {
    printf("Quantization Type is 32 bit\n\n");
  } else {
    printf("Quantization Type is unknown\n\n");
  }

  for (uint32_t i = 0; i < io_info->nInputSize; ++i) {
    auto& input = io_info->pInputs[i];
    printf("Input[%d]: %s\n", i, input.pName);
    printf("    Shape [");
    for (uint32_t j = 0; j < input.nShapeSize; ++j) {
      printf("%d", (int)input.pShape[j]);
      if (j + 1 < input.nShapeSize) printf(", ");
    }
    printf("] %s %s %s %s\n", describe_shape_type(input.eLayout),
           describe_data_type(input.eDataType),
           input.pExtraMeta
               ? describe_color_space(input.pExtraMeta->eColorSpace)
               : "",
           input.nQuantizationValue > 0
               ? ("Q=" + std::to_string(input.nQuantizationValue)).c_str()
               : "");
    printf("    Memory %s\n", describe_memory_type(input.eMemoryType));
    printf("    Size %u\n", input.nSize);
  }
  for (uint32_t i = 0; i < io_info->nOutputSize; ++i) {
    auto& output = io_info->pOutputs[i];
    printf("Output[%d]: %s\n", i, output.pName);
    printf("    Shape [");
    for (uint32_t j = 0; j < output.nShapeSize; ++j) {
      printf("%d", (int)output.pShape[j]);
      if (j + 1 < output.nShapeSize) printf(", ");
    }
    printf("] %s %s %s\n", describe_shape_type(output.eLayout),
           describe_data_type(output.eDataType),
           output.nQuantizationValue > 0
               ? ("Q=" + std::to_string(output.nQuantizationValue)).c_str()
               : "");
    printf("    Memory %s\n", describe_memory_type(output.eMemoryType));
    printf("    Size %u\n", output.nSize);
  }
}

AX_S32 parse_npu_mode_from_joint(const AX_CHAR* data, AX_U32 data_size,
                                 AX_NPU_SDK_EX_HARD_MODE_T* pMode) {
  AX_NPU_SDK_EX_MODEL_TYPE_T npu_type;
  auto ret = AX_JOINT_GetJointModelType(data, data_size, &npu_type);
  if (ret != AX_ERR_NPU_JOINT_SUCCESS) {
    printf("\n[WARN]: get joint model type failed, disable virtual npu!\n");
    assert(0);
  }
  if (npu_type == AX_NPU_MODEL_TYPE_DEFUALT) {
    printf("\nvirtual npu disable!\n\n");
    *pMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_DISABLE;
  }
#if defined(CHIP_AX630A)
  else if (npu_type == AX_NPU_MODEL_TYPE_3_1_1 ||
           npu_type == AX_NPU_MODEL_TYPE_3_1_2) {
    printf("\nvirtual npu mode is 3_1\n\n");
    *pMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_3_1;
  } else if (npu_type == AX_NPU_MODEL_TYPE_2_2_1 ||
             npu_type == AX_NPU_MODEL_TYPE_2_2_2) {
    printf("\nvirtual npu mode is 2_2\n\n");
    *pMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_2_2;
  }
#elif defined(CHIP_AX620)
  else if (npu_type == AX_NPU_MODEL_TYPE_1_1_1 ||
           npu_type == AX_NPU_MODEL_TYPE_1_1_2) {
    printf("\nvirtual npu mode is 1_1\n\n");
    *pMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_1_1;
  }
#endif
  else {
    printf("unknown npu mode %d\n", (int)npu_type);
    assert(0);
  }
  return ret;
}

void alloc_test_buffer_manually(const AX_JOINT_IOMETA_T* pMeta,
                                AX_JOINT_IO_BUFFER_T* pBuf,
                                uint32_t batch_size) {
  memset(pBuf, 0, sizeof(AX_JOINT_IO_BUFFER_T));
  pBuf->nSize = pMeta->nSize / pMeta->pShape[0] * batch_size;
  if (pMeta->eMemoryType == AX_JOINT_MEMORY_TYPE_T::AX_JOINT_MT_VIRTUAL) {
    pBuf->phyAddr = 0;
    pBuf->pVirAddr = new uint8_t[pBuf->nSize];
  } else {
    assert(AX_SYS_MemAlloc((AX_U64*)&pBuf->phyAddr, &pBuf->pVirAddr,
                           pBuf->nSize, SYS_MEM_ALIGN, NULL) == 0);
  }
}

void alloc_test_buffer_with_stride(const AX_JOINT_IOMETA_T* pMeta,
                                   AX_JOINT_IO_BUFFER_T* pBuf,
                                   uint32_t stride_w, uint32_t batch_size) {
  memset(pBuf, 0, sizeof(AX_JOINT_IO_BUFFER_T));
  pBuf->nStrideSize = pMeta->nShapeSize;
  pBuf->pStride = new AX_S32[pBuf->nStrideSize];
  pBuf->pStride[0] =
      pMeta->pShape[1] * (pMeta->pShape[2] + stride_w) * pMeta->pShape[3];
  pBuf->pStride[1] = (pMeta->pShape[2] + stride_w) * pMeta->pShape[3];
  pBuf->pStride[2] = pMeta->pShape[3];
  pBuf->pStride[3] = 1;
  pBuf->nSize = pBuf->pStride[0] * batch_size;
  if (AX_SYS_MemAllocCached((AX_U64*)&pBuf->phyAddr, &pBuf->pVirAddr,
                            pBuf->nSize, SYS_MEM_ALIGN, NULL)) {
    fprintf(stderr, "AX_SYS_MemAllocCached failed\n");
  }
  memset(pBuf->pVirAddr, 0, pBuf->nSize);
}

void alloc_test_buffer(const AX_JOINT_IOMETA_T* pMeta,
                       AX_JOINT_IO_BUFFER_T* pBuf) {
  AX_JOINT_IOMETA_T meta = *pMeta;
  assert(AX_ERR_NPU_JOINT_SUCCESS ==
         AX_JOINT_AllocBuffer(&meta, pBuf, AX_JOINT_ABST_DEFAULT));
}

void free_test_buffer_manually(AX_JOINT_IO_BUFFER_T* pBuf) {
  if (pBuf->phyAddr == 0) {
    delete[] reinterpret_cast<uint8_t*>(pBuf->pVirAddr);
  } else {
    assert(AX_SYS_MemFree(pBuf->phyAddr, pBuf->pVirAddr) == 0);
  }
  pBuf->phyAddr = 0;
  pBuf->pVirAddr = nullptr;
  if (pBuf->pStride) {
    delete[] pBuf->pStride;
    pBuf->pStride = nullptr;
  }
}

void free_test_buffer(AX_JOINT_IO_BUFFER_T* pBuf) {
  assert(AX_ERR_NPU_JOINT_SUCCESS == AX_JOINT_FreeBuffer(pBuf));
}

void set_rtv_tensor_to_use_default_value(const AX_JOINT_IOMETA_T* pMeta,
                                         AX_JOINT_IO_BUFFER_T* pBuf) {
  assert(pMeta != nullptr);
  assert(pBuf != nullptr);
  assert(pMeta->nShapeSize > 0);
  assert(pMeta->pShape[0] == 1);
  memset(pBuf, 0, sizeof(AX_JOINT_IO_BUFFER_T));
  pBuf->pVirAddr = nullptr;
  pBuf->phyAddr = 0;
  pBuf->nSize = pMeta->nSize;
};

void load_input_data(std::vector<char> buf, AX_JOINT_IO_BUFFER_T* pBuf) {
  assert(buf.size() <= pBuf->nSize);
  memcpy(pBuf->pVirAddr, buf.data(), buf.size());
}

void load_input_data_with_stride(std::vector<char> buf,
                                 const AX_JOINT_IOMETA_T* pMeta,
                                 AX_JOINT_IO_BUFFER_T* pBuf) {
  assert(buf.size() == pMeta->nSize);
  uint32_t write_len_wc = pMeta->pShape[3] * pMeta->pShape[2];
  uint32_t stride_len_wc = pBuf->pStride[1];
  for (int j = 0; j < pMeta->pShape[0]; j++) {
    uint32_t dst_addr_offset = j * pBuf->pStride[0];
    uint32_t src_addr_offset = pMeta->nSize * j / pMeta->pShape[0];
    for (int i = 0; i < pMeta->pShape[1]; i++) {
      memcpy((uint8_t*)pBuf->pVirAddr + dst_addr_offset + i * stride_len_wc,
             buf.data() + src_addr_offset + i * write_len_wc, write_len_wc);
    }
  }
}

void prepare_test_buffer_with_stride(const AX_JOINT_IO_INFO_T* io_info,
                                     AX_JOINT_IO_T& io,
                                     std::vector<std::string> input_fns,
                                     int stride_w) {
  io.nInputSize = io_info->nInputSize;
  io.pInputs = new AX_JOINT_IO_BUFFER_T[io.nInputSize];
  for (size_t i = 0; i < io.nInputSize; ++i) {
    if (stride_w != 0) {
      printf("mock input data with extra w stride %d\n", stride_w);
    }
    AX_JOINT_IO_BUFFER_T* pBuf = io.pInputs + i;
    AX_JOINT_IOMETA_T* pMeta = io_info->pInputs + i;
    memset(pBuf, 0, sizeof(AX_JOINT_IO_BUFFER_T));
    pBuf->nStrideSize = pMeta->nShapeSize;
    pBuf->pStride = new AX_S32[pBuf->nStrideSize];

    pBuf->pStride[0] =
        pMeta->pShape[1] * (pMeta->pShape[2] + stride_w) * pMeta->pShape[3];
    pBuf->pStride[1] = (pMeta->pShape[2] + stride_w) * pMeta->pShape[3];
    pBuf->pStride[2] = pMeta->pShape[3];
    pBuf->pStride[3] = 1;

    pBuf->nSize = pBuf->pStride[0] * pMeta->pShape[0];
    if (AX_SYS_MemAllocCached((AX_U64*)&pBuf->phyAddr, &pBuf->pVirAddr,
                              pBuf->nSize, SYS_MEM_ALIGN, NULL)) {
      fprintf(stderr, "AX_SYS_MemAllocCached failed\n");
    }
    memset(pBuf->pVirAddr, 0, pBuf->nSize);

    if (i < input_fns.size()) {  // user define input
      auto& fn = input_fns[i];
      std::vector<char> buf;
      if (read_file(fn.c_str(), buf)) {
        printf("init input[%zd] %s with file %s\n", i,
               io_info->pInputs[i].pName, fn.c_str());
        if (stride_w == 0) {
          memcpy(io.pInputs[i].pVirAddr, buf.data(), buf.size());
        } else {
          AX_JOINT_IO_BUFFER_T* pBuf = io.pInputs + i;
          AX_JOINT_IOMETA_T* pMeta = io_info->pInputs + i;
          uint32_t write_len_wc = pMeta->pShape[3] * pMeta->pShape[2];
          uint32_t stride_len_wc = pBuf->pStride[1];
          for (int j = 0; j < pMeta->pShape[0]; j++) {
            uint32_t dst_addr_offset = j * pBuf->pStride[0];
            uint32_t src_addr_offset = pMeta->nSize * j / pMeta->pShape[0];
            for (int i = 0; i < pMeta->pShape[1]; i++) {
              memcpy((uint8_t*)pBuf->pVirAddr + dst_addr_offset +
                         i * stride_len_wc,
                     buf.data() + src_addr_offset + i * write_len_wc,
                     write_len_wc);
            }
          }
        }
      } else {
        printf(
            "read file %s for input[%zd] %s data failed, using random data\n",
            fn.c_str(), i, io_info->pInputs[i].pName);
      }
    } else {
      printf("input[%zd] %s data not provided, using random data\n", i,
             io_info->pInputs[i].pName);
    }
  }
  io.nOutputSize = io_info->nOutputSize;
  io.pOutputs = new AX_JOINT_IO_BUFFER_T[io.nOutputSize];
  for (size_t i = 0; i < io.nOutputSize; ++i) {
    assert(AX_ERR_NPU_JOINT_SUCCESS ==
           AX_JOINT_AllocBuffer(io_info->pOutputs + i, io.pOutputs + i,
                                AX_JOINT_ABST_DEFAULT));
  }
}

void prepare_io(AX_JOINT_IO_T& io, const AX_JOINT_IO_INFO_T* io_info,
                const std::vector<std::string>& input_fns, int stride_w = 0,
                bool manual_alloc = false, bool enable_trunc = false,
                uint32_t override_batch_size = 0) {
  memset(&io, 0, sizeof(io));
  int batch_size = io_info->nMaxBatchSize == 0 ? 1 : io_info->nMaxBatchSize;
  io.nBatchSize = batch_size;
  if (override_batch_size > 0 && io_info->bDynamicBatchSize) {
    batch_size = override_batch_size;
    io.nBatchSize = override_batch_size;
  }
  printf("Using batch size %d\n", batch_size);
  io.nInputSize = io_info->nInputSize;
  io.pInputs = new AX_JOINT_IO_BUFFER_T[io.nInputSize];
  for (size_t i = 0; i < io.nInputSize; ++i) {
    const AX_JOINT_IOMETA_T* pMeta = io_info->pInputs + i;
    AX_JOINT_IO_BUFFER_T* pBuf = io.pInputs + i;
    assert(pMeta->nShapeSize > 0);
    int actual_data_size = pMeta->nSize / pMeta->pShape[0] * batch_size;
    if (pMeta->pExtraMeta != nullptr &&
        pMeta->pExtraMeta->eRtvType != AX_JOINT_NOT_RTV) {
      if (i < input_fns.size()) {  // user define input
        auto& fn = input_fns[i];
        std::vector<char> buf;
        if (read_file(fn.c_str(), buf)) {
          if (enable_trunc) {
            buf.resize(actual_data_size);
          }
          if ((int)buf.size() != actual_data_size) {
            set_rtv_tensor_to_use_default_value(pMeta, pBuf);
            printf(
                "read file %s for input[%zd] %s data size wrong, using default "
                "value\n",
                fn.c_str(), i, io_info->pInputs[i].pName);
          } else {
            if (manual_alloc) {
              alloc_test_buffer_manually(pMeta, pBuf, batch_size);
            } else {
              alloc_test_buffer(pMeta, pBuf);
            }
            load_input_data(buf, pBuf);
          }
        } else {
          set_rtv_tensor_to_use_default_value(pMeta, pBuf);
          printf(
              "read file %s for input[%zd] %s data failed, using default "
              "value\n",
              fn.c_str(), i, io_info->pInputs[i].pName);
        }
      } else {
        set_rtv_tensor_to_use_default_value(pMeta, pBuf);
        printf("input[%zd] %s data not provided, using default value\n", i,
               io_info->pInputs[i].pName);
      }
    } else {
      if (stride_w) {
        alloc_test_buffer_with_stride(pMeta, pBuf, stride_w, batch_size);
      } else if (manual_alloc) {
        alloc_test_buffer_manually(pMeta, pBuf, batch_size);
      } else {
        alloc_test_buffer(pMeta, pBuf);
      }
      if (i < input_fns.size()) {  // user define input
        auto& fn = input_fns[i];
        std::vector<char> buf;
        if (read_file(fn.c_str(), buf)) {
          if (enable_trunc) {
            buf.resize(actual_data_size);
          }
          if ((int)buf.size() != actual_data_size) {
            printf(
                "read file %s for input[%zd] %s data size wrong, using random "
                "data\n",
                fn.c_str(), i, io_info->pInputs[i].pName);
          } else if (stride_w) {
            load_input_data_with_stride(buf, pMeta, pBuf);
          } else {
            load_input_data(buf, pBuf);
          }
        } else {
          printf(
              "read file %s for input[%zd] %s data failed, using random data\n",
              fn.c_str(), i, io_info->pInputs[i].pName);
        }
      } else {
        printf("input[%zd] %s data not provided, using random data\n", i,
               io_info->pInputs[i].pName);
      }
    }
  }
  io.nOutputSize = io_info->nOutputSize;
  io.pOutputs = new AX_JOINT_IO_BUFFER_T[io.nOutputSize];
  for (size_t i = 0; i < io.nOutputSize; ++i) {
    const AX_JOINT_IOMETA_T* pMeta = io_info->pOutputs + i;
    AX_JOINT_IO_BUFFER_T* pBuf = io.pOutputs + i;
    if (manual_alloc) {
      alloc_test_buffer_manually(pMeta, pBuf, batch_size);
    } else {
      alloc_test_buffer(pMeta, pBuf);
    }
  }
}

int main(int argc, char* argv[]) {
  assert(AX_SYS_Init() == 0);

  cmdline::parser args;
  args.footer("joint-file");
#if defined(CHIP_AX630A)
  args.add<std::string>("mode", '\0',
                        "NPU mode, disable for no virtual npu; 3_1 for "
                        "AX_NPU_VIRTUAL_3_1; 2_2 for AX_NPU_VIRTUAL_2_2",
                        false, "");
#elif defined(CHIP_AX620)
  args.add<std::string>(
      "mode", '\0',
      "NPU mode, disable for no virtual npu; 1_1 for AX_NPU_VIRTUAL_1_1", false,
      "");
#else
#error "please select the chip type"
#endif
  args.add<std::string>(
      "data", 'd',
      "The format is file0;file1... to specify data files for input vars.\n\
\t'file*' would be directly loaded in binary format to tensor in order",
      false);
  args.add<std::string>("bin-out-dir", '\0',
                        "Dump output tensors in binary format", false);
  args.add<int>("repeat", '\0', "Repeat times for inference", false, 1);
  args.add<int>("warmup", '\0', "Repeat times for warmup", false, 0);
  args.add<int>("stride_w", '\0', "mock input data with extra width stride",
                false, 0);
  args.add<int>("override_batch_size", '\0',
                "override batch size for dynamic batch model", false, 0);
  args.add<int>("wbt_index", '\0', "select WBT for inference", false, 0);
  args.add("manual_alloc", 'p',
           "manually alloc buffer with ax sys api instead of joint api");
  args.add(
      "enable_trunc", 't',
      "truncate input data size to model required size when using a larger "
      "input data, experimental function, will be removed in future release");
  args.parse_check(argc, argv);

  printf("run joint version: %s\n", AX_JOINT_GetVersion());

  std::string arg_data = args.get<std::string>("data");
  std::vector<std::string> input_fns = split_string(arg_data, ";");

  AX_JOINT_HANDLE joint_handle;
  if (args.rest().empty()) {
    printf("must specify joint file\n");
    return -1;
  }
  std::vector<char> joint;
  auto model_fn = args.rest()[0];
  if (!read_file(model_fn.c_str(), joint)) {
    printf("read file %s failed\n", model_fn.c_str());
    return -1;
  }

  AX_JOINT_SDK_ATTR_T attr;
  memset(&attr, 0, sizeof(AX_JOINT_SDK_ATTR_T));
  {
    auto mode = args.get<std::string>("mode");
    if (mode == "") {
      parse_npu_mode_from_joint(joint.data(), joint.size(), &attr.eNpuMode);
    } else if (mode == "disable") {
      attr.eNpuMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_DISABLE;
      printf("\nvirtual npu disable\n\n");
#if defined(CHIP_AX630A)
    } else if (mode == "3_1") {
      attr.eNpuMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_3_1;
      printf("\nvirtual npu mode is 3_1\n\n");
    } else if (mode == "2_2") {
      attr.eNpuMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_2_2;
      printf("\nvirtual npu mode is 2_2\n\n");
#elif defined(CHIP_AX620)
    } else if (mode == "1_1") {
      attr.eNpuMode = AX_NPU_SDK_EX_HARD_MODE_T::AX_NPU_VIRTUAL_1_1;
      printf("\nvirtual npu mode is 1_1\n\n");
#endif
    } else {
      printf("unknown npu mode %s\n", mode.c_str());
      assert(0);
    }
  }
  assert(AX_JOINT_Adv_Init(&attr) == AX_ERR_NPU_JOINT_SUCCESS);

  auto init_time_start = std::chrono::system_clock::now();
  assert(AX_ERR_NPU_JOINT_SUCCESS ==
         AX_JOINT_CreateHandle(&joint_handle, joint.data(), joint.size()));
  uint32_t init_time_us =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now() - init_time_start)
          .count();

  const AX_CHAR* version = AX_JOINT_GetModelToolsVersion(joint_handle);
  printf("tools version: %s\n", version);

  std::vector<char>().swap(joint);  // release joint memory

  int wbt_index = args.get<int>("wbt_index");
  if (wbt_index < 0) {
    printf("invalid WBT index %d\n", wbt_index);
    assert(0);
  } else {
    printf("Using wbt %d\n", wbt_index);
  }

  auto io_info = AX_JOINT_GetIOInfo(joint_handle);
  brief_io_info(io_info);

  AX_JOINT_EXECUTION_CONTEXT joint_ctx;
  AX_JOINT_EXECUTION_CONTEXT_SETTING_T ctx_settings;
  memset(&ctx_settings, 0, sizeof(ctx_settings));
  assert(AX_ERR_NPU_JOINT_SUCCESS ==
         AX_JOINT_CreateExecutionContextV2(joint_handle, &joint_ctx,
                                           &ctx_settings));

  int stride_w = args.get<int>("stride_w");
  bool manual_alloc = args.exist("manual_alloc");
  bool enable_trunc = args.exist("enable_trunc");
  if (stride_w) {
    printf("alloc buffer with stride %d\n", stride_w);
  }

  int override_batch_size = args.get<int>("override_batch_size");
  std::vector<AX_JOINT_IO_T> io_arr(1);
  printf("\n");

  AX_JOINT_IO_SETTING_T io_setting;
  memset(&io_setting, 0, sizeof(io_setting));
  io_setting.nWbtIndex = wbt_index;

  for (auto& io : io_arr) {
    prepare_io(io, io_info, input_fns, stride_w, manual_alloc, enable_trunc,
               override_batch_size);
    io.pIoSetting = &io_setting;
  }
  printf("\n");

  int test_count = args.get<int>("repeat");
  int warmup_iters = args.get<int>("warmup");
  auto time_start = std::chrono::system_clock::now();
  int duration_neu_core_us = 0, duration_neu_total_us = 0;
  int duration_onnx_core_us = 0, duration_onnx_total_us = 0;
  int duration_axe_core_us = 0, duration_axe_total_us = 0;
  bool has_neu_comp = false, has_onnx_comp = false, has_axe_comp = false;
  int duration_neu_init_us = 0;
  int duration_onnx_init_us = 0;
  int duration_axe_init_us = 0;
  AX_JOINT_COMPONENT_T* comps;
  uint32_t comp_size;
  assert(AX_ERR_NPU_JOINT_SUCCESS ==
         AX_JOINT_ADV_GetComponents(joint_ctx, &comps, &comp_size));
  for (uint32_t j = 0; j < comp_size; ++j) {
    auto& comp = comps[j];
    if (comp.eType == AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_NEU) {
      has_neu_comp = true;
      duration_neu_init_us += comp.tProfile.nInitUs;
    } else if (comp.eType ==
               AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_ONNX) {
      has_onnx_comp = true;
      duration_onnx_init_us += comp.tProfile.nInitUs;
    } else if (comp.eType ==
               AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_AXE) {
      has_axe_comp = true;
      duration_axe_init_us += comp.tProfile.nInitUs;
    } else {
      printf("Warning: unknown component type %d\n", (int)comp.eType);
    }
  }

  for (int i = 0; i < warmup_iters; i++) {
    auto& io = io_arr[i % io_arr.size()];
    assert(AX_ERR_NPU_JOINT_SUCCESS ==
           AX_JOINT_RunSync(joint_handle, joint_ctx, &io));
  }

  for (int i = 0; i < test_count; ++i) {
    auto& io = io_arr[i % io_arr.size()];
    assert(AX_ERR_NPU_JOINT_SUCCESS ==
           AX_JOINT_RunSync(joint_handle, joint_ctx, &io));
    assert(AX_ERR_NPU_JOINT_SUCCESS ==
           AX_JOINT_ADV_GetComponents(joint_ctx, &comps, &comp_size));
    if (test_count > 1 && i == 0) {
      time_start = std::chrono::system_clock::now();
      continue;
    }
    for (uint32_t j = 0; j < comp_size; ++j) {
      auto& comp = comps[j];
      if (comp.eType ==
          AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_NEU) {
        duration_neu_core_us += comp.tProfile.nCoreUs;
        duration_neu_total_us += comp.tProfile.nTotalUs;
      } else if (comp.eType ==
                 AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_ONNX) {
        duration_onnx_core_us += comp.tProfile.nCoreUs;
        duration_onnx_total_us += comp.tProfile.nTotalUs;
      } else if (comp.eType ==
                 AX_JOINT_COMPONENT_TYPE_T::AX_JOINT_COMPONENT_TYPE_AXE) {
        duration_axe_core_us += comp.tProfile.nCoreUs;
        duration_axe_total_us += comp.tProfile.nTotalUs;
      } else {
      }
    }
  }

  report_memory_usage("Report memory usage after AX_JOINT_RunSync");

  AX_JOINT_MODEL_CMM_INFO cmm;
  memset(&cmm, 0, sizeof(cmm));
  
  auto cmm_ret = AX_JOINT_GetCMMUsage(joint_handle, &cmm);
  if (cmm_ret != 0) {
    printf("Get CMM usage failed\n\n");
  } else {
    printf("CMM usage: %u\n\n", cmm.nCMMSize);
  }

  if (test_count > 1) test_count--;
  auto time_end = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                      time_end - time_start) /
                  test_count;

  printf(
      "Create handle took %.2f ms (neu %.2f ms, onnx %.2f ms, axe %.2f ms, "
      "overhead %.2f ms)\n",
      init_time_us / 1000., duration_neu_init_us / 1000.,
      duration_onnx_init_us / 1000., duration_axe_init_us / 1000.,
      (init_time_us - duration_neu_init_us - duration_onnx_init_us -
       duration_axe_init_us) /
          1000.);
  printf("Run task took %d us (%d rounds for average)\n", (int)duration.count(),
         test_count);
  if (has_neu_comp) {
    printf("\tRun NEU took an average of %d us (overhead %d us)\n",
           duration_neu_core_us / test_count,
           (duration_neu_total_us - duration_neu_core_us) / test_count);
  }
  if (has_onnx_comp) {
    printf("\tRun ONNX took an average of %d us (overhead %d us)\n",
           duration_onnx_core_us / test_count,
           (duration_onnx_total_us - duration_onnx_core_us) / test_count);
  }
  if (has_axe_comp) {
    printf("\tRun AXE took an average of %d us (overhead %d us)\n",
           duration_axe_core_us / test_count,
           (duration_axe_total_us - duration_axe_core_us) / test_count);
  }
  printf("\n");

  AX_NPU_SDK_EX_WORK_CYCLE_T work_cycle;
  auto ret = AX_NPU_SDK_EX_Get_Work_Cycle(&work_cycle);
  if (ret != AX_NPU_DEV_STATUS_SUCCESS) {
    throw std::runtime_error("AX_NPU_SDK_EX_Get_Work_Cycle failed " +
                             std::to_string((int)ret));
  }
#if defined(CHIP_AX630A)
  printf("NPU perf cnt total: %ld\n", work_cycle.total_cycle);
#else
  printf("NPU perf cnt total: %lld\n", work_cycle.total_cycle);
#endif
#if defined(CHIP_AX630A)
  for (int k = 0; k < AX_NPU_SDK_EX_EU_NUM; k++)
#else
  for (uint32_t k = 0; k < work_cycle.valid_eu_num; k++)
#endif
    printf("\tNPU perf cnt of eu(%d): %d\n", k, work_cycle.eu_cycle[k]);

  auto out_dir = args.get<std::string>("bin-out-dir");
  if (!out_dir.empty()) {
    int batch_size = io_info->nMaxBatchSize == 0 ? 1 : io_info->nMaxBatchSize;
    if (override_batch_size) {
      batch_size = override_batch_size;
    }

    // 因为output的tensor name可能包含'/',
    // 在dump输出时造成文件路径错误，这里将'/'替换成'_'
    std::unordered_set<std::string> output_names;
    auto convert_slash_and_record_output_names =
        [&output_names](const std::string& name) -> std::string {
      std::string temp_name = name;
      std::size_t slash_pos = temp_name.find('/');
      while (slash_pos != std::string::npos) {
        temp_name.replace(slash_pos, 1, 1, '_');
        slash_pos = temp_name.find('/');
      }
      if (output_names.count(temp_name) > 0) {
        printf("output name (%s) conflicts!\n", temp_name.c_str());
        assert(0);
      }
      output_names.insert(temp_name);
      return temp_name;
    };

    auto& io = io_arr[0];
    for (size_t i = 0; i < io_info->nOutputSize; ++i) {
      auto& info = io_info->pOutputs[i];
      auto& output = io.pOutputs[i];
      auto name = convert_slash_and_record_output_names(info.pName);
      std::string fn = out_dir + "/" + name + ".bin";
      int actual_data_size = output.nSize / info.pShape[0] * batch_size;
      if (output.pVirAddr != nullptr && output.nSize > 0) {
        if (save_file(fn.c_str(), output.pVirAddr, actual_data_size)) {
          printf("dump output[%zd] %s as %s success\n", i, info.pName,
                 fn.c_str());
        } else {
          printf("dump output[%zd] %s as %s failed\n", i, info.pName,
                 fn.c_str());
        }
      } else {
        printf("can not dump output[%zd] %s, it is none\n", i, info.pName);
      }
    }
  }

  for (auto& io : io_arr) {
    for (size_t i = 0; i < io.nInputSize; ++i) {
      AX_JOINT_IO_BUFFER_T* pBuf = io.pInputs + i;
      if (pBuf->pVirAddr != nullptr) {
        if (manual_alloc) {
          free_test_buffer_manually(pBuf);
        } else {
          free_test_buffer(pBuf);
        }
      }
    }
    for (size_t i = 0; i < io.nOutputSize; ++i) {
      AX_JOINT_IO_BUFFER_T* pBuf = io.pOutputs + i;
      if (manual_alloc) {
        free_test_buffer_manually(pBuf);
      } else {
        free_test_buffer(pBuf);
      }
    }
    delete[] io.pInputs;
    delete[] io.pOutputs;
  }

  assert(AX_ERR_NPU_JOINT_SUCCESS ==
         AX_JOINT_DestroyExecutionContext(joint_ctx));
  assert(AX_ERR_NPU_JOINT_SUCCESS == AX_JOINT_DestroyHandle(joint_handle));

  AX_JOINT_Adv_Deinit();
  AX_SYS_Deinit();
  return 0;
}
