@stagger.s

/* Variable counter for if mario should be staggered */

.global stagger
stagger:
    @variables
    mov r3, #15
    mov r2, #1
.start:
    @compare to know what to change
    cmp r0, r3
    bge .reset
    add r0, r0, r2
    b .done
.reset:
    mov r0, #0
.done:
    mov pc, lr
