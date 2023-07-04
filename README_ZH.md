# pimsim-nn

Pimsim-nn是一个面向RRAM神经网络加速器的模拟器，在给定架构配置和指令序列后可模拟得到性能和能耗情况。

## 使用方法

### 软件要求
- cmake >= 3.6
- gcc >= 4.8.5
### 编译项目
本项目使用cmake进行构建，执行如下代码进行构建
```shell
cd pimsim-nn
mkdir build && cd build 
cmake ..
make 
```
构建结束后，生成的相关文件位于build目录下，ChipTest为我们需要的二进制文件
### 模拟样例
本项目自带了一个resnet18的模拟样例，配置文件和指令文件在`test/resnet18`文件夹下，使用如下方式即可进行模拟

```shell
# 须在build目录下找到ChipTest
ChipTest ~/pimsim-nn/test/resnet18/full.gz ~/pimsim-nn/test/resnet18/config.json
```

输出结果:
```shell
        SystemC 2.3.4-Accellera --- Jul  4 2023 15:44:33
        Copyright (c) 1996-2022 by all Contributors,
        ALL RIGHTS RESERVED
Loading Inst and Config
Load finish
Reading Inst From Json
hereRead finish
Start Simulation
Progress --- <10%>
Progress --- <20%>
Progress --- <30%>
Progress --- <40%>
Progress --- <50%>
Progress --- <60%>
Progress --- <70%>
Progress --- <80%>
Progress --- <90%>
Simulation Finish
|*************** Simulation Report ***************|
Basic Information:
  - config file:        ../test/resnet18/config.json
  - inst file:          ../test/resnet18/full.gz
  - verbose level:      0
  - core count:         136
  - simulation mode:    0
  - simulation time:    200 ms
Chip Simulation Result:
  - output count:       2.24 samples
  - throughput:         11.2 samples/s
  - average latency:    89.5 ms
  - average power:      6.09e+03 mW
  - average energy:     5.45e+11 pJ/it
```

## 架构

 Pimsim-nn假设一个芯片是由多个通过NoC连接的core组成的，core的架构如下：

![image-20230704172641128](https://s2.loli.net/2023/07/04/Y9ZeKzpTORIiakJ.png)

core的架构类似于一个RISC处理器，但是带有4个专用的处理单元，分别是Scalar Unit, Vector Unit, Matrix Unit和Transfer Unit。Scalar Unit处理标量运算，Vector Unit处理向量运算。Matrix Unit主要由RRAM crossbar array构成，能够高效执行矩阵向量乘运算。Transfer Unit负责核与核之间的数据交换和同步。

## 模拟器输入

模拟器需要3个文件：

- 架构配置文件
- NoC配置文件
- 程序指令序列文件

架构配置文件需要配置模拟器中各个部件的延迟和功耗信息。NoC配置文件给出了NoC的延迟和功耗信息。NoC配置本质上是架构配置的一部分，但因为NoC配置参数过多，因此独立成一个文件。方便起见，在架构配置中有一个参数指定了NoC配置文件地址，模拟器会自动加载NoC配置。程序指令序列由编译器生成。

最终模拟器需要输入两个参数，一个是程序指令序列文件的地址，另一个是架构配置文件的地址

```shell
ChipTest  path_to_program_instructions_file  path_to_archtecture_configuration_file 
```

在架构配置文件中有一些参数能用来改变模拟器行为

| 参数                 | 描述                                                         |
| -------------------- | ------------------------------------------------------------ |
| sim_time             | `sim_time`表示模拟时间，单位是ms                             |
| sim_mode             | 当设置为0时，模拟器会假设存在大量输入，给出吞吐率的信息，当设置为1时，模拟器会模拟单个输入情况下的延迟能耗情况 |
| report_verbose_level | 当设置为0时，模拟器会给出芯片级别的性能和能耗信息，当设置为1时，还会同时给出每个核的信息。 |

## 作者

- [Xinyu Wang](wangxinyu22s@ict.ac.cn) (Institute of Computing Technology,CAS)

## 鸣谢
- [systemc](https://github.com/accellera-official/systemc)
- [fmt](https://github.com/fmtlib/fmt)
- [zlib](https://github.com/madler/zlib)
- [nlohmann/json](https://github.com/nlohmann/json)
- [better-enums](https://github.com/aantron/better-enums)
- [filesystem](https://github.com/gulrak/filesystem)
- [zstr](https://github.com/mateidavid/zstr)