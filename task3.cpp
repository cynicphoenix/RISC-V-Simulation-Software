// Task 3: Pipelining

#include <bits/stdc++.h>
#define lli long long int
using namespace std;

#define OFF 0
#define ON 1
#define TRUE 0
#define FALSE 1

struct Buffer_IF_ID{
    unsigned int PC;
    int IR;
    bool en;
    bool en2;
    Buffer_IF_ID(){
        PC = 0;
        IR = 0;
        en = 0;
        en2 = 1;
    }
};

struct Buffer_ID_EX{
    unsigned int PC;
    int RA;
    int RB;
    int RZ; // for branch instructions
    int addressC;
    int immediate;
    int ALU_OP, B_SELECT, PC_SELECT, INC_SELECT, Y_SELECT;
    int MEM_READ;
    int MEM_WRITE;
    int RF_WRITE;
    int addressA, addressB;
    unsigned int returnAddress;
    bool branchTaken;
    bool isBranchInstruction;
    bool en, en2;
    Buffer_ID_EX(){
        en = 0;
        en2 = 1;
        branchTaken = FALSE;
        isBranchInstruction = FALSE;
    }
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
    bool en, en2;
    Buffer_EX_MEM(){
        en = 0;
        en2 = 1;
    }
};

struct Buffer_MEM_WB{
    unsigned int PC;
    int addressC;
    int RF_WRITE;
    int RY;
    bool en, en2;
    Buffer_MEM_WB(){
        en = 0;
        en2 = 1;
    }
};

struct Stats_Count{
    unsigned int aluInstructions = 0;
    unsigned int controlInstructions = 0;
    unsigned int dataTransferInstructions = 0;

    unsigned int RTypeInstructions = 0;
    unsigned int ITypeInstructions = 0;
    unsigned int STypeInstructions = 0;
    unsigned int SBTypeInstructions = 0;
    unsigned int UJTypeInstructions = 0;
    unsigned int UTypeInstructions = 0;

    unsigned int stalls = 0;
    unsigned int stalls_data_hazard = 0;
    unsigned int stalls_control_hazard = 0;

    unsigned int data_hazard = 0;
    unsigned int control_hazard = 0;
    unsigned int branch_mispredictions = 0;

    unsigned int cycleCount = 0;
    unsigned int total_instructions = 0;
    double CPI = 0;
};

Buffer_IF_ID buffer_IF_ID;
Buffer_ID_EX buffer_ID_EX;
Buffer_EX_MEM buffer_EX_MEM;
Buffer_MEM_WB buffer_MEM_WB;
Stats_Count stats_count;

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

#define NO_DATA_DEPEND 0
#define DATA_DEPEND_RA 1
#define DATA_DEPEND_RB 2
#define DATA_DEPEND_RA_RB 3
#define DATA_DEPENED_MtoM 4

int PC_of_stalledStageEtoE = INT_MAX;
int PC_of_stalledStageMtoE = INT_MAX;
int PC_of_stalledStageMtoM = INT_MAX;
unsigned char memory[1 << 24]; //Processor Memory
int regArray[32] = {0};
int cycleCount = 0;
int prevPC = -1;

