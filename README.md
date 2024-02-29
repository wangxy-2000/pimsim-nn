# pimsim-nn

Pimsim-nn is a simulator designed for RRAM-/PIM-based neural network accelerators. By taking an instruction sequence as input, pimsim-nn evaluates performance (inference latency and/or throughput), power dissipation, and energy consumption under a given architecture configuration.

Pimsim-nn should be used with an associated compiler, [pimcomp-nn](https://github.com/sunxt99/PIMCOMP-NN). The compiler accepts an ONNX file and the architecture configuration (same as the architecture configuration used in pimsim-nn) as inputs and produces the instruction sequence.

The ISA of PIMSIM-NN is based on the document published [here](https://arxiv.org/abs/2308.06449).


## Usage

### Requirements

- cmake >= 3.6
- gcc >= 4.8.5

### Build

Cmake is used to build the whole project, run codes below:

```shell
cd pimsim-nn
mkdir build
cd build 
cmake ..
make 
```

In `build` directory, checkout executable file `ChipTest`.

### Simulation Example

There is a built-in resnet-18 example. Configuration and instructions file is under folder `test/resnet18`. Use codes below to simulate resnet-18:

```shell
./ChipTest ../example/resnet18.gz ../example/config/latency_config.json
```
outputs:
```shell
        SystemC 2.3.4-Accellera --- Dec 20 2023 21:10:27
        Copyright (c) 1996-2022 by all Contributors,
        ALL RIGHTS RESERVED
Loading Inst and Config ---
Load finish
Reading Instructions From File ---
Read finish
Start Simulation ---
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
simulator execution time:59.4615s
|*************** Simulation Report ***************|
Basic Information:
  - config file:        ../example/config/latency_config.json
  - inst file:          ../example/resnet18.gz
  - verbose level:      0
  - core count:         64
  - simulation mode:    0
  - simulation time:    100 ms
Chip Simulation Result:
  - output count:       4 samples
  - throughput:         40 samples/s
  - average latency:    25 ms
```

## Architecture

Pimsim-nn assumes a chip consists of many cores connected via NoC, and the core architecture is shown below:

![Drawing2.png](https://s2.loli.net/2023/12/20/DCyJl81rfTSxqG7.png)

The architecture of core is very similar to a RISC processor, but with four dedicated execute units, namely Scalar Unit, Vector Unit, Matrix Unit and Transfer Unit. **Scalar Unit** is used to process scalar operations. **Vector Unit** performs vector-vector operations. **Matrix Unit** is mainly composed of RRAM crossbar arrays and executes matrix-vector multiply efficiently. **Transfer Unit** is responsible for inter-core data exchange and synchronization. 

## Simulator Inputs

Simulator requires three files:
- Architecture Configuration file 
- NoC Configuration file 
- Program Instructions file

The architecture configuration file primarily defines the latency and power of different components in the simulator. The NoC configuration file gives the latency and power of NoC. Actually, NoC configuration is a part of the architecture configuration, but is separated as an independent file due to the large number of configuration parameters it requires. For simplicity, there is a parameter in architecture configuration that indicates the path of NoC configuration file and the simulator can load NoC configuration automatically. The program instruction file is generated by [pimcomp-nn](https://github.com/sunxt99/PIMCOMP-NN).

Finally, only two inputs are required: one is the path of program instruction file, and the other is the path of architecture configuration file.   


``` shell
ChipTest  path_to_program_instructions_file  path_to_archtecture_configuration_file 
```

There are some parameters in architecture configuration file to change simulation behavior.

| Parameter            | Description                                                  |
| -------------------- | ------------------------------------------------------------ |
| sim_time             | `sim_time` represents simulation time in unit `ms`           |
| sim_mode             | When set to `0`, simulator assumes enough input samples and reports throughout rate. When set to `1`,  simulator will only process one input sample and gives its latency. |
| report_verbose_level | When set to `0`, simulator will only give chip level performance and power consumption statistics. When set to `1`, simulator will also give core level statistics. |

## Citing pimsim-nn

[1] Xinyu Wang, Xiaotian Sun, Yinhe Han, Xiaoming Chen, ["PIMSIM-NN: An ISA-based Simulation Framework for Processing-in-Memory Accelerators"](https://github.com/wangxy-2000/pimsim-nn/blob/main/doc/DATE_LBR.pdf), in Design, Automation, and Test in Europe (DATE'24).  [[Bibtex](https://github.com/wangxy-2000/pimsim-nn/blob/main/doc/bibtex.txt)] [[ArXiv](https://arxiv.org/abs/2402.18089)]

## Code Author
- [Xinyu Wang](wangxinyu22s@ict.ac.cn) (Institute of Computing Technology, Chinese Academy of Sciences)

## Project PI
- [Xiaoming Chen](https://people.ucas.edu.cn/~chenxm)

## Acknowledgements
- [systemc](https://github.com/accellera-official/systemc)
- [fmt](https://github.com/fmtlib/fmt)
- [zlib](https://github.com/madler/zlib)
- [nlohmann/json](https://github.com/nlohmann/json)
- [better-enums](https://github.com/aantron/better-enums)
- [filesystem](https://github.com/gulrak/filesystem)
- [zstr](https://github.com/mateidavid/zstr)