#include "cpu.h"

cpu::cpu(): A(0), X(0), Y(0), SP(0x01FD), cycles(0), initialCycles(0),C(false),Z(false), I(false), D(false),
		B(false), V(false), N(false)
{
	debugFile.open("debug.txt");
	debugFile.setf(ios::hex, ios::basefield);
}

cpu::~cpu()
{
	debugFile.close();
}

unsigned short cpu::emulateCycle(memory* memory, ppu* ppu)
{
	if(ppu->NMI) NMI(memory, ppu);
	else
	{
		unsigned short temp1 = 0, temp2 = 0; 	//Used when reading from other addresses
		unsigned char data = 0;			//Holds the data to write to memory
		char offset = 0;			//Used with branches
		short compare = 0;			//Used with compare instructions
		unsigned char statusByte;		//Used for putting status flags into a byte
		bool statusFlags[8];			//Used for putting status flags into a byte
		bool outputFlags[8] = {false, false, false, false, false, false, false, false}; //For getting flags off stack
		char debugOffset;			//Used for debugging branches
		bool oldBit7, oldBit0;			//Used for rotation instructions
		unsigned char opcode;

	
		//InitialA and initialCycles are used in the switch.  Rest are for debugging
		initialA = A;
		initialX = X;
		initialY = Y;
		initialSP = SP;
		initialCycles = (initialCycles + (cycles * 3)) % 341;
		statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
		statusFlags[4] = B, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
		initialP = encodeBits(statusFlags);

		opcode = memory->readRAM(PC, ppu); 		//Fetching opcode
		PC++;						//Increments after fetch

		switch(opcode)
		{
		case 0x69:	//Immidiate add with carry
			data = memory->readRAM(PC, ppu);
			temp1 = A + data + C;
			A = temp1 & 0xFF;
			C = (temp1 >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			V = (initialA ^ temp1) & (data ^ temp1) & 0x80;		//Checks the sign of the inputs and result
			debugImm(opcode, data, "ADC");
			cycles =  2;
			PC++;
			break; 
		case 0x65:	//Zeropage add with carry
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			temp2 = A + data + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "ADC");
			cycles =  3;
			PC++;
			break; 
		case 0x75:	//Zeropage,X add with carry
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			temp2 = A + data + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "ADC");
			cycles =  4;
			PC++;
			break;
		case 0x6D:	//Absolute add with carry
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			temp2 = A + data + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "ADC");
			cycles =  4;
			PC += 2;
			break;
		case 0x7D:	//Absolute,X add with carry
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			temp2 = A + data + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "ADC");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0x79:	//Absolute,Y add with carry
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			temp2 = A + data + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "ADC");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0x61:	//Indirect,X add with carry
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;	//Needs to use the initial value for the right bit.
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, data, "ADC");
			cycles =  6;
			PC++;
			break;
		case 0x71:	//Indirect,Y add with carry
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, data, "ADC");
			pageBoundry((temp2 - Y), temp2, 5);
			PC++;
			break;
		case 0x29:	//Immediate AND with A & memory
			A = A & memory->readRAM(PC, ppu);
			Z = !(A);
			N = A & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "AND");
			cycles =  2;
			PC++;
			break;
		case 0x25:	//Zeropage AND with A & memory
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "AND");
			cycles =  3;
			PC++;
			break;
		case 0x35:	//Zeropage,X AND with A & memory
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			data = memory->readRAM(temp1, ppu);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data,"AND");
			cycles =  4;
			PC++;
			break;
		case 0x2D:	//Absolute AND
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "AND");
			cycles =  4;
			PC += 2;
			break;
		case 0x3D:	//Absolute,X AND
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "AND");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0x39:	//Absolute,Y AND
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "AND");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0x21:	//Indirect,X AND
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, data, "AND");
			cycles =  6;
			PC++;
			break;
		case 0x31:	//Indirect,Y AND
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, data, "AND");
			pageBoundry((temp2 - Y), temp2, 5);
			PC++;
			break;
		case 0x0A:	//Accumulator shift left
			C = A & 0x80;
			A = A << 1;
			Z = !(A);
			N = A & 0x80;
			cycles =  2;
			debugAcc(opcode, "ASL");
			break;
		case 0x06:	//Zeropage shift left
			temp1 = memory->readRAM(PC, ppu);	//Gets the ZP address
			data = memory->readRAM(temp1, ppu);	//Gets data at the address
			C = data & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "ASL");
			data <<= 1;				//Shift left
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles =  5;
			PC++;
			break;
		case 0x16:	//Zeropage,X shift left
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			data = memory->readRAM(temp1, ppu);
			C = data & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "ASL");
			data <<= 1;				//Shift left
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles =  6;
			PC++;
			break;
		case 0x0E:	//Absolute shift left
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			C = data & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "ASL");
			data <<= 1;				//Shift left
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles =  6;
			PC += 2;
			break;
		case 0x1E:	//Absolute,X shift left
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			C = data & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "ASL");
			data <<= 1;				//Shift left
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles =  7;
			PC += 2;
			break;
		case 0x90:	//Branch if carry clear
			data = memory->readRAM(PC, ppu);			//Needs unsigned information
			debugOffset = memory->readRAM(PC, ppu);
			debugRelative(opcode, debugOffset, data, "BCC");
			if(!C) {
				offset = memory->RAM[PC++];
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
				cycles =  3;
			}
			else	{
				PC++; 
				cycles =  2;
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		 case 0xB0:	//Branch if carry set
			data = memory->readRAM(PC, ppu);			//Needs unsigned information
			debugOffset = memory->readRAM(PC, ppu);
			debugRelative(opcode, debugOffset, data, "BCS");
			if(C) {
				offset = memory->RAM[PC++];
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
				cycles =  3;
			}
			else	{
				PC++; 
				cycles =  2;
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0xF0:	//Branch if equal (zero flag set)
			data = memory->readRAM(PC, ppu);			//Needs unsigned information
			debugOffset = memory->readRAM(PC, ppu);
			debugRelative(opcode, debugOffset, data, "BEQ");
			if(Z) {
				offset = memory->RAM[PC++];
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
				cycles =  3;
			}
			else	{
				PC++; 
				cycles =  2;
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x24:	//Zeropage bit test
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			Z = !(A & data);
			V = data & 0x40;	//Gets the 6th bit only
			N = data & 0x80;	//Gets the 7th bit only
			debugZero(opcode, memory->readRAM(PC, ppu), data, "BIT");
			cycles =  3;
			PC++;
			break;
		case 0x2C:	//Absolute bit test
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			Z = !(A & data);
			V = data & 0x40;	//Gets the 6th bit only
			N = data & 0x80;	//Gets the 7th bit only
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "BIT"); 
			cycles =  4;
			PC += 2;
			break;
		case 0x30:	//Branch if minus (N set)
			data = memory->readRAM(PC, ppu);			//Needs unsigned information
			debugOffset = memory->readRAM(PC, ppu);
			debugRelative(opcode, debugOffset, data, "BMI");
			if(N) {
				offset = memory->RAM[PC++];
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
				cycles =  3;
			}
			else	{
				PC++; 
				cycles =  2;
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0xD0:	//Branch if not equal (Z not set)
			data = memory->readRAM(PC, ppu);			//Needs unsigned information
			debugOffset = memory->readRAM(PC, ppu);
			debugRelative(opcode, debugOffset, data, "BNE");
			if(!Z) {
				offset = memory->RAM[PC++];
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
				cycles =  3;
			}
			else	{
				PC++; 
				cycles =  2;
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x10:	//Branch if positive (N not set)
			data = memory->readRAM(PC, ppu);			//Needs unsigned information
			debugOffset = memory->readRAM(PC, ppu);
			debugRelative(opcode, debugOffset, data, "BPL");
			if(!N) {
				offset = memory->RAM[PC++];
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
				cycles =  3;
			}
			else	{
				PC++; 
				cycles =  2;
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x00:	//Force interrupt
			data = (PC & 0xFF00) >> 8;		//Push high byte first
			pushStack(memory, data, ppu);
			data = PC & 0x00FF;
			pushStack(memory, data, ppu);	
			//Bit-----7--6--5--4--3--2--1--0 PHP always has B and bit 5 true
			//Order = N, V, 1, B, D, I, Z, C
			statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
			statusFlags[4] = 1, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
			statusByte = encodeBits(statusFlags);	//Puts the status flags into a
			pushStack(memory, statusByte, ppu);				//byte	
			PC = (memory->RAM[0xFFFF] << 8) | memory->RAM[0xFFFE];
			B = true;
			debugImplied(opcode, "BRK");
			cycles =  7;
			break;
		case 0x50:	//Branch if overflow clear (V not set)
			data = memory->readRAM(PC, ppu);			//Needs unsigned information
			debugOffset = memory->readRAM(PC, ppu);
			debugRelative(opcode, debugOffset, data, "BVC");
			if(!V) {
				offset = memory->RAM[PC++];
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
				cycles =  3;
			}
			else	{
				PC++; 
				cycles =  2;
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x70:	//Branch if overflow set (V set)
			data = memory->readRAM(PC, ppu);			//Needs unsigned information
			debugOffset = memory->readRAM(PC, ppu);
			debugRelative(opcode, debugOffset, data, "BVS");
			if(V) {
				offset = memory->RAM[PC++];
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
				cycles =  3;
			}
			else	{
				PC++; 
				cycles =  2;
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x18:	//Clear carry flag
			C = false;
			debugImplied(opcode, "CLC");
			cycles =  2;
			break;
		case 0xD8:	//Clear decimal flag
			D = false;
			debugImplied(opcode, "CLD");
			cycles =  2;
			break;
		case 0x58:	//Clear interrupt flag
			I = false;
			debugImplied(opcode, "CLI");
			cycles =  2;
			break;
		case 0xB8:	//Clear overflow flag
			V = false;
			debugImplied(opcode, "CLV");
			cycles =  2;
			break;
		case 0xC9:	//Immediate A compare
			compareFlags(A, memory->readRAM(PC, ppu));
			debugImm(opcode, memory->readRAM(PC, ppu), "CMP");
			cycles =  2;
			PC++;
			break;
		case 0xC5:	//Zeropage A compare
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			compareFlags(A, data);
			debugZero(opcode, memory->readRAM(PC, ppu), data, "CMP");
			cycles =  3;
			PC++;
			break;
		case 0xD5:	//Zeropage,X A compare
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			compareFlags(A, data);
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "CMP");
			cycles =  4;
			PC++;
			break;
		case 0xCD:	//Absolute A compare
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			compareFlags(A, data);
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "CMP");
			cycles =  4;
			PC += 2;
			break;
		case 0xDD:	//Absolute,X A compare
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			compareFlags(A, data);
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "CMP");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0xD9:	//Absolute,Y A compare
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			compareFlags(A, data);
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "CMP");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0xC1:	//Indirect,X A compare
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			compareFlags(A, data);
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, data, "CMP");
			cycles =  6;
			PC++;
			break;
		case 0xD1:	//Indirect,Y A compare
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			compareFlags(A, data);
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, data, "CMP");
			pageBoundry((temp2 - Y), temp2, 5);
			PC++;
			break;
		case 0xE0:	//Immediate X compare
			data = memory->readRAM(PC, ppu);
			compareFlags(X, data);
			debugImm(opcode, memory->readRAM(PC, ppu), "CPX");
			cycles =  2;
			PC++;
			break;
		case 0xE4:	//Zeropage X compare
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			compareFlags(X, data);
			debugZero(opcode, memory->readRAM(PC, ppu), data, "CPX");
			cycles =  3;
			PC++;
			break;
		case 0xEC:	//Absolute X compare
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			compareFlags(X, data);
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "CPX");
			cycles =  4;
			PC += 2;
			break;
		case 0xC0:	//Immediate Y compare
			data = memory->readRAM(PC, ppu);
			compareFlags(Y, data);
			debugImm(opcode, memory->readRAM(PC, ppu), "CPY");
			cycles =  2;
			PC++;
			break;
		case 0xC4:	//Zeropage Y compare
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			compareFlags(Y, data);
			debugZero(opcode, memory->readRAM(PC, ppu), data, "CPY");
			cycles =  3;
			PC++;
			break;
		case 0xCC:	//Absolute Y compare
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			compareFlags(Y, data);
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "CPY");
			cycles =  4;
			PC += 2;
			break;
		case 0xC6:	//Zeropage decrement memory
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);		//Gets the data
			debugZero(opcode, memory->readRAM(PC, ppu), data, "DEC");
			memory->writeRAM(temp1, --data, ppu);	//Decrements before assigning
			Z = !(data);
			N = data & 0x80;
			cycles =  5;
			PC++;
			break;
		case 0xD6:	//Zeropage,X decrement memory
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);		//Gets the data
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "DEC");
			memory->writeRAM(temp1, --data, ppu);	//Decrements before assigning
			Z = !(data);
			N = data & 0x80;
			cycles =  6;
			PC++;
			break;
		case 0xCE:	//Absolute decrement memory
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);		//Gets the data
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "DEC");
			memory->writeRAM(temp1, --data, ppu);	//Decrements before assigning
			Z = !(data);
			N = data & 0x80;
			cycles =  6;
			PC += 2;
			break;
		case 0xDE:	//Absolute,X decrement memory
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);		//Gets the data
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "DEC");
			memory->writeRAM(temp1, --data, ppu);	//Decrements before assigning
			Z = !(data);
			N = data & 0x80;
			cycles =  7;
			PC += 2;
			break;
		case 0xCA:	//Implied decrement X register
			X--;
			Z = !(X);
			N = X & 0x80;
			debugImplied(opcode, "DEX");
			cycles =  2;
			break;
		case 0x88:	//Implied decrement Y register
			Y--;
			Z = !(Y);
			N = Y & 0x80;
			debugImplied(opcode, "DEY");
			cycles =  2;
			break;
		case 0x49:	//Immediate XOR
			A = A ^ memory->readRAM(PC, ppu);
			Z = !(A);
			N = A & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "EOR");
			cycles =  2;
			PC++;
			break;
		case 0x45:	//Zeropage XOR
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "EOR");
			cycles =  3;
			PC++;
			break;
		case 0x55:	//Zeropage,X XOR
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "EOR");
			cycles =  4;
			PC++;
			break;
		case 0x4D:	//Absolute XOR
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "EOR");
			cycles =  4;
			PC += 2;
			break;
		case 0x5D:	//Absolute,X XOR
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "EOR");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0x59:	//Absolute,Y XOR
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "EOR");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0x41:	//Indirect,X XOR
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, data, "EOR");
			cycles =  6;
			PC++;
			break;
		case 0x51:	//Indirect,Y XOR
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, data, "EOR");
			pageBoundry((temp2 - Y), temp2, 5);
			PC++;
			break;
		case 0xE6:	//Zeropage increment memory
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);		//Gets the data
			debugZero(opcode, memory->readRAM(PC, ppu), data, "INC");
			memory->writeRAM(temp1, ++data, ppu);	//Increments before assigning
			Z = !(data);
			N = data & 0x80;
			cycles =  5;
			PC++;
			break;
		case 0xF6:	//Zeropage,X increment memory
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);		//Gets the data
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "INC");
			memory->writeRAM(temp1, ++data, ppu);	//Increments before assigning
			Z = !(data);
			N = data & 0x80;
			cycles =  6;
			PC++;
			break;
		case 0xEE:	//Absolute increment memory
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);		//Gets the data
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "INC");
			memory->writeRAM(temp1, ++data, ppu);	//Increments before assigning
			Z = !(data);
			N = data & 0x80;
			cycles =  6;
			PC += 2;
			break;
		case 0xFE:	//Absolute,X increment memory
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);		//Gets the data
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "INC");
			memory->writeRAM(temp1, ++data, ppu);	//Increments before assigning
			Z = !(data);
			N = data & 0x80;
			cycles =  7;
			PC += 2;
			break;
		case 0xE8:	//Increment X register
			X++;
			Z = !(X);
			N = X & 0x80;
			debugImplied(opcode, "INX");
			cycles =  2;
			break;
		case 0xC8:	//Increment Y register
			Y++;
			Z = !(Y);
			N = Y & 0x80;
			debugImplied(opcode, "INY");
			cycles =  2;
			break;
		case 0x4C:	//Absolute, Sets PC to specified address
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = 0;		//Used for debugging
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "JMP");
			cycles =  3;
			PC = temp1;
			break;
		case 0x6C:	//Indirect, Sets PC to specified address
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			/* The 6502 has a JMP bug where the page wraps around if at the end.  For example, temp1 = 0x02FF will
			read temp2 = (memory->RAM[0x0200] << 8) | memory->RAM[0x02FF].  Address 0x0300 will not be read.*/
			if((temp1 & 0x00FF) == 0xFF)	temp2 = (memory->RAM[temp1 & 0xFF00] << 8) | memory->readRAM(temp1, ppu);
			else	temp2 = (memory->RAM[temp1 + 1] << 8) | memory->readRAM(temp1, ppu);
			debugIndirect(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp2, "JMP");
			cycles =  5;
			PC = temp2;
			break;
		case 0x20:	//Jump to subroutine
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = 0;				//This is for debugging.
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "JSR");
			PC++;					//Add 1 so it's at the address - 1 spot
			data = (PC & 0xFF00) >> 8;		//High byte first
			pushStack(memory,data, ppu);
			data = PC & 0x00FF;
			pushStack(memory, data, ppu);
			cycles =  6;
			PC = temp1;
			break;
		case 0xA9:	//Immediate load A
			A = memory->readRAM(PC, ppu);
			Z = !(A);
			N = A & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "LDA");
			cycles =  2;
			PC++;
			break;
		case 0xA5:	//Zeropage load A
			temp1 = memory->readRAM(PC, ppu);
			A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "LDA");
			cycles =  3;
			PC++;
			break;
		case 0xB5:	//Zeropage,X load A
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LDA");
			cycles =  4;
			PC++;
			break;
		case 0xAD:	//Absolute load A
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "LDA");
			cycles =  4;
			PC += 2;
			break;
		case 0xBD:	//Absolute,X load A
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LDA");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0xB9:	//Absolute,Y load A
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LDA");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0xA1:	//Indirect,X load A
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			A = memory->readRAM(temp2, ppu);
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "LDA");
			cycles =  6;
			PC++;
			break;
		case 0xB1:	//Indirect,Y load A
			temp1 = memory->readRAM(PC, ppu);		//Gets the zero page address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; 	//Gets the real address
			A = memory->readRAM(temp2, ppu);
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "LDA");
			pageBoundry((temp2 - Y), temp2, 5);
			PC++;
			break;
		case 0xA2:	//Immediate load X
			X = memory->readRAM(PC, ppu);
			Z = !(X);
			N = X & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "LDX");
			cycles =  2;
			PC++;
			break;
		case 0xA6:	//Zeropage load X
			temp1 = memory->readRAM(PC, ppu);
			X = memory->readRAM(temp1, ppu);
			Z = !(X);
			N = X & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "LDX");
			cycles =  3;
			PC++;
			break;
		case 0xB6:	//Zeropage,Y load X
			temp1 = (memory->readRAM(PC, ppu) + Y) & 0xFF;
			X = memory->readRAM(temp1, ppu);
			Z = !(X);
			N = X & 0x80;
			debugZeroY(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LDX");
			cycles =  4;
			PC++;
			break;
		case 0xAE:	//Absolute load X
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			X = memory->readRAM(temp1, ppu);
			Z = !(X);
			N = X & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "LDX");
			cycles =  4;
			PC += 2;
			break;
		case 0xBE:	//Absolute,Y load X
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			X = memory->readRAM(temp1, ppu);
			Z = !(X);
			N = X & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LDX");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0xA0:	//Immediate load Y
			Y = memory->readRAM(PC, ppu);
			Z = !(Y);
			N = Y & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "LDY");
			cycles =  2;
			PC++;
			break;
		case 0xA4:	//Zeropage load Y
			temp1 = memory->readRAM(PC, ppu);
			Y = memory->readRAM(temp1, ppu);
			Z = !(Y);
			N = Y & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "LDY");
			cycles =  3;
			PC++;
			break;
		case 0xB4:	//Zeropage,X load Y
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			Y = memory->readRAM(temp1, ppu);
			Z = !(Y);
			N = Y & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LDY");
			cycles =  4;
			PC++;
			break;
		case 0xAC:	//Absolute load Y
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			Y = memory->readRAM(temp1, ppu);
			Z = !(Y);
			N = Y & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "LDY");
			cycles =  4;
			PC += 2;
			break;
		case 0xBC:	//Absolute,X load Y
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			Y = memory->readRAM(temp1, ppu);
			Z = !(Y);
			N = Y & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LDY");
			pageBoundry((temp1 -X), temp1, 4);
			PC += 2;
			break;
		case 0x4A:	//Accumulator shift right
			C = A & 0x01;
			A = A >> 1;
			Z = !(A);
			N = A & 0x80;
			debugAcc(opcode, "LSR");
			cycles =  2;
			break;
		case 0x46:	//Zeropage shift right
			temp1 = memory->readRAM(PC, ppu);	//Gets the ZP address
			data = memory->readRAM(temp1, ppu);	//Gets data at the address
			C = data & 0x01;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "LSR");
			data >>= 1;
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles =  5;
			PC++;
			break;
		case 0x56:	//Zeropage,X shift right
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			data = memory->readRAM(temp1, ppu);	//Gets data at the address
			C = data & 0x01;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "LSR");
			data >>= 1;
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles =  6;
			PC++;
			break;
		case 0x4E:	//Absolute shift right
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);	//Gets data at the address
			C = data & 0x01;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "LSR");
			data >>= 1;
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles =  6;
			PC += 2;
			break;
		case 0x5E:	//Absolute,X shift right
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);	//Gets data at the address
			C = data & 0x01;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "LSR");
			data >>= 1;
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles =  7;
			PC += 2;
			break;
		case 0xEA:	//No instruction, only moves PC
			debugImplied(opcode, "NOP");
			cycles =  2;
			break;
		case 0x09:	//Immediate OR
			A = A | memory->readRAM(PC, ppu);
			Z = !(A);
			N = A & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "ORA");
			cycles =  2;
			PC++;
			break;
		case 0x05:	//Zeropage OR
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "ORA");
			cycles =  3;
			PC++;
			break;
		case 0x15:	//Zeropage,X OR
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			data = memory->readRAM(temp1, ppu);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "ORA");
			cycles =  4;
			PC++;
			break;
		case 0x0D:	//Absolute OR
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "ORA");
			cycles =  4;
			PC += 2;
			break;
		case 0x1D:	//Absolute,X OR
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "ORA");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0x19:	//Absolute,Y OR
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "ORA");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0x01:	//Indirect,X OR
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, data, "ORA");
			cycles =  6;
			PC++;
			break;
		case 0x11:	//Indirect,Y OR
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, data, "ORA");
			pageBoundry((temp2 - Y), temp2, 5);
			PC++;
			break;
		case 0x48:	//Push A onto stack
			pushStack(memory, A, ppu);
			debugImplied(opcode, "PHA");
			cycles =  3;
			break;
		case 0x08:	//Push copy of status flags onto stack
			//Bit-----7--6--5--4--3--2--1--0 PHP B and bit 5 true
			//Order = N, V, 1, B, D, I, Z, C
			statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
			statusFlags[4] = 1, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
			statusByte = encodeBits(statusFlags);	//Puts the status flags into a byte
			pushStack(memory, statusByte, ppu);			
			debugImplied(opcode, "PHP");
			cycles =  3;
			break;
		case 0x68:	//Pops value off stack into A
			A = popStack(memory, ppu);
			Z = !(A);
			N = A & 0x80;
			debugImplied(opcode, "PLA");
			cycles =  4;
			break;
		case 0x28:	//Pops process status flags off stack
			statusByte = popStack(memory, ppu);
			//Bit-----7--6--5--4--3--2--1--0 
			//Order = N, V, 1, B, D, I, Z, C
			decodeBits(outputFlags, statusByte);
			C = outputFlags[0];
			Z = outputFlags[1];
			I = outputFlags[2];
			D = outputFlags[3];
			B = 0;			//B is always zero after a PLP
			V = outputFlags[6];
			N = outputFlags[7];
			debugImplied(opcode, "PLP");
			cycles =  4;
			break;
		case 0x2A:	//Accumulator rotate left
			oldBit7 = A & 0x80; //Old bit 7
			debugAcc(opcode, "ROL");
			A = A << 1;
			A |= C;
			C = oldBit7;
			Z = !(A);
			N = A & 0x80;
			cycles =  2;
			break;
		case 0x26:	//Zeropage rotate left
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			oldBit7 = data & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "ROL");
			data <<= 1;
			data |= C;
			memory->writeRAM(temp1, data, ppu);
			C = oldBit7;
			N = data & 0x80;
			cycles =  5;
			PC++;
			break;
		case 0x36:	//Zeropage,X rotate left
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			oldBit7 = data & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "ROL");
			data <<= 1;
			data |= C;
			memory->writeRAM(temp1, data, ppu);
			C = oldBit7;
			N = data & 0x80;
			cycles =  6;
			PC++;
			break;
		case 0x2E:	//Absolute rotate left
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);	//Gets address
			data = memory->readRAM(temp1, ppu);					//Gets data
			oldBit7 = data & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "ROL");
			data <<= 1;
			data |= C;				//Sets bit 0 to carry flag
			memory->writeRAM(temp1, data, ppu);
			C = oldBit7;
			N = data & 0x80;
			cycles =  6;
			PC += 2;
			break;
		case 0x3E:	//Absolute,X rotate left
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			oldBit7 = data & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "ROL");
			data <<= 1;
			data |= C;
			memory->writeRAM(temp1, data, ppu);
			C = oldBit7;
			N = data & 0x80;
			cycles =  7;
			PC += 2;
			break;
		case 0x6A:	//Accumulator rotate right
			oldBit0 = A & 0x01; //Old bit 0
			debugAcc(opcode, "ROR");
			A = A >> 1;
			A |= (C << 7);
			C = oldBit0;
			Z = !(A);
			N = A & 0x80;
			cycles =  2;
			break;
		case 0x66:	//Zeropage rotate right
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			oldBit0 = data & 0x01;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "ROR");
			data >>= 1;
			data |= (C << 7);
			memory->writeRAM(temp1, data, ppu);
			C = oldBit0;
			N = data & 0x80;
			cycles =  5;
			PC++;
			break;
		case 0x76:	//Zeropage,X rotate right
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			oldBit0 = data & 0x01;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "ROR");
			data >>= 1;
			data &= ~0x80;
			data |= (C << 7);
			memory->writeRAM(temp1, data, ppu);
			C = oldBit0;
			N = data & 0x80;
			cycles = 6;
			PC++;
			break;
		case 0x6E:	//Absolute rotate right
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			oldBit0 = data & 0x01;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "ROR");
			data >>= 1;
			data |= (C << 7);
			memory->writeRAM(temp1, data, ppu);
			C = oldBit0;
			N = data & 0x80;
			cycles =  6;
			PC += 2;
			break;
		case 0x7E:	//Absolute,X rotate right
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			oldBit0 = data & 0x01;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "ROR");
			data >>= 1;
			data |= (C << 7);
			memory->writeRAM(temp1, data, ppu);
			C = oldBit0;
			N = data & 0x80;
			cycles =  7;
			PC += 2;
			break;
		case 0x40:	//Return from interrupt
			//Pull processor stat flags
			statusByte = popStack(memory, ppu);
			//Bit-----7--6--5--4--3--2--1--0 
			//Order = N, V, 1, B, D, I, Z, C
			decodeBits(outputFlags, statusByte);
			C = outputFlags[0];
			Z = outputFlags[1];
			I = outputFlags[2];
			D = outputFlags[3];
			B = outputFlags[4];
			V = outputFlags[6];
			N = outputFlags[7];
			data = popStack(memory, ppu);		//Gets the low byte
			PC = data;
			data = popStack(memory, ppu);		//Gets the high byte
			PC += (data << 8);
			debugImplied(opcode, "RTI");
			cycles =  6;
			break;
		case 0x60:	//Return from subroutine
			debugImplied(opcode, "RTS");
			data = popStack(memory, ppu);		//Gets the low byte
			PC = data;
			data = popStack(memory, ppu);		//Gets the high byte
			PC += (data << 8);
			cycles =  6;
			PC++;					//Add one once off stack for right address
			break;
		case 0xE9:	//Immidiate SBC
			data = memory->readRAM(PC, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp1 = A + data + C;
			C = (temp1 >> 8) & 1;
			A = temp1 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp1) & (data ^ temp1) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "SBC");
			cycles =  2;
			PC++;
			break; 
		case 0xE5:	//Zeropage SBC
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "SBC");
			cycles =  3;
			PC++;
			break;
		case 0xF5:	//Zeropage,X SBC
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "SBC");
			cycles =  4;
			PC++;
			break;
		case 0xED:	//Absolute SBC
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "SBC");
			cycles =  4;
			PC += 2;
			break;
		case 0xFD:	//Absolute,X SBC
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "SBC");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0xF9:	//Absolute,Y SBC
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "SBC");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0xE1:	//Indirect,X SBC
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "SBC");
			cycles =  6;
			PC++;
			break;
		case 0xF1:	//Indirect,Y SBC
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "SBC");
			pageBoundry((temp2 - Y), temp2, 5);
			PC++;
			break;
		case 0x38:	//Set carry flag
			C = true;
			debugImplied(opcode, "SEC");
			cycles =  2;
			break;
		case 0xF8:	//Set decimal flag
			D = true;
			debugImplied(opcode, "SED");
			cycles =  2;
			break;
		case 0x78:	//Set interrupt disable
			I = true;
			debugImplied(opcode, "SEI");
			cycles =  2;
			break;
		case 0x85:	//Zeropage store A to memory
			temp1 = memory->readRAM(PC, ppu);
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "STA");
			memory->writeRAM(temp1, A, ppu);
			cycles =  3;
			PC++;
			break;
		case 0x95:	//Zeropage,X store A to memory
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "STA");
			memory->writeRAM(temp1, A, ppu);
			cycles =  4;
			PC++;
			break;
		case 0x8D:	//Absolute store A to memory
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "STA");
			memory->writeRAM(temp1, A, ppu);
			cycles = 4;
			PC += 2;
			break;
		case 0x9D:	//Absolute,X store A to memory
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "STA");
			memory->writeRAM(temp1, A, ppu);
			cycles =  5;
			PC += 2;
			break;
		case 0x99:	//Absolute,Y store A to memory
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "STA");
			memory->writeRAM(temp1, A, ppu);
			cycles =  5;
			PC += 2;
			break;
		case 0x81:	//Indirect,X store A to memory
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "STA");
			memory->writeRAM(temp2, A, ppu);
			cycles =  6;
			PC++;
			break;
		case 0x91:	//Indirect,Y store A to memory
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "STA");
			memory->writeRAM(temp2, A, ppu);
			cycles =  6;
			PC++;
			break;
		case 0x86:	//Zeropage store X to memory
			temp1 = memory->readRAM(PC, ppu);
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "STX");
			memory->writeRAM(temp1, X, ppu);
			cycles =  3;
			PC++;
			break;
		case 0x96:	//Zeropage,Y store X to memory
			temp1 = (memory->readRAM(PC, ppu) + Y) & 0xFF;
			debugZeroY(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "STX");
			memory->writeRAM(temp1, X, ppu);
			cycles =  4;
			PC++;
			break;
		case 0x8E:	//Absolute store X to memory
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "STX");
			memory->writeRAM(temp1, X, ppu);
			cycles =  4;
			PC += 2;
			break;
		case 0x84:	//Zeropage store Y to memory
			temp1 = memory->readRAM(PC, ppu);
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "STY");
			memory->writeRAM(temp1, Y, ppu);
			cycles =  3;
			PC++;
			break;
		case 0x94:	//Zeropage,X store Y to memory
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "STY");
			memory->writeRAM(temp1, Y, ppu);
			cycles =  4;
			PC++;
			break;
		case 0x8C:	//Absolute store Y to memory
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "STY");
			memory->writeRAM(temp1, Y, ppu);
			cycles =  4;
			PC += 2;
			break;
		case 0xAA:	//Transfer A to X
			X = A;
			Z = !(X);
			N = X & 0x80;
			debugImplied(opcode, "TAX");
			cycles =  2;
			break;
		case 0xA8:	//Transfer A to Y
			Y = A;
			Z = !(Y);
			N = Y & 0x80;
			debugImplied(opcode, "TAY");
			cycles =  2;
			break;
		case 0xBA:	//Transfer SP to X
			X = SP - 0x0100;	//Puts the value within byte range
			Z = !(X);
			N = X & 0x80;
			debugImplied(opcode, "TSX");
			cycles =  2;
			break;
		case 0x8A:	//Transfer X to A
			A = X;
			Z = !(A);
			N = A & 0x80;
			debugImplied(opcode, "TXA");
			cycles =  2;
			break;
		case 0x9A:	//Transfers X into SP
			SP = X + 0x0100;	//Puts the value into the right range for the stack.
			debugImplied(opcode, "TXS");
			cycles =  2;
			break;
		case 0x98:	//Transfers Y to A
			A = Y;
			Z = !(A);
			N = A & 0x80;
			debugImplied(opcode, "TYA");
			cycles =  2;
			break;
		//Illegal opcodes-------------------------------------------------
		case 0x0B:	//Immediate And byte with A
			data = memory->readRAM(PC,ppu);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			C = N;
			debugImm(opcode, memory->readRAM(PC, ppu), "ANC");
			cycles = 2;
			PC++;
			break;
		case 0x2B:	//Immediate And byte with A
			data = memory->readRAM(PC,ppu);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			C = N;
			debugImm(opcode, memory->readRAM(PC, ppu), "ANC");
			cycles = 2;
			PC++;
			break;
		case 0x87:	//Zeropage AND X with A and store in memory
			data = A & X;
			temp1 = memory->readRAM(PC, ppu);
			memory->writeRAM(temp1, data, ppu);
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "SAX");
			Z = !(data);
			N = data & 0x80;
			cycles = 3;
			PC++;
			break;
		case 0x97:	//Zeropage,Y AND X with A and store in memory
			data = A & X;
			temp1 = (memory->readRAM(PC, ppu) + Y) & 0xFF;
			debugZeroY(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "SAX");
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles = 4;
			PC++;
			break;
		case 0x8F:	//Absolute AND X with A and story in memory
			data = A & X;
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "SAX");
			memory->writeRAM(temp1, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles = 4;
			PC += 2;
			break;
		case 0x83:	//Indirect,X AND X with A and story in memory
			data = A & X;
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "SAX");
			memory->writeRAM(temp2, data, ppu);
			Z = !(data);
			N = data & 0x80;
			cycles = 6;
			PC++;
			break;
		case 0x6B:	//Immediate AND byte with A and rotate
			A &= memory->readRAM(PC, ppu);
			A >>= 1;
			Z = !(A);
			N = A & 0x80;
			if((A & 0x40) && (A & 0x20)) {
				C = true;
				V = false;
			}
			else if(A & 0x40) {
				C = true;
				V = true;
			}
			else if(A & 0x20) {
				V = true;
				C = false;
			}
			else {
				V = false;
				C = false;
			}
			debugImm(opcode, memory->readRAM(PC, ppu), "ARR");
			cycles = 2;
			PC++;
			break;
		case 0x4B:	//Immediate and byte with A and shift right
 			A &= memory->readRAM(PC, ppu);
			A >>= 1;
			Z = !(A);
			N = A & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "ALR");
			cycles = 2;
			PC++;
			break;
		case 0xAB:	//Immediate and with A and A = X
			A &= memory->readRAM(PC, ppu);
			X = A;
			Z = !(A);
			N = A & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "OAL");
			cycles = 2;
			PC++;
			break;
		case 0x9F:	//Absolute,Y 
			data = (A & X) & 7;
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "AXA");
			memory->writeRAM(temp1, data, ppu);
			cycles = 5;
			PC += 2;
			break;
		case 0x93:	//Indirect,Y
			data = (A & X) & 7;
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "AXA");
			memory->writeRAM(temp2, data, ppu);
			cycles =  6;
			PC++;
			break;
		case 0xCB:	//Immediate DCP
			X &= A;
			data = memory->readRAM(PC, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp1 = X + data;
			C = (temp1 >> 8) & 1;
			X = temp1 & 0xFF;
			Z = !(X);
			N = X & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "SAX");
			cycles =  2;
			PC++;
			break;
		case 0xC7:	//Zeropage DCP
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			temp2 = data + (0x01 ^ 0xFF);
			C = (temp2 >> 8) & 1;
			memory->RAM[temp1] = temp2 & 0xFF;
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "DCP");
			cycles =  5;
			PC++;
			break;
		case 0xD7:	//Zeropage,X DCP
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			temp2 = data + (0x01 ^ 0xFF);
			C = (temp2 >> 8) & 1;
			memory->RAM[temp1] = temp2 & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DCP");
			cycles =  6;
			PC++;
			break;
		case 0xCF:	//Absolute DCP
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			temp2 = data + (0x01 ^ 0xFF);
			C = (temp2 >> 8) & 1;
			memory->RAM[temp1] = temp2 & 0xFF;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "DCP");
			cycles =  6;
			PC += 2;
			break;
		case 0xDF:	//Absolute,X DCP
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			temp2 = data + (0x01 ^ 0xFF);
			C = (temp2 >> 8) & 1;
			memory->RAM[temp1] = temp2 & 0xFF;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DCP");
			cycles = 7;
			PC += 2;
			break;
		case 0xDB:	//Absolute,Y DCP
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			temp2 = data + (0x01 ^ 0xFF);
			C = (temp2 >> 8) & 1;
			memory->RAM[temp1] = temp2 & 0xFF;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DCP");
			cycles = 7;
			PC += 2;
			break;
		case 0xC3:	//Indirect,X DCP
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			memory->RAM[temp2] = data + (0x01 ^ 0xFF);
			C = ((data + (0x01 ^ 0xFF)) >> 8) & 1;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "DCP");
			cycles = 8;
			PC++;
			break;
		case 0xD3:	//Indirect,Y SBC
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			memory->RAM[temp2] = data + (0x01 ^ 0xFF);
			C = ((data + (0x01 ^ 0xFF)) >> 8) & 1;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "DCP");
			cycles = 8;
			PC++;
			break;
		case 0x04:	//Zeropage DOP
			temp1 = memory->readRAM(PC, ppu);
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "DOP");
			cycles = 3;
			PC++;
			break;
		case 0x44:	//Zeropage DOP
			temp1 = memory->readRAM(PC, ppu);
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "DOP");
			cycles = 3;
			PC++;
			break;
		case 0x64:	//Zeropage DOP
			temp1 = memory->readRAM(PC, ppu);
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "DOP");
			cycles = 3;
			PC++;
			break;
		case 0x14:	//Zeropage,X DOP
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DOP");
			cycles =  4;
			PC++;
			break;
		case 0x34:	//Zeropage,X DOP
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DOP");
			cycles =  4;
			PC++;
			break;
		case 0x54:	//Zeropage,X DOP
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DOP");
			cycles =  4;
			PC++;
			break;
		case 0x74:	//Zeropage,X DOP
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DOP");
			cycles =  4;
			PC++;
			break;
		case 0xD4:	//Zeropage,X DOP
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DOP");
			cycles =  4;
			PC++;
			break;
		case 0xF4:	//Zeropage,X DOP
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "DOP");
			cycles =  4;
			PC++;
			break;
		case 0x80:	//Immediate DOP
			debugImm(opcode, memory->readRAM(PC, ppu), "DOP");
			cycles = 2;
			PC++;
			break;
		case 0x82:	//Immediate DOP
			debugImm(opcode, memory->readRAM(PC, ppu), "DOP");
			cycles = 2;
			PC++;
			break;
		case 0x89:	//Immediate DOP
			debugImm(opcode, memory->readRAM(PC, ppu), "DOP");
			cycles = 2;
			PC++;
			break;
		case 0xC2:	//Immediate DOP
			debugImm(opcode, memory->readRAM(PC, ppu), "DOP");
			cycles = 2;
			PC++;
			break;
		case 0xE2:	//Immediate DOP
			debugImm(opcode, memory->readRAM(PC, ppu), "DOP");
			cycles = 2;
			PC++;
			break;
		case 0xE7:	//Zeropage ISC
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = (data + 1) ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "ISC");
			cycles = 5;
			PC++;
			break;
		case 0xF7:	//Zeropage,X ISC
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = (data + 1) ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "ISC");
			cycles =  6;
			PC++;
			break;
		case 0xEF:	//Absolute ISC
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = (data + 1) ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "ISC");
			cycles =  6;
			PC += 2;
			break;
		case 0xFF:	//Absolute,X ISC
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = (data + 1) ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "ISC");
			cycles = 7;
			PC += 2;
			break;
		case 0xFB:	//Absolute,Y ISC
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = (data + 1) ^ 0xFF;
			temp2 = A + data + C;
			C = (temp2 >> 8) & 1;
			A = temp2 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "ISC");
			cycles = 7;
			PC += 2;
			break;
		case 0xE3:	//Indirect,X ISC
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = (data + 1) ^ 0xFF;
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "ISC");
			cycles =  8;
			PC++;
			break;
		case 0xF3:	//Indirect,Y SBC
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = (data + 1) ^ 0xFF;
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "ISC");
			cycles = 8;
			PC++;
			break;
		case 0xBB:	//Absolute,Y LAR
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			data &= SP;
			A = X = SP = data;
			Z = !(data);
			N = data & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LAR");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0xA7:	//Zeropage LAX
			temp1 = memory->readRAM(PC, ppu);
			X = A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "LAX");
			cycles = 3;
			PC++;
			break;
		case 0xB7:	//Zeropage,Y LAX
			temp1 = (memory->readRAM(PC, ppu) + Y) & 0xFF;
			X = A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugZeroY(opcode, memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LAX");
			cycles = 4;
			PC++;
			break;
		case 0xAF:	//Absolute LAX
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			X = A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), memory->readRAM(temp1, ppu), "LAX");
			cycles = 4;
			PC += 2;
			break;
		case 0xBF:	//Absolute,Y LAX
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			X = A = memory->readRAM(temp1, ppu);
			Z = !(A);
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, memory->readRAM(temp1, ppu), "LAX");
			pageBoundry((temp1 - Y), temp1, 4);
			PC += 2;
			break;
		case 0xA3:	//Indirect,X LAX
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			X = A = memory->readRAM(temp2, ppu);
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "LAX");
			cycles = 6;
			PC++;
			break;
		case 0xB3:	//Indirect,Y LAX
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			X = A = memory->readRAM(temp2, ppu);
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, memory->RAM[temp2], "LAX");
			pageBoundry((temp2 - Y), temp2, 5);
			PC++;
			break;
		case 0x1A:	//Implied NOP
			debugImplied(opcode, "NOP");
			cycles =  2;
			break;
		case 0x3A:	//Implied NOP
			debugImplied(opcode, "NOP");
			cycles =  2;
			break;
		case 0x5A:	//Implied NOP
			debugImplied(opcode, "NOP");
			cycles =  2;
			break;
		case 0x7A:	//Implied NOP
			debugImplied(opcode, "NOP");
			cycles =  2;
			break;
		case 0xDA:	//Implied NOP
			debugImplied(opcode, "NOP");
			cycles =  2;
			break;
		case 0xFA:	//Implied NOP
			debugImplied(opcode, "NOP");
			cycles =  2;
			break;
		case 0x67:	//Zeropage RRA
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			temp2 = A + (data >> 1) + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "RRA");
			cycles = 5;
			PC++;
			break; 
		case 0x77:	//Zeropage,X RRA
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			temp2 = A + (data >> 1) + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "RRA");
			cycles =  6;
			PC++;
			break;
		case 0x6F:	//Absolute RRA
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			temp2 = A + (data >> 1) + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "RRA");
			cycles = 6;
			PC += 2;
			break;
		case 0x7F:	//Absolute,X RRA
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			temp2 = A + (data >> 1) + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "RRA");
			cycles = 7;
			PC += 2;
			break;
		case 0x7B:	//Absolute,Y RRA
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			temp2 = A + (data >> 1) + C;
			A = temp2 & 0xFF;
			C = (temp2 >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp2) & (data ^ temp2) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "RRA");
			cycles = 7;
			PC += 2;
			break;
		case 0x63:	//Indirect,X RRA
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			data >>= 1;
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;	//Needs to use the initial value for the right bit.
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, data, "RRA");
			cycles = 8;
			PC++;
			break;
		case 0x73:	//Indirect,Y add with carry
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			data >>= 1;
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, data, "RRA");
			cycles = 8;
			PC++;
			break;
		case 0xEB:	//Immidiate SBC - same as legal opcode
			data = memory->readRAM(PC, ppu);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp1 = A + data + C;
			C = (temp1 >> 8) & 1;
			A = temp1 & 0xFF;
			Z = !(A);
			V = (initialA ^ temp1) & (data ^ temp1) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			debugImm(opcode, memory->readRAM(PC, ppu), "SBC");
			cycles =  2;
			PC++;
			break; 
		case 0x07:	//Zeropage SLO
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A |= (data << 1);
			Z = !(A);
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "SLO");
			cycles = 5;
			PC++;
			break; 
		case 0x17:	//Zeropage,X SLO
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			A |= (data << 1);
			Z = !(A);
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "SLO");
			cycles =  6;
			PC++;
			break;
		case 0x0F:	//Absolute SLO
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A |= (data << 1);
			Z = !(A);
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "SLO");
			cycles = 6;
			PC += 2;
			break;
		case 0x1F:	//Absolute,X SLO
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			A |= (data << 1);
			Z = !(A);
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "SLO");
			cycles = 7;
			PC += 2;
			break;
		case 0x1B:	//Absolute,Y SLO
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			A |= (data << 1);
			Z = !(A);
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "SLO");
			cycles = 7;
			PC += 2;
			break;
		case 0x03:	//Indirect,X SLO
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			A |= (data << 1);
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, data, "SLO");
			cycles = 8;
			PC++;
			break;
		case 0x13:	//Indirect,Y SLO
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			A |= (data << 1);
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, data, "SLO");
			cycles = 8;
			PC++;
			break;
		case 0x47:	//Zeropage SRE
			temp1 = memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A ^= (data >> 1);
			Z = !(A);
			N = A & 0x80;
			debugZero(opcode, memory->readRAM(PC, ppu), data, "SRE");
			cycles = 5;
			PC++;
			break; 
		case 0x57:	//Zeropage,X SRE
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF;
			data = memory->readRAM(temp1, ppu);
			A ^= (data >> 1);
			Z = !(A);
			N = A & 0x80;
			debugZeroX(opcode, memory->readRAM(PC, ppu), temp1, data, "SRE");
			cycles =  6;
			PC++;
			break;
		case 0x4F:	//Absolute SRE
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			data = memory->readRAM(temp1, ppu);
			A ^= (data >> 1);
			Z = !(A);
			N = A & 0x80;
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "SRE");
			cycles = 6;
			PC += 2;
			break;
		case 0x5F:	//Absolute,X SRE
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = memory->readRAM(temp1, ppu);
			A ^= (data >> 1);
			Z = !(A);
			N = A & 0x80;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "SRE");
			cycles = 7;
			PC += 2;
			break;
		case 0x5B:	//Absolute,Y SRE
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = memory->readRAM(temp1, ppu);
			A ^= (data >> 1);
			Z = !(A);
			N = A & 0x80;
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "SRE");
			cycles = 7;
			PC += 2;
			break;
		case 0x43:	//Indirect,X SRE
			temp1 = (memory->readRAM(PC, ppu) + X) & 0xFF; //Wraps around if >255
			temp2 = (memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu); //Gets address
			data = memory->readRAM(temp2, ppu);
			A ^= (data >> 1);
			Z = !(A);
			N = A & 0x80;
			debugIndirectX(opcode, memory->readRAM(PC, ppu), temp2, data, "SRE");
			cycles = 8;
			PC++;
			break;
		case 0x53:	//Indirect,Y SRE
			temp1 = memory->readRAM(PC, ppu);			//Gets Zeropage address
			temp2 = ((memory->readRAM((temp1 + 1) & 0xFF, ppu) << 8) | memory->readRAM(temp1, ppu)) + Y; //Gets real address
			data = memory->readRAM(temp2, ppu);
			A ^= (data >> 1);
			Z = !(A);
			N = A & 0x80;
			debugIndirectY(opcode, memory->readRAM(PC, ppu), temp2, data, "SRE");
			cycles = 8;
			PC++;
			break;
		case 0x9E:	//Absolute,Y SXA
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			data = (temp1 & 0xFF00) >> 8;
			data = X & (data + 1);
			memory->writeRAM(temp1, data, ppu);
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "SXA");
			cycles = 5;
			PC += 2;
			break;
		case 0x9C:	//Absolute,X SYA
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			data = (temp1 & 0xFF00) >> 8;
			data = Y & (data + 1);
			memory->writeRAM(temp1, data, ppu);
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "SYA");
			cycles = 5;
			PC += 2;
			break;
		case 0x0C:	//Absolute TOP
			temp1 = (memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu);
			debugAbs(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), data, "TOP");
			cycles = 4;
			PC += 2;
			break;
		case 0x1C:	//Absolute,X TOP
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "TOP");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0x3C:	//Absolute,X TOP
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "TOP");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0x5C:	//Absolute,X TOP
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "TOP");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0x7C:	//Absolute,X TOP
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "TOP");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0xDC:	//Absolute,X TOP
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "TOP");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0xFC:	//Absolute,X TOP
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + X;
			debugAbsX(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "TOP");
			pageBoundry((temp1 - X), temp1, 4);
			PC += 2;
			break;
		case 0x8B:	//Immediate XAA
			debugImm(opcode, memory->readRAM(PC, ppu), "XAA");
			cycles = 2;
			PC++;
			break;
		case 0x9B:	//Absolute,Y XAS
			temp1 = ((memory->readRAM(PC + 1, ppu) << 8) | memory->readRAM(PC, ppu)) + Y;
			SP = X & A;
			data = (temp1 & 0xFF00) >> 8;
			data = SP & (data + 1);
			memory->writeRAM(temp1, data, ppu);
			debugAbsY(opcode, memory->readRAM(PC + 1, ppu), memory->readRAM(PC, ppu), temp1, data, "XAS");
			cycles = 5;
			PC += 2;
			break;
		}
	}

	return cycles;
}