//Call in decode stage & Writeback Stage
void readWriteRegFile(int stage)
{
    if(stage == WB_STAGE){
        if (buffer_MEM_WB.RF_WRITE == 1)
        {
            if (buffer_MEM_WB.addressC)
                regArray[buffer_MEM_WB.addressC] = buffer_MEM_WB.RY;
            //cout<<regArray[buffer_MEM_WB.addressC]<<endl;
            return;
        }
    }
    if(stage == DECODE_STAGE){
            if(buffer_ID_EX.addressA<32)
                buffer_ID_EX.RA = regArray[buffer_ID_EX.addressA];
            if(buffer_ID_EX.addressB<32)
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
void fetch(bool en)
{
    buffer_IF_ID.en = en;
    if(en == 0){
        return;
    }

    buffer_IF_ID.IR = readWriteMemory(3, 0, buffer_IF_ID.PC);
    //cout << buffer_IF_ID.PC << endl;
    buffer_IF_ID.PC = iag(FETCH_STAGE);
    // cout << hex << buffer_IF_ID.IR <<" "<<buffer_IF_ID.PC << dec << endl;
    
}
//End of fetch

/* Stage 2: Decode Stage
RA & RB will be updated after this stage */
void decode()
{
    if (buffer_IF_ID.en == 0)
        return;
    buffer_ID_EX.isBranchInstruction = FALSE;
    buffer_ID_EX.branchTaken = FALSE;
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
        if(prevPC != buffer_IF_ID.PC){
            stats_count.ITypeInstructions++;
            stats_count.dataTransferInstructions++;
        }
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
        if(prevPC != buffer_IF_ID.PC){
            stats_count.ITypeInstructions++;
            stats_count.aluInstructions++;
        }
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
        if(prevPC != buffer_IF_ID.PC){
            stats_count.ITypeInstructions++;
            stats_count.aluInstructions++;
        }
        
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
        if(prevPC != buffer_IF_ID.PC){
            stats_count.ITypeInstructions++;
            stats_count.controlInstructions;
        }
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

        //Execute moved to decode : Control Hazard
        int InA = regArray[rs1];
        int InB = immediate;
        buffer_ID_EX.RZ = InA + InB;
        buffer_EX_MEM.RZ = buffer_ID_EX.RZ;
        buffer_ID_EX.PC -= 4;
        buffer_ID_EX.returnAddress = iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
        buffer_ID_EX.isBranchInstruction = TRUE;
        buffer_ID_EX.branchTaken = TRUE;
    }

    else if (opcode == OPCODE_S1) //store
    {
        if(prevPC != buffer_IF_ID.PC){
            stats_count.STypeInstructions++;
            stats_count.dataTransferInstructions++;
        }
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
        if(prevPC != buffer_IF_ID.PC){
            stats_count.UTypeInstructions++;
            stats_count.aluInstructions++;
        }
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
        if(prevPC != buffer_IF_ID.PC){
            stats_count.UTypeInstructions++;
            stats_count.aluInstructions++;
        }
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
        if(prevPC != buffer_IF_ID.PC){
            stats_count.RTypeInstructions++;
            stats_count.aluInstructions++;
        }
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
    { //jal
        if(prevPC != buffer_IF_ID.PC){
            stats_count.UJTypeInstructions++;
            stats_count.controlInstructions++;
        }
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

        buffer_ID_EX.PC -= 4;
        PC_SELECT = 1;
        returnAddress = iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
        buffer_ID_EX.isBranchInstruction = TRUE;
        buffer_ID_EX.branchTaken = TRUE;

        //buffer_IF_ID.PC=buffer_ID_EX.PC+4;
    }

    else if (opcode == OPCODE_SB1)
    {
        if(prevPC != buffer_IF_ID.PC){
            stats_count.SBTypeInstructions++;
            stats_count.controlInstructions++;
        }
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

        PC_SELECT = 1;
        INC_SELECT = 1;
        RF_WRITE = 0;
        B_SELECT = 0;
        buffer_ID_EX.isBranchInstruction = TRUE;
        int InA = regArray[rs1];
        int InB = regArray[rs2];

        if (funct3 == 5)
        { //bge
            ALU_OP = 3;
            if (InA >= InB)
            {
                buffer_ID_EX.branchTaken = TRUE;
                buffer_ID_EX.PC -= 4;
                iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
            }
        }
        else if (funct3 == 7)
        { //bgeu
            ALU_OP = 34;
            if ((unsigned)InA >= (unsigned)InB)
            {
                buffer_ID_EX.branchTaken = TRUE;
                buffer_ID_EX.PC -= 4;
                iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
            }
        }
        else if (funct3 == 4)
        { //blt
            ALU_OP = 4;
            if ((unsigned)InA < (unsigned)InB)
            {
                buffer_ID_EX.branchTaken = TRUE;
                buffer_ID_EX.PC -= 4;
                iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
            }
        }
        else if (funct3 == 6)
        { //bltu
            ALU_OP = 35;
            if ((unsigned)InA < (unsigned)InB)
            {
                buffer_ID_EX.branchTaken = TRUE;
                buffer_ID_EX.PC -= 4;
                iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
            }
        }
        else if (funct3 == 0)
        { //beq
            ALU_OP = 2;
            if (InA == InB)
            {
                buffer_ID_EX.branchTaken = TRUE;
                buffer_ID_EX.PC -= 4;
                iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
            }

        }
        else if (funct3 == 1) //bne
        {
            ALU_OP = 5;
            if (InA != InB)
            {
                buffer_ID_EX.branchTaken = TRUE;
                buffer_ID_EX.PC -= 4;
                iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
            }
        }
        
        addressA = rs1;
        addressB = rs2;
        addressC = 0;
        MEM_READ = 0;
        MEM_WRITE = 0;
    }
  //  cout<<addressA<<' '<<addressB<<' '<<addressC<<' '<<immediate<<endl;
    if(buffer_ID_EX.isBranchInstruction==FALSE || buffer_ID_EX.branchTaken==FALSE)
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
    prevPC= buffer_IF_ID.PC;
    readWriteRegFile(DECODE_STAGE);
}
//End of decode

/* Arithmetic Logic Unit
Input: ALU_OP, MUXB select, immediate(if any)
(these input will be provided by decode stage)
Updates RZ */
void alu(int ALU_OP, int B_SELECT, int immediate = 0)
{
    buffer_ID_EX.en = buffer_IF_ID.en;
    if (buffer_ID_EX.en == 0)
        return;

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

    /*else if (ALU_OP == 2) //beq
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
        //cout << "blt: " << buffer_ID_EX.PC << endl;
        //cout<<InA<<" "<<InB<<endl;
        if (InA < InB)
        {
            INC_SELECT = 1;
            buffer_ID_EX.PC -= 4;
            PC_SELECT = 1;
            iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
            //cout << "blt: " << buffer_ID_EX.PC << endl;
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
    }*/

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
    /*else if (ALU_OP == 22) //jalr
    {
        buffer_EX_MEM.RZ = InA + InB;
        RZ=InA+InB;
        buffer_ID_EX.PC -= 4;
        returnAddress = iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
    }*/
    else if (ALU_OP == 25) //mul
        RZ = RA * RB;

    else if (ALU_OP == 29 || ALU_OP == 30) //div, divu
        RZ = RA / RB;

    else if (ALU_OP == 31 || ALU_OP == 32) // rem, remu
        RZ = RA % RB;

    /*else if (ALU_OP == -1)
    { // jal
        buffer_ID_EX.PC -= 4;
        PC_SELECT = 1;
        returnAddress = iag(EXECUTE_STAGE, INC_SELECT, PC_SELECT, immediate);
    }*/
    if(ALU_OP != 22)//jalr
        buffer_EX_MEM.RZ = RZ;
    buffer_EX_MEM.returnAddress = returnAddress;
    buffer_EX_MEM.INC_SELECT = INC_SELECT;
    buffer_EX_MEM.PC_SELECT = PC_SELECT;
    buffer_EX_MEM.PC = buffer_ID_EX.PC;
}
//end of ALU function

/*Stage 4: Memory & RY get updated
Input: Y_SELECT, MEM_READ, MEM_WRITE, address from RZ/RM, data */
void memoryStage(int Y_SELECT, int MEM_READ, int MEM_WRITE, int address = 0, int data = 0)
{
    buffer_EX_MEM.en = buffer_ID_EX.en;
    if (buffer_EX_MEM.en == 0)
        return;
    //cout << hex << "dsd " << buffer_EX_MEM.PC << dec << endl;
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
    buffer_MEM_WB.PC = buffer_EX_MEM.PC;
}

//End of memoryStage

//Stage 5: WriteBack
void writeBack(int RF_WRITE, int addressC)
{
    buffer_MEM_WB.en = buffer_EX_MEM.en;
    //cout << hex << "decode " << buffer_MEM_WB.PC<< dec << endl;
    if (buffer_MEM_WB.en == 0)
        return;
    readWriteRegFile(WB_STAGE);
    // buffer_IF_ID.PC = buffer_MEM_WB.PC;
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

int forward_dependency_EtoE(bool knob2)
{
    if(buffer_EX_MEM.addressC == 0)
        return NO_DATA_DEPEND;

     if(buffer_ID_EX.addressA == buffer_EX_MEM.addressC && buffer_ID_EX.addressB == buffer_EX_MEM.addressC)
    {
        if(knob2)
            buffer_ID_EX.RA = buffer_ID_EX.RB = buffer_EX_MEM.RZ;
        return DATA_DEPEND_RA_RB;
    }    
    if(buffer_ID_EX.addressA == buffer_EX_MEM.addressC)
    {
        if(knob2)
            buffer_ID_EX.RA = buffer_EX_MEM.RZ;
        return DATA_DEPEND_RA;
    }
    if(buffer_ID_EX.addressB == buffer_EX_MEM.addressC)
    {
        if(knob2)
            buffer_ID_EX.RB = buffer_EX_MEM.RZ;
        return DATA_DEPEND_RB;
    }
    return NO_DATA_DEPEND;
}
//End of forward_dependency_EtoE()

//Check forward dependency Memory to Execute
int forward_dependency_MtoE(bool knob2)
{
    if(buffer_MEM_WB.addressC == 0)
        return NO_DATA_DEPEND;

     if(buffer_ID_EX.addressA == buffer_MEM_WB.addressC && buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
    {
        if(knob2)
            buffer_ID_EX.RA = buffer_ID_EX.RB = buffer_MEM_WB.RY;
        return DATA_DEPEND_RA_RB;
    }    

    if(buffer_ID_EX.addressA == buffer_MEM_WB.addressC)
    {     
        if(knob2)
            buffer_ID_EX.RA = buffer_MEM_WB.RY;
        return DATA_DEPEND_RA;
    }
    if(buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
    {
        if(knob2)
            buffer_ID_EX.RB = buffer_MEM_WB.RY;
        return DATA_DEPEND_RB;
    }
    return NO_DATA_DEPEND;
}
//End of forward_dependency_MtoE()

//Check data dependency Memory to Memory
int forward_dependency_MtoM(bool knob2)
{
    if(buffer_MEM_WB.addressC == 0)
        return NO_DATA_DEPEND;

    if(buffer_EX_MEM.addressC == buffer_MEM_WB.addressC)
    {
        if(knob2)
            buffer_EX_MEM.RZ = buffer_MEM_WB.RY;
        return DATA_DEPENED_MtoM;
    }
    return NO_DATA_DEPEND;
}
//End of forward_dependency_MtoM()

//Check data stalling Execute to Execute
int stall_check_EtoE()
{
    if(buffer_EX_MEM.addressC == 0)
        return NO_DATA_DEPEND;

     if(buffer_ID_EX.addressA == buffer_EX_MEM.addressC && buffer_ID_EX.addressB == buffer_EX_MEM.addressC)
    {
        PC_of_stalledStageEtoE = buffer_ID_EX.PC;
        return DATA_DEPEND_RA_RB;
    }    
    if(buffer_ID_EX.addressA == buffer_EX_MEM.addressC)
    {
        PC_of_stalledStageEtoE = buffer_ID_EX.PC;
        return DATA_DEPEND_RA;
    }
    if(buffer_ID_EX.addressB == buffer_EX_MEM.addressC)
    {
        PC_of_stalledStageEtoE = buffer_ID_EX.PC;
        return DATA_DEPEND_RB;
    }
    return NO_DATA_DEPEND;
}
//End of stall_check_EtoE()

//Check data stalling Memory to Execute
int stall_check_MtoE()
{
    if(buffer_MEM_WB.addressC == 0)
        return NO_DATA_DEPEND;

     if(buffer_ID_EX.addressA == buffer_MEM_WB.addressC && buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
    {
        PC_of_stalledStageMtoE = buffer_ID_EX.PC;
        return DATA_DEPEND_RA_RB;
    }    

    if(buffer_ID_EX.addressA == buffer_MEM_WB.addressC)
    {     
        PC_of_stalledStageMtoE = buffer_ID_EX.PC;    
        return DATA_DEPEND_RA;
    }
    if(buffer_ID_EX.addressB == buffer_MEM_WB.addressC)
    {
        PC_of_stalledStageMtoE = buffer_ID_EX.PC;
        return DATA_DEPEND_RB;
    }
    return NO_DATA_DEPEND;
}
//End of stall_check_MtoE()

//Print stats in stats.txt file
void stats_print()
{
    stats_count.total_instructions = stats_count.aluInstructions + stats_count.controlInstructions + stats_count.dataTransferInstructions;
    stats_count.stalls = stats_count.stalls_control_hazard + stats_count.stalls_data_hazard;
    stats_count.CPI = (double)stats_count.cycleCount/(double)stats_count.total_instructions;
    fstream fileWriting;
    fileWriting.open("stats.txt", ios::out);
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting<<"Total Cycles               :  "<<stats_count.cycleCount<<endl;
    fileWriting<<"Total Instructions         :  "<<stats_count.total_instructions<<endl;
    fileWriting<<"CPI                        :  "<<stats_count.CPI<<endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting<<"R Type                     :  "<<stats_count.RTypeInstructions<<endl;
    fileWriting<<"I Type                     :  "<<stats_count.ITypeInstructions<<endl;
    fileWriting<<"S Type                     :  "<<stats_count.STypeInstructions<<endl;
    fileWriting<<"SB Type                    :  "<<stats_count.SBTypeInstructions<<endl;
    fileWriting<<"U Type                     :  "<<stats_count.UTypeInstructions<<endl;
    fileWriting<<"UJ Type                    :  "<<stats_count.UJTypeInstructions<<endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting<<"Data-Transfer Instructions :  "<<stats_count.dataTransferInstructions<<endl;
    fileWriting<<"ALU Instruction            :  "<<stats_count.aluInstructions<<endl;
    fileWriting<<"Control Instructions       :  "<<stats_count.controlInstructions<<endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting<<"Data Hazards               :  "<<stats_count.data_hazard<<endl;
    fileWriting<<"Control Hazards            :  "<<stats_count.control_hazard<<endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting<<"Stalls due Data Hazard     :  "<<stats_count.stalls_data_hazard<<endl;
    fileWriting<<"Stalls due Control Hazard  :  "<<stats_count.stalls_control_hazard<<endl;
    fileWriting<<"Total Stalls               :  "<<stats_count.stalls<<endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting<<"Branch Mis-predictions     :  "<<stats_count.branch_mispredictions<<endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting.close();
}
//End of stats_print()

//Prints all register file & their value
void printRegisterFile()
{  
    fstream fileWriting;
    fileWriting.open("regFile.txt", ios::app);
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting<<"CYCLE NUMBER\t\t:\t"<<stats_count.cycleCount<<endl;
    for (int i = 0; i < 10; i++)
        fileWriting << "REGISTER x" << i << "\t\t\t:\t" << regArray[i] << endl;
    for (int i = 10; i < 32; i++)
        fileWriting << "REGISTER x" << i << "\t\t:\t" << regArray[i] << endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting.close();
}
//Check data dependency Execute to Execute

//Prints memory that has been alloted with data or instruction!
void printMemory()
{
    fstream fileWriting;
    fileWriting.open("memory.txt", ios::out);
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    for (int i = 0; i < 1 << 22; i++)
        if (memory[i] != '\0')
            fileWriting << i << "\t" << (int)memory[i] << endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting.close();
}
//End of printMemory

//Prints pipeline registers data
void printPipelineRegisters()
{
    fstream fileWriting;
    fileWriting.open("pipelineRegisters.txt", ios::app);
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting<<"Cycle\t:\t"<<stats_count.cycleCount<<endl;
    fileWriting<<"RA\t\t:\t"<<buffer_ID_EX.RA<<endl;
    fileWriting<<"RB\t\t:\t"<<buffer_ID_EX.RB<<endl;
    fileWriting<<"RZ\t\t:\t"<<buffer_EX_MEM.RZ<<endl;
    fileWriting<<"RY\t\t:\t"<<buffer_MEM_WB.RY<<endl;
    fileWriting<<"----------------------------------------------------------------------"<<endl;
    fileWriting.close();
}
//End of printpipelineRegisters

//Run Instructions: pipelined
void runCode()
{
    bool knob1; //Enable-disable pipeling
    bool knob2; //Enable-disable data forwarding
    bool knob3; //Print Register File after every cycle
    bool knob4; //Print Data in pipeline register along with cycle number
    bool knob5 ; //Print data of particular instruction with cycle number
    bool en = 1;
    int instruction_number; //for knob5
    cout<<"----------------------------------------------------------------------"<<endl;
    cout<<"Press 0 : Turn OFF the KNOB"<<endl<<"Press 1 : Turn ON the KNOB"<<endl;
    cout<<"----------------------------------------------------------------------"<<endl;
    cout<<"Knob 1 (Pipeline Knob)                  :  ";
    cin>>knob1;
    cout<<"Knob 2 (Data Forwarding Knob)           :  ";
    cin>>knob2;
    cout<<"Knob 3 (Register File Knob)             :  ";
    cin>>knob3;
    cout<<"Knob 4 (Pipeline Register Knob)         :  ";
    cin>>knob4;
    cout<<"Knob 5 (Instruction Knob)               :  ";
    cin>>knob5;
    if(knob5 == ON){
        cout<<"Instruction Number                      :  ";
        cin>>instruction_number;
        instruction_number = instruction_number<<2;
    }
    cout<<"----------------------------------------------------------------------"<<endl;

    if(knob1 == ON){ 
        while (1)
        {          
            stats_count.cycleCount++;
            if(knob3 == ON)
                printRegisterFile();

            if (memory[buffer_IF_ID.PC] == 0 && memory[buffer_IF_ID.PC + 1] == 0 && memory[buffer_IF_ID.PC + 2] == 0 && memory[buffer_IF_ID.PC + 3] == 0)
                en = 0;
            if(buffer_MEM_WB.en2==1 && PC_of_stalledStageEtoE > buffer_MEM_WB.PC && PC_of_stalledStageMtoE > buffer_MEM_WB.PC)
                writeBack(buffer_MEM_WB.RF_WRITE, buffer_MEM_WB.addressC);

            if(buffer_EX_MEM.en2==1 && PC_of_stalledStageEtoE > buffer_EX_MEM.PC && PC_of_stalledStageMtoE > buffer_EX_MEM.PC)
            {  
               buffer_MEM_WB.en2=1;
                memoryStage(buffer_EX_MEM.Y_SELECT, buffer_EX_MEM.MEM_READ, buffer_EX_MEM.MEM_WRITE, buffer_EX_MEM.RZ, buffer_EX_MEM.RB);
            }
            else  buffer_MEM_WB.en2=0;

            if(buffer_ID_EX.en2==1 && PC_of_stalledStageEtoE > buffer_ID_EX.PC && PC_of_stalledStageMtoE > buffer_ID_EX.PC)
            {
                buffer_EX_MEM.en2=1;
                alu(buffer_ID_EX.ALU_OP, buffer_ID_EX.B_SELECT, buffer_ID_EX.immediate);
            }
            else buffer_EX_MEM.en2=0;
            
            if(buffer_IF_ID.en2==1)
            {
                buffer_ID_EX.en2=1;
                decode();
            }
            else buffer_ID_EX.en2=0; 

            
            int dataDependencyEtoE= stall_check_EtoE();
            int dataDependencyMtoE= stall_check_MtoE();
            if(buffer_MEM_WB.en2==0 && buffer_EX_MEM.en2==0)
            {
                PC_of_stalledStageEtoE = INT_MAX;
            }
            if(buffer_EX_MEM.en2==0)
                PC_of_stalledStageMtoE = INT_MAX;

                            
            if(PC_of_stalledStageEtoE == INT_MAX && PC_of_stalledStageMtoE == INT_MAX)
            {
                buffer_IF_ID.en2=1;
                fetch(en);
            }
            else
                stats_count.stalls_data_hazard++;
            
            if(buffer_ID_EX.isBranchInstruction==TRUE && buffer_ID_EX.branchTaken==TRUE)
            {
                buffer_IF_ID.PC=buffer_ID_EX.PC+4;
                buffer_IF_ID.en2=0;
                buffer_ID_EX.isBranchInstruction=FALSE; 
                buffer_ID_EX.branchTaken=FALSE;
                stats_count.stalls_control_hazard++;
            }

            if(knob4 == ON)
                printPipelineRegisters();

            if(knob5 == ON){
                if(buffer_ID_EX.PC == instruction_number){
                    cout<<"Cycle Number\t:\t"<<stats_count.cycleCount<<endl;
                    cout<<"RA\t\t:\t"<<buffer_ID_EX.RA<<endl;
                    cout<<"RB\t\t:\t"<<buffer_ID_EX.RB<<endl;
                }
                if(buffer_EX_MEM.PC == instruction_number){
                    cout<<"Cycle Number\t:\t"<<stats_count.cycleCount<<endl;
                    cout<<"RZ\t\t:\t"<<buffer_EX_MEM.RZ<<endl;
                }
                if(buffer_MEM_WB.PC == instruction_number){
                    cout<<"Cycle Number\t:\t"<<stats_count.cycleCount<<endl;
                    cout<<"RY\t\t:\t"<<buffer_MEM_WB.RY<<endl;
                }
            }

            if(en == 0 && buffer_IF_ID.en == 0 && buffer_ID_EX.en == 0 && buffer_EX_MEM.en == 0 && buffer_MEM_WB.en == 0)
                break;       
        }
    }
    if(knob3 == OFF){
         printRegisterFile();
    }
    cout<<"----------------------------------------------------------------------"<<endl;
}
//End of runCode

//main function
int main()
{
    regArray[2] = 0xFFFFFF; //initialize x2, x3
    regArray[3] = 0x100000;

    fstream fileWriting; //To clear data of existing regFile.txt
    fileWriting.open("regFile.txt", ios::out);
    fileWriting.close();

    fileWriting.open("pipelineRegisters.txt", ios::out); //To clear data of existing pipelineRegisters.txt
    fileWriting.close();

    updateMemory(); //Update memory with data & instructions
    runCode();
    stats_print();
    printMemory();
}
//End of main
