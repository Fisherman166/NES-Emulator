#include "cpu.h"


cpu::cpu(): SP(0x01FD), A(0), X(0), Y(0), C(false), Z(false), I(true), D(false),
		V(false), N(false), RAM(NULL), video(NULL), cycles(0)
{
	//debugFile.open("//debug.txt");
	//debugFile.setf(ios::hex, ios::basefield);
}

cpu::~cpu()
{
	//debugFile.close();
}

void cpu::setPCStart()
{
	PC = (RAM->readRAM(0xFFFD) << 8) | RAM->readRAM(0xFFFC);		//Starts at the reset vector
}

void cpu::printDebug()
{
	using namespace std;
	bool statusFlags[8];			//Used for putting status flags into a byte

	//Create flag byte
	statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
	statusFlags[4] = 0, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
	unsigned char procFlags = encodeBits(statusFlags);

	cout << hex << setfill('0') << "A: " << setw(2) << (int)A;
	cout << hex << setfill('0') << "X: " << setw(2) << (int)X;
	cout << hex << setfill('0') << "Y: " << setw(2) << (int)Y;
	cout << hex << setfill('0') << "P: " << setw(2) << (int)procFlags;
	cout << hex << setfill('0') << "SP: " << setw(4) << (int)SP;
	cout << hex << setfill('0') << "PC: " << setw(4) << (int)PC << endl;
}


//This sets the RAM and video pointers to the objects in main
bool cpu::setPointers(memory* memory, ppu* ppu)
{
	bool retval = true;
	
	RAM = memory;
	video = ppu;

	if(RAM == NULL || video == NULL) retval = false;

	return retval;
}

