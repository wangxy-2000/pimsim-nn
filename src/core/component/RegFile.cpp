//
// Created by xyfuture on 2023/3/6.
//

#include "RegFile.h"
#include "core/Core.h"

std::uint64_t concat(const std::uint32_t& leftHalf, const std::uint32_t& rightHalf){
    std::uint64_t concatenated = (static_cast<std::uint64_t>(leftHalf) << 32) | rightHalf;

    return concatenated;
}


RegFile::RegFile(const sc_module_name &name, const CoreConfig &config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk)
: BaseCoreModule(name,config,sim_config,core_ptr,clk),
 reg_data(32), write_info_reg("reg",clk){
    write_info_reg.input.bind(reg_in);
    write_info_reg.output.bind(reg_out);

    SC_METHOD(readValue);
    sensitive << reg_out << reg_file_write_port << reg_file_read_addr_port; // 三个端口都需要
    SC_METHOD(writeValue);
    sensitive << reg_file_write_port;
    SC_METHOD(updateValue);
    sensitive<<reg_out;

    energy_counter.setStaticPowerMW(config.reg_file_static_power);

}

void RegFile::readValue() {
    auto last_write= reg_out.read();
    auto cur_write = reg_file_write_port.read();

    auto read_info = reg_file_read_addr_port.read();
    RegFileReadValue value_info = RegFileReadValue::empty();
    if (!read_info.double_word) {
        int values[] = {0, 0, 0}; // rd rs1 rs2
        int addrs[] = {read_info.rd_addr, read_info.rs1_addr, read_info.rs2_addr};

        for (int i = 0; i < 3; i++) { // first use bypass value then register value
            auto addr = addrs[i];
            if (addr == 0)
                values[i] = 0;
            else if (addr == cur_write.rd_addr)
                values[i] = cur_write.rd_value;
            else if (addr == last_write.rd_addr)
                values[i] = last_write.rd_value;
            else
                values[i] = reg_data[addr];
        }
        value_info =  {.rd_value = values[0],.rs1_value=values[1],.rs2_value=values[2]};
    }
    else {
        // double int -> long long

        int values[] = {0, 0, 0, 0, 0}; // rd rs1 rs2
        int addrs[] = {read_info.rd_addr,read_info.rd_addr+1, read_info.rs1_addr, read_info.rs1_addr+1, read_info.rs2_addr};

        for (int i=0;i<5;i++){
            auto addr = addrs[i];
            if (addr == 0)
                values[i] = 0;
            else if (addr == cur_write.rd_addr)
                values[i] = cur_write.rd_value;
            else if (addr == last_write.rd_addr)
                values[i] = last_write.rd_value;
            else
                values[i] = reg_data[addr];
        }
        long long rd_value_double = concat(values[0],values[1]);
        long long rs1_value_double = concat(values[2],values[3]);
        int rs2_value = values[4];

        value_info.rs2_value=rs2_value;
        value_info.rd_value_double=rd_value_double;
        value_info.rs1_value_double=rs1_value_double;

    }

    reg_file_read_value_port.write(value_info);

    energy_counter.addDynamicEnergyPJ(0,sc_time_stamp(),core_config.period,core_config.reg_file_read_dynamic_power);

}

void RegFile::writeValue() {
    auto cur_write = reg_file_write_port.read();
    reg_in.write(cur_write);
    energy_counter.addDynamicEnergyPJ(1,sc_time_stamp(),core_config.period,core_config.reg_file_write_dynamic_power);
}

void RegFile::updateValue() {
    // bypass , postpone the update time
    auto last_write = reg_out.read();
    if (last_write.rd_addr != 0)
        reg_data[last_write.rd_addr] = last_write.rd_value;
}
