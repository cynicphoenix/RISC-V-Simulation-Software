/* Task 2: Data Path & Control Circuitry
All ALU Operations are not added
Stages will be defined later on
Update control Circuitry */

#include <bits/stdc++.h>
#define lli long long int
using namespace std;

#define OPCODE_I1 3   //for load
#define OPCODE_I2 19  // for shift, ori, andi
#define OPCODE_I3 27  // for shiftw and addiw
#define OPCODE_I4 103 //for jalr

#define OPCODE_U1 23 // for auipc
#define OPCODE_U2 55 // for lui

#define OPCODE_S1 35 //for sd, sw, sl, sh

#define OPCODE_R1 51 //for add, sub, and etc
#define OPCODE_R2 59 // for addw, subw etc

#define OPCODE_SB1 99 //for branch jump

#define OPCODE_UJ 111 //for jal

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

char memory[1 << 22]; //Processor Memory
lli regArray[32] = {0};
lli PC = 0;				//Program Counter
lli IR;					//Instruction Register
lli RA, RB, RZ, RY, RM; //Interstage Buffers
lli addressA, addressB;
lli immediate;	 // for immediate values
lli addressC;	  //destination register
lli returnAddress; //Return Address in case of jal/jalr
int ALU_OP, B_SELECT, PC_SELECT, INC_SELECT;
int MEM_READ;
int MEM_WRITE;
int RF_WRITE;

//Call in decode stage & Writeback Stage
void readWriteRegFile(int RF_WRITE, int addressA, int addressB, int addressC)
{
	if (RF_WRITE == 1)
	{
		regArray[addressC] = RY;
		return;
	}
	RA = regArray[addressA];
	RB = regArray[addressB];
}
//End of readWriteRegFile

//Processor Memory Interface
lli readWriteMemory(int MEM_READ, int MEM_WRITE, int address = 0, lli data_w = 0)
{
	if (MEM_READ > 0)
	{
		if (MEM_READ == 1) //lb
			return memory[address];
		else if (MEM_READ == 2)
		{ //lh
			int data = memory[address];
			data += (int)memory[address + 1] << 8;
			return data;
		}
		else if (MEM_READ == 3)
		{ //lw
			int data = memory[address];
			data += (int)memory[address + 1] << 8;
			data += (int)memory[address + 2] << 16;
			data += (int)memory[address + 3] << 24;
			return data;
		}
		else if (MEM_READ == 4)
		{ //ld
			lli data = memory[address];
			data += (lli)memory[address + 1] << 8;
			data += (lli)memory[address + 2] << 16;
			data += (lli)memory[address + 3] << 24;
			data += (lli)memory[address + 3] << 32;
			data += (lli)memory[address + 3] << 40;
			data += (lli)memory[address + 3] << 48;
			data += (lli)memory[address + 3] << 56;
			return data;
		}
	}
	if (MEM_READ == 1)
	{ //sb
		memory[address] = data_w;
	}
	else if (MEM_WRITE == 2)
	{ //sh
		memory[address] = data_w && (1 << 8 - 1);
		memory[address + 1] = data_w >> 8;
	}
	else if (MEM_WRITE == 3)
	{ //sw
		int setb8 = (1 << 8) - 1;
		memory[address] = data_w & setb8;
		memory[address + 1] = (data_w & (setb8 << 8)) >> 8;
		memory[address + 2] = (data_w & (setb8 << 16)) >> 16;
		memory[address + 3] = data_w >> 24;
	}
	else if (MEM_WRITE == 4)
	{ //sd
		lli setb8 = 1 << 8 - 1;
		memory[address] = data_w & setb8;
		memory[address + 1] = (data_w & (setb8 << 8)) >> 8;
		memory[address + 2] = (data_w & (setb8 << 16)) >> 16;
		memory[address + 3] = (data_w & (setb8 << 24)) >> 24;
		memory[address + 4] = (data_w & (setb8 << 32)) >> 32;
		memory[address + 5] = (data_w & (setb8 << 16)) >> 40;
		memory[address + 6] = (data_w & (setb8 << 48)) >> 48;
		memory[address + 7] = data_w >> 56;
	}
	return 0;
}
//End of readWriteMemory

