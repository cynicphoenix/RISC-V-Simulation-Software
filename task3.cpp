// Task 3: Pipelining

#include <bits/stdc++.h>
#define lli long long int
using namespace std;

struct Buffer_IF_ID{
    unsigned int PC;
    int IR;
    Buffer_IF_ID(){
        PC = 0;
        IR = 0;
    }
};

struct Buffer_ID_EX{
    unsigned int PC;
    int RA;
    int RB;
    int addressC;
    int immediate;
    int ALU_OP, B_SELECT, PC_SELECT, INC_SELECT, Y_SELECT;
    int MEM_READ;
    int MEM_WRITE;
    int RF_WRITE;
    int addressA, addressB;
    unsigned int returnAddress; 
};

struct Buffer_EX_MEM{
    unsigned int PC;
    int RA;
    int RB;
    int RZ;
    int addressC;
    int immediate;
    int PC_SELECT, INC_SELECT, Y_SELECT;
    int MEM_READ;
    int MEM_WRITE;
    int RF_WRITE;
    int addressA, addressB;
    unsigned int returnAddress;
};

struct Buffer_MEM_WB{
    int addressC;
    int RF_WRITE;
    int RY;
};

Buffer_IF_ID buffer_IF_ID;
Buffer_ID_EX buffer_ID_EX;
Buffer_EX_MEM buffer_EX_MEM;
Buffer_MEM_WB buffer_MEM_WB;

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

#define FETCH_STAGE 1
#define DECODE_STAGE 2
#define EXECUTE_STAGE 3
#define MEM_STAGE 4
#define WB_STAGE 5

unsigned char memory[1 << 24]; //Processor Memory
int regArray[32] = {0};
int cycleCount = 0;

//Call in decode stage & Writeback Stage
void readWriteRegFile(int stage)
{
    if(stage == WB_STAGE){
        if (buffer_MEM_WB.RF_WRITE == 1)
        {
            if (buffer_MEM_WB.addressC)
                regArray[buffer_MEM_WB.addressC] = buffer_MEM_WB.RY;
            return;
        }
    }
    if(stage == DECODE_STAGE){
        buffer_ID_EX.RA = regArray[buffer_ID_EX.addressA];
        buffer_ID_EX.RB = regArray[buffer_ID_EX.addressB];
    }
}
//End of readWriteRegFile

//Processor Memory Interface
int readWriteMemory(int MEM_READ, int MEM_WRITE, int address = 0, int data_w = 0)
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
    }
    if (MEM_WRITE == 1)
    { //sb
        memory[address] = data_w;
    }
    else if (MEM_WRITE == 2)
    { //sh
        memory[address] = data_w & ((1 << 8) - 1);
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
    return 0;
}
//End of readWriteMemory

/*Instruction Address Generator
returns returnAddress*/
lli iag(int stage, int INC_SELECT = 0, int PC_SELECT = 0, lli immediate = 0){
    if(stage == FETCH_STAGE){        //call from fetch
        return buffer_IF_ID.PC + 4;
    }

    if(stage == EXECUTE_STAGE){                   //call from alu
        lli PC_Temp = buffer_ID_EX.PC + 4;
        if (PC_SELECT == 0)
            buffer_ID_EX.PC = buffer_EX_MEM.RZ;
        else
        {
            if (INC_SELECT == 1)
                buffer_ID_EX.PC = buffer_ID_EX.PC + immediate;
            else
                buffer_ID_EX.PC = buffer_ID_EX.PC + 4;
        }
        return PC_Temp;
    }
}

//End of function iag

//Stage 1: Fetch Stage
void fetch()
{
    buffer_IF_ID.IR = readWriteMemory(3, 0, buffer_IF_ID.PC);
    buffer_IF_ID.PC = iag(FETCH_STAGE);
//    cout << hex << buffer_IF_ID.IR <<" "<<buffer_IF_ID.PC << dec << endl;
}
//end of fetch

