@ activeBullet.s

/* If value is 1, change to 0 or if value is 0 change to 1 
 * Used for disabling bullets
 */

.global activeBullet
activeBullet:
    @variables
    mov r3, #1
.top:
    @compare value so we know if to change it
    cmp r0, #1
    beq .odd
    add r0, r0, r3
    b .done
.odd:
    sub r0, r0, r3
    b .done
.done:
    mov pc, lr
