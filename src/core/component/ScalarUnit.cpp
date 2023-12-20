//
// Created by xyfuture on 2023/3/6.
//

#include "ScalarUnit.h"
#include "core/Core.h"
#include <sstream>

ScalarUnit::ScalarUnit(const sc_module_name &name, const CoreConfig &config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk)
: BaseCoreModule(name,config,sim_config,core_ptr,clk),
 scalar_reg("scalar_reg",clk){
    scalar_reg.input.bind(id_scalar_port);
    scalar_reg.output.bind(scalar_reg_out);

    scalar_reg.in_ready.bind(scalar_ready_port);
    scalar_reg.out_ready.bind(self_ready);

    self_ready.write(true);

    SC_METHOD(process);
    sensitive<<scalar_reg_out;

    energy_counter.setStaticPowerMW(config.scalar_static_power);
}

void ScalarUnit::process() {
    const auto& cur_info = scalar_reg_out.read();

    if (not cur_info.valid){
        return;
    }

    const auto& scalar_info = cur_info.payload;

    if (scalar_info.op == +Opcode::nop) // for nop inst, lack of checkInst
        return;

    RegFileWrite write_info = RegFileWrite::empty();

    switch (scalar_info.op) {
        case Opcode::sadd:
            write_info = {.rd_addr=scalar_info.rd_addr,
                          .rd_value=scalar_info.rs1_value + scalar_info.rs2_value};
            break;
        case Opcode::saddi:
            write_info = {.rd_addr=scalar_info.rd_addr,
                          .rd_value=scalar_info.rs1_value + scalar_info.imm};
            break;
        case Opcode::ssub:
            write_info = {.rd_addr=scalar_info.rd_addr,
                          .rd_value=scalar_info.rs1_value - scalar_info.rs2_value};
            break;
        case Opcode::smul:
            write_info = {.rd_addr=scalar_info.rd_addr,
                          .rd_value=scalar_info.rs1_value * scalar_info.rs2_value};
            break;
        case Opcode::smuli:
            write_info = {.rd_addr=scalar_info.rd_addr,
                          .rd_value=scalar_info.rs1_value * scalar_info.imm};
            break;
        case Opcode::sldi:
            write_info = {.rd_addr=scalar_info.rd_addr,
                          .rd_value=scalar_info.imm};
            break;
        case Opcode::sld:
            break;
        default: // nop
            return;
    }

    reg_file_write_port.write(write_info);

//    if (sim_config.sim_mode == 1)
//        if (isEndPC(scalar_info.pc))
//            core_ptr->setFinish();

    energy_counter.addDynamicEnergyPJ(0,sc_time_stamp(),core_config.period,core_config.scalar_dynamic_power);
}

std::string ScalarUnit::getStatus() {
    std::stringstream  s;

    const auto& cur_info = id_scalar_port.read();

    if (cur_info.valid){
        auto scalar_info = cur_info.payload;

        s<<"Scalar>"<<" time:"<<sc_time_stamp().to_string()<<"\n"
        <<"pc:" << scalar_info.pc<<" op:"<<scalar_info.op._to_string()<<"\n\n";
    }

    return s.str();
}


