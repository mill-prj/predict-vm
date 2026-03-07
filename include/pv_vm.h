#ifndef PV_VM_H
#define PV_VM_H

#include "pv_opcode.h"
#include <stdint.h>

typedef struct {
    PV_Opcode opcode;
    int8_t dst;
    union {
        int8_t src;
        struct {
            int8_t src1;
            int8_t src2;
        };
    };
    union {
        int32_t offset;
        int32_t imm;
    };
} PV_Instruction;

typedef struct {
    union {
        // 스택에 정수와 float 을 둘다 저장합니다
        int32_t i32;
        float f32;
    }* stack;
    int32_t stack_size;
    int32_t sp;
    int32_t pc;
} PV_VM;

void pv_vm_execute(PV_VM* vm, const PV_Instruction* code, int code_size);

#endif