A Brainfuck interpreter that generates bytecode from Brainfuck source code and applies several optimizations to enhance execution speed. The interpreter uses a custom bytecode structure to represent Brainfuck commands and implements an efficient execution model. Additionally, a Mandelbrot set generator has been used as a benchmark to stress test and measure the performance of the optimizations.

The project is structured as follows:
- `brainfuck.cpp`: Core interpreter and bytecode generator.
- `bf`: A sample Brainfuck program used to test the system.
- `run.sh`: A script to automate the execution and testing of the program.

## Bytecode Generation

The Brainfuck language is compiled into an intermediate bytecode format to simplify interpretation and optimization. Each Brainfuck command corresponds to a bytecode operation that performs the required memory manipulation or loop control.

### Example
```cpp
enum Bytecode {
    INC_PTR,    // '>' - Increment data pointer
    DEC_PTR,    // '<' - Decrement data pointer
    INC_VAL,    // '+' - Increment the value at the data pointer
    DEC_VAL,    // '-' - Decrement the value at the data pointer
    OUTPUT,     // '.' - Output the value at the data pointer
    INPUT,      // ',' - Input a value and store it at the data pointer
    LOOP_START, // '[' - Start of a loop
    LOOP_END    // ']' - End of a loop
};
```

The `std::vector<Instruction>` structure holds the compiled bytecode, making it easier to apply transformations and optimizations later in the execution pipeline.

## Optimizations

Several optimizations are applied to the bytecode to improve the efficiency of the interpretation:

1. **Combining Consecutive Operations**:
   - Sequences of repetitive instructions such as `++++` or `----` are collapsed into a single operation with a multiplier. This reduces the number of instructions and speeds up execution.
   
   ```cpp
   Bytecode bytecode = INC_VAL;
   bytecode.repetitions = 4;  // Equivalent to ++++ in Brainfuck
   ```

2. **Set Zero Operation**:
   - Instead of looping over a memory cell to set it to zero, a custom `SET_ZERO` operation is used, which directly zeroes out the memory cell, avoiding unnecessary iterations.
   
   ```cpp
   Bytecode bytecode = SET_ZERO;  // Translates loops like [-]
   ```

3. **Memory Shift**:
   - Consecutive memory pointer movements (e.g., `>>>`) are collapsed into one instruction with a corresponding shift amount.

## Optimization During Interpretation

While interpreting the bytecode, the system dynamically applies optimizations to reduce the overhead of certain patterns commonly found in Brainfuck programs.

1. **Loop Unrolling**:
   - Tight loops with known bounds are unrolled to reduce the overhead of managing loop control.

2. **Dynamic Zero Detection**:
   - During the execution, if the interpreter detects a loop that merely resets a memory cell to zero (e.g., `[-]`), it replaces the entire loop with a `SET_ZERO` bytecode operation.

### Example
A loop like:
```cpp
while (memory[pointer] != 0) {
    memory[pointer]--;
}
```
Is replaced with:
```cpp
memory[pointer] = 0;  // SET_ZERO
```

## Mandelbrot Test

The Mandelbrot set generation is used as a benchmark to test the performance of the interpreter and optimizations. The Mandelbrot algorithm, implemented in Brainfuck, stresses both the memory manipulation and control flow of the interpreter.

To run the Mandelbrot test, first compile and test the interpreter with the script then run it on the Brainfuck program as an argument:

```bash
./run.sh
./brainfuck bf
```
