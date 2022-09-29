# 版本说明
run_neu_v2在功能设计上是完全cover test_npu和run_neu的，但目前run_neu_v2还有以下两个功能点未实现：
* 异步推理模式
* 一次跑多个模型

在run_neu_v2实现上述两个功能后，test_npu和run_neu将被移除，所以，无上述两个功能需求的应尽快切换到run_neu_v2

# 简介
run_neu_v2 用于上板验证 dot-neu 模型，主要包括以下功能：

* 支持查看模型信息
    *  I/O 信息，默认开启，可通过指定“\-\-show-io-info 0”关闭
    * 虚拟NPU模式
    * 是否支持动态batch
    * 是否需要RuntimeVar

* 支持模型推理
    * 支持同步推理模式，**暂不支持异步推理模式**
    * 支持使用随机的输入数据
    * 支持使用指定的输入数据
    * 支持使用指定输出的GT数据进行NPU直出对分

* 支持动态batch
    * 支持指定batch size，范围为[1, MAX_BATCH_SIZE]
    * 支持指定batch类型，BLOCK或GATHER

* 支持将输入和输出的数据保存成文件
    * **输入和输出存在相同名称的模型不支持此功能**

* 支持统计模型推理的平均耗时

* 支持统计模型推理的NPU Cycle
    * 默认开启，可通过指定“\-\-enable-npu-cycle 0”关闭

* 支持RuntimeVar
    * **暂仅支持librosetta生成的RuntimeVar的bin数据**

# 使用示例

* 仅查看模型信息
    ```
    run_neu_v2 --neu dot.neu --only-info
    ```

* 使用随机数据跑一次模型
    ```
    run_neu_v2 --neu dot.neu
    ```

* 保存输入和输出的数据
    ```
    run_neu_v2 --neu dot.neu --bin-out-dir out
    ```
    * 在out目录下得到输入和输出的数据文件，输入的文件名是tensor_name.bin, 输出的文件名是tensor_name_inner.bin

* 使用指定输入数据和输出对分数据跑一次模型
    ```
    run_neu_v2 --neu dot.neu --input input.bin --check gt_output.bin
    ```
    * 模型包含多个输入时，**需要按模型I/O信息中输入的顺序，依次使用--input来指定输入文件**，输出类似
    * 输出对分结果log
        ```
        OUTPUT(op_108:dma_affine2) GT check success
        ```

* 使用指定输入数据和输出对分数据跑一次模型（已知模型IO的tensor name，未知IO的顺序）
    ```
    run_neu_v2 \
        --neu dot.neu --input a2.bin --input a0.bin --input a1.bin \
        --input_tensor input_layer2 --input_tensor input_layer0 --input_tensor input_layer1 \
        --check gt1.bin --check gt2.bin --check gt0.bin \
        --output_tensor output_layer1 --output_tensor output_layer2 --output_tensor output_layer0
    ```
    * \-\-input[i]指定的文件将被加载到\-\-input_tensor[i]指定的input tensor(由tensor name指定)
    * \-\-check[i]指定的文件将用于\-\-output_tensor[i]指定的output tensor(由tensor name指定)的NPU直出对分

* 模型推理测速
    ```
    run_neu_v2 --neu dot.neu --repeat 100
    ```

* 使用旧的指定输入数据的模式跑一次模型
    ```
    run_neu_v2 --neu dot.neu --data "input:input.bin"
    ```

* RuntimeVar
    * 以上的使用示例中，除去“仅查看模型信息”，其他示例均可通过增加"\-\-rtv runtime_var.bin"来运行带RuntimeVar的模型
    * **“仅查看模型信息”无需"\-\-rtv"**

# 更多信息
```
run_neu_v2 --help
```
