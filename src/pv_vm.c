// gcc, clang 의 goto 를 사용하지 않았습니다
// clang 과 gcc 에서만 작동하게 한다면 되겠지만
// 다양한 컴파일러에서 지원을 하고싶었기에 그렇게 하지 않았습니다
// c 기반 vm 이기에 기계어를 직접 생성하지 않습니다
// 그렇기에 ffi 구현은 너무 어렵습니다
// 그러므로 구현하지 못했습니다
#include "pv_vm.h"
#include <stdio.h>
#include <stdint.h>

// 문자열풀 alloc 을 위한 포인터입니다
// 문자열의 포인터의 리스트입니다
// like [[char]*]
const char*** _actual_str_pool = NULL;
// 스트링풀입니다
// 문자열의 포인터 리스트입니다
// like [[char]*]*
extern const char**** const PV_STR_POOL = &_actual_str_pool;

// 프로그램을 vm 에 올려 실행합니다
// 프로그램 인스트럭션 코드 리스트 를 받습니다
// c 의 특성상 어레이 사이즈를 모르기에 code size 도 함께 받습니다
void pv_vm_execute(PV_VM* vm, const PV_Instruction** code, int code_size) {
    // 레지스터들을 정의합니다 (R0~R7)
    // register 키워드는 최근의 C 컴파일러에서는 거의 무의미하지만
    // 일단 컴파일러에게 알려줍니다
    // 정수용 레지스터입니다 float 용 레지스터는 총 4개 입니다
    register int32_t r0, r1, r2, r3, r4, r5, r6, r7 = 0;
    
    // 계산과 저장용 f0 f1 f2, temp 용 f3
    // float32 를 사용합니다
    register float f0, f1, f2, f3 = 0.0;

    // 문자열의 포인터입니다
    // 캐싱되어있는 문자열을 참조합니다 
    const char** s0 = NULL;

    // 스텍 주소를 고정시킵니다
    // 캐싱하기 쉽게 만들어줍니다
    int32_t* const stack = vm->stack;

    // Program Counter, Stack Pointer 를 초기화 시킵니다
    vm->pc = 0;
    vm->sp = 0;

    while (vm->pc < code_size) {
        PV_Instruction* inst = code[vm->pc];

        switch (inst->opcode) {
            // 오퍼레이터 연산의 레지스터는
            // src1=R0, src2=R1, dst=R3 가 고정입니다
            case OP_ADD:
                if (inst->dst == 2 && inst->src1 == 0 && inst->src2 == 1)
                    r2 = r0 + r1;
                break;
            case OP_SUB:
                if (inst->dst == 2 && inst->src1 == 0 && inst->src2 == 1)
                    r2 = r0 - r1;
                break;
            case OP_MUL:
                if (inst->dst == 2 && inst->src1 == 0 && inst->src2 == 1)
                    r2 = r0 * r1;
                break;
            case OP_DIV:
                if (inst->dst == 2 && inst->src1 == 0 && inst->src2 == 1)
                    r2 = r0 / r1;
                break;
            // float 을 연산합니다
            // float 용 레지스터를 사용합니다
            case OP_ADD_F:
                if (inst->dst == 2 && inst->src1 == 0 && inst->src2 == 1)
                    f2 = f0 + f1;
                break;
            case OP_SUB_F:
                if (inst->dst == 2 && inst->src1 == 0 && inst->src2 == 1)
                    f2 = f0 - f1;
                break;
            case OP_MUL_F:
                if (inst->dst == 2 && inst->src1 == 0 && inst->src2 == 1)
                    f2 = f0 * f1;
                break;
            case OP_DIV_F:
                if (inst->dst == 2 && inst->src1 == 0 && inst->src2 == 1)
                    f2 = f0 / f1;
                break;
            // int 를 로드합니다
            // dst 는 각각 하드코딩 해서 넣습니다
            // dst(Register) 에 값을 저장합니다
            case OP_LOAD_INT:
                if (inst->dst == 0) r0 = inst->imm;
                else if (inst->dst == 1) r1 = inst->imm;
                else if (inst->dst == 2) r2 = inst->imm;
                else if (inst->dst == 3) r3 = inst->imm;
                else if (inst->dst == 4) r4 = inst->imm;
                else if (inst->dst == 5) r5 = inst->imm;
                else if (inst->dst == 6) r6 = inst->imm;
                else if (inst->dst == 7) r7 = inst->imm;
                break;
            // float 을 로드합니다
            // dst 는 각각 하드코딩 해서 넣습니다
            // dst(FRegister) 에 값을 저장합니다
            case OP_LOAD_FLOAT:
                if (inst->dst == 0) f0 = inst->imm;
                else if (inst->dst == 1) f1 = inst->imm;
                else if (inst->dst == 2) f2 = inst->imm;
                else if (inst->dst == 3) f3 = inst->imm;
                break;
            // 문자열을 로드합니다
            // s_reg 의 주소값을 스트링풀에서 가져옵니다
            // 이때의 offset 은 문자열풀의 문자열 위치입니다
            case OP_LOAD_STR:
                s0 = PV_STR_POOL[inst->offset];
                break;
            // 문자열을 풀에 저장합니다
            // offset 으로는 pool 에 저장될 위치를 넣습니다
            case OP_STORE_STR:
                // 문자열을 저장할때마다 메모리를 realloc 합니다
                *PV_STR_POOL = (const char**)realloc(*PV_STR_POOL, (inst->offset + 1) * sizeof(char*));
                (*PV_STR_POOL)[inst->offset] = s0;
                break;
            // stack 을 할당합니다
            case OP_ALLOC_STACK:
                vm->sp += inst->offset; 
                break;
            // stack 을 할당해제합니다
            case OP_FREE_STACK:
                vm->sp -= inst->offset;
                break;
            // 스텍에 레지스터값을 저장합니다
            // src(레지스터값)을 하드코딩하여 가져옵니다
            // 스텍 저장 주소는 inst->offset 입니다
            case OP_STORE_STACK:
                if (inst->src == 0) stack[inst->offset] = r0;
                else if (inst->src == 1) stack[inst->offset] = r1;
                else if (inst->src == 2) stack[inst->offset] = r2;
                else if (inst->src == 3) stack[inst->offset] = r3;
                else if (inst->src == 4) stack[inst->offset] = r4;
                else if (inst->src == 5) stack[inst->offset] = r5;
                else if (inst->src == 6) stack[inst->offset] = r6;
                else if (inst->src == 7) stack[inst->offset] = r7;
                break;
            // 스텍에서 레지스터로 값을 가져옵니다
            // dst 를 하드코딩하여 가져옵니다
            // 가져오는 위치는 inst->offset 입니다 
            case OP_LOAD_STACK:
                if (inst->dst == 0) r0 = stack[inst->offset];
                else if (inst->dst == 1) r1 = stack[inst->offset];
                else if (inst->dst == 2) r2 = stack[inst->offset];
                else if (inst->dst == 3) r3 = stack[inst->offset];
                else if (inst->dst == 4) r4 = stack[inst->offset];
                else if (inst->dst == 5) r5 = stack[inst->offset];
                else if (inst->dst == 6) r6 = stack[inst->offset];
                else if (inst->dst == 7) r7 = stack[inst->offset];
                break;
            case OP_DEBUG_REGISTERS:
                printf("r0 %d, r1 %d, r2 %d, r3 %d, r4 %d, r5 %d, r6 %d, r7 %d", r0, r1, r2, r3, r4, r5, r6, r7);
                break;
        }
        vm->pc++;
    }
}