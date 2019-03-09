//Task 1: Assembly Code to Machine Code
#include <bits/stdc++.h>
#include<unordered_map>
#define lli long long int
using namespace std;

//Verified: dec2Binary working correctly
//Function to convert integer number to binary string 
string dec2Binary(lli decimalNum, int length){
	int i=0,l=length;
	string binaryNum="";
	while(l--)
		binaryNum=binaryNum+"0";
    while(decimalNum>0){
       	binaryNum[length-i-1]=decimalNum%2+'0'; 
       	decimalNum=decimalNum/2; 
       	i++; 
    }
    return binaryNum;
}
//End of function dec2Binary

//Verified: createMap working correctly
//Function to create map between binary number and its equivalent hexadecimal 
void createMap(unordered_map<string, char> *um){ 
    (*um)["0000"] = '0'; 
    (*um)["0001"] = '1'; 
    (*um)["0010"] = '2'; 
    (*um)["0011"] = '3'; 
    (*um)["0100"] = '4'; 
    (*um)["0101"] = '5'; 
    (*um)["0110"] = '6'; 
    (*um)["0111"] = '7'; 
    (*um)["1000"] = '8'; 
    (*um)["1001"] = '9'; 
    (*um)["1010"] = 'A'; 
    (*um)["1011"] = 'B'; 
    (*um)["1100"] = 'C'; 
    (*um)["1101"] = 'D'; 
    (*um)["1110"] = 'E'; 
    (*um)["1111"] = 'F'; 
}
//End of function createMap
 
//Verified: bin2Hex working correctly
//Functiom to convert binary string to hexadecimal string 
string bin2Hex(string bin){ 
    int l=bin.size(); 
    int t=bin.find_first_of('.');
    int len_left=t!=-1?t:l;

    for(int i=1;i<=(4-len_left%4)%4;i++) 
        bin='0'+bin;   
    if(t!=-1){
        int len_right=l-len_left-1; 
        for(int i=1;i<=(4-len_right%4)%4;i++) 
            bin=bin+'0';
    }
    unordered_map<string, char> bin_hex_map; 
    createMap(&bin_hex_map); 
      
    int i=0; 
    string hex= "";
    while(1){ 
        hex+=bin_hex_map[bin.substr(i, 4)]; 
        i+=4; 
        if(i==bin.size()) 
        	break;
        if(bin.at(i)=='.'){ 
            hex += '.'; 
            i++; 
        }
    } 
    return hex;     
}
//End of function bin2hex




//Data Values on Stack



//Verified: otherDataFieldRtype working correctly
//Function to extract RS1, RS2 & RD of R type instructions
void otherDataFieldRtype(string &line, string &machineCodeInstructionBinary, string &rs1, string &rs2, string &rd, int i){
	int temp=0;
	while(line[i]!='x')
			i++;
	i++;
	while(line[i]!=',')
		temp=temp*10+(int)(line[i++]-'0');
	rd=dec2Binary(temp, 5);

	temp=0;
	while(line[i]!='x')
		i++;
	i++;
	while(line[i]!=',')
		temp=temp*10+(int)(line[i++]-'0');
	rs1=dec2Binary(temp, 5);

	temp=0;
	while(line[i]!='x')
		i++;
	i++;
	while(line[i]=='0' || line[i]=='1' || line[i]=='2' || line[i]=='3' || line[i]=='4' || line[i]=='5' || line[i]=='6' || line[i]=='7' || line[i]=='8' || line[i]=='9')
		temp=temp*10+(int)(line[i++]-'0');
	rs2=dec2Binary(temp, 5);
}
//End of function otherDataFieldRtype

