// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

@16384          //R0 = 0x4000
D = A
@R0
M = D
@R2             //R2 point to current screen
M = D
@24575
D = A
@R1             //R1 = 0x5FFF
M = D


(MAIN)
@KBD
D = M
@FILL
D;JGT
@CLEAR
0;JMP

(FILL)
@R1
D = M
@R2
D = D - M
@MAIN
D;JLE           //full of screen

@R2
A = M
M = -1
@R2
M = M + 1

@MAIN
0;JMP

(CLEAR)

@R2
D = M
@R0
D = D - M
@MAIN
D;JLE           //screen is empty

@R2
A = M
M = 0
@R2
M = M - 1

@MAIN
0;JMP
