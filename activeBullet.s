@ activeBullet.s

/* If value is 1, change to 0 or if value is 0 change to 1 
 * Used for disabling bullets
 */

.global activeBullet
activeBullet:
    @variables
    mov r1, #1
.top:
    @compare value so we know if to change it
    cmp r0, r1
    beq .isone
    add r0, r0, r1
    b .done
.isone:
    sub r0, r0, r1
.done:
    mov pc, lr