//Function to extract rs, rd and immediate values
void otherDataFieldItype(string &line, string &rs1, string &immediate, string &rd, int i, int ISubType){
	int temp = 0;
	// read destination register
	while (line[i] != 'x'){
		i++;
	}
	i++;
	while (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
		temp = temp * 10 + (int)(line[i++] - '0');
	rd = dec2Binary(temp, 5);

	temp = 0;
	if(ISubType == 0 || ISubType == 2){ //immediate value

		temp = 0;
		//read source register
		while (line[i] != 'x'){
			i++;
		}
		i++;
		while (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
			temp = temp * 10 + (int)(line[i++] - '0');
		rs1 = dec2Binary(temp, 5);

		// skip non numeric values
		while (line[i]!='0' && line[i] != '1' && line[i] != '2' && line[i] != '3' && line[i] != '4' && line[i] != '5' && line[i] != '6' && line[i] != '7' && line[i] != '8' && line[i] != '9'){
			i++;
		}
		temp = 0;
		// read offset
		while (i < line.size() && (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')){
			// cout<<i<<" "<<line[i]<<endl;
			temp = temp * 10 + (int)(line[i++] - '0');
		}

		if(ISubType == 0)
			immediate = dec2Binary(temp, 12);
		else
			immediate = dec2Binary(temp ,5);
	}
	else if(ISubType == 1){	//with offset
		temp = 0;
		//skip any non numeric character
		while (line[i] != '1' && line[i] != '2' && line[i] != '3' && line[i] != '4' && line[i] != '5' && line[i] != '6' && line[i] != '7' && line[i] != '8' && line[i] != '9' && line[i]!='0'){
			i++;
		}
		//calculate offset
		while (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9')
			temp = temp * 10 + (int)(line[i++] - '0');
		immediate = dec2Binary(temp, 12);

		//read source register
		while(line[i]!='x'){
			i++;
		}
		i++;
		temp = 0;
		
		while (i<line.size() && (line[i] == '0' || line[i] == '1' || line[i] == '2' || line[i] == '3' || line[i] == '4' || line[i] == '5' || line[i] == '6' || line[i] == '7' || line[i] == '8' || line[i] == '9'))
			temp = temp * 10 + (int)(line[i++] - '0');
		rs1 = dec2Binary(temp, 5);

	}
}
//Verified: asm2mc working correctly
/*Assembly to Machine Code
Input: each line of assembly code
Output: equivalent machine code(hexadecimal string)*/
string asm2mc(string line){
	string instruction="";//first word of each assembly line
	string machineCodeInstructionBinary="";
	string machineCodeInstructionHex="";
	string opcode="",funct3="",funct7="";
	string immediate="";
	string rs1="", rs2="", rd=""; 
	string type="";
	int ISubType = 0;//subtypes for I-type instructions 0 for addi etc, 1 for instruction with offset, 2 for shift instructions

	int i=0;
	while(line[i]!=' ')
		instruction+=line[i++];

	if(instruction=="add")
		opcode="0110011", funct3="000", funct7="0000000", type="R";

	else if(instruction=="and")
		opcode="0110011", funct3="111", funct7="0000000", type="R";
		
	else if(instruction=="or")
		opcode="0110011", funct3="110", funct7="0000000", type ="R";

	else if(instruction=="sll")
		opcode="0110011", funct3="001", funct7="0000000", type ="R";
		
	else if(instruction=="slt")
		opcode="0110011", funct3="010", funct7="0000000", type ="R";

	else if(instruction=="sltu")
		opcode="0110011", funct3="011", funct7="0000000", type ="R";

	else if(instruction=="sra")
		opcode="0110011", funct3="101", funct7="0100000", type ="R";
	
	else if(instruction=="srl")
		opcode="0110011", funct3="101", funct7="0000000", type ="R";
	
	else if(instruction=="sub")
		opcode="0110011", funct3="000", funct7="0100000", type ="R";
	
	else if(instruction=="xor")
		opcode="0110011", funct3="100", funct7="0000000", type ="R";
	
	else if(instruction=="mul")
		opcode="0110011", funct3="000", funct7="0000001", type ="R";
		
	else if(instruction=="mulh")
		opcode="0110011", funct3="001", funct7="0000001", type ="R";
		
	else if(instruction=="mulhsu")
		opcode="0110011", funct3="010", funct7="0000001", type ="R";
		
	else if(instruction=="mulhu")
		opcode="0110011", funct3="011", funct7="0000001", type ="R";
	
	else if(instruction=="div")
		opcode="0110011", funct3="100", funct7="0000001", type ="R";
		
	else if(instruction=="divu")
		opcode="0110011", funct3="101", funct7="0000001", type ="R";
		
	else if(instruction=="rem")
		opcode="0110011", funct3="110", funct7="0000001", type ="R";
		
	else if(instruction=="remu")
		opcode="0110011", funct3="111", funct7="0000001", type ="R";
		
	else if(instruction=="addw")
		opcode="0111011", funct3="000", funct7="0000000", type  ="R";
		
	else if(instruction=="subw")
		opcode="0111011", funct3="000", funct7="0100000", type ="R";
		
	else if(instruction=="sllw")
		opcode="0111011", funct3="001", funct7="0000000", type ="R";
		
	else if(instruction=="srlw")
		opcode="0111011", funct3="101", funct7="0000000", type ="R";
		
	else if(instruction=="sraw")
		opcode="0111011", funct3="101", funct7="0100000", type ="R";
	
	else if(instruction == "lb")
		opcode = "0000011", funct3 = "000", type = "I", ISubType = 1;

	else if (instruction == "lh")
		opcode = "0000011", funct3 = "001", type = "I", ISubType = 1;

	else if (instruction == "lw")
		opcode = "0000011", funct3 = "010", type = "I", ISubType = 1;
	
	else if (instruction == "ld")
		opcode = "0000011", funct3 = "011", type = "I", ISubType = 1;

	else if (instruction == "lbu")
		opcode = "0000011", funct3 = "100", type = "I", ISubType = 1;

	else if (instruction == "lhu")
		opcode = "0000011", funct3 = "101", type = "I", ISubType = 1;

	else if (instruction == "lwu")
		opcode = "0000011", funct3 = "110", type = "I", ISubType = 1;

	else if (instruction == "fence")
		opcode = "0001111", funct3 = "000", type = "I";

	else if (instruction == "fence.i")
		opcode = "0001111", funct3 = "001", type = "I";

	else if (instruction == "addi")
		opcode = "0010011", funct3 = "000", type = "I";
	
	else if (instruction == "slli")
		opcode = "0010011", funct3 = "001",funct7 = "0000000", type = "I", ISubType = 2;

	else if (instruction == "srli")
		opcode = "0010011", funct3 = "101", funct7 = "0000000", type = "I", ISubType = 2;

	else if (instruction == "srai")
		opcode = "0010011", funct3 = "001", funct7 = "0100000", type = "I", ISubType = 2;

	else if (instruction == "slliw")
		opcode = "0011011", funct3 = "001", funct7 = "0000000", type = "I", ISubType = 2;

	else if (instruction == "srliw")
		opcode = "0011011", funct3 = "101", funct7 = "0000000", type = "I", ISubType = 2;

	else if (instruction == "sraiw")
		opcode = "0011011", funct3 = "101", funct7 = "0100000", type = "I", ISubType = 2;

	else if (instruction == "slti")
		opcode = "0010011", funct3 = "001", type = "I";

	else if (instruction == "sltiu")
		opcode = "0010011", funct3 = "011", type = "I";

	else if (instruction == "xori")
		opcode = "0010011", funct3 = "100", type = "I";

	else if (instruction == "slti")
		opcode = "0010011", funct3 = "001", type = "I";

	else if (instruction == "ori")
		opcode = "0010011", funct3 = "110", type = "I";
	
	else if (instruction == "andi")
		opcode = "0010011", funct3 = "111", type = "I";

	else if (instruction == "addiw")
		opcode = "0010011", funct3 = "000", type = "I";

	else if (instruction == "sd")
		opcode = "0010011", funct3 = "011", type = "I";

	else if (instruction == "jalr")
		opcode = "1100111", funct3 = "000", type = "I", ISubType = 1;

	else if (instruction == "CSRRW")
		opcode = "1110011", funct3 = "001", type = "I";

	else if (instruction == "CSRRS")
		opcode = "1110011", funct3 = "010", type = "I";

	else if (instruction == "CSRRC")
		opcode = "1110011", funct3 = "011", type = "I";

	else if (instruction == "CSRRWI")
		opcode = "1110011", funct3 = "101", type = "I";

	else if (instruction == "CSRRSI")
		opcode = "1110011", funct3 = "110", type = "I";

	else if (instruction == "CSRRCI")
		opcode = "1110011", funct3 = "111", type = "I";
	
	else if (instruction == "ecall")
		opcode = "1110011", funct3 = "000", immediate = "000000000000", type = "I";

	else if (instruction == "ebreak")
		opcode = "1110011", funct3 = "000", immediate = "000000000001", type = "I";

	else
	{

	}
	if(type=="R")
	{
		otherDataFieldRtype(line, machineCodeInstructionBinary, rs1, rs2, rd, i);
		machineCodeInstructionBinary=funct7+rs2+rs1+funct3+rd+opcode;
	}

	else if(type == "I"){
		if(instruction == "ecall"){
			machineCodeInstructionBinary = "00000000000000000000000001110011";
		}

		else if(instruction == "ebreak"){
			machineCodeInstructionBinary = "00000000000100000000000001110011";
		}
		else{
			otherDataFieldItype(line, rs1, immediate, rd, i, ISubType);
			if(ISubType == 0 || ISubType == 1){
				machineCodeInstructionBinary = immediate + rs1 + funct3 + rd + opcode;
			}
			else if(ISubType == 2){
				machineCodeInstructionBinary = funct7 + immediate + rs1 + funct3 + rd + opcode;
			}
		}
		
	}

	machineCodeInstructionHex="0x"+bin2Hex(machineCodeInstructionBinary);
	return machineCodeInstructionHex;
}
//End of function asm2mc

////Verified: main input-output stream working correctly
//Main File : File Read & Write
int main(){
	lli instructionAddress=0;
	string hexInstructionAddress;
	string binaryInstructionAddress;
	string assemblyLine;
	string machineLine;

	fstream fileReading;
	fstream fileWriting;
	fileReading.open("assemblyCode.asm");
	fileWriting.open("sample_out.mc");

	//To read input from Assembly Code File 
	while(getline(fileReading, assemblyLine)){
		try{
		machineLine=asm2mc(assemblyLine);
		}
		catch(exception e){
			cout<<"HEY2";
		}
		binaryInstructionAddress=dec2Binary(instructionAddress, 32);
		instructionAddress+=4;
		hexInstructionAddress="0x"+bin2Hex(binaryInstructionAddress);
		machineLine=hexInstructionAddress+" "+machineLine;
		fileWriting<<machineLine<<endl;
	}
	fileWriting.close();
	fileReading.close();
}
//End of main
