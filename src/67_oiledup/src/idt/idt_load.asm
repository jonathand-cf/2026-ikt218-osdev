.globl idt_load

idt_load:
    mov 4(%esp), %eax   ; Get the address for idtr from the stack
    lidt (%eax)
    ret