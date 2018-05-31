FunctionBuilder builder;

Function({Instr::Pushc(Object("string")),
          Instr::Pushc(Object("string")),
          Instr::Pushn(2),
          Instr::CallBuiltin(concat)})
             
Function({cons->make()})        
builder
 .data(object)
 .data(object)
 .pushc(0)
 .pushc(1)
 .callBuiltin(builtin);


FORM---+
|      \
LAMBDA  MACRO

Argument lists
--------------
list				positional_args			key_args		has_rest
()											false
(x y)				x,y
(x y &keys z)			x,y				z			false
(x y &rest rst)			x,y							true
(x y &keys z &rest rst)		x,y				z			true
(&rest rst)  	   									true


---------------------------------------------------
1
SETV &1      ; V=1
&1 1

builder
.data(Numeric.make(1))
.setv(0)

---------------------------------------------------
(+ 1 2)
PUSHC &1     ; S=1
PUSHC &2     ; S=2,1
PUSHN 2      ; S=2,2,1
CALL '+      ; S= V=3
&1 1
&2 2

---------------------------------------------------
(+ 1 2 (+ 3 4))
PUSHC &1     ; S=1
PUSHC &2     ; S=2,1
PUSHC &3     ; S=3,2,1
PUSHC &4     ; S=4,3,2,1
PUSHN 2      ; S=2,4,3,2,1
CALL '+      ; S=2,1 V=7
PUSHV        ; S=7,2,1
PUSHN 3      ; S=3,7,2,1
CALL '+      ; S= V=10
&1 1
&2 2
&3 3
&4 4

---------------------------------------------------
(lambda (x) (+ x 1))
PUSHC &1     ; S=(x)
PUSHC &2     ; S=(+ x 1), (x)
PUSHN 2      ; S=2, (+ x 1), (x) 
CALL 'LAMBDA ; S= V=#LAMBDA
&1 (x)
&2 (+ x 1)

#LAMBDA:
PUSHA @1        ; S=@1
PUSHC &1        ; S=1,@1
PUSHN 2         ; S=2,1,@1
CALL '+         ; S=  V=@1+1
RET             ; S=
&1 1

---------------------------------------------------
((lambda (x) (+ x 1)) 2)
PUSHC &1        ; S=2
PUSHN 1         ; S=1,2
PUSHA @1        ; S=@1,1,2              S=2
PUSHC &2        ; S=1,@1,1,2            S=1,2
PUSHN 2         ; S=2,1,@1,1,2          S=2,1,2
CALL '+         ; S=1,2 V=3             S= V=3
RET             ; S= V=3
&1 2
&2 1
