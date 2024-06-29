.syntax unified
    .global DelayCycles
DelayCycles:
    subs r0, r0, #1
    bcs DelayCycles
    bx lr