cpu::byte cpu::emulateCycle()
{
	if(!video->NMI)
	{
		word temp; 				//Used when reading from other addresses
		byte data;				//Holds the data to write to memory
		char offset = 0;			//Used with branches
		byte statusByte;			//Used for putting status flags into a byte
		bool statusFlags[8];			//Used for putting status flags into a byte
		bool outputFlags[8] = {false, false, false, false, false, false, false, false}; //For getting flags off stack
		bool oldBit7, oldBit0;			//Used for rotation instructions

	
		//InitialA and video->dotNumber  are used in the switch.  Rest are for //debugging
		initialA = A;
		/*initialX = X;
		initialY = Y;
		initialSP = SP;
		//initialCycles = (initialCycles + cycles * 3) % 341;
		char //debugOffset;				//Used for //debugging branches
		statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
		statusFlags[4] = 0, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
		initialP = encodeBits(statusFlags);*/

		opcode = RAM->readRAM(PC);			//Fetching opcode
		PC++;						//Increments after fetch

		switch(opcode)
		{
		case 0x69:	//Immidiate add with carry
			data = RAM->readRAM(PC);
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			//debugImm(opcode, data, "ADC");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break; 
		case 0x65:	//Zeropage add with carry
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			//debugZero(opcode, RAM->readRAM(PC), data, "ADC");
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break; 
		case 0x75:	//Zeropage,X add with carry
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "ADC");
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x6D:	//Absolute add with carry
			temp = absolute();
			data = RAM->readRAM(temp);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "ADC");
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x7D:	//Absolute,X add with carry
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "ADC");
			pageBoundry((temp - X), temp);
			temp= A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			PC += instr_lens[opcode] - 1;
			break;
		case 0x79:	//Absolute,Y add with carry
			temp = absoluteY();
			data = RAM->readRAM(temp);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "ADC");
			pageBoundry((temp - Y), temp);
			temp= A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			PC += instr_lens[opcode] - 1;
			break;
		case 0x61:	//Indirect,X add with carry
			temp= indirectX();
			data = RAM->readRAM(temp);
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;	//Needs to use the initial value for the right bit.
			Z = !(A);
			N = A & 0x80;
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "ADC");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x71:	//Indirect,Y add with carry
			temp = indirectY();
			data = RAM->readRAM(temp);
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "ADC");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x29:	//Immediate AND with A & memory
			A = A & RAM->readRAM(PC);
			Z = !(A);
			N = A & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "AND");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x25:	//Zeropage AND with A & memory
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), data, "AND");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x35:	//Zeropage,X AND with A & memory
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data,"AND");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x2D:	//Absolute AND
			temp = absolute();
			data = RAM->readRAM(temp);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "AND");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x3D:	//Absolute,X AND
			temp = absoluteX();
			data = RAM->readRAM(temp);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "AND");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x39:	//Absolute,Y AND
			temp = absoluteY();
			data = RAM->readRAM(temp);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "AND");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x21:	//Indirect,X AND
			temp= indirectX();
			data = RAM->readRAM(temp);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "AND");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x31:	//Indirect,Y AND
			temp = indirectY();
			data = RAM->readRAM(temp);
			A = A & data;
			Z = !(A);
			N = A & 0x80;
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "AND");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x0A:	//Accumulator shift left
			C = A & 0x80;
			A = A << 1;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			//debugAcc(opcode, "ASL");
			break;
		case 0x06:	//Zeropage shift left
			temp = RAM->readRAM(PC);	//Gets the ZP address
			data = RAM->readRAM(temp);	//Gets data at the address
			C = data & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), data, "ASL");
			data <<= 1;				//Shift left
			RAM->writeRAM(temp, data);
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x16:	//Zeropage,X shift left
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			C = data & 0x80;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "ASL");
			data <<= 1;				//Shift left
			RAM->writeRAM(temp, data);
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x0E:	//Absolute shift left
			temp = absolute();
			data = RAM->readRAM(temp);
			C = data & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "ASL");
			data <<= 1;				//Shift left
			RAM->writeRAM(temp, data);
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x1E:	//Absolute,X shift left
			temp = absoluteX();
			data = RAM->readRAM(temp);
			C = data & 0x80;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "ASL");
			data <<= 1;				//Shift left
			RAM->writeRAM(temp, data);
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x90:	//Branch if carry clear
			data = RAM->readRAM(PC);			//Needs unsigned information
			//debugOffset = RAM->readRAM(PC);
			//debugRelative(opcode, //debugOffset, data, "BCC");
			if(!C) {
				offset = RAM->readRAM(PC);
				PC += instr_lens[opcode] - 1;
				cycles = cycleCount[opcode] + 1;
				pageBranch(offset);
			}
			else	{
				PC += instr_lens[opcode] - 1; 
				cycles = cycleCount[opcode];
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		 case 0xB0:	//Branch if carry set
			data = RAM->readRAM(PC);			//Needs unsigned information
			//debugOffset = RAM->readRAM(PC);
			//debugRelative(opcode, //debugOffset, data, "BCS");
			if(C) {
				offset = RAM->readRAM(PC);
				PC += instr_lens[opcode] - 1;
				cycles = cycleCount[opcode] + 1;
				pageBranch(offset);
			}
			else	{
				PC += instr_lens[opcode] - 1; 
				cycles = cycleCount[opcode];
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0xF0:	//Branch if equal (zero flag set)
			data = RAM->readRAM(PC);			//Needs unsigned information
			//debugOffset = RAM->readRAM(PC);
			//debugRelative(opcode, //debugOffset, data, "BEQ");
			if(Z) {
				offset = RAM->readRAM(PC);
				PC += instr_lens[opcode] - 1;
				cycles = cycleCount[opcode] + 1;
				pageBranch(offset);
			}
			else	{
				PC += instr_lens[opcode] - 1; 
				cycles = cycleCount[opcode];
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x24:	//Zeropage bit test
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			Z = !(A & data);
			V = data & 0x40;	//Gets the 6th bit only
			N = data & 0x80;	//Gets the 7th bit only
			//debugZero(opcode, RAM->readRAM(PC), data, "BIT");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x2C:	//Absolute bit test
			temp = absolute();
			data = RAM->readRAM(temp);
			Z = !(A & data);
			V = data & 0x40;	//Gets the 6th bit only
			N = data & 0x80;	//Gets the 7th bit only	
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "BIT"); 
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x30:	//Branch if minus (N set)
			data = RAM->readRAM(PC);			//Needs unsigned information
			//debugOffset = RAM->readRAM(PC);
			//debugRelative(opcode, //debugOffset, data, "BMI");
			if(N) {
				offset = RAM->readRAM(PC);
				PC += instr_lens[opcode] - 1;
				cycles = cycleCount[opcode] + 1;
				pageBranch(offset);
			}
			else	{
				PC += instr_lens[opcode] - 1; 
				cycles = cycleCount[opcode];
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0xD0:	//Branch if not equal (Z not set)
			data = RAM->readRAM(PC);			//Needs unsigned information
			//debugOffset = RAM->readRAM(PC);
			//debugRelative(opcode, //debugOffset, data, "BNE");
			if(!Z) {
				offset = RAM->readRAM(PC);
				PC += instr_lens[opcode] - 1;
				cycles = cycleCount[opcode] + 1;
				pageBranch(offset);
			}
			else	{
				PC += instr_lens[opcode] - 1; 
				cycles = cycleCount[opcode];
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x10:	//Branch if positive (N not set)
			data = RAM->readRAM(PC);			//Needs unsigned information
			//debugOffset = RAM->readRAM(PC);
			//debugRelative(opcode, //debugOffset, data, "BPL");
			if(!N) {
				offset = RAM->readRAM(PC);
				PC += instr_lens[opcode] - 1;
				cycles = cycleCount[opcode] + 1;
				pageBranch(offset);			//Will only add the extra 2 cycles if page crossed
			}
			else	{
				PC += instr_lens[opcode] - 1; 
				cycles = cycleCount[opcode];
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x00:	//BRK
			//debugImplied(opcode, RAM->readRAM(PC), "BRK");
			PC++;					//BRK pushes PC + 2
			data = ((PC & 0xFF00) >> 8);		//Push high byte first
			pushStack(data);
			data = PC & 0xFF;
			pushStack(data);	
			//Bit-----7--6--5--4--3--2--1--0 PHP always has B and bit 5 true
			//Order = N, V, 1, B, D, I, Z, C
			statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
			statusFlags[4] = 1, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
			statusByte = encodeBits(statusFlags);	//Puts the status flags into a
			pushStack(statusByte);				//byte	
			PC = (RAM->readRAM(0xFFFF) << 8);
			PC |= RAM->readRAM(0xFFFE);
			I = true;
			cycles = cycleCount[opcode];
			break;
		case 0x50:	//Branch if overflow clear (V not set)
			data = RAM->readRAM(PC);			//Needs unsigned information
			//debugOffset = RAM->readRAM(PC);
			//debugRelative(opcode, //debugOffset, data, "BVC");
			if(!V) {
				offset = RAM->readRAM(PC);
				PC += instr_lens[opcode] - 1;
				cycles = cycleCount[opcode] + 1;
				pageBranch(offset);
			}
			else	{
				PC += instr_lens[opcode] - 1; 
				cycles = cycleCount[opcode];
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x70:	//Branch if overflow set (V set)
			data = RAM->readRAM(PC);			//Needs unsigned information
			//debugOffset = RAM->readRAM(PC);
			//debugRelative(opcode, //debugOffset, data, "BVS");
			if(V) {
				offset = RAM->readRAM(PC);
				PC += instr_lens[opcode] - 1;
				cycles = cycleCount[opcode] + 1;
				pageBranch(offset);
			}
			else	{
				PC += instr_lens[opcode] - 1; 
				cycles = cycleCount[opcode];
			}
			PC += offset;	//Moves the PC around if branch suceeds
			break;
		case 0x18:	//Clear carry flag
			C = false;
			//debugImplied(opcode, RAM->readRAM(PC), "CLC");
			cycles = cycleCount[opcode];
			break;
		case 0xD8:	//Clear decimal flag
			D = false;
			//debugImplied(opcode, RAM->readRAM(PC), "CLD");
			cycles = cycleCount[opcode];
			break;
		case 0x58:	//Clear interrupt flag
			I = false;
			//debugImplied(opcode, RAM->readRAM(PC), "CLI");
			cycles = cycleCount[opcode];
			break;
		case 0xB8:	//Clear overflow flag
			V = false;
			//debugImplied(opcode, RAM->readRAM(PC), "CLV");
			cycles = cycleCount[opcode];
			break;
		case 0xC9:	//Immediate A compare
			compareFlags(A, RAM->readRAM(PC));
			//debugImm(opcode, RAM->readRAM(PC), "CMP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xC5:	//Zeropage A compare
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			compareFlags(A, data);
			//debugZero(opcode, RAM->readRAM(PC), data, "CMP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xD5:	//Zeropage,X A compare
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			compareFlags(A, data);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "CMP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xCD:	//Absolute A compare
			temp = absolute();
			data = RAM->readRAM(temp);
			compareFlags(A, data);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "CMP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xDD:	//Absolute,X A compare
			temp = absoluteX();
			data = RAM->readRAM(temp);
			compareFlags(A, data);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "CMP");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xD9:	//Absolute,Y A compare
			temp = absoluteY();
			data = RAM->readRAM(temp);
			compareFlags(A, data);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "CMP");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xC1:	//Indirect,X A compare
			temp= indirectX();
			data = RAM->readRAM(temp);
			compareFlags(A, data);
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "CMP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xD1:	//Indirect,Y A compare
			temp = indirectY();
			data = RAM->readRAM(temp);
			compareFlags(A, data);
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "CMP");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xE0:	//Immediate X compare
			data = RAM->readRAM(PC);
			compareFlags(X, data);
			//debugImm(opcode, RAM->readRAM(PC), "CPX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xE4:	//Zeropage X compare
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			compareFlags(X, data);
			//debugZero(opcode, RAM->readRAM(PC), data, "CPX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xEC:	//Absolute X compare
			temp = absolute();
			data = RAM->readRAM(temp);
			compareFlags(X, data);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "CPX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xC0:	//Immediate Y compare
			data = RAM->readRAM(PC);
			compareFlags(Y, data);
			//debugImm(opcode, RAM->readRAM(PC), "CPY");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xC4:	//Zeropage Y compare
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			compareFlags(Y, data);
			//debugZero(opcode, RAM->readRAM(PC), data, "CPY");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xCC:	//Absolute Y compare
			temp = absolute();
			data = RAM->readRAM(temp);
			compareFlags(Y, data);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "CPY");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xC6:	//Zeropage decrement memory
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);		//Gets the data
			//debugZero(opcode, RAM->readRAM(PC), data, "DEC");
			data--;
			RAM->writeRAM(temp, data);	//Decrements before assigning
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xD6:	//Zeropage,X decrement memory
			temp = zeroPageX();
			data = RAM->readRAM(temp);		//Gets the data
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "DEC");
			data--;
			RAM->writeRAM(temp, data);	//Decrements before assigning
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xCE:	//Absolute decrement memory
			temp = absolute();
			data = RAM->readRAM(temp);		//Gets the data
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "DEC");
			data--;
			RAM->writeRAM(temp, data);	//Decrements before assigning
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xDE:	//Absolute,X decrement memory
			temp = absoluteX();
			data = RAM->readRAM(temp);		//Gets the data
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "DEC");
			data--;
			RAM->writeRAM(temp, data);	//Decrements before assigning
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xCA:	//Implied decrement X register
			X--;
			Z = !(X);
			N = X & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "DEX");
			cycles = cycleCount[opcode];
			break;
		case 0x88:	//Implied decrement Y register
			Y--;
			Z = !(Y);
			N = Y & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "DEY");
			cycles = cycleCount[opcode];
			break;
		case 0x49:	//Immediate XOR
			A = A ^ RAM->readRAM(PC);
			Z = !(A);
			N = A & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "EOR");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x45:	//Zeropage XOR
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), data, "EOR");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x55:	//Zeropage,X XOR
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "EOR");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x4D:	//Absolute XOR
			temp = absolute();
			data = RAM->readRAM(temp);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "EOR");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x5D:	//Absolute,X XOR
			temp = absoluteX();
			data = RAM->readRAM(temp);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "EOR");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x59:	//Absolute,Y XOR
			temp = absoluteY();
			data = RAM->readRAM(temp);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "EOR");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x41:	//Indirect,X XOR
			temp= indirectX();
			data = RAM->readRAM(temp);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "EOR");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x51:	//Indirect,Y XOR
			temp = indirectY();
			data = RAM->readRAM(temp);
			A = A ^ data;
			Z = !(A);
			N = A & 0x80;
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "EOR");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xE6:	//Zeropage increment memory
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);		//Gets the data
			//debugZero(opcode, RAM->readRAM(PC), data, "INC");
			data++;
			RAM->writeRAM(temp, data);	//Increments before assigning
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xF6:	//Zeropage,X increment memory
			temp = zeroPageX();
			data = RAM->readRAM(temp);		//Gets the data
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "INC");
			data++;
			RAM->writeRAM(temp, data);	//Increments before assigning
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xEE:	//Absolute increment memory
			temp = absolute();
			data = RAM->readRAM(temp);		//Gets the data
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "INC");
			data++;
			RAM->writeRAM(temp, data);	//Increments before assigning
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xFE:	//Absolute,X increment memory
			temp = absoluteX();
			data = RAM->readRAM(temp);		//Gets the data
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "INC");
			data++;
			RAM->writeRAM(temp, data);	//Increments before assigning
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xE8:	//Increment X register
			X++;
			Z = !(X);
			N = X & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "INX");
			cycles = cycleCount[opcode];
			break;
		case 0xC8:	//Increment Y register
			Y++;
			Z = !(Y);
			N = Y & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "INY");
			cycles = cycleCount[opcode];
			break;
		case 0x4C:	//Absolute, Sets PC to specified address
			temp = absolute();
			data = 0;		//Used for //debugging
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "JMP");
			cycles = cycleCount[opcode];
			PC = temp;
			break;
		case 0x6C:	//Indirect, Sets PC to specified address
			temp = absolute();
			/* The 6502 has a JMP bug where the page wraps around if at the end.  For example, temp = 0x02FF will
			read temp= (RAM->RAM[0x0200] << 8) | RAM->RAM[0x02FF].  Address 0x0300 will not be read.*/
			if((temp & 0x00FF) == 0xFF)	temp= (RAM->readRAM(temp & 0xFF00) << 8) | RAM->readRAM(temp);
			else	temp= (RAM->readRAM(temp + 1) << 8) | RAM->readRAM(temp);
			//debugIndirect(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, "JMP");
			cycles = cycleCount[opcode];
			PC = temp;
			break;
		case 0x20:	//Jump to subroutine
			temp = absolute();
			data = 0;				//This is for //debugging.
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "JSR");
			PC++;					//Add 1 so it's at the address - 1 spot
			data = (PC & 0xFF00) >> 8;		//High byte first
			pushStack(data);
			data = PC & 0x00FF;
			pushStack(data);
			cycles = cycleCount[opcode];
			PC = temp;
			break;
		case 0xA9:	//Immediate load A
			A = RAM->readRAM(PC);
			Z = !(A);
			N = A & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "LDA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xA5:	//Zeropage load A
			temp = RAM->readRAM(PC);
			A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "LDA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xB5:	//Zeropage,X load A
			temp = zeroPageX();
			A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xAD:	//Absolute load A
			temp = absolute();
			A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "LDA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xBD:	//Absolute,X load A
			temp = absoluteX();
			A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDA");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xB9:	//Absolute,Y load A
			temp = absoluteY();
			A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDA");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xA1:	//Indirect,X load A
			temp = indirectX();
			A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xB1:	//Indirect,Y load A
			temp = indirectY();
			A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDA");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xA2:	//Immediate load X
			X = RAM->readRAM(PC);
			Z = !(X);
			N = X & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "LDX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xA6:	//Zeropage load X
			temp = RAM->readRAM(PC);
			X = RAM->readRAM(temp);
			Z = !(X);
			N = X & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "LDX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xB6:	//Zeropage,Y load X
			temp = (RAM->readRAM(PC) + Y) & 0xFF; 
			X = RAM->readRAM(temp);
			Z = !(X);
			N = X & 0x80;
			//debugZeroY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xAE:	//Absolute load X
			temp = absolute();
			X = RAM->readRAM(temp);
			Z = !(X);
			N = X & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "LDX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xBE:	//Absolute,Y load X
			temp = absoluteY();
			X = RAM->readRAM(temp);
			Z = !(X);
			N = X & 0x80;
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDX");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xA0:	//Immediate load Y
			Y = RAM->readRAM(PC);
			Z = !(Y);
			N = Y & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "LDY");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xA4:	//Zeropage load Y
			temp = RAM->readRAM(PC);
			Y = RAM->readRAM(temp);
			Z = !(Y);
			N = Y & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "LDY");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xB4:	//Zeropage,X load Y
			temp = zeroPageX();
			Y = RAM->readRAM(temp);
			Z = !(Y);
			N = Y & 0x80;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDY");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xAC:	//Absolute load Y
			temp = absolute();
			Y = RAM->readRAM(temp);
			Z = !(Y);
			N = Y & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "LDY");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xBC:	//Absolute,X load Y
			temp = absoluteX();
			Y = RAM->readRAM(temp);
			Z = !(Y);
			N = Y & 0x80;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "LDY");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x4A:	//Accumulator shift right
			C = A & 0x01;
			A = A >> 1;
			Z = !(A);
			N = A & 0x80;
			//debugAcc(opcode, "LSR");
			cycles = cycleCount[opcode];
			break;
		case 0x46:	//Zeropage shift right
			temp = RAM->readRAM(PC);	//Gets the ZP address
			data = RAM->readRAM(temp);	//Gets data at the address
			C = data & 0x01;
			//debugZero(opcode, RAM->readRAM(PC), data, "LSR");
			data >>= 1;
			RAM->writeRAM(temp, data);
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x56:	//Zeropage,X shift right
			temp = zeroPageX();
			data = RAM->readRAM(temp);	//Gets data at the address
			C = data & 0x01;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "LSR");
			data >>= 1;
			RAM->writeRAM(temp, data);
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x4E:	//Absolute shift right
			temp = absolute();
			data = RAM->readRAM(temp);	//Gets data at the address
			C = data & 0x01;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "LSR");
			data >>= 1;
			RAM->writeRAM(temp, data);
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x5E:	//Absolute,X shift right
			temp = absoluteX();
			data = RAM->readRAM(temp);	//Gets data at the address
			C = data & 0x01;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "LSR");
			data >>= 1;
			RAM->writeRAM(temp, data);
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xEA:	//No instruction, only moves PC
			//debugImplied(opcode, RAM->readRAM(PC), "NOP");
			cycles = cycleCount[opcode];
			break;
		case 0x09:	//Immediate OR
			A = A | RAM->readRAM(PC);
			Z = !(A);
			N = A & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "ORA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x05:	//Zeropage OR
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), data, "ORA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x15:	//Zeropage,X OR
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "ORA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x0D:	//Absolute OR
			temp = absolute();
			data = RAM->readRAM(temp);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "ORA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x1D:	//Absolute,X OR
			temp = absoluteX();
			data = RAM->readRAM(temp);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "ORA");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x19:	//Absolute,Y OR
			temp = absoluteY();
			data = RAM->readRAM(temp);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "ORA");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x01:	//Indirect,X OR
			temp = indirectX();
			data = RAM->readRAM(temp);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "ORA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x11:	//Indirect,Y OR
			temp = indirectY();
			data = RAM->readRAM(temp);
			A = A | data;
			Z = !(A);
			N = A & 0x80;
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "ORA");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x48:	//Push A onto stack
			pushStack(A);
			//debugImplied(opcode, RAM->readRAM(PC), "PHA");
			cycles = cycleCount[opcode];
			break;
		case 0x08:	//Push copy of status flags onto stack
			//Bit-----7--6--5--4--3--2--1--0 PHP B and bit 5 true
			//Order = N, V, 1, 1, D, I, Z, C
			statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
			statusFlags[4] = 1, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
			statusByte = encodeBits(statusFlags);	//Puts the status flags into a byte
			pushStack(statusByte);			
			//debugImplied(opcode, RAM->readRAM(PC), "PHP");
			cycles = cycleCount[opcode];
			break;
		case 0x68:	//Pops value off stack into A
			A = popStack();
			Z = !(A);
			N = A & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "PLA");
			cycles = cycleCount[opcode];
			break;
		case 0x28:	//Pops process status flags off stack
			statusByte = popStack();
			//Bit-----7--6--5--4--3--2--1--0 
			//Order = N, V, 1, X, D, I, Z, C
			decodeBits(outputFlags, statusByte);
			C = outputFlags[0];
			Z = outputFlags[1];
			I = outputFlags[2];
			D = outputFlags[3];
			V = outputFlags[6];
			N = outputFlags[7];
			//debugImplied(opcode, RAM->readRAM(PC), "PLP");
			cycles = cycleCount[opcode];
			break;
		case 0x2A:	//Accumulator rotate left
			oldBit7 = A & 0x80; //Old bit 7
			//debugAcc(opcode, "ROL");
			A = A << 1;
			A |= C;
			C = oldBit7;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			break;
		case 0x26:	//Zeropage rotate left
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			oldBit7 = data & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), data, "ROL");
			data <<= 1;
			data |= C;
			RAM->writeRAM(temp, data);
			C = oldBit7;
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x36:	//Zeropage,X rotate left
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			oldBit7 = data & 0x80;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "ROL");
			data <<= 1;
			data |= C;
			RAM->writeRAM(temp, data);
			C = oldBit7;
			N = data & 0x80;
			Z = !(data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x2E:	//Absolute rotate left
			temp = absolute();	//Gets address
			data = RAM->readRAM(temp);					//Gets data
			oldBit7 = data & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "ROL");
			data <<= 1;
			data |= C;				//Sets bit 0 to carry flag
			RAM->writeRAM(temp, data);
			C = oldBit7;
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x3E:	//Absolute,X rotate left
			temp = absoluteX();
			data = RAM->readRAM(temp);
			oldBit7 = data & 0x80;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "ROL");
			data <<= 1;
			data |= C;
			RAM->writeRAM(temp, data);
			C = oldBit7;
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x6A:	//Accumulator rotate right
			oldBit0 = A & 0x01; //Old bit 0
			//debugAcc(opcode, "ROR");
			A = A >> 1;
			A |= (C << 7);
			C = oldBit0;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			break;
		case 0x66:	//Zeropage rotate right
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			oldBit0 = data & 0x01;
			//debugZero(opcode, RAM->readRAM(PC), data, "ROR");
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x76:	//Zeropage,X rotate right
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			oldBit0 = data & 0x01;
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "ROR");
			data >>= 1;
			if(C) data |= 0x80;
			RAM->writeRAM(temp, data);
			C = oldBit0;
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x6E:	//Absolute rotate right
			temp = absolute();
			data = RAM->readRAM(temp);
			oldBit0 = data & 0x01;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "ROR");
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x7E:	//Absolute,X rotate right
			temp = absoluteX();
			data = RAM->readRAM(temp);
			oldBit0 = data & 0x01;
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "ROR");
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			Z = !(data);
			N = data & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x40:	//Return from interrupt
			//Pull processor stat flags
			statusByte = popStack();
			//Bit-----7--6--5--4--3--2--1--0 
			//Order = N, V, 1, B, D, I, Z, C
			decodeBits(outputFlags, statusByte);
			C = outputFlags[0];
			Z = outputFlags[1];
			I = outputFlags[2];
			D = outputFlags[3];
			V = outputFlags[6];
			N = outputFlags[7];
			//debugImplied(opcode, RAM->readRAM(PC), "RTI");
			data = popStack();		//Gets the low byte
			PC = data;
			data = popStack();		//Gets the high byte
			PC |= (data << 8);
			cycles = cycleCount[opcode];
			break;
		case 0x60:	//Return from subroutine
			//debugImplied(opcode, RAM->readRAM(PC), "RTS");
			data = popStack();		//Gets the low byte
			PC = data;
			data = popStack();		//Gets the high byte
			PC |= (data << 8);
			cycles = cycleCount[opcode];
			PC++;					//Add one once off stack for right address
			break;
		case 0xE9:	//Immidiate SBC
			data = RAM->readRAM(PC);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp = A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "SBC");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break; 
		case 0xE5:	//Zeropage SBC
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "SBC");
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp= A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xF5:	//Zeropage,X SBC
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "SBC");
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp= A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xED:	//Absolute SBC
			temp = absolute();
			data = RAM->readRAM(temp);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "SBC");
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp= A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xFD:	//Absolute,X SBC
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "SBC");
			pageBoundry((temp - X), temp);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp= A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			PC += instr_lens[opcode] - 1;
			break;
		case 0xF9:	//Absolute,Y SBC
			temp = absoluteY();
			data = RAM->readRAM(temp);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "SBC");
			pageBoundry((temp - Y), temp);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp= A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			PC += instr_lens[opcode] - 1;
			break;
		case 0xE1:	//Indirect,X SBC
			temp = indirectX();
			data = RAM->readRAM(temp);
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "SBC");
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp = A + data + C;
			V = (initialA ^ temp) & (data ^ temp) & 0x80;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xF1:	//Indirect,Y SBC
			temp = indirectY();
			data = RAM->readRAM(temp);
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "SBC");
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			V = (initialA ^ (A + data + C)) & (data ^ (A + data + C)) & 0x80;
			A = (A + data + C) & 0xFF;
			C = ((initialA + data + C) >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x38:	//Set carry flag
			C = true;
			//debugImplied(opcode, RAM->readRAM(PC), "SEC");
			cycles = cycleCount[opcode];
			break;
		case 0xF8:	//Set decimal flag
			D = true;
			//debugImplied(opcode, RAM->readRAM(PC), "SED");
			cycles = cycleCount[opcode];
			break;
		case 0x78:	//Set interrupt disable
			I = true;
			//debugImplied(opcode, RAM->readRAM(PC), "SEI");
			cycles = cycleCount[opcode];
			break;
		case 0x85:	//Zeropage store A to memory
			temp = RAM->readRAM(PC);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "STA");
			RAM->writeRAM(temp, A);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x95:	//Zeropage,X store A to memory
			temp = zeroPageX();
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "STA");
			RAM->writeRAM(temp, A);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x8D:	//Absolute store A to memory
			temp = absolute();
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "STA");
			RAM->writeRAM(temp, A);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x9D:	//Absolute,X store A to memory
			temp = absoluteX();
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "STA");
			RAM->writeRAM(temp, A);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x99:	//Absolute,Y store A to memory
			temp = absoluteY();
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "STA");
			RAM->writeRAM(temp, A);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x81:	//Indirect,X store A to memory
			temp = indirectX();
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "STA");
			RAM->writeRAM(temp, A);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x91:	//Indirect,Y store A to memory
			temp = indirectY();
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "STA");
			RAM->writeRAM(temp, A);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x86:	//Zeropage store X to memory
			temp = RAM->readRAM(PC);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "STX");
			RAM->writeRAM(temp, X);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x96:	//Zeropage,Y store X to memory
			temp = (RAM->readRAM(PC)+ Y) & 0xFF;
			//debugZeroY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "STX");
			RAM->writeRAM(temp, X);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x8E:	//Absolute store X to memory
			temp = absolute();
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "STX");
			RAM->writeRAM(temp, X);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x84:	//Zeropage store Y to memory
			temp = RAM->readRAM(PC);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "STY");
			RAM->writeRAM(temp, Y);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x94:	//Zeropage,X store Y to memory
			temp = zeroPageX();
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "STY");
			RAM->writeRAM(temp, Y);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x8C:	//Absolute store Y to memory
			temp = absolute();
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "STY");
			RAM->writeRAM(temp, Y);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xAA:	//Transfer A to X
			X = A;
			Z = !(X);
			N = X & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "TAX");
			cycles = cycleCount[opcode];
			break;
		case 0xA8:	//Transfer A to Y
			Y = A;
			Z = !(Y);
			N = Y & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "TAY");
			cycles = cycleCount[opcode];
			break;
		case 0xBA:	//Transfer SP to X
			X = SP & 0xFF;	//Puts the value within byte range
			Z = !(X);
			N = X & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "TSX");
			cycles = cycleCount[opcode];
			break;
		case 0x8A:	//Transfer X to A
			A = X;
			Z = !(A);
			N = A & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "TXA");
			cycles = cycleCount[opcode];
			break;
		case 0x9A:	//Transfers X into SP
			SP = X + 0x0100;	//Puts the value into the right range for the stack.
			//debugImplied(opcode, RAM->readRAM(PC), "TXS");
			cycles = cycleCount[opcode];
			break;
		case 0x98:	//Transfers Y to A
			A = Y;
			Z = !(A);
			N = A & 0x80;
			//debugImplied(opcode, RAM->readRAM(PC), "TYA");
			cycles = cycleCount[opcode];
			break;
		//Illegal opcodes-------------------------------------------------
	#ifdef illegal
		case 0x0B:	//Immediate ANC
			data = RAM->readRAM(PC);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			C = N;
			//debugImm(opcode, RAM->readRAM(PC), "ANC");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x2B:	//Immediate ANC
			data = RAM->readRAM(PC);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			C = N;
			//debugImm(opcode, RAM->readRAM(PC), "ANC");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x87:	//Zeropage SAX
			data = A & X;
			temp = RAM->readRAM(PC);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "*SAX");
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x97:	//Zeropage,Y SAX
			data = A & X;
			temp = (RAM->readRAM(PC)+ Y) & 0xFF;
			//debugZeroY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*SAX");
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x8F:	//Absolute SAX
			data = A & X;
			temp = absolute();
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "*SAX");
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x83:	//Indirect,X SAX
			data = A & X;
			temp = indirectX();
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*SAX");
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x6B:	//Immediate ARR - Not working
			A &= RAM->readRAM(PC);
			oldBit0 = A & 1;
			A >>= 1;
			A |= (C << 7);
			Z = !(A);
			N = A & 0x80;
			C = A & 0x40;
			V = (A & 0x40) ^ (A & 0x20);
			//debugImm(opcode, RAM->readRAM(PC), "ARR");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x4B:	//Immediate ALR
 			A &= RAM->readRAM(PC);
			C = A & 0x01;
			A >>= 1;
			Z = !(A);
			N = A & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "ALR");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xAB:	//Immediate ATX - Not working
			A &= RAM->readRAM(PC);
			X = A;
			Z = !(A);
			N = A & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "*ATX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x9F:	//Absolute,Y ATX
			data = (A & X) & 7;
			temp = absoluteY();
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*ATX");
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x93:	//Indirect,Y
			data = (A & X) & 7;
			temp = indirectY();
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*ATX");
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xC7:	//Zeropage DCP
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "*DCP");
			data--;
			compareFlags(A, data);
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xD7:	//Zeropage,X DCP
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*DCP");
			data--;
			compareFlags(A, data);
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xCF:	//Absolute DCP
			temp = absolute();
			data = RAM->readRAM(temp);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "*DCP");
			data--;
			compareFlags(A, data);
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xDF:	//Absolute,X DCP
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*DCP");
			data--;
			compareFlags(A, data);
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xDB:	//Absolute,Y DCP
			temp = absoluteY();
			data = RAM->readRAM(temp);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*DCP");
			data--;
			compareFlags(A, data);
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xC3:	//Indirect,X DCP
			temp = indirectX();
			data = RAM->readRAM(temp);
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*DCP");
			data--;
			compareFlags(A, data);
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xD3:	//Indirect,Y DCP
			temp = indirectY();
			data = RAM->readRAM(temp);
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*DCP");
			data--;
			compareFlags(A, data);
			RAM->writeRAM(temp, data);
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x04:	//Zeropage DOP
			temp = RAM->readRAM(PC);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x44:	//Zeropage DOP
			temp = RAM->readRAM(PC);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x64:	//Zeropage DOP
			temp = RAM->readRAM(PC);
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x14:	//Zeropage,X DOP
			temp = zeroPageX();
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x34:	//Zeropage,X DOP
			temp = zeroPageX();
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x54:	//Zeropage,X DOP
			temp = zeroPageX();
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x74:	//Zeropage,X DOP
			temp = zeroPageX();
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xD4:	//Zeropage,X DOP
			temp = zeroPageX();
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xF4:	//Zeropage,X DOP
			temp = zeroPageX();
			//debugZeroX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x80:	//Immediate DOP
			//debugImm(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x82:	//Immediate DOP
			//debugImm(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x89:	//Immediate DOP
			//debugImm(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xC2:	//Immediate DOP
			//debugImm(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xE2:	//Immediate DOP
			//debugImm(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xE7:	//Zeropage ISC
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			//debugZero(opcode, RAM->readRAM(PC), data, "*ISB");
			data++;
			RAM->writeRAM(temp, data);
			//SBC is similar to ADC.  The bits are inverted.
			data ^= 0xFF;
			temp= A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xF7:	//Zeropage,X ISC
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "*ISB");
			data++;
			RAM->writeRAM(temp, data);
			//SBC is similar to ADC.  The bits are inverted.
			data ^= 0xFF;
			temp= A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xEF:	//Absolute ISC
			temp = absolute();
			data = RAM->readRAM(temp);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "*ISB");
			data++;
			RAM->writeRAM(temp, data);
			//SBC is similar to ADC.  The bits are inverted.
			data ^= 0xFF;
			temp= A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xFF:	//Absolute,X ISC
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*ISB");
			data++;
			RAM->writeRAM(temp, data);
			//SBC is similar to ADC.  The bits are inverted.
			data ^= 0xFF;
			temp = A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xFB:	//Absolute,Y ISC
			temp = absoluteY();
			data = RAM->readRAM(temp);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*ISB");
			data++;
			RAM->writeRAM(temp, data);
			//SBC is similar to ADC.  The bits are inverted.
			data ^= 0xFF;
			temp = A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xE3:	//Indirect,X ISC
			temp = indirectX();
			data = RAM->readRAM(temp);
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "*ISB");
			data++;
			RAM->writeRAM(temp, data);
			//SBC is similar to ADC.  The bits are inverted.
			data ^= 0xFF;
			temp = A + data + C;
			V = (initialA ^ temp) & (data ^ temp) & 0x80;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xF3:	//Indirect,Y SBC
			temp = indirectY();
			data = RAM->readRAM(temp);
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "*ISB");
			data++;
			RAM->writeRAM(temp, data);
			//SBC is similar to ADC.  The bits are inverted.
			data ^= 0xFF;
			temp = A + data + C;
			V = (initialA ^ temp) & (data ^ temp) & 0x80;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xBB:	//Absolute,Y LAR
			temp = absoluteY();
			data = RAM->readRAM(temp);
			data &= SP;
			A = X = SP = data;
			Z = !(data);
			N = data & 0x80;
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "LAR");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xA7:	//Zeropage LAX
			temp = RAM->readRAM(PC);
			X = A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugZero(opcode, RAM->readRAM(PC), RAM->readRAM(temp), "*LAX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xB7:	//Zeropage,Y LAX
			temp = (RAM->readRAM(PC)+ Y) & 0xFF;
			X = A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugZeroY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*LAX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xAF:	//Absolute LAX
			temp = absolute();
			X = A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "*LAX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xBF:	//Absolute,Y LAX
			temp = absoluteY();
			X = A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*LAX");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xA3:	//Indirect,X LAX
			temp = indirectX();
			X = A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*LAX");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xB3:	//Indirect,Y LAX
			temp = indirectY();
			X = A = RAM->readRAM(temp);
			Z = !(A);
			N = A & 0x80;
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, RAM->readRAM(temp), "*LAX");
			pageBoundry((temp - Y), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x1A:	//Implied NOP
			//debugImplied(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			break;
		case 0x3A:	//Implied NOP
			//debugImplied(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			break;
		case 0x5A:	//Implied NOP
			//debugImplied(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			break;
		case 0x7A:	//Implied NOP
			//debugImplied(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			break;
		case 0xDA:	//Implied NOP
			//debugImplied(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			break;
		case 0xFA:	//Implied NOP
			//debugImplied(opcode, RAM->readRAM(PC), "*NOP");
			cycles = cycleCount[opcode];
			break;
		case 0x67:	//Zeropage RRA
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			//debugZero(opcode, RAM->readRAM(PC), data, "*RRA");
			oldBit0 = data & 1;
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break; 
		case 0x77:	//Zeropage,X RRA
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "*RRA");
			oldBit0 = data & 1;
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x6F:	//Absolute RRA
			temp = absolute();
			data = RAM->readRAM(temp);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "*RRA");
			oldBit0 = data & 1;
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x7F:	//Absolute,X RRA
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*RRA");
			oldBit0 = data & 1;
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x7B:	//Absolute,Y RRA
			temp = absoluteY();
			data = RAM->readRAM(temp);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*RRA");
			oldBit0 = data & 1;
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x63:	//Indirect,X RRA
			temp = indirectX();
			data = RAM->readRAM(temp);
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "*RRA");
			oldBit0 = data & 1;
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x73:	//Indirect,Y RRA
			temp = indirectY();
			data = RAM->readRAM(temp);
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "*RRA");
			oldBit0 = data & 1;
			data >>= 1;
			data |= (C << 7);
			RAM->writeRAM(temp, data);
			C = oldBit0;
			temp = A + data + C;
			A = temp & 0xFF;
			C = (temp >> 8) & 1;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xEB:	//Immidiate SBC - same as legal opcode
			data = RAM->readRAM(PC);
			//SBC is similar to ADC.  The bits are inverted.
			data = data ^ 0xFF;
			temp = A + data + C;
			C = (temp >> 8) & 1;
			A = temp & 0xFF;
			Z = !(A);
			V = (initialA ^ temp) & (data ^ temp) & 0x80;		//Checks the sign of the inputs and result
			N = A & 0x80;
			//debugImm(opcode, RAM->readRAM(PC), "*SBC");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break; 
		case 0x07:	//Zeropage SLO
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			//debugZero(opcode, RAM->readRAM(PC), data, "*SLO");
			C = data & 0x80;
			data <<= 1;
			RAM->writeRAM(temp, data);
			A |= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break; 
		case 0x17:	//Zeropage,X SLO
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "*SLO");
			C = data & 0x80;
			data <<= 1;
			RAM->writeRAM(temp, data);
			A |= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x0F:	//Absolute SLO
			temp = absolute();
			data = RAM->readRAM(temp);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "*SLO");
			C = data & 0x80;
			data <<= 1;
			RAM->writeRAM(temp, data);
			A |= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x1F:	//Absolute,X SLO
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*SLO");
			C = data & 0x80;
			data <<= 1;
			RAM->writeRAM(temp, data);
			A |= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x1B:	//Absolute,Y SLO
			temp = absoluteY();
			data = RAM->readRAM(temp);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*SLO");
			C = data & 0x80;
			data <<= 1;
			RAM->writeRAM(temp, data);
			A |= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x03:	//Indirect,X SLO
			temp = indirectX();
			data = RAM->readRAM(temp);
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "*SLO");
			C = data & 0x80;
			data <<= 1;
			RAM->writeRAM(temp, data);
			A |= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x13:	//Indirect,Y SLO
			temp = indirectY();
			data = RAM->readRAM(temp);
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "*SLO");
			C = data & 0x80;
			data <<= 1;
			RAM->writeRAM(temp, data);
			A |= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x47:	//Zeropage SRE
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			//debugZero(opcode, RAM->readRAM(PC), data, "*SRE");
			C = data & 1;
			data >>= 1;
			RAM->writeRAM(temp, data);
			A ^= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break; 
		case 0x57:	//Zeropage,X SRE
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "*SRE");
			C = data & 1;
			data >>= 1;
			RAM->writeRAM(temp, data);
			A ^= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x4F:	//Absolute SRE
			temp = absolute();
			data = RAM->readRAM(temp);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "*SRE");
			C = data & 1;
			data >>= 1;
			RAM->writeRAM(temp, data);
			A ^= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x5F:	//Absolute,X SRE
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*SRE");
			C = data & 1;
			data >>= 1;
			RAM->writeRAM(temp, data);
			A ^= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x5B:	//Absolute,Y SRE
			temp = absoluteY();
			data = RAM->readRAM(temp);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*SRE");
			C = data & 1;
			data >>= 1;
			RAM->writeRAM(temp, data);
			A ^= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x43:	//Indirect,X SRE
			temp = indirectX();
			data = RAM->readRAM(temp);
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "*SRE");
			C = data & 1;
			data >>= 1;
			RAM->writeRAM(temp, data);
			A ^= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x53:	//Indirect,Y SRE
			temp = indirectY();
			data = RAM->readRAM(temp);
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "*SRE");
			C = data & 1;
			data >>= 1;
			RAM->writeRAM(temp, data);
			A ^= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x9E:	//Absolute,Y SXA - Not working
			temp = absoluteY();
			data = (temp & 0xFF00) >> 8;
			data = (X & data) + 1;
			RAM->writeRAM(temp, data);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "SXA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x9C:	//Absolute,X SYA - Not working
			temp = absoluteX();
			data = (temp & 0xFF00) >> 8;
			data = (Y & data) + 1;
			RAM->writeRAM(temp, data);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "SYA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x0C:	//Absolute TOP
			temp = absolute();
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), RAM->readRAM(temp), "*NOP");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x1C:	//Absolute,X TOP
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x3C:	//Absolute,X TOP
			temp = absoluteX();
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x5C:	//Absolute,X TOP
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x7C:	//Absolute,X TOP
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xDC:	//Absolute,X TOP
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0xFC:	//Absolute,X TOP
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, RAM->readRAM(temp), "*NOP");
			pageBoundry((temp - X), temp);
			PC += instr_lens[opcode] - 1;
			break;
		case 0x8B:	//Immediate XAA
			//debugImm(opcode, RAM->readRAM(PC), "XAA");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x9B:	//Absolute,Y XAS
			temp = absoluteY();
			SP = X & A;
			data = (temp & 0xFF00) >> 8;
			data = SP & (data + 1);
			RAM->writeRAM(temp, data);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "XAS");
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x27:	//Zeropage RLA
			temp = RAM->readRAM(PC);
			data = RAM->readRAM(temp);
			//debugZero(opcode, RAM->readRAM(PC), data, "*RLA");
			oldBit7 = data & 0x80;
			data <<= 1;
			data |= C;
			C = oldBit7;
			RAM->writeRAM(temp, data);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x37:	//Zeropage,X RLA
			temp = zeroPageX();
			data = RAM->readRAM(temp);
			//debugZeroX(opcode, RAM->readRAM(PC), temp, data, "*RLA");
			oldBit7 = data & 0x80;
			data <<= 1;
			data |= C;
			C = oldBit7;
			RAM->writeRAM(temp, data);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x2F:	//Absolute RLA
			temp = absolute();
			data = RAM->readRAM(temp);
			//debugAbs(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), data, "*RLA");
			oldBit7 = data & 0x80;
			data <<= 1;
			data |= C;
			C = oldBit7;
			RAM->writeRAM(temp, data);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x3F:	//Absolute,X RLA
			temp = absoluteX();
			data = RAM->readRAM(temp);
			//debugAbsX(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*RLA");
			oldBit7 = data & 0x80;
			data <<= 1;
			data |= C;
			C = oldBit7;
			RAM->writeRAM(temp, data);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x3B:	//Absolute,Y RLA
			temp = absoluteY();
			data = RAM->readRAM(temp);
			//debugAbsY(opcode, RAM->readRAM(PC + 1), RAM->readRAM(PC), temp, data, "*RLA");
			oldBit7 = data & 0x80;
			data <<= 1;
			data |= C;
			C = oldBit7;
			RAM->writeRAM(temp, data);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x23:	//Indirect,X RLA
			temp = indirectX();
			data = RAM->readRAM(temp);
			//debugIndirectX(opcode, RAM->readRAM(PC), temp, data, "*RLA");
			oldBit7 = data & 0x80;
			data <<= 1;
			data |= C;
			C = oldBit7;
			RAM->writeRAM(temp, data);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0x33:	//Indirect,Y RLA
			temp = indirectY();
			data = RAM->readRAM(temp);
			//debugIndirectY(opcode, RAM->readRAM(PC), temp, data, "*RLA");
			oldBit7 = data & 0x80;
			data <<= 1;
			data |= C;
			C = oldBit7;
			RAM->writeRAM(temp, data);
			A &= data;
			Z = !(A);
			N = A & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
		case 0xCB:	//Immediate AXS	- Not working
			X &= A;
			data = RAM->readRAM(PC);
			//SBC is similar to ADC.  The bits are inverted.
			data ^= 0xFF;
			temp = X + data;
			C = (temp >> 8) & 1;
			X = temp & 0xFF;
			Z = !(X);
			N = X & 0x80;
			cycles = cycleCount[opcode];
			PC += instr_lens[opcode] - 1;
			break;
	#endif
		default: //For any opcode that doesn't hit another case
			PC += instr_lens[opcode] - 1;
			break;
		}
	}
	else NMI();
		
	return cycles;
}


