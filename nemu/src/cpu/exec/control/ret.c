#include "cpu/exec/helper.h"

make_helper(ret) {
	current_sreg = R_SS;
	cpu.eip = swaddr_read(cpu.esp, 4) - 1;
	cpu.esp += 4;

	print_asm("ret");

	return 1;
}

make_helper(ret_i) {
	uint16_t imm = instr_fetch(eip + 1, 2);
	current_sreg = R_SS;
	cpu.eip = swaddr_read(cpu.esp, 4) - (2 + 1);
	cpu.esp += 4 + imm;

	print_asm("ret $0x%x", imm);

	return 2 + 1;
}