void cpu::setPCStart(memory* memory)
{
	PC = (memory->RAM[0xFFFD] << 8) | memory->RAM[0xFFFC];		//Starts at the reset vector
	//PC = 0xC000;
}



//Private
//This pushes a piece of data onto the stack and decrements SP
const void cpu::pushStack(memory* memory, unsigned char &data, ppu* ppu)
{
	//stackFile << instruction << " PUSHSTACK = " << hex << uppercase << (int)data << endl;
	memory->writeRAM(SP, data, ppu);
	SP--;				//Decrememnt after writing memory.
	if(SP < 0x0100) SP = 0x01FF;
}

//This pops a piece off data off the stack and increments SP
const unsigned char cpu::popStack(memory* memory,ppu* ppu)
{
	unsigned char temp = memory->readRAM(++SP, ppu);
	if(SP == 0x0200) SP = 0x0100;
	//stackFile << instruction << " POPSTACK = " << hex << uppercase << (int)temp << endl;
	return(temp);
}

//This sets the flags after a compare instruction
const void cpu::compareFlags(unsigned char& reg, unsigned char memoryValue)
{
	short temp = reg - memoryValue;
	if(!temp)
	{
		Z = true, C = true;
		N = temp & 0x80;
	}
	else if(temp < 0)
	{
		Z = false, C = false;
		N = temp & 0x80;
	}
	else
	{
		Z = false, C = true;
		N = temp & 0x80;
	}
}

