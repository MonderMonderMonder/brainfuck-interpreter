#include <iostream>
#include <vector>
#include <stack>
#include <fstream>

// 1. Optimizations are valuable when dealing with repetitive instructions, loops and operations that affect multiple cells,
//    like memory initialization, but also trivial operations, adjacent pointer movements, and adjacent input/output operations.
// 2. Advanced optimizations like loop unrolling, dead code elimination, or just-in-time (JIT)
//    compilation are challenging because they require a better understanding of the program's runtime
//    behavior. For example, loops in Brainfuck can have dependencies on memory locations, and optimizing them
//    requires analyzing how memory cells interact across different iterations of the loop.
// 3. Optimization example: `[-]` is used to set the current memory cell to zero. Instead of interpreting
//    this loop every time, we can replace it with a single "SET_ZERO" instruction, resulting in much
//    faster execution.

// Optimizations applied:
// - Combine repeated operations (e.g., `+++` → `INC_VAL 3`)
// - Combine repeated pointer movements (e.g., `>>><<>` → `INC_PTR 2`)
// - Collapse zero-setting loops (e.g., `[-]` → `SET_ZERO`)
// - Collapse adjacent input/output operations (e.g., `..` → `OUTPUT 2`)

enum Bytecode {
    INC_PTR,
    DEC_PTR,
    INC_VAL,
    DEC_VAL,
    OUTPUT,
    INPUT,
    LOOP_START,
    LOOP_END,
    SET_ZERO,           // Optimization for `[-]` pattern
    CLEAR_RANGE,        // Optimization for clearing a range of cells to zero
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

std::vector<Instruction> optimize_bytecode(const std::vector<Instruction>& bytecode) {
    std::vector<Instruction> optimized;
    size_t bytecode_size = bytecode.size();

    for (size_t i = 0; i < bytecode_size; ++i) {

        // Combine consecutive value modifications
        if (bytecode[i].op == INC_VAL || bytecode[i].op == DEC_VAL) {
            int modification = (bytecode[i].op == INC_VAL) ? bytecode[i].value : -bytecode[i].value;
            while (i + 1 < bytecode_size && (bytecode[i + 1].op == INC_VAL || bytecode[i + 1].op == DEC_VAL)) {
                modification += (bytecode[i + 1].op == INC_VAL ? bytecode[i + 1].value : -bytecode[i + 1].value);
                ++i;
            }
            if (modification > 0)
                optimized.push_back({INC_VAL, modification});
            else if (modification < 0)
                optimized.push_back({DEC_VAL, -modification});
            // If modification == 0, we skip adding any instruction since it has no effect
        }

        // Combine consecutive pointer modifications
        else if (bytecode[i].op == INC_PTR || bytecode[i].op == DEC_PTR) {
            int modification = (bytecode[i].op == INC_PTR) ? bytecode[i].value : -bytecode[i].value;
            while (i + 1 < bytecode_size && (bytecode[i + 1].op == INC_PTR || bytecode[i + 1].op == DEC_PTR)) {
                modification += (bytecode[i + 1].op == INC_PTR ? bytecode[i + 1].value : -bytecode[i + 1].value);
                ++i;
            }
            if (modification > 0)
                optimized.push_back({INC_PTR, modification});
            else if (modification < 0)
                optimized.push_back({DEC_PTR, -modification});
        }

        // Optimize `[-]' patterns that may have went undetected due to comments
        else if (i + 2 < bytecode_size &&
            bytecode[i].op == LOOP_START && bytecode[i].value == bytecode[i + 2].value &&
            bytecode[i + 1].op == DEC_VAL && bytecode[i + 1].value == 1 &&
            bytecode[i + 2].op == LOOP_END) {

            optimized.push_back({SET_ZERO, 0});
            i += 2;
        }

        // Optimize loops that clear a range of cells (e.g., `[-]>[-]>[-]`)
        else if (i + 1 < bytecode_size && bytecode[i].op == SET_ZERO) {
            int clear_count = 1;
            while (i + 1 < bytecode_size && bytecode[i + 1].op == SET_ZERO) { ++clear_count; ++i; }
            if (clear_count > 1)
                optimized.push_back({CLEAR_RANGE, clear_count});
            else
                optimized.push_back(bytecode[i]);
        }

        else
            optimized.push_back(bytecode[i]); // Default case: push the current instruction
    }
    return optimized;
}


void interpret_bytecode(const std::vector<Instruction>& bytecode) {
    std::vector<unsigned char> memory(30000, 0); // "Brainfuck uses 30,000 cells"
    size_t ptr = 0;
    std::vector<size_t> loop_starts(bytecode.size(), 0); // Precomputed loop jumps
    std::stack<size_t> loop_stack;

    // Precompute loop start/end positions for faster jumping
    for (size_t pc = 0; pc < bytecode.size(); ++pc) {
        if (bytecode[pc].op == LOOP_START) {
            loop_stack.push(pc);
        } else if (bytecode[pc].op == LOOP_END) {
            size_t start = loop_stack.top();
            loop_stack.pop();
            loop_starts[start] = pc;
            loop_starts[pc] = start;
        }
    }

    for (size_t pc = 0; pc < bytecode.size(); ++pc) {
        const auto& instr = bytecode[pc];
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
            case CLEAR_RANGE:
                for (int j = 0; j < instr.value; ++j)
                    memory[ptr + j] = 0;
                ptr += instr.value;
                break;
            case LOOP_START:
                if (memory[ptr] == 0)
                    pc = loop_starts[pc];
                break;
            case LOOP_END:
                if (memory[ptr] != 0)
                    pc = loop_starts[pc] - 1; // -1 to adjust for the upcoming increment
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
    for (int i = 0; i < 7; ++i)
        bytecode = optimize_bytecode(bytecode);

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
                case CLEAR_RANGE: std::cout << "CLEAR_RANGE "; break;
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
