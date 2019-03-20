/* Task 2: Data Path & Control Circuitry
All ALU Operations are not added
Stages will be defined later on
Update control Circuitry */

#include <bits/stdc++.h>
#define lli long long int
using namespace std;

#define OPCODE_I1 3 	//for load
#define OPCODE_I2 19	// for shift, ori, andi
#define OPCODE_U1 23	// for auipc
#define OPCODE_I3 27	// for shiftw and addiw
#define OPCODE_S1 35	//for sd, sw, sl, sh
#define OPCODE_R1 51 	//for add, sub, and etc
#define OPCODE_U2 55	// for lui
#define OPCODE_R2 59	// for addw, subw etc
#define OPCODE_SB1 99	//for branch jump
#define OPCODE_UJ  111	//for jal
#define OPCODE_I4	103 //for jalr

#define f3_0 0
#define f3_1 1
#define f3_2 2
#define f3_3 3
#define f3_4 4
#define f3_5 5
#define f3_6 6
#define f3_7 7

#define f7_0 0
#define f7_1 32

lli memory[1 << 22]; //Processor Memory
lli regArray[32] = {0};
lli PC=0;//Program Counter
lli IR;//Instruction Register
lli RA, RB, RZ, RY, RM;//Interstage Buffers
lli addressA, addressB;
lli immediate; // for immediate values
lli addressC; //destination register
lli returnAddress;//Return Address in case of jal/jalr
int ALU_OP, B_SELECT;
int MEM_READ;	
int MEM_WRITE;


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
void decode()
{
	int opcode = IR << 25;
	opcode >>= 25;
	int funct3 = IR << 17;
	funct3 >>= 29;
	int funct7 = IR >> 25;
	if(opcode == OPCODE_I1){
		int imm = IR >> 20;
		int rs1 = IR << 12;
		rs1 >>= 27;
		int rd = IR << 20;
		rd >>= 27;
		B_SELECT = 1;
		ALU_OP = 0;
		addressA = rs1;
		immediate = imm;
		addressC = rd;
		MEM_READ = 1;
		MEM_WRITE = 0;
		if(funct3 == 0){

		}

		else if (funct3 == 1){
		
		}

		else if (funct3 == 2){
		
		}

		else if (funct3 == 3){

		}

		else if (funct3 == 4){

		}

		else if (funct3 == 5){

		}

		else if (funct3 == 6){

		}
	}

	else if(opcode == OPCODE_I2){

	}

	else if(opcode == OPCODE_I3){

	}

	else if(opcode == OPCODE_I4){
		int imm = IR >> 20;
		int rs1 = IR << 12;
		rs1 >>= 27;
		int rd = IR << 20;
		rd >>= 27;

		B_SELECT = 1;
		ALU_OP = 10;
		addressA = rs1;
		immediate = imm;
		addressC = rd;
		MEM_READ = 0;
		MEM_WRITE = 0;
	}

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
		RZ = InA;

	else if (ALU_OP == 7) //sub, subw
		RZ = InA - InB;

	else if (ALU_OP == 8) //Xor, xori
		RZ = InA ^ InB;
	// for jalr ALU_OP = 10
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
