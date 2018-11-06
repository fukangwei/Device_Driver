    AREA beep_test, CODE, READONLY
    ENTRY

pGPBCON EQU 0x56000010 ;Port B control
pGPBDAT EQU 0x56000014 ;Port B data
pGPBUP  EQU 0x56000018 ;Pull-up control B

start
    LDR R0, =pGPBCON ; 设置GPB0为输出
    LDR R1, [R0]
    ORR R1, R1, #0x01
    STR R1, [R0]

    LDR R0, =pGPBUP ;禁止附加上拉功能到GPB0
    LDR R1, [R0]
    ORR R1, R1, #0x01
    STR R1, [R0]

;Beep on
beep_on
    LDR R0, =pGPBDAT
    LDR R1, [R0]
    ORR R1, R1, #0x01
    STR R1, [R0]

    MOV R2, #0x1000000
    BL  delay

;Beep off
beep_off
    LDR R0, =pGPBDAT
    LDR R1, [R0]
    AND R1, R1,#0x0
    STR R1, [R0]

    MOV R2, #0x1000000
    BL  delay

    b   beep_on

delay
    SUB R2, R2, #0x1
    CMP R2, #0x0
    BNE delay
    MOV PC, LR

    NOP
    END