Risc V Simulation Software (ReadMe File)
----------------------------------------------------------
Overview :

Task 1 : Converts assembly code to machine code

Task 2 : Unpipelined execution of machine code
Task 3 : Pipelined/(Unpipelined) Stalling, Forwarding, Static Branch Prediction, Cache Support 
For only stalling, control detection moved to decode(1 stall penalty for misprediction)
For forwarding, control detection in ALU(2 stalls penanlty for misprediction)
Branch Prediction : Static & Always Not Taken
Cache : Direct Map, Set Assosiative, Fully Associative

----------------------------------------------------------
Run :

In terminal go to file location :
g++ task1.cpp -o ./a
./a
(Enter file name : assemblyCode.asm or any other)
g++ task3.cpp -o ./b
./b