/* Stage 2: Decode Stage
RA & RB will be updated after this stage */
void decode()
{

    int addressA, addressB = 0, addressC;
    int IR = buffer_IF_ID.IR;
    unsigned int PC = buffer_IF_ID.PC;
    unsigned int returnAddress; //Return Address in case of jal/jalr
    int ALU_OP, B_SELECT, PC_SELECT, INC_SELECT, Y_SELECT;
    int MEM_READ;
    int MEM_WRITE;
    int RF_WRITE;
    int immediate;              // for immediate values
    unsigned int opcode = IR << 25;
    opcode >>= 25;
    unsigned int funct3 = IR << 17;
    funct3 >>= 29;
    unsigned int funct7 = IR >> 25;
    PC_SELECT = 1;
    INC_SELECT = 0;
    Y_SELECT = 0;
    if (opcode == OPCODE_I1)
    {
        RF_WRITE = 1;
        int imm = IR >> 20;
        unsigned int rs1 = IR << 12;
        rs1 >>= 27;
        unsigned int rd = IR << 20;
        rd >>= 27;
        B_SELECT = 1;
        ALU_OP = 0;
        addressA = rs1;
        immediate = imm;
        addressC = rd;
        Y_SELECT = 1;
        if (funct3 == 0)
            MEM_READ = 1;
        else if (funct3 == 1)
            MEM_READ = 2;
        else if (funct3 == 2)
            MEM_READ = 3;
        else if (funct3 == 3)
            MEM_READ = 4;
        MEM_WRITE = 0;
    }

    else if (opcode == OPCODE_I2)
    {
        RF_WRITE = 1;
        int imm = IR >> 20;
        unsigned int rs1 = IR << 12;
        rs1 >>= 27;
        unsigned int rd = IR << 20;
        rd >>= 27;

        B_SELECT = 1;
        addressA = rs1;
        immediate = imm;
        addressC = rd;
        MEM_READ = 3;
        MEM_WRITE = 0;
        if (funct3 == 0) ///addi
        {
            ALU_OP = 0;
        }

        else if (funct3 == 1) //slli
        {
            unsigned int shamt = IR << 7;
            shamt >>= 27;
            immediate = shamt;
            ALU_OP = 7;
        }

        else if (funct3 == 2) //slti
        {
            ALU_OP = 20;
        }

        else if (funct3 == 3) //sltiu
        {
            ALU_OP = 20;
        }

        else if (funct3 == 4) //xori
        {
            ALU_OP = 8;
        }

        else if (funct3 == 5) //srli
        {
            unsigned int shamt = IR << 7;
            shamt >>= 27;
            immediate = shamt;
            if (funct7 == 0)
                ALU_OP = 19;
            else
                ALU_OP = 21;
        }

        else if (funct3 == 6) //ori
        {
            ALU_OP = 6;
        }

        else if (funct3 == 7) //andi
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
        unsigned int shamt = IR << 7;
        shamt >>= 27;

        B_SELECT = 1;

        if (funct3 == 0) //addiw
        {
            ALU_OP = 0;
            immediate = IR >> 20;
        }

        else if (funct3 == 1) //slliw
        {
            ALU_OP = 7;
            immediate = shamt;
        }

        else if (funct3 == 5) //srliw,sraiw
        {
            immediate = shamt;
            if (funct7 == 0)
                ALU_OP = 19;
            else
                ALU_OP = 21;
        }
    }

    else if (opcode == OPCODE_I4)
    { //for jalr
        RF_WRITE = 1;
        int imm = IR >> 20;
        unsigned int rs1 = IR << 12;
        rs1 >>= 27;
        unsigned int rd = IR << 20;
        rd >>= 27;
        PC_SELECT = 0;
        B_SELECT = 1;
        Y_SELECT = 2;
        ALU_OP = 22;
        addressA = rs1;
        immediate = imm;
        addressC = rd;
        MEM_READ = 0;
        MEM_WRITE = 0;
    }

    else if (opcode == OPCODE_S1) //store
    {

        int tmp = (1 << 5) - 1;
        tmp <<= 7;
        int imm1 = IR & tmp;
        imm1 >>= 7;
        int imm2 = IR >> 25;
        immediate = imm1 + (imm2 << 5);

        addressB = IR << 7;
        addressB >>= 27;

        addressA = IR << 12;
        addressA >>= 27;
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
        ALU_OP = 0;
    }

    else if (opcode == OPCODE_U1) //auipc
    {
        immediate = IR >> 12;

        addressC = IR << 20;
        addressC >>= 27;

        B_SELECT = 1;
        MEM_READ = 0;
        MEM_WRITE = 0;
        RF_WRITE = 1;
        ALU_OP = 12;
    }

    else if (opcode == OPCODE_U2) //lui
    {
        immediate = IR >> 12;

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
        unsigned int rs1 = IR << 12;
        rs1 >>= 27;
        unsigned int rs2 = IR << 7;
        rs2 >>= 27;
        unsigned int rd = IR << 20;
        rd >>= 27;
        RF_WRITE = 1;
        B_SELECT = 0;
        if (funct3 == 0)
        {
            if (funct7 == 0)
                ALU_OP = 0; //add,addw
            else if (funct7 == 1)
                ALU_OP = 25; //mul
            else
                ALU_OP = 18; //sub,subw
        }

        else if (funct3 == 1)
            ALU_OP = 7; //sll,sllw

        else if (funct3 == 2)
            ALU_OP = 20; //slt

        else if (funct3 == 3)
            ALU_OP = 20; //sltu

        else if (funct3 == 4)
        {
            if (funct7 == 1)
                ALU_OP = 29; //div
            else
                ALU_OP = 8; //xor
        }

        else if (funct3 == 5)
        {
            if (funct7 == 0)
                ALU_OP = 19; //srl,srlw
            else if (funct7 == 1)
                ALU_OP = 30; // divu
            else
                ALU_OP = 21; //sra,sraw
        }

        else if (funct3 == 6)
        {
            if (funct7 == 1)
                ALU_OP = 31; //rem
            else
                ALU_OP = 6; //or
        }

        else if (funct3 == 7)
        {
            if (funct7 == 1)
                ALU_OP = 32; //remu
            else
                ALU_OP = 1; //and
        }
        addressA = rs1;
        addressB = rs2;
        addressC = rd;
        MEM_READ = 0;
        MEM_WRITE = 0;
    }

    else if (opcode == OPCODE_UJ)
    {
        unsigned int rd = IR << 20;
        rd >>= 27;
        addressC = rd;
        bitset<20> tmp2(IR >> 12), res;
        for (int i = 18; i >= 11; --i)
            res[i] = tmp2[i - 11];
        res[10] = tmp2[8];
        for (int i = 9; i >= 0; --i)
            res[i] = tmp2[i + 9];
        int tmp1 = res.to_ulong();
        if (tmp2[19])
            tmp1 = tmp1 - (1 << 19);
        immediate = tmp1 * 2;
        RF_WRITE = 1;
        B_SELECT = 0;
        INC_SELECT = 1;
        ALU_OP = -1;
        addressA = 0;
        addressB = 0;
        MEM_READ = 0;
        MEM_WRITE = 0;
        Y_SELECT = 2;
    }

    else if (opcode == OPCODE_SB1)
    {
        unsigned int rs1 = IR << 12;
        rs1 >>= 27;
        unsigned int rs2 = IR << 7;
        rs2 >>= 27;
        unsigned int rd = IR << 20;
        rd >>= 27;
        int bit_11 = (rd & 1) << 10;
        int bit_1_4 = rd >> 1;
        int bit_12 = (funct7 >> 6) << 11;
        int bit_5_10 = (funct7 - (bit_12 << 6)) << 4;
        immediate = bit_1_4 | bit_5_10 | bit_11 | bit_12;
        immediate <<= 1;

        RF_WRITE = 0;
        B_SELECT = 0;
        if (funct3 == 5) //bge
            ALU_OP = 3;
        else if (funct3 == 7)
        { //bgeu
            ALU_OP = 34;
        }
        else if (funct3 == 4) //blt
            ALU_OP = 4;

        else if (funct3 == 6)
        { //bltu
            ALU_OP = 35;
        }
        else if (funct3 == 0) //beq
            ALU_OP = 2;
        else if (funct3 == 1) //bne Update task2.cpp
            ALU_OP = 5;
        addressA = rs1;
        addressB = rs2;
        addressC = 0;
        MEM_READ = 0;
        MEM_WRITE = 0;
    }
  //  cout<<addressA<<' '<<addressB<<' '<<addressC<<' '<<immediate<<endl;
    buffer_ID_EX.PC = buffer_IF_ID.PC;
    buffer_ID_EX.addressC = addressC;
    buffer_ID_EX.immediate = immediate;
    buffer_ID_EX.ALU_OP = ALU_OP;
    buffer_ID_EX.B_SELECT = B_SELECT;
    buffer_ID_EX.PC_SELECT = PC_SELECT;
    buffer_ID_EX.INC_SELECT = INC_SELECT;
    buffer_ID_EX.Y_SELECT = Y_SELECT;
    buffer_ID_EX.MEM_READ = MEM_READ;
    buffer_ID_EX.MEM_WRITE = MEM_WRITE;
    buffer_ID_EX.RF_WRITE = RF_WRITE;
    buffer_ID_EX.addressA = addressA;
    buffer_ID_EX.addressB = addressB;
    buffer_ID_EX.returnAddress = returnAddress;
    readWriteRegFile(DECODE_STAGE);
}
//End of decode

