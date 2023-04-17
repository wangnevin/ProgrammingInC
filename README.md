# Parser / Intepreter in C

The programming language LISP, developed in 1958, is one of the oldest languages still in common use. The langauage is famous for: being fully parenthesised (that is, every instruction is inside its own brackets), having a prefix notation (e.g. functions are written ```(PLUS 1 2)``` and not ```(1 PLUS 2)```) and its efficent linked-list Car/Cdr structure for (de-)composing lists.

Here, I developed a means to parse and interpret the instructions for a simple langauge inspired by these concepts.

The Formal Grammar for this language can be found here: [Grammar](./grammar.txt)

## Run the program

The suffix of the source code file name is ```.ncl```
- To parse a source code
  1. In comman line, type ```make parse```
  2. If the name for the source code file is ```code.ncl```, then type ```parse code.ncl```
  3. If the output is ```Parsed OK```, then there is no syntax errors in the code. Otherwise an error message will be shown.
- To intepret a source code
  1. In command line, type ```make interp```
  2. If the name for the source code file is ```code.ncl```, then type ```interp code.ncl```
  3. The result of executing the code will be shown.
- To clear all the built files, type ```make clean```

## Examples
For the following code:
```
(
  (SET A ’1’)
  (PRINT A)
)
```
The parser would output:
```
Parsed OK
```
The intepreter would output:
```
1
```
### CONS
The ```CONS``` instruction is used to construct lists:
```
(
  (PRINT (CONS ’1’ (CONS ’2’ NIL)))
)
```
The intepreter would output:
```
(1 2)
```
### CARS
The CAR instruction is used to deconstruct lists:
```
(
  (SET A ’(5 (1 2 3))’)
  (PRINT (CAR A))
)
```
The intepreter would output:
```
5
```
### WHILE
Here is a loop counts down from 5 to 1, using the variable C as a counter and a Boolean test:
```
(
  (SET C ’5’)
  (WHILE (LESS ’0’ C)(
    (PRINT C)
    (SET A (PLUS ’-1’ C))
    (SET C A))
  )
)
```
The intepreter would output:
```
5
4
3
2
1
```
### IF
The IF is similar; based on a Boolean, one of two possible sets of instructions are taken:
```
(
  (IF (EQUAL ’1’ ’1’) ((PRINT "YES"))((PRINT "NO")))
)
```
The intepreter would output:
```
YES
```

## Testing Strategy
Testing is divided into three parts:
1. helper test
   - Defined in nuclei.c -> void helper_test(void)
   - For testing small tool functions used in parse and interpret
2. read file test
   - Defined in readfile.c -> void read_file_test(void)
   - For testing functions related to file reading
3. main test
   - Defined in driver.c -> void main_test(void)
   - For testing main and sub functions related to parsing grammar and interpreting code

None of the testing above involve reading files, all tests are called in 'void test(void)' in driver.c

- In helper test:
    It's not grammar related, just to verify that every function operates in expected way under certain circumistances.
- In read file test:
    String is used to represent every line in .ncl file. A single string is converted to an array of strings by read-file functions, and assertions are used to check correctness of the output. Eg:
```
code = ((SET A '(1 2)'))
tokens = read_file(code)
assert(tokens[0] == ()
assert(tokens[1] == ))
assert(tokens[2] == SET)
assert(tokens[3] == A)
......
```
- In main test:
    It dose not involve any reading file process, instead hard-coded tokens are provided as test cases. Assertions are applied to the generated variable to verify the interpreter is working properly. Eg:
```
code = ((SET A '(1 2)'))
VariableList = Interp(code)
assert(VariableList(A) == (1 2))
```
All the test can be run by typing command ```make runtest```, the testing results will be redirected to ```test_results.txt```.
