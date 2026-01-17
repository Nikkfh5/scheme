#pragma once

#include <string>
#include <object.h>
Enviromnent MakeGlobalEnv(Heap* heap);
class Interpreter {
public:
    Interpreter() :heap_(),  env_(MakeGlobalEnv(&heap_)) {
    }
    std::string Run(const std::string&);

private:
    Heap heap_;
    Enviromnent env_;
};