//Private
//This pushes a piece of data onto the stack and decrements SP
const void cpu::pushStack(byte &data)
{
	RAM->writeRAM(SP, data);
	SP--;				//Decrememnt after writing memory.
	if(SP == 0x00FF) SP = 0x01FF;
}

//This pops a piece off data off the stack and increments SP
const cpu::byte cpu::popStack()
{
	SP++;
	if(SP == 0x0200) SP = 0x0100;
	return( RAM->readRAM(SP) );
}

//This sets the flags after a compare instruction
const void cpu::compareFlags(byte& reg, byte memoryValue)
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
const cpu::byte cpu::encodeBits(bool bools[8])
{
	byte retval = 0;
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
const void cpu::decodeBits(bool (&bools)[8], byte input)
{
	for(int i = 0; i < 8; i++) 
	{
        	bools[i] = (input & 0x00000001 ? true : false);
        	input = (i < 7 ? input >> 1 : input);	//? like an if statement
    	}
}

const void cpu::pageBoundry(word address1, word& address2)
{
	//If the high bytes are the same, then a page boundry has not been crossed.
	//If they are different, one has been crossed, so add a cycle.
	if( (address1 & 0xFF00) == (address2 & 0xFF00) )	cycles = cycleCount[opcode];
	else	cycles = cycleCount[opcode] + 1;
}

const void cpu::pageBranch(char& offset)
{
	//If the high bytes of the address do not match, then a page has been crossed.  Add two cycles
	if( ((PC + offset) & 0xFF00) != (PC & 0xFF00) ) cycles += 1;
}

const void cpu::NMI()
{
	byte data, statusByte;
	bool statusFlags[8];

	video->NMI = false;
	data = (PC & 0xFF00) >> 8;		//Push high byte first
	pushStack(data);
	data = PC & 0x00FF;
	pushStack(data);
	//Bit-----7--6--5--4--3--2--1--0 PHP always has B and bit 5 true
	//Order = N, V, 1, 0, D, I, Z, C
	statusFlags[0] = C, statusFlags[1] = Z, statusFlags[2] = I, statusFlags[3] = D,
	statusFlags[4] = 0, statusFlags[5] = 1, statusFlags[6] = V, statusFlags[7] = N;
	statusByte = encodeBits(statusFlags);	//Puts the status flags into a
	pushStack(statusByte);				//byte	
	PC = (RAM->readRAM(0xFFFB) << 8) | RAM->readRAM(0xFFFA);
	I = true;
	cycles = cycleCount[opcode];	//7 cycles to process interrupt handler
}



//Get address functions
const cpu::word cpu::zeroPageX()
{
	word temp = RAM->readRAM(PC);
	temp += X;
	temp &= 0xFF;		//Wraps around
	return temp;
}

const cpu::word cpu::zeroPageY()
{
	word address = RAM->readRAM(PC);
	address += Y;
	address &= 0xFF;
	return address;
}

const cpu::word cpu::absolute()
{
	word high = RAM->readRAM(PC + 1) << 8;
	word low = RAM->readRAM(PC);
	word address = high | low;
	return address;
}
	
const cpu::word cpu::absoluteX()
{
	word high = RAM->readRAM(PC + 1) << 8;
	word low = RAM->readRAM(PC);
	word address = high | low;
	address += X;
	return address;
}

const cpu::word cpu::absoluteY()
{
	word high = RAM->readRAM(PC + 1) << 8;
	word low = RAM->readRAM(PC);
	word address = high | low;
	address += Y;
	return address;
}

const cpu::word cpu::indirectX()
{
	word zeropageAddress = zeroPageX();
	word low = RAM->readRAM(zeropageAddress);
	zeropageAddress++;
	zeropageAddress &= 0xFF;
	word high = RAM->readRAM(zeropageAddress) << 8;
	word address = high | low;
	return address;
}

const cpu::word cpu::indirectY()
{
	word zeropageAddress = RAM->readRAM(PC);
	word low = RAM->readRAM(zeropageAddress);
	zeropageAddress++;
	zeropageAddress &= 0xFF;
	word high = RAM->readRAM(zeropageAddress) << 8;
	word address = high | low;
	address += Y;
	return address;
}


cpu::byte const cpu::instr_lens[] = { // lengths including opcode
	 //0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
	   2,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,// 0
	   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 1
	   3,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,// 2
	   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 3
	   2,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,// 4
	   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 5
	   1,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,// 6
	   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 7
	   2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,// 8
	   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// 9
	   2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,// A
	   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// B
	   2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,// C
	   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// D
	   2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,// E
	   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,// F
	};

cpu::byte const cpu::cycleCount[] = { // cycles including opcode
	 //0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
	   7,6,1,8,3,3,5,5,3,2,2,2,4,4,6,6,// 0
	   2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// 1
	   6,6,1,8,3,3,5,5,4,2,2,2,4,4,6,6,// 2
	   2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// 3
	   6,6,1,8,3,3,5,5,3,2,2,2,3,4,6,6,// 4
	   2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// 5
	   6,6,1,8,3,3,5,5,4,2,2,2,5,4,6,6,// 6
	   2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// 7
	   2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,// 8
	   2,6,1,6,4,4,4,4,2,5,2,3,3,5,3,5,// 9
	   2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,// A
	   2,5,1,5,4,4,4,4,2,4,2,4,4,4,4,4,// B
	   2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,// C
	   2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// D
	   2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,// E
	   2,5,1,8,4,4,6,6,2,4,2,7,4,4,7,7,// F
	};


////debugging stuff
//This outputs //debug information for accumulator instructions
#ifdef debugOn
const void cpu:://debugAcc(byte &opcode, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//cout << "GOOD" << endl;
		//Left side of //debug file
		//debugFile << setw(4) << uppercase  << setfill('0') << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";				
		//debugFile << setw(2) << uppercase << (int)opcode;
		//debugFile << setw(11) << setfill(' ') << OPname;		//Name of the opcode
		//debugFile << setw(2) << " A";
		//debugFile << setw(27) << " ";

		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for immidiate instructions
const void cpu:://debugImm(byte &opcode, byte data, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//cout << "GOOD" << endl;
		//Left side of //debug file
		//debugFile << setw(4) << uppercase << setfill('0') << (PC - 1); 
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase << setfill('0') << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << setfill('0') << (int)data;
		//debugFile << setw(8) << setfill(' ') << OPname;				//Name of the opcode
		//debugFile << setfill(' ') << setw(3) << "#$";
		//debugFile << setw(2) << uppercase << setfill('0') << (int)data;
		//debugFile << setfill(' ') << setw(24) << " ";

		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;	
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for zeropage instructions
const void cpu:://debugZero(byte &opcode, byte dataPC, byte dataAddress, 
			std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//cout << "GOOD" << endl;
		//debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase  << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase  << (int)dataPC;
		//debugFile << setw(8) << setfill(' ') << OPname;				//Name of the opcode
		//debugFile << setw(2) << "$" << setw(2) << uppercase << setfill('0') << (int)dataPC
			  << setw(3) << " = " << setw(2) << (int)dataAddress;
		//debugFile << setw(20) << setfill(' ') << " ";
		
		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for zeropage + X instructions
const void cpu:://debugZeroX(byte &opcode, byte dataPC,
				word totalAddress, byte dataAddress, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//cout << "GOOD" << endl;
		//debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)dataPC;
		//debugFile << setw(8) << setfill(' ') << OPname;			//Name of the opcode
		//debugFile << setw(2) << " $" << uppercase  << setfill('0') <<  setw(2) << (int)dataPC 
			  << setw(5) << ",X @ " << setw(2) << (totalAddress & 0xFF) << setw(3) << " = "
			  << setw(2) << (int)dataAddress;
		//debugFile << setw(13) << setfill(' ') << " ";
		
		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for zeropage + Y instructions
const void cpu:://debugZeroY(byte &opcode, byte dataPC,
				word totalAddress, byte dataAddress, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//cout << "GOOD" << endl;
		//debugFile << setw(4) << setfill('0') << uppercase  << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase  << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase  << (int)dataPC;
		//debugFile << setw(8) << setfill(' ') << OPname;			//Name of the opcode
		//debugFile << setw(2) << " $" << uppercase << setfill('0') << setw(2) << (int)dataPC 
			  << setw(5) << ",Y @ " << setw(2) << (totalAddress & 0xFF) << setw(3) << " = "
			  << setw(2) << (int)dataAddress;
		//debugFile << setw(13) << setfill(' ') << " ";
		
		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for absolute instructions
const void cpu:://debugAbs(byte &opcode, byte highAddress, byte lowAddress, 
		    byte data, std::string OPname)	
{
	using namespace std;
	word tempAddress;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//cout << "GOOD" << endl;
		//debugFile << setw(4) << setfill('0') << uppercase  << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase  << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase  << (int)lowAddress;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase  << (int)highAddress;
		//debugFile << setw(5) << setfill(' ') << OPname;			//Name of the opcode
		if(OPname == "JMP" || OPname == "JSR")
		{
			//debugFile << setw(2) << "$" << setw(2) << setfill('0') << uppercase  << (int)highAddress
			 << setw(2) << (int)lowAddress;
			//debugFile << setw(23) << setfill(' ') << " ";
		}
		else
		{
			//debugFile << setw(2) << "$" << setw(2) << setfill('0')  << uppercase  << (int)highAddress
			 << setw(2) <<(int)lowAddress << setw(3) << " = " << setw(2);
			tempAddress = (highAddress << 8) | lowAddress;
			if(tempAddress > 0x1FFF && tempAddress < 0x2008) //debugFile << "FF";
			else //debugFile << (int)data;
			//debugFile << setw(18) << setfill(' ') << " ";
		}
		
		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for absolute,X instructions
const void cpu:://debugAbsX(byte &opcode, byte highAddress, byte lowAddress, 
			word totalAddress, byte data, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//cout << "GOOD" << endl;
		//debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)lowAddress;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)highAddress;
		//debugFile << setw(5) << setfill(' ') << OPname;			//Name of the opcode
		//debugFile << setw(2) << " $" << uppercase << setfill('0') << setw(2) << (int)highAddress 
			  << setw(2) << (int)lowAddress << setw(5) << ",X @ " << setw(4) << totalAddress 
			  << setw(3) << " = " << setw(2) << (int)data;
		//debugFile << setw(9) << setfill(' ') << " ";
		
		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for absolute, Y instructions
const void cpu:://debugAbsY(byte &opcode, byte highAddress, byte lowAddress, 
			word totalAddress, byte data, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//cout << "GOOD" << endl;
		//debugFile << setw(4) << setfill('0') << uppercase << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)lowAddress;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)highAddress;
		//debugFile << setw(5) << setfill(' ') << OPname;			//Name of the opcode
		//debugFile << setw(2) << " $" << uppercase << setfill('0') << setw(2) << (int)highAddress 
			  << setw(2) << (int)lowAddress << setw(5) << ",Y @ " << setw(4) << totalAddress 
			  << setw(3) << " = " << setw(2) << (int)data;
		//debugFile << setw(9) << setfill(' ') << " ";
		
		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for indirect,X instructions
const void cpu:://debugIndirectX(byte &opcode, byte pcAddress,word indirectAddress, 			byte data, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//cout << "GOOD" << endl;
		//debugFile << setw(4) << setfill('0') << uppercase << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)pcAddress;	//Gives the indirect address
		//debugFile << setw(8) << setfill(' ') << OPname;				//Name of the opcode
		//debugFile << setw(3) << " ($" << uppercase  << setw(2) << setfill('0') << (int)pcAddress;
		//debugFile << setw(6) << ",X) @ " << setw(2) << (((int)pcAddress + initialX) & 0xFF) 
			  << setw(3) << " = "; 
		//debugFile << setw(4) << indirectAddress << setw(3) << " = " << setw(2) << (int)data;
		//debugFile << setw(4) << setfill(' ') << " ";

		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for indirect,Y instructions
const void cpu:://debugIndirectY(byte &opcode, byte pcAddress,
			  word indirectAddress, byte data, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//debugFile << setw(4) << setfill('0') << uppercase << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)pcAddress;	//Gives the indirect address
		//debugFile << setw(8) << setfill(' ') << OPname;				//Name of the opcode
		//debugFile << setw(3) << " ($" << uppercase  << setw(2) << setfill('0') << (int)pcAddress;
		//debugFile << setw(6) << "),Y = " << setw(4) << ((indirectAddress - Y) & 0xFFFF)  << setw(3) << " @ "; 
		//debugFile << setw(4) << indirectAddress << setw(3) << " = " << setw(2) << (int)data;
		//debugFile << setw(2) << setfill(' ') << " ";

		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for relative instructions
const void cpu:://debugRelative(byte &opcode, char offset, byte dataPC, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//cout << "GOOD" << endl;
		//Left side of //debug file
		//If branch doesn't succeed, then offset = 0
		//debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)dataPC;
		//debugFile << setw(8) << setfill(' ') << OPname;			//Name of the opcode
		//debugFile << setw(2) << "$" << uppercase  << setw(4) << setfill('0') << (PC + offset + 1);
		//debugFile << setw(23) << setfill(' ') << " ";

		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug information for implied instructions
const void cpu:://debugImplied(byte &opcode, byte BRK, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Left side of //debug file
		//debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase  << (int)opcode;
		if(opcode == 0x00)
		{
			//debugFile << setw(3) << setfill(' ') << uppercase << (int)BRK;
			//debugFile << setw(8) << OPname;
			//debugFile << setw(3) << " #$";
			//debugFile << setw(2) << setfill('0') << uppercase << (int)BRK;
			//debugFile << setw(24) << setfill(' ') << " ";
		}
		else
		{	
			//debugFile << setw(11) << setfill(' ') << OPname;		//Name of the opcode
			//debugFile << setw(29) << " ";
		}

		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber  << hex;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}