/* Arithmetic Logic Unit
Input: ALU_OP, MUXB select, immediate(if any)
(these input will be provided by decode stage)
Updates RZ */
void alu(int ALU_OP, int B_SELECT, int immediate = 0)
{
    
    int INC_SELECT = buffer_ID_EX.INC_SELECT;
    int PC_SELECT = buffer_ID_EX.PC_SELECT;
    unsigned int returnAddress; //Return Address in case of jal/jalr
    immediate = buffer_ID_EX.immediate;

    int RA =  buffer_ID_EX.RA;
    buffer_EX_MEM.RA = buffer_ID_EX.RA;
    int RB = buffer_ID_EX.RB;
    buffer_EX_MEM.RB = buffer_ID_EX.RB;
    int RZ;
    buffer_EX_MEM.addressC = buffer_ID_EX.addressC;
    buffer_EX_MEM.immediate = buffer_ID_EX.immediate;
    buffer_EX_MEM.PC_SELECT = buffer_ID_EX.PC_SELECT;
    buffer_EX_MEM.INC_SELECT = buffer_ID_EX.INC_SELECT;
    buffer_EX_MEM.Y_SELECT = buffer_ID_EX.Y_SELECT;

    buffer_EX_MEM.MEM_READ = buffer_ID_EX.MEM_READ;
    buffer_EX_MEM.MEM_WRITE = buffer_ID_EX.MEM_WRITE;
    buffer_EX_MEM.RF_WRITE = buffer_ID_EX.RF_WRITE;

    returnAddress = buffer_ID_EX.returnAddress;
    buffer_EX_MEM.addressA = buffer_ID_EX.addressA;
    buffer_EX_MEM.addressB = buffer_ID_EX.addressB;


    int InA = RA;
    int InB;
    if (B_SELECT == 0)
        InB = RB;
    else
        InB = immediate;

    if (ALU_OP == 0) //addi,load,
    {
        RZ = InA + InB;
    }

    else if (ALU_OP == 1) //andi
        RZ = InA & InB;

    else if (ALU_OP == 2) //beq
    {
        if (InA == InB)
        {
            INC_SELECT = 1;
            buffer_ID_EX.PC -= 4;
            PC_SELECT = 1;
            iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
        }
    }
    else if (ALU_OP == 3) //bge
    {
        if (InA >= InB)
        {
            INC_SELECT = 1;
            buffer_ID_EX.PC -= 4;
            PC_SELECT = 1;
            iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
        }
    }

    //bgeu
    else if (ALU_OP == 34)
    {
        if ((unsigned)InA >= (unsigned)InB)
        {
            INC_SELECT = 1;
            buffer_ID_EX.PC -= 4;
            PC_SELECT = 1;
            iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
        }
    }

    //bltu
    else if (ALU_OP == 35)
    {
        if ((unsigned)InA < (unsigned)InB)
        {
            INC_SELECT = 1;
            buffer_ID_EX.PC -= 4;
            PC_SELECT = 1;
            iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
        }
    }

    else if (ALU_OP == 4) //blt
    {
        cout << "blt: " << buffer_ID_EX.PC << endl;
        cout<<InA<<" "<<InB<<endl;
        if (InA < InB)
        {
            INC_SELECT = 1;
            buffer_ID_EX.PC -= 4;
            PC_SELECT = 1;
            iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
            cout << "blt: " << buffer_ID_EX.PC << endl;
        }
        
    }

    else if (ALU_OP == 5) //bne
    {
        if (InA != InB)
        {
            INC_SELECT = 1;
            buffer_ID_EX.PC -= 4;
            PC_SELECT = 1;
            iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
        }
    }

    else if (ALU_OP == 6) //ori
        RZ = InA | InB;

    else if (ALU_OP == 7) //slli
        RZ = InA << InB;

    else if (ALU_OP == 18) //sub
        RZ = InA - InB;

    else if (ALU_OP == 8) //xori
        RZ = InA ^ InB;

    else if (ALU_OP == 12) //auipc
    {
        RZ = buffer_ID_EX.PC - 4 + (InB << 12);
    }
    else if (ALU_OP == 13) //lui
        RZ = InB << 12;

    else if (ALU_OP == 19) //srli
        RZ = InA >> InB;

    else if (ALU_OP == 20) //slti,sltiu
        RZ = (InA < InB) ? 1 : 0;

    else if (ALU_OP == 21) //sra, sraw
    {
        RZ = InA >> InB;
        RZ |= InA & (1 << 31);
    }
    else if (ALU_OP == 22) //jalr
    {
//        cout<<InA<<' '<<InB<<' ';
        buffer_EX_MEM.RZ = InA + InB;
        RZ=InA+InB;
        buffer_ID_EX.PC -= 4;
        returnAddress = iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
    }
    else if (ALU_OP == 25) //mul
        RZ = RA * RB;

    else if (ALU_OP == 29 || ALU_OP == 30) //div, divu
        RZ = RA / RB;

    else if (ALU_OP == 31 || ALU_OP == 32) // rem, remu
        RZ = RA % RB;

    else if (ALU_OP == -1)
    { // jal
        buffer_ID_EX.PC -= 4;
        PC_SELECT = 1;
        returnAddress = iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
    }

    buffer_EX_MEM.RZ = RZ;
    buffer_EX_MEM.returnAddress = returnAddress;
    buffer_EX_MEM.INC_SELECT = INC_SELECT;
    buffer_EX_MEM.PC_SELECT = PC_SELECT;
    buffer_EX_MEM.PC = buffer_ID_EX.PC;
    //cout<<hex<<buffer_EX_MEM.PC<<dec<<endl;
}
//end of ALU function

