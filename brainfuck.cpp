#include <iostream>
#include <vector>
#include <stack>
#include <fstream>

// 1. Optimizations worth applying: collapse sequences of repeated operations, loops that reset values,
//    trivial operations, adjacent pointer movements, and adjacent input/output operations.
// 2. Advanced optimizations are difficult because Brainfuck operates directly on memory and loops may have
//    dependencies on memory locations.
// 3. Optimization example: `[-]` sets a memory location to zero, this can be collapsed into a `SET_ZERO` operation.

// Optimizations applied:
// 1. Combine repeated operations (e.g., `+++` → `INC_VAL 3`).
// 2. Combine repeated pointer movements (e.g., `>>>` → `INC_PTR 3`).
// 3. Collapse zero-setting loops (e.g., `[-]` → `SET_ZERO`).
// 4. Simplify loops like `[->+<]` (copy current value to another cell).
// 5. Collapse adjacent input/output operations (e.g., `..` → `2 * OUTPUT`).

enum Bytecode {
    INC_PTR,
    DEC_PTR,
    INC_VAL,
    DEC_VAL,
    OUTPUT,         // Optimized multiple output instructions
    INPUT,          // Optimized multiple input instructions
    LOOP_START,
    LOOP_END,
    SET_ZERO,      // Optimization for `[-]` pattern
    ADD_TO_NEXT    // Optimization for `[->+<]` and similar patterns
};

struct Instruction {
    Bytecode op;
    int value;  // for repeated operations, default 1
};

// Compiles Brainfuck code to optimized bytecode
std::vector<Instruction> compile_to_bytecode(const std::string& program) {
    std::vector<Instruction> bytecode;
    std::stack<int> loop_stack;  // matches LOOP_START with LOOP_END

    size_t program_size = program.size();
    char cmd;
    int count;
    for (size_t i = 0; i < program_size; ++i) {
        cmd = program[i];
        count = 1;
        switch (cmd) {
            case '>': {
                while (i + 1 < program_size && program[i + 1] == '>') { ++count; ++i; }
                bytecode.push_back({INC_PTR, count});
                break;
            }
            case '<': {
                while (i + 1 < program_size && program[i + 1] == '<') { ++count; ++i; }
                bytecode.push_back({DEC_PTR, count});
                break;
            }
            case '+': {
                while (i + 1 < program_size && program[i + 1] == '+') { ++count; ++i; }
                bytecode.push_back({INC_VAL, count});
                break;
            }
            case '-': {
                while (i + 1 < program_size && program[i + 1] == '-') { ++count; ++i; }
                bytecode.push_back({DEC_VAL, count});
                break;
            }
            case '.': {
                while (i + 1 < program_size && program[i + 1] == '.') { ++count; ++i; }
                bytecode.push_back({OUTPUT, count});
                break;
            }
            case ',': {
                while (i + 1 < program_size && program[i + 1] == ',') { ++count; ++i; }
                bytecode.push_back({INPUT, count});
                break;
            }
            case '[':
                if (i + 2 < program_size && program[i + 1] == '-' && program[i + 2] == ']') {
                    bytecode.push_back({SET_ZERO, 1});
                    i += 2;
                } else if (i + 5 < program_size && program.substr(i, 6) == "[->+<]") {
                    bytecode.push_back({ADD_TO_NEXT, 1});
                    i += 5;
                } else {
                    bytecode.push_back({LOOP_START, 0});
                    loop_stack.push(bytecode.size() - 1);
                }
                break;
            case ']':
                bytecode.push_back({LOOP_END, 0});
                if (loop_stack.empty()) {
                    std::cerr << "Unmatched ']' at position " << i << std::endl;
                    exit(1);
                }
                loop_stack.pop();
        }
    }
    if (!loop_stack.empty()) {
        std::cerr << "Unmatched '[' at position " << loop_stack.top() << std::endl;
        exit(1);
    }
    return bytecode;
}

// Interprets the compiled bytecode
void interpret_bytecode(const std::vector<Instruction>& bytecode) {
    std::vector<unsigned char> memory(30000, 0); // "Brainfuck uses 30,000 cells"
    size_t ptr = 0;
    std::stack<size_t> loop_stack;

    for (size_t pc = 0; pc < bytecode.size(); ++pc) {
        const auto& instr = bytecode[pc]; // TODO fix
        switch (instr.op) {
            case INC_PTR: ptr += instr.value; break;
            case DEC_PTR: ptr -= instr.value; break;
            case INC_VAL: memory[ptr] += instr.value; break;
            case DEC_VAL: memory[ptr] -= instr.value; break;
            case OUTPUT:
                for (int j = 0; j < instr.value; ++j)
                    std::cout << memory[ptr];
                break;
            case INPUT:
                for (int j = 0; j < instr.value; ++j)
                    memory[ptr] = std::cin.get();
                break;
            case SET_ZERO: memory[ptr] = 0; break;
            case ADD_TO_NEXT:
                memory[ptr + 1] += memory[ptr];
                memory[ptr] = 0;
                break;
            case LOOP_START:
                if (memory[ptr] == 0) { // Jump to matching LOOP_END
                    int depth = 1;
                    while (depth != 0) {
                        ++pc;
                        if (bytecode[pc].op == LOOP_START) ++depth;
                        else if (bytecode[pc].op == LOOP_END) --depth;
                    }
                }
                break;
            case LOOP_END:
                if (memory[ptr] != 0) { // Jump to matching LOOP_START
                    int depth = 1;
                    while (depth != 0) {
                        --pc;
                        if (bytecode[pc].op == LOOP_END) ++depth;
                        else if (bytecode[pc].op == LOOP_START) --depth;
                    }
                }
                break;
        }
    }
}

// Reads program file (or stdin if piped) into a string
std::string read_program(const std::string& program_file) {
    std::ifstream file(program_file);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << program_file << std::endl;
        exit(1);
    }
    std::string program((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return program;
}

// Main function to handle the CLI
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./brainfuck [-c] program_file\n";
        return 1;
    }

    bool print_bytecode = false;
    std::string program_file;
    if (std::string(argv[1]) == "-c") {
        print_bytecode = true;
        program_file = argv[2];
    } else {
        program_file = argv[1];
    }

    std::string program = read_program(program_file);

    std::vector<Instruction> bytecode = compile_to_bytecode(program);

    if (print_bytecode) { // Output the bytecode instead of executing
        for (const Instruction& instr : bytecode) {
            switch (instr.op) {
                case INC_PTR: std::cout << "INC_PTR " << instr.value << " "; break;
                case DEC_VAL: std::cout << "DEC_VAL " << instr.value << " "; break;
                case INC_VAL: std::cout << "INC_VAL " << instr.value << " "; break;
                case DEC_PTR: std::cout << "DEC_PTR " << instr.value << " "; break;
                case OUTPUT: std::cout << "OUTPUT " << instr.value << " "; break;
                case INPUT: std::cout << "INPUT " << instr.value << " "; break;
                case SET_ZERO: std::cout << "SET_ZERO "; break;
                case ADD_TO_NEXT: std::cout << "ADD_TO_NEXT "; break;
                case LOOP_START: std::cout << "LOOP_START "; break;
                case LOOP_END: std::cout << "LOOP_END "; break;
                default: std::cout << "UNKNOWN "; break;
            }
        }
        std::cout << std::endl;
    } else { // Execute the bytecode
        interpret_bytecode(bytecode);
    }
    return 0;
}
