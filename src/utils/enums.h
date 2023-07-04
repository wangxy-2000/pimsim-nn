//
// Created by xyfuture on 2023/7/3.
//


#ifndef UTILS_ENUMS_H_
#define UTILS_ENUMS_H_

// support default constructor in better-enums

#define BETTER_ENUMS_DEFAULT_CONSTRUCTOR(Enum) \
  public:                                      \
    Enum() = default;

#include "better-enums/enum.h"


#endif //UTILS_ENUMS_H_
