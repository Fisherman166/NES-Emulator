#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include "memory.h"
#include "ppu.h"

using namespace std;
#define illegal

class cpu
{
public:
	typedef unsigned char byte;
	typedef unsigned short word;			//Two bytes

	cpu();
	~cpu();
	word PC, SP;	//Stack pointer can be between 0x0100 and 0x01FF in memory
	byte A, X, Y;

	/* C = Carry flag Z = Zero flag  I = Interrupt disable D = decimal mode
	V = Overflow flag N = Negative flag*/
	bool C, Z, I, D, V, N;
	static byte const instr_lens[];
	static byte const cycleCount[];

	byte emulateCycle(memory*, ppu*);
	void setPCStart(memory*, ppu*);					//Sets the PC to the reset vector
	
private:
	byte initialA;
	byte cycles, opcode;


	//Functions
	const void pushStack(memory*, byte&, ppu*);
	const byte popStack(memory*, ppu*);
	const void compareFlags(byte&, byte);
	const byte encodeBits(bool bools[8]);
	const void decodeBits(bool (&bools)[8], byte input);
	const void pageBoundry(word, word&);
	const void pageBranch(char&);

	//Get address functions
	const word zeroPageX(memory*, ppu*);
	const word zeroPageY(memory*, ppu*);
	const word absolute(memory*, ppu*);
	const word absoluteX(memory*, ppu*);
	const word absoluteY(memory*, ppu*);
	const word indirectX(memory*, ppu*);
	const word indirectY(memory*, ppu*);

	const void NMI(memory*, ppu*);

	//Debug functions
	//DataPC = data from the PC counter memory address.  dataAddress = data from the PC counters addresses given
	//in dataPC.

#ifdef debugOn
	ofstream debugFile;
	byte initialX, initialY, initialP;
	word initialSP, initialCycles;
	
	const void debugAcc(ppu*, byte &opcode, string OPname);			//Accumulator
	
	const void debugImm(ppu*, byte &opcode, byte data, string OPname); //Immidiate

	const void debugZero(ppu*, byte &opcode,  byte dataPC, byte dataAddress, string OPname); 		//Zeropage

	const void debugZeroX(ppu*, byte &opcode, byte dataPC, word totalAddress, byte dataAddress, string Opname); //Zeropage, X

	const void debugZeroY(ppu*, byte &opcode, byte dataPC, word totalAddress, byte dataAddress, string Opname); //Zeropage, Y

	const void debugAbs(ppu*, byte &opcode, byte highAddress, byte lowAddress, byte data, string OPname);	   //Absolute

	const void debugAbsX(ppu*, byte &opcode, byte highAddress, byte lowAddress, word totalAddress, byte data, string OPname);	//Absolute,X

	const void debugAbsY(ppu*, byte &opcode, byte highAddress, byte lowAddress, word totalAddress, byte data, string OPname);	//Absolute,Y

	const void debugIndirectX(ppu*, byte &opcode, byte pcAddress, word indirectAddress, byte data, string OPname);	//Indirect, X

	const void debugIndirectY(ppu*, byte &opcode, byte pcAddress, word indirectAddress, byte data, string OPname);	//Indirect, Y

	const void debugRelative(ppu*, byte &opcode, char offset, byte dataPC, string OPname);
	
	const void debugImplied(ppu*, byte &opcode, byte BRK, string OPname);

	const void debugIndirect(ppu*, byte &opcode, byte highAddress, byte lowAddress, word jumpAddress, string OPname);
#endif
};

#endif
