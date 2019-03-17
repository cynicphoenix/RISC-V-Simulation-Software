/* Task 2: Data Path & Control Circuitry
All ALU Operations are not added
Stages will be defined later on
Update control Circuitry */

#include <bits/stdc++.h>
#define lli long long int
using namespace std;

lli PC=0;//Program Counter
lli IR;//Instruction Register
lli RA, RB, RZ, RY, RM;//Interstage Buffers
int memory[1 << 22];//Processor Memory

//Register File Class (32-bit registers): Stage 1
class registerFile{
	long long int regArray[32]={0};
	int addressA, addressB, addressC;
	int outputA, outputB, inputC;

	registerFile(){
		regArray[2]=0x7FFFFC;
		regArray[3]=0x100000;
	}
	void readRegFile(){
		outputA=regArray[addressA];
		outputB=regArray[addressB];
	}

	void WriteRegFile(int RF_WRITE){
		if(RF_WRITE==1)
			regArray[addressC]=inputC;
	}
};
//End of class registerFile

//Processor Memory Interface
int readWriteMemory(int MEM_READ, int MEM_WRITE, int address=0, int data=0){
	if(MEM_READ==1)
		return memory[address/4];
	if(MEM_WRITE==1)
		memory[address/4]=data;
	return 0;
}
//End of readWriteMemory

//Arithmetic Logic Unit
int alu(int ALU_OP, int B_SELECT, lli immediate=0){
	lli InA=RA;
	lli InB;
	if(B_SELECT==0)
		InB=RB;
	else
		InB=immediate;
	
	if (ALU_OP == 0)//Add, addi, addw, addiw, all load, all store, 
		RZ = InA + InB;

	else if (ALU_OP == 1)//And, andi
		RZ = InA & InB;

	else if (ALU_OP == 2)//Beq
		return InA==InB;

	else if (ALU_OP == 3)//Bge, bgeu
		return InA>=InB;

	else if (ALU_OP == 4)//Blt, bltu
		return InA<InB;

	else if (ALU_OP == 5)//Bne
		return InA!=InB;

	else if (ALU_OP == 6)//Or
		RZ = InA | InB;

	//incomplete from here:
	else if (ALU_OP == 7)//slli, sll, sllw, slliw
		RZ = InA  InB;

	else if (ALU_OP == 7)//sub, subw
		RZ = InA - InB;

	else if (ALU_OP == 8)//Xor, xori
		RZ = InA ^ InB;
}
//end of ALU function

//Instruction Address Generator
lli iag(int INC_SELECT, int PC_SELECT, int PC_ENABLE, lli immediate=0){
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

int main(){
	string machineLine;
	fstream fileReading;
	fileReading.open("machineCode.mc");
	while (getline(fileReading, machineLine)){


	}
}
