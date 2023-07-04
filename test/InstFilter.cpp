//
// Created by xyfuture on 2023/4/17.
//

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "isa/Instruction.h"
#include <zstr.hpp>

using json = nlohmann::json;

nlohmann::json filter(nlohmann::json& json_inst){
    nlohmann::json new_json_inst;
    for(auto& core_json_inst:json_inst.items()){
        nlohmann::json tmp_core ;
        for (auto& inst:core_json_inst.value()){
            auto op = inst.at("op").get<std::string>();
            if (op == "send" or op == "recv" or op == "sync" or op == "wait"){
                tmp_core.push_back(inst);
            }
        }
        if (!tmp_core.empty()) {
            new_json_inst[core_json_inst.key()] = tmp_core;
        }
    }
    return new_json_inst;
}


int main() {

//    std::ifstream f ("D:\\code\\ScPIMsim\\test\\inst\\resnet18_inst.json");

    zstr::ifstream f_read("D:\\code\\ScPIMsim\\test\\resnet18\\full.json");

//    zstr::ifstream f_read("D:\\code\\ScPIMsim\\test\\resnet18\\full.gz",std::ios::binary);

    std::ofstream f_write("D:\\code\\ScPIMsim\\test\\resnet18\\sync.json");


    json inst_data = json::parse(f_read);



    auto new_inst_data = filter(inst_data);

    f_write << new_inst_data;

    f_write.close();
    return 0;

}




//    int cnt = 0;
//    for(auto& core_json_inst:inst_data.items()){
//        auto tmp_core_inst = readSingleCoreInstFromJson(core_json_inst.value());
//        cnt = 0;
//        for (const auto& inst:tmp_core_inst){
//            if (inst.op == +Opcode::send or inst.op == +Opcode::recv){
//                std::cout<<"Core:"<<core_json_inst.key()
//                    <<" op:"<<inst.op._to_string()<<" pc:"<<cnt<<std::endl;
//            }
//            cnt++ ;
//        }
//    }



//    auto new_core_inst = spiltInst(inst_data);
//    // write prettified JSON to another file
//    std::ofstream o("resnet18_sync_inst.json");
//    o << std::setw(4) << new_core_inst << std::endl;

    //    std::cout<<new_core_inst;
//    for (auto& tmp:new_core_inst.items())
//        if(tmp.key() == "core1")
//            std::cout<<tmp.value();

//    return 0;
//}


//struct test{
//    std::vector<int> array = {0,0};
//
//    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(test,array);
//};



//int main(){
//    json ex1 = json::parse(R"(
//          {
//            "array":[1,2,3,4]
//          }
//        )");
//
//    auto tmp = ex1.get<test>();
//
//    return 0;
//}
