Risc V Simulation Software (ReadMe File)

----------------------------------------------------------

Task 1 Work Division :

R & S type Instructions : Bhawna
I type Instructions : Soumil
SB type Instructions : Amit
UJ & U type Instructions : Aman

dec2Binary() : Amit & Aman
createMap() & bin2Hex() : (Source - Geeksforgeeks)
assignLineNumberToLabel() : Amit
main() : Bhawna

Syntax Analysis; comment, .data & .text support : Bhawna 
Debugged by Bhawna & Deepak

----------------------------------------------------------

Task 2 Work Division :

Data Path : Aman, Amit & Bhawna
Control Circuitry : Deepak & Soumil

Stages:
fetch() : Amit & Aman
decode() : Deepak & Soumil
alu() : Amit & Aman
memory() : Amit & Bhawna
writeback () : Aman & Bhawna

readWriteRegisterFile() : Amit & Aman
readWriteMemory() : Deepak
iag() : Amit & Aman
updateMemory() : Amit & Aman
printMemory() : Bhawna
printRegisterFile() : Bhawna
runCode() : Aman
main() : Amit

Debugging done by ALL (Instructions were assigned to each)
Most of the task 2 code debugged by Deepak

----------------------------------------------------------

List of not supported instructions :

64-bit instructions
floating point instructions
pseudo instructions
csrrc, csrrs, csrrsi, csrrw, csrrwi,
ebreak, ecall, fence, fence.i

-----------------------------------------------------------

Limitations :
Immediate Values can only be in integer (no hex support)