//This encodes the status flags into a byte.
//Bit-----7--6--5--4--3--2--1--0 
//Order = N, V, 1, B, D, I, Z, C
const unsigned char cpu::encodeBits(bool bools[8])
{
	unsigned char retval = 0;
    	for(int i = 7; i > -1; i--) 
	{
        	retval += (bools[i] ? 1 : 0);
                retval = (i > 0 ? retval << 1 : retval);
    	}
 
        return retval;
}

//Decodes the statusByte back into their own flags
//Bit-----7--6--5--4--3--2--1--0 
//Order = N, V, 1, B, D, I, Z, C
const void cpu::decodeBits(bool (&bools)[8], unsigned char input)
{
	for(int i = 0; i < 8; i++) 
	{
        	bools[i] = (input & 0x00000001 ? true : false);
        	input = (i < 7 ? input >> 1 : input);	//? like an if statement
    	}
}

const void cpu::pageBoundry(unsigned short address1, unsigned short& address2, unsigned char numCycles)
{
	//If the high bytes are the same, then a page boundry has not been crossed.
	//If they are different, one has been crossed, so add a cycle.
	if( (address1 & 0xFF00) == (address2 & 0xFF00) )	cycles = numCycles;
	else	cycles = numCycles + 1;
}

const void cpu::pageBranch(char& offset)
{
	//If the high bytes of the address do not match, then a page has been crossed.  Add two cycles
	if( ((PC + offset) & 0xFF00) != (PC & 0xFF00) ) cycles =  2;
}

