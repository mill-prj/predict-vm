#ifndef PV_VM_OPCODE_H
#define PV_VM_OPCODE_H

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_ADD_F,
    OP_SUB_F,
    OP_MUL_F,
    OP_DIV_F,
    OP_LOAD_INT,
    OP_LOAD_FLOAT,
    OP_LOAD_STR,
    OP_STORE_STR,
    OP_ALLOC_STACK,
    OP_FREE_STACK,
    OP_STORE_STACK,
    OP_LOAD_STACK,
    OP_CCALL,
    OP_DEBUG_REGISTERS,
} PV_Opcode;

#endif