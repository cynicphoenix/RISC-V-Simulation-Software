//Task 1: Assembly Code to Machine Code
#include <bits/stdc++.h>
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

//Verified: asm2mc working correctly
/*Assembly to Machine Code
Input: each line of assembly code
Output: equivalent machine code(hexadecimal string)*/
string asm2mc(string line){
	string instruction="";//first word of each assembly line
	string type="";//instruction type
	string machineCodeInstructionBinary="";
	string machineCodeInstructionHex="";
	string opcode="",funct3="",funct7="";
	string immediate="";
	string rs1="", rs2="", rd=""; 

	int i=0;
	while(line[i]!=' ')
		instruction+=line[i++];
	
	//match instruction to write predefined values like opcode, funct7 etc
	//all R types here:
	if(instruction=="add")
		opcode="0110011", funct3="000", funct7="0000000", type="R";
	else if(instruction=="sub")
		opcode="", funct3="", funct7="", type="";
	
	
	
	
	//all I type here:
	
	
	
	
	
	
	
	//match instruction type to get other data fields like rs1, rs2, rd etc
	if(type=="R"){
		otherDataFieldRtype(line, machineCodeInstructionBinary, rs1, rs2, rd, i);
		machineCodeInstructionBinary=funct7+rs2+rs1+funct3+rd+opcode;
	}
	else if(type=="I"){
		
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
	fileWriting.open("machineCode.mc");

	//To read input from Assembly Code File 
	while(getline(fileReading,assemblyLine)){
		machineLine=asm2mc(assemblyLine);
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
