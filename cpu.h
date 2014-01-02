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

	//Functions
	bool setPointers(memory*, ppu*);			//Set pointers to the object
	void setPCStart();					//Sets the PC to the reset vector
	void printDebug();					//Prints opcode, PC, registers
	byte emulateCycle();

	//Variables
	word PC, SP;	//Stack pointer can be between 0x0100 and 0x01FF in memory
	byte A, X, Y;

	/* C = Carry flag Z = Zero flag  I = Interrupt disable D = decimal mode
	V = Overflow flag N = Negative flag*/
	bool C, Z, I, D, V, N;
	static byte const instr_lens[];
	static byte const cycleCount[];
	
private:
	memory* RAM;							//Pointer to system memory
	ppu* video;							//Pointer to ppu

	//Private variables
	byte initialA;
	byte cycles, opcode;

	//Functions
	const void pushStack(byte&);
	const byte popStack();
	const void compareFlags(byte&, byte);
	const byte encodeBits(bool bools[8]);
	const void decodeBits(bool (&bools)[8], byte input);
	const void pageBoundry(word, word&);
	const void pageBranch(char&);

	//Get address functions
	const word zeroPageX();
	const word zeroPageY();
	const word absolute();
	const word absoluteX();
	const word absoluteY();
	const word indirectX();
	const word indirectY();

	const void NMI();

	//Debug functions
	//DataPC = data from the PC counter memory address.  dataAddress = data from the PC counters addresses given
	//in dataPC.

#ifdef debugOn
	ofstream debugFile;
	byte initialX, initialY, initialP;
	word initialSP, initialCycles;
	
	const void debugAcc( byte &opcode, string OPname);			//Accumulator
	
	const void debugImm(byte &opcode, byte data, string OPname); //Immidiate

	const void debugZero(byte &opcode,  byte dataPC, byte dataAddress, string OPname); 		//Zeropage

	const void debugZeroX(byte &opcode, byte dataPC, word totalAddress, byte dataAddress, string Opname); //Zeropage, X

	const void debugZeroY(byte &opcode, byte dataPC, word totalAddress, byte dataAddress, string Opname); //Zeropage, Y

	const void debugAbs(byte &opcode, byte highAddress, byte lowAddress, byte data, string OPname);	   //Absolute

	const void debugAbsX(byte &opcode, byte highAddress, byte lowAddress, word totalAddress, byte data, string OPname);	//Absolute,X

	const void debugAbsY(byte &opcode, byte highAddress, byte lowAddress, word totalAddress, byte data, string OPname);	//Absolute,Y

	const void debugIndirectX(byte &opcode, byte pcAddress, word indirectAddress, byte data, string OPname);	//Indirect, X

	const void debugIndirectY(byte &opcode, byte pcAddress, word indirectAddress, byte data, string OPname);	//Indirect, Y

	const void debugRelative(byte &opcode, char offset, byte dataPC, string OPname);
	
	const void debugImplied(byte &opcode, byte BRK, string OPname);

	const void debugIndirect(byte &opcode, byte highAddress, byte lowAddress, word jumpAddress, string OPname);
#endif
};

#endif