const void cpu::NMI(memory* memory, ppu* ppu)
{
	unsigned char data, statusByte;
	bool statusFlags[8];

	ppu->NMI = false;
	data = (PC & 0xFF00) >> 8;		//Push high byte first
	pushStack(memory, data, ppu);
	data = PC & 0x00FF;
	pushStack(memory, data, ppu);
	//Bit-----7--6--5--4--3--2--1--0 PHP always has B and bit 5 true
	//Order = N, V, 1, B, D, I, Z, C
	statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
	statusFlags[4] = 1, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
	statusByte = encodeBits(statusFlags);	//Puts the status flags into a
	pushStack(memory, statusByte, ppu);				//byte	
	PC = (memory->RAM[0xFFFB] << 8) | memory->RAM[0xFFFA];

	cycles =  7;	//7 cycles to process interrupt handler
}


//debugging stuff
//This outputs debug information for accumulator instructions
const void cpu::debugAcc(unsigned char &opcode, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//cout << "GOOD" << endl;
		//Left side of debug file
		debugFile << setw(4) << uppercase  << setfill('0') << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";				
		debugFile << setw(2) << uppercase << (int)opcode;
		debugFile << setw(11) << setfill(' ') << OPname;		//Name of the opcode
		debugFile << setw(2) << " A";
		debugFile << setw(27) << " ";

		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for immidiate instructions
const void cpu::debugImm(unsigned char &opcode, unsigned char data, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//cout << "GOOD" << endl;
		//Left side of debug file
		debugFile << setw(4) << uppercase << setfill('0') << (PC - 1); 
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase << setfill('0') << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << setfill('0') << (int)data;
		debugFile << setw(8) << setfill(' ') << OPname;				//Name of the opcode
		debugFile << setfill(' ') << setw(3) << "#$";
		debugFile << setw(2) << uppercase << setfill('0') << (int)data;
		debugFile << setfill(' ') << setw(24) << " ";

		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for zeropage instructions
const void cpu::debugZero(unsigned char &opcode, unsigned char dataPC, unsigned char dataAddress, 
			std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		//cout << "GOOD" << endl;
		debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase  << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase  << (int)dataPC;
		debugFile << setw(8) << setfill(' ') << OPname;				//Name of the opcode
		debugFile << setw(2) << "$" << setw(2) << uppercase << setfill('0') << (int)dataPC
			  << setw(3) << " = " << setw(2) << (int)dataAddress;
		debugFile << setw(20) << setfill(' ') << " ";
		
		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for zeropage + X instructions
const void cpu::debugZeroX(unsigned char &opcode, unsigned char dataPC,
				unsigned short totalAddress, unsigned char dataAddress, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		//cout << "GOOD" << endl;
		debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)dataPC;
		debugFile << setw(8) << setfill(' ') << OPname;			//Name of the opcode
		debugFile << setw(2) << " $" << uppercase  << setfill('0') <<  setw(2) << (int)dataPC 
			  << setw(5) << ",X @ " << setw(2) << (totalAddress & 0xFF) << setw(3) << " = "
			  << setw(2) << (int)dataAddress;
		debugFile << setw(13) << setfill(' ') << " ";
		
		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for zeropage + Y instructions
const void cpu::debugZeroY(unsigned char &opcode, unsigned char dataPC,
				unsigned short totalAddress, unsigned char dataAddress, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		//cout << "GOOD" << endl;
		debugFile << setw(4) << setfill('0') << uppercase  << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase  << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase  << (int)dataPC;
		debugFile << setw(8) << setfill(' ') << OPname;			//Name of the opcode
		debugFile << setw(2) << " $" << uppercase << setfill('0') << setw(2) << (int)dataPC 
			  << setw(5) << ",Y @ " << setw(2) << (totalAddress & 0xFF) << setw(3) << " = "
			  << setw(2) << (int)dataAddress;
		debugFile << setw(13) << setfill(' ') << " ";
		
		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for absolute instructions
const void cpu::debugAbs(unsigned char &opcode, unsigned char highAddress, unsigned char lowAddress, 
		    unsigned char data, std::string OPname)	
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		//cout << "GOOD" << endl;
		debugFile << setw(4) << setfill('0') << uppercase  << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase  << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase  << (int)lowAddress;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase  << (int)highAddress;
		debugFile << setw(5) << setfill(' ') << OPname;			//Name of the opcode
		if(OPname == "JMP" || OPname == "JSR")
		{
			debugFile << setw(2) << "$" << setw(2) << setfill('0') << uppercase  << (int)highAddress
			 << setw(2) << (int)lowAddress;
			debugFile << setw(23) << setfill(' ') << " ";
		}
		else
		{
			debugFile << setw(2) << "$" << setw(2) << setfill('0')  << uppercase  << (int)highAddress
			 << setw(2) <<(int)lowAddress << setw(3) << " = " << setw(2) << (int)data;
			debugFile << setw(18) << setfill(' ') << " ";
		}
		
		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for absolute,X instructions
const void cpu::debugAbsX(unsigned char &opcode, unsigned char highAddress, unsigned char lowAddress, 
			unsigned short totalAddress, unsigned char data, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		//cout << "GOOD" << endl;
		debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)lowAddress;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)highAddress;
		debugFile << setw(5) << setfill(' ') << OPname;			//Name of the opcode
		debugFile << setw(2) << " $" << uppercase << setfill('0') << setw(2) << (int)highAddress 
			  << setw(2) << (int)lowAddress << setw(5) << ",X @ " << setw(4) << totalAddress 
			  << setw(3) << " = " << setw(2) << (int)data;
		debugFile << setw(9) << setfill(' ') << " ";
		
		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for absolute, Y instructions
const void cpu::debugAbsY(unsigned char &opcode, unsigned char highAddress, unsigned char lowAddress, 
			unsigned short totalAddress, unsigned char data, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		//cout << "GOOD" << endl;
		debugFile << setw(4) << setfill('0') << uppercase << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)lowAddress;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)highAddress;
		debugFile << setw(5) << setfill(' ') << OPname;			//Name of the opcode
		debugFile << setw(2) << " $" << uppercase << setfill('0') << setw(2) << (int)highAddress 
			  << setw(2) << (int)lowAddress << setw(5) << ",Y @ " << setw(4) << totalAddress 
			  << setw(3) << " = " << setw(2) << (int)data;
		debugFile << setw(9) << setfill(' ') << " ";
		
		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for indirect,X instructions
const void cpu::debugIndirectX(unsigned char &opcode, unsigned char pcAddress,unsigned short indirectAddress, 			unsigned char data, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		//cout << "GOOD" << endl;
		debugFile << setw(4) << setfill('0') << uppercase << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)pcAddress;	//Gives the indirect address
		debugFile << setw(8) << setfill(' ') << OPname;				//Name of the opcode
		debugFile << setw(3) << " ($" << uppercase  << setw(2) << setfill('0') << (int)pcAddress;
		debugFile << setw(6) << ",X) @ " << setw(2) << (((int)pcAddress + initialX) & 0xFF) 
			  << setw(3) << " = "; 
		debugFile << setw(4) << indirectAddress << setw(3) << " = " << setw(2) << (int)data;
		debugFile << setw(4) << setfill(' ') << " ";

		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for indirect,Y instructions
const void cpu::debugIndirectY(unsigned char &opcode, unsigned char pcAddress,
			  unsigned short indirectAddress, unsigned char data, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		debugFile << setw(4) << setfill('0') << uppercase << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)pcAddress;	//Gives the indirect address
		debugFile << setw(8) << setfill(' ') << OPname;				//Name of the opcode
		debugFile << setw(3) << " ($" << uppercase  << setw(2) << setfill('0') << (int)pcAddress;
		debugFile << setw(6) << "),Y = " << setw(4) << ((indirectAddress - Y) & 0xFFFF)  << setw(3) << " @ "; 
		debugFile << setw(4) << indirectAddress << setw(3) << " = " << setw(2) << (int)data;
		debugFile << setw(2) << setfill(' ') << " ";

		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for relative instructions
const void cpu::debugRelative(unsigned char &opcode, char offset, unsigned char dataPC, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//cout << "GOOD" << endl;
		//Left side of debug file
		//If branch doesn't succeed, then offset = 0
		debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)dataPC;
		debugFile << setw(8) << setfill(' ') << OPname;			//Name of the opcode
		debugFile << setw(2) << "$" << uppercase  << setw(4) << setfill('0') << (PC + offset + 1);
		debugFile << setw(23) << setfill(' ') << " ";

		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug information for implied instructions
const void cpu::debugImplied(unsigned char &opcode, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Left side of debug file
		debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase  << (int)opcode;
		debugFile << setw(11) << setfill(' ') << OPname;		//Name of the opcode
		debugFile << setw(29) << " ";

		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs debug informatiuon for implicit instructions
const void cpu::debugIndirect(unsigned char &opcode, unsigned char highAddress, unsigned char lowAddress,
				unsigned short jumpAddress, std::string OPname)
{
	using namespace std;

	if(debugFile.is_open())
	{
		//Jump address is the data at the jump address.
		//Left side of debug file
		debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		debugFile << setw(2) << "  ";
		debugFile << setw(2) << uppercase << (int)opcode;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)lowAddress;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << uppercase << (int)highAddress;
		debugFile << setw(5) << setfill(' ') << OPname;			//Name of the opcode
		debugFile << setw(3) << " ($" << uppercase << setfill('0') << setw(2) << (int)highAddress 
			  << setw(2) << (int)lowAddress << setw(4) << ") = " << setw(4) << jumpAddress;
		debugFile << setw(14) << setfill(' ') << " ";
		
		//Right side of debug file
		debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		debugFile << setw(1) << " ";
		debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		debugFile << setw(1) << " ";
		debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		debugFile << setw(1) << " ";
		debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << initialCycles << hex;
		debugFile << endl;
	}
	else cout << "Unable to open file";
}
