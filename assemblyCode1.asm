addi x31, x0, 2
addi x29 x0 -1
bltu x31, x29, EXIT

add x30 x31 x31
sub x30 x31 x0
jal x1, EXIT2

EXIT: addi x3, x0, 9
EXIT2:
