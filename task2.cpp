/* Task 2: Data Path & Control Circuitry
All ALU Operations are not added
Stages will be defined later on
Update control Circuitry */

#include <bits/stdc++.h>
#define lli long long int
using namespace std;

lli memory[1 << 22]; //Processor Memory
lli regArray[32] = {0};
lli PC=0;//Program Counter
lli IR;//Instruction Register
lli RA, RB, RZ, RY, RM;//Interstage Buffers
lli returnAddress;//Return Address in case of jal/jalr

//Call in decode stage & Writeback Stage
void readWriteRegFile(int RF_WRITE, int addressA, int addressB, int addressC)
{
	if (RF_WRITE == 1){
		regArray[addressC] = RY;
		return;
	}
	RA = regArray[addressA];
	RB = regArray[addressB];
}
//End of readWriteRegFile

//Processor Memory Interface
int readWriteMemory(int MEM_READ, int MEM_WRITE, int address = 0, int data = 0)
{
	if (MEM_READ == 1)
		return memory[address / 4];
	if (MEM_WRITE == 1)
		memory[address / 4] = data;
	return 0;
}
//End of readWriteMemory

/*Instruction Address Generator
returns return Address*/
lli iag(int INC_SELECT, int PC_SELECT, lli immediate=0){
	lli PC_Temp=PC+4;
	if(PC_SELECT==0)
		PC=RA;
	else{	
		if(INC_SELECT==1)
			PC=PC+immediate;
		else
			PC=PC+4;
	}
	return PC_Temp;
}
//End of function IAG

//Stage 1: Fetch Stage
void fetch(){
	IR=readWriteMemory(1, 0, PC);
	returnAddress=iag(0, 1);
}
//end of fetch


/*Task to be completed by Soumil & Deepak
Stage 2: Decode Stage
RA & RB will be updated after this stage 
*/
void decode(){


}
//End of decode

/* Arithmetic Logic Unit(not complete)
Input: ALU_OP, MUXB select, immediate(if any)
(these input will be provided by decode stage)
Result:
Update RZ
Output: only for branch instruction
branch taken:1
branch not taken:0 */
int alu(int ALU_OP, int B_SELECT, lli immediate = 0)
{
	lli InA = RA;
	lli InB;
	if (B_SELECT == 0)
		InB = RB;
	else
		InB = immediate;

	if (ALU_OP == 0) //Add, addi, addw, addiw, all load, all store,
		RZ = InA + InB;

	else if (ALU_OP == 1) //And, andi
		RZ = InA & InB;

	else if (ALU_OP == 2) //Beq
		return InA == InB;

	else if (ALU_OP == 3) //Bge, bgeu
		return InA >= InB;

	else if (ALU_OP == 4) //Blt, bltu
		return InA < InB;

	else if (ALU_OP == 5) //Bne
		return InA != InB;

	else if (ALU_OP == 6) //Or
		RZ = InA | InB;

	//incomplete from here:
	else if (ALU_OP == 7) //slli, sll, sllw, slliw
		RZ = InA InB;

	else if (ALU_OP == 7) //sub, subw
		RZ = InA - InB;

	else if (ALU_OP == 8) //Xor, xori
		RZ = InA ^ InB;
}
//end of ALU function

/*Stage 4: Memory & RY get updated
Input: Y_SELECT, MEM_READ, MEM_WRITE, address from RZ/RM, data*/
void memoryStage(int Y_SELECT, int MEM_READ, int MEM_WRITE, int address = 0, int data = 0)
{
	int dataFromMem = readWriteMemory(MEM_READ, MEM_WRITE, address, data);
	if(Y_SELECT==0)
		RY=RZ;
	if(Y_SELECT==1)
		RY=dataFromMem;
	if(Y_SELECT==2)
		RY=returnAddress;
}
//End of memoryStage

//Stage 5: WriteBack
void writeBack(int RF_WRITE, int addressC){
	readWriteRegFile(RF_WRITE, 0, 0, addressC);
}
//End of writeBack

//main function
int main()
{
	//initialize x2, x3
	regArray[2] = 0x7FFFFC;
	regArray[3] = 0x100000;

	string machineLine;
	string machineCode;
	fstream fileReading;
	fileReading.open("machineCode.mc");
	while (getline(fileReading, machineLine))
	{
		//store instructions & data in memory
		//incomplete
	}
}