/*Stage 4: Memory & RY get updated
Input: Y_SELECT, MEM_READ, MEM_WRITE, address from RZ/RM, data */
void memoryStage(int Y_SELECT, int MEM_READ, int MEM_WRITE, int address = 0, int data = 0)
{
    int returnAddress = buffer_EX_MEM.returnAddress;
    int RY;
    
    int dataFromMem = readWriteMemory(MEM_READ, MEM_WRITE, address, data);
    if (Y_SELECT == 0)
        RY = buffer_EX_MEM.RZ;
    if (Y_SELECT == 1)
        RY = dataFromMem;
    if (Y_SELECT == 2)
        RY = returnAddress;

    buffer_MEM_WB.RY = RY;
    buffer_MEM_WB.addressC = buffer_EX_MEM.addressC;
    buffer_MEM_WB.RF_WRITE = buffer_EX_MEM.RF_WRITE;
}

//End of memoryStage

//Stage 5: WriteBack
void writeBack(int RF_WRITE, int addressC)
{
    readWriteRegFile(WB_STAGE);
    buffer_IF_ID.PC = buffer_EX_MEM.PC;
}
//End of writeBack

//Update memory with data & instructions
void updateMemory()
{
    string machineLine;
     string machineCode;
    fstream fileReading;

    map<char, int> hexadecimal;
    for (int i = 0; i <= 9; ++i)
        hexadecimal[i + '0'] = i;
    for (int i = 0; i <= 6; ++i)
        hexadecimal[i + 'A'] = i + 10;

    fileReading.open("machineData.txt");
    while (getline(fileReading, machineLine))
    {
        lli value = 0, address = 0;
        string type = "";
        int i = 0;
        while (machineLine[i] != ' ')
            value = value * 10 + (machineLine[i++] - '0');
        i = i + 3;
        while (machineLine[i] != ' ')
            address = address * 16 + hexadecimal[machineLine[i++]];
        i++;

        while (i < machineLine.length() && machineLine[i] != ' ')
            type += machineLine[i++];
        if (type == "byte")
            readWriteMemory(0, 1, address, value);
        else if (type == "halfword")
            readWriteMemory(0, 2, address, value);
        else if (type == "word")
            readWriteMemory(0, 3, address, value);
        else if (type == "doubleword")
            readWriteMemory(0, 4, address, value);
    }
    fileReading.close();

    fileReading.open("machineCode.mc");
    lli address = 0;
    while (getline(fileReading, machineLine))
    {
        lli value = 0;

        int i = 2; //initially : 0x
//        while (machineLine[i] != ' ')
//            address = address * 16 + hexadecimal[machineLine[i++]];

//        i += 3; //between : 0x
        while (i < machineLine.length())
            value = value * 16 + hexadecimal[machineLine[i++]];

        readWriteMemory(0, 3, address, value);
        address+=4;
    }
    fileReading.close();
}
//End of updateMemory

