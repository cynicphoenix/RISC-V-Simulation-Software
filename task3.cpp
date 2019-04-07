// Task 3: Pipelining

#include <bits/stdc++.h>
#define lli long long int
using namespace std;

struct Buffer_IF_ID{
    unsigned int PC;
    int IR;
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