//This outputs //debug informatiuon for implicit instructions
const void cpu:://debugIndirect(byte &opcode, byte highAddress, byte lowAddress,
				word jumpAddress, std::string OPname)
{
	using namespace std;

	if(//debugFile.is_open())
	{
		//Jump address is the data at the jump address.
		//Left side of //debug file
		//debugFile << setw(4) << uppercase << setfill('0') << (PC - 1);	//Prints in uppercase hex
		//debugFile << setw(2) << "  ";
		//debugFile << setw(2) << uppercase << (int)opcode;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)lowAddress;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << uppercase << (int)highAddress;
		//debugFile << setw(5) << setfill(' ') << OPname;			//Name of the opcode
		//debugFile << setw(3) << " ($" << uppercase << setfill('0') << setw(2) << (int)highAddress 
			  << setw(2) << (int)lowAddress << setw(4) << ") = " << setw(4) << jumpAddress;
		//debugFile << setw(14) << setfill(' ') << " ";
		
		//Right side of //debug file
		//debugFile << setw(2) << "A:" << uppercase << setw(2) << setfill('0') << (int)initialA;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "X:" << uppercase << setw(2) << setfill('0') << (int)initialX;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "Y:" << uppercase << setw(2) << setfill('0') << (int)initialY;
		//debugFile << setw(1) << " ";
		//debugFile << setw(2) << "P:" << uppercase << setw(2) << setfill('0') << (int)initialP;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SP:" << uppercase << setw(2) << setfill('0') << (initialSP - 0x0100);
		//debugFile << setw(1) << " ";
		//debugFile << setw(4) << "CYC:" << setfill(' ') << setw(3) << dec << video->dotNumber ;
		//debugFile << setw(1) << " ";
		//debugFile << setw(3) << "SL:";
		if(video->scanline < 10) //debugFile << setw(1) << dec << video->scanline << hex;
		else if(video->scanline < 100) //debugFile << setw(2) << dec << video->scanline << hex;
		else if(video->scanline == 261) //debugFile << setw(2) << "-1"; 
		else //debugFile << setw(3) << dec << video->scanline << hex;
		//debugFile << endl;
	}
	else cout << "Unable to open file";
}
#endif