/*Instruction Address Generator
returns return Address*/
lli iag(int INC_SELECT, int PC_SELECT, lli immediate = 0)
{
	lli PC_Temp = PC + 4;
	if (PC_SELECT == 0)
		PC = RA;
	else
	{
		if (INC_SELECT == 1)
			PC = PC + immediate;
		else
			PC = PC + 4;
	}
	return PC_Temp;
}
//End of function IAG

//Stage 1: Fetch Stage
void fetch()
{
	IR = readWriteMemory(1, 0, PC);
	returnAddress = iag(0, 1);
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
	PC_SELECT = 1;
	INC_SELECT = 0;
	if (opcode == OPCODE_I1)
	{
		RF_WRITE = 1;
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
		MEM_READ = 3;
		MEM_WRITE = 0;
	}

	else if (opcode == OPCODE_I2)
	{
		RF_WRITE = 1;
		int imm = IR >> 20;
		int rs1 = IR << 12;
		rs1 >>= 27;
		int rd = IR << 20;
		rd >>= 27;

		B_SELECT = 1;
		addressA = rs1;
		immediate = imm;
		addressC = rd;
		MEM_READ = 3;
		MEM_WRITE = 0;

		if (funct3 == 0)
		{
			ALU_OP = 0;
		}

		else if (funct3 == 1)
		{
			int shamt = IR << 7;
			shamt >>= 27;
			immediate = shamt;
			ALU_OP = 7;
		}

		else if (funct3 == 2)
		{
			ALU_OP = 7;
		}

		else if (funct3 == 3)
		{
			ALU_OP = 7;
		}

		else if (funct3 == 4)
		{
			ALU_OP = 8;
		}

		else if (funct3 == 5)
		{
			int shamt = IR << 7;
			shamt >>= 27;
			immediate = shamt;
			ALU_OP = 7;
		}

		else if (funct3 == 6)
		{
			ALU_OP = 6;
		}

		else if (funct3 == 7)
		{
			ALU_OP = 1;
		}
	}

	else if (opcode == OPCODE_I3)
	{
		RF_WRITE = 1;
		addressA = IR << 12;
		addressA >>= 27;
		addressC = IR << 20;
		addressC >>= 27;
		MEM_READ = 0;
		MEM_WRITE = 0;
		int shamt = IR << 7;
		shamt >>= 27;

		B_SELECT = 1;

		if (funct3 == 0)
		{
			ALU_OP = 0;
			immediate = IR >> 20;
		}

		else if (funct3 == 1)
		{
			ALU_OP = 7;
			immediate = shamt;
		}

		else if (funct3 == 5)
		{
			immediate = shamt;
			ALU_OP = 7;
		}
	}

	else if (opcode == OPCODE_I4)
	{ //for jalr
		RF_WRITE = 1;
		int imm = IR << 20;
		int rs1 = IR << 12;
		rs1 >>= 27;
		int rd = IR << 20;
		rd >>= 27;
		PC_SELECT = 0;
		B_SELECT = 1;
		ALU_OP = 10;
		addressA = rs1;
		immediate = imm;
		addressC = rd;
		MEM_READ = 0;
		MEM_WRITE = 0;
	}

	else if (opcode == OPCODE_S1)
	{
		int imm1 = IR << 20;
		imm1 >>= 27;
		int imm2 = IR >> 25;
		immediate = imm1 + (imm2 << 5);

		addressB = IR << 7;
		addressB >>= 27;

		addressA = IR << 12;
		addressA = IR >> 27;

		if (funct3 == 0)
			MEM_WRITE = 1;
		else if (funct3 == 1)
			MEM_WRITE = 2;
		else if (funct3 == 2)
			MEM_WRITE = 3;
		else if (funct3 == 3)
			MEM_WRITE = 4;
		MEM_READ = 0;
		B_SELECT = 1;
		RF_WRITE = 0;
		ALU_OP = 11;
	}

	else if (opcode == OPCODE_U1)
	{
		immediate = IR >> 11;

		addressC = IR << 20;
		addressC >>= 27;

		B_SELECT = 1;
		MEM_READ = 0;
		MEM_WRITE = 0;
		RF_WRITE = 1;
		ALU_OP = 12;
	}

	else if (opcode == OPCODE_U2)
	{
		immediate = IR >> 11;

		addressC = IR << 20;
		addressC >>= 27;

		ALU_OP = 13;
		B_SELECT = 1;
		MEM_READ = 0;
		MEM_WRITE = 0;
		RF_WRITE = 1;
	}

	else if (opcode == OPCODE_R1 || opcode == OPCODE_R2)
	{
		int rs1 = IR << 12;
		rs1 >>= 27;
		int rs2 = IR << 7;
		rs2 <<= 27;
		int rd = IR << 20;
		rd >>= 27;
		RF_WRITE = 1;
		B_SELECT = 0;
		ALU_OP = 0;
		addressA = rs1;
		addressB = rs2;
		addressC = rd;
		MEM_READ = 0;
		MEM_WRITE = 0;
	}

	else if (opcode == OPCODE_UJ)
	{
		int rd = IR << 20;
		rd >>= 27;
		addressC = rd;
		int tmp1 = IR >> 12;
		int bit_20 = tmp1 && (1 << 19);
		int bit_1_10 = (tmp1 >> 9) && (1 << 11 - 1);
		int bit_11 = (tmp1 && (1 << 8)) << 2;
		int bit_19_12 = (tmp1 << 12) >> 1;
		immediate = bit_1_10 | bit_11 | bit_19_12 | bit_20;
		RF_WRITE = 1;
		B_SELECT = 0;
		INC_SELECT = 1;
		ALU_OP = -1;
		addressA = 0;
		addressB = 0;
		MEM_READ = 0;
		MEM_WRITE = 0;
	}

	else if (opcode == OPCODE_SB1)
	{
		int rs1 = IR << 12;
		rs1 >>= 27;
		int rs2 = IR << 7;
		rs2 <<= 27;
		int rd = IR << 20;
		rd >>= 27;
		int bit_11 = (rd & 1) << 10;
		int bit_1_4 = rd >> 1;
		int bit_12 = (funct7 >> 6) << 11;
		int bit_5_10 = (funct7 - bit_12 << 6) << 4;
		int immediate = bit_1_4 | bit_5_10 | bit_11 | bit_12;
		RF_WRITE = 0;
		B_SELECT = 0;
		INC_SELECT = 1;
		if (funct3 == 5 || funct3 == 7)
			ALU_OP = 3;
		else if (funct3 == 4 || funct3 == 6)
			ALU_OP = 4;
		else if (funct3 == 0)
			ALU_OP = 2;
		else if (funct3 == 1)
			ALU_OP = 5;
		addressA = rs1;
		addressB = rs2;
		addressC = 0;
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
	// for sd, sw, sh, sb ALU_OP = 11
	// for auipc use ALU_OP = 12
	// for lui use ALU_op = 13
}
//end of ALU function

/*Stage 4: Memory & RY get updated
Input: Y_SELECT, MEM_READ, MEM_WRITE, address from RZ/RM, data*/
void memoryStage(int Y_SELECT, int MEM_READ, int MEM_WRITE, int address = 0, int data = 0)
{
	int dataFromMem = readWriteMemory(MEM_READ, MEM_WRITE, address, data);
	if (Y_SELECT == 0)
		RY = RZ;
	if (Y_SELECT == 1)
		RY = dataFromMem;
	if (Y_SELECT == 2)
		RY = returnAddress;
}
//End of memoryStage

//Stage 5: WriteBack
void writeBack(int RF_WRITE, int addressC)
{
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
	fileReading.open("machineData.txt");
	while (getline(fileReading, machineLine))
	{
		lli value=0, address=0;
		string type="";
		int i=0;
		while(machineLine[i]!=' ')
			value=value*10+(machineLine[i++]-'0');
		i=i+3;
		while(machineLine[i]!=' ')
			address=address*16+(machineLine[i++]-'0');
		i++;
	
		while(i<machineLine.length() && machineLine[i]!=' ')
			type+=machineLine[i++];
		if(type=="byte")	
        	readWriteMemory(0,1,address,value);
		else if(type=="halfword")
			readWriteMemory(0,2,address,value);
		else if(type=="word")
			readWriteMemory(0,3,address,value);
		else if(type=="doubleword")
			readWriteMemory(0,4,address,value);
		//store instructions & data in memory
		//incomplete
	}

}
