//
// Created by xyfuture on 2023/3/2.
//


#ifndef ISA_ISA_H_
#define ISA_ISA_H_
#pragma once

#include "better-enums/enum.h"

BETTER_ENUM(Opcode,int,
            nop=0,
            sldi,sld,sadd,ssub,smul,saddi,smuli,
            setbw,
            mvmul,
            vvadd,vvsub,vvmul,vvdmul,
            vvmax,vvsll,vvsra,vavg,
            vrelu,vtanh,vsigm,
            vmv,
            vrsu,vrsl,
            ld,st,lldi,lmv,
            send,recv,wait,sync
            ); // add nop



BETTER_ENUM(InstType,int,nop=0,scalar,matrix,vector,transfer,percesion);


#endif //ISA_ISA_H_
