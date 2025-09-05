[data]
    nums        5   3   2   6   8   9
    num_count   6

[program]

.factorial
    loadc       ax      1
    cmp         bx      ax
    jmpz        factorial_end
    push        bx
    sub         bx      ax
    copy        ax      bx
    call        factorial               ; recursive call
    pop         bx
    mul         ax      bx
.factorial_end
    ret

.main
    loadc       cx      0
.main_loop0
    copy        cx      ax
    loadc       bx      4               ; increment ptr by 4
    mul         ax      bx
    
    loadc       bx      nums
    add         ax      bx              ; get nums ptr
    load        bx      ax

    call        factorial
    
    loadc       bx      0
    syscall     0x41                    ; print reg 0 (ax)

    loadc       ax      1
    add         cx      ax
    copy        ax      cx

    loadc       ax      num_count
    load        ax      ax
    cmp         cx      ax
    
    jmpz        main_end
    jmp         main_loop0
.main_end