//Prints memory that has been alloted with data or instruction!
void printMemory()
{
    cout << "----------------Memory--------------------" << endl;
    for (int i = 0; i < 1 << 22; i++)
        if (memory[i] != '\0')
            cout << i << "\t" << (int)memory[i] << endl;
    cout << "-------------------------------------------" << endl;
}
//End of printMemory

//Prints all register file & their value
void printRegisterFile()
{
    cout << "-------------Register File-----------------" << endl;
    for (int i = 0; i < 32; i++)
        cout << "REGISTER x" << i << "\t" << regArray[i] << endl;
    cout << "-------------------------------------------" << endl;
}
//End of print RegisterFile

//Run Instructions: unpipelined
void runCode()
{
    int choice;
    cout << "-------------------------------------------" << endl;
    cout << "Press 1 to run the whole pogram " << endl;
    cout << "Press 2 to run it step by step" << endl;
    cin >> choice;
    cout << "-------------------------------------------" << endl;
    switch (choice)
    {
    case 1:
    {
        while (1)
        {
            if (memory[buffer_IF_ID.PC] == 0 && memory[buffer_IF_ID.PC + 1] == 0 && memory[buffer_IF_ID.PC + 2] == 0 && memory[buffer_IF_ID.PC + 3] == 0)
                break;
            fetch();
            decode();
            alu(buffer_ID_EX.ALU_OP, buffer_ID_EX.B_SELECT, buffer_ID_EX.immediate);
            memoryStage(buffer_EX_MEM.Y_SELECT, buffer_EX_MEM.MEM_READ, buffer_EX_MEM.MEM_WRITE, buffer_EX_MEM.RZ, buffer_EX_MEM.RB);
            writeBack(buffer_MEM_WB.RF_WRITE, buffer_MEM_WB.addressC);
            cycleCount++;
        }
        break;
    }
    case 2:
    {

        int printinfo = 0;

        while (1)
        {
            // // cout << hex << buffer_ID_EX.PC  << " " << buffer_EX_MEM.PC   << dec << endl;
            
            cout << "Press 0 to move on" << endl;
            cout << "Press 1 to print register file " << endl;
            cout << "Press 2 to print memory" << endl;
            cout << "Press 3 to execute to end" << endl;
            cout << "-------------------------------------------" << endl;
            cin >> printinfo;
            if (printinfo == 1)
            {
                printRegisterFile();
                continue;
            }
            else if (printinfo == 2)
            {
                printMemory();
                continue;
            }
            else if (printinfo == 0)
            {
 
                if (memory[buffer_IF_ID.PC] == 0 && memory[buffer_IF_ID.PC + 1] == 0 && memory[buffer_IF_ID.PC + 2] == 0 && memory[buffer_IF_ID.PC + 3] == 0)
                    break;
                fetch();
                decode(); 
                alu(buffer_ID_EX.ALU_OP, buffer_ID_EX.B_SELECT, buffer_ID_EX.immediate);
                memoryStage(buffer_EX_MEM.Y_SELECT, buffer_EX_MEM.MEM_READ, buffer_EX_MEM.MEM_WRITE, buffer_EX_MEM.RZ, buffer_EX_MEM.RB);
                writeBack(buffer_MEM_WB.RF_WRITE, buffer_MEM_WB.addressC);
                cycleCount++;
            }
            else if (printinfo == 3)
            {
                while (1)
                {
                    if (memory[buffer_IF_ID.PC] == 0 && memory[buffer_IF_ID.PC + 1] == 0 && memory[buffer_IF_ID.PC + 2] == 0 && memory[buffer_IF_ID.PC + 3] == 0)
                        break;
                    fetch();
                    decode();
                    alu(buffer_ID_EX.ALU_OP, buffer_ID_EX.B_SELECT, buffer_ID_EX.immediate);
                    memoryStage(buffer_EX_MEM.Y_SELECT, buffer_EX_MEM.MEM_READ, buffer_EX_MEM.MEM_WRITE, buffer_EX_MEM.RZ, buffer_EX_MEM.RB);
                    writeBack(buffer_MEM_WB.RF_WRITE, buffer_MEM_WB.addressC);
                    cycleCount++;
                }
                break;
            }
            

        }
        break;
    }
    default:
        cout << "Wrong choice" << endl;
    }
}
//End of runCode

//main function
int main()
{
    //initialize x2, x3
    regArray[2] = 0xFFFFFF;
    regArray[3] = 0x100000;

    updateMemory(); //Update memory with data & instructions
    runCode();
    printRegisterFile();
    cout << "Number of Cycles = " << cycleCount << endl;
}
//End of main
