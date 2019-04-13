.text
addi x3, x0, 4
addi x8, x0, 1
jal x1,fib
beq x0,x0,exit

fib: bgt x3,x8,l1
    addi x4,x3,0
    jalr x0,0(x1) #return
l1: addi x2,x2,-12
    sw x1,0(x2)#save return address
    sw x3,4(x2)#save n
    addi x3,x3,-1#n-1
    jal x1,fib
    lw x3,4(x2)#restore n
    sw x4,8(x2)#save return value from fib(n-1)
    addi x3,x3,-2
    jal x1,fib
    lw x5,8(x2)#restore return value from fib(n-1)
    add x4,x4,x5#our return value
    lw x1,0(x2)
    addi x2,x2,12
    jalr x0,0(x1) #return
exit: