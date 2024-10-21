#!/bin/bash
: <<'````````bash'
# Programming Exercise 1: Brainfuck-Interpreter

## Implementation

Write an efficient interpreter for Brainfuck with 8-bit cells by compiling Brainfuck to bytecode. Unmatched brackets are valid as long as the branch is never taken (i.e., `+[` is a valid program). You can define your bytecode as you like, but focus on efficient generation, optimization, and interpretation. Combine instruction sequences (both sequences of identical operations and loop patterns) and implement super-instructions/combined execution of multiple neighboring instructions as optimization.


## Analysis (write answers as comment in your code)

Find suitably large and complex code examples to test the correctness and performance of your submission and document the results.

Compare the compile-time and the run-time -- at what point are the optimizations worth the effort?

How can more advanced optimizations be implemented? Why is this difficult? Give one Brainfuck code example and explain which optimization could lead to better bytecode.


## Command Line Interface

    usage: ./brainfuck [-c] program_file
        Execute a Brainfuck program.
        -c: print bytecode instead of executing
        program_file: file that contains the Brainfuck program

Note that program_file is not necessarily a regular file, but can also be a pipe as used in the tests below, where you cannot determine the input size without reading it.

You can add extra options, e.g., to control optimizations or for debugging.


## Submission

- Submission deadline: 2024-10-23 23:59
- Submit using `curl --data-binary "@<your-submission-file>" 'https://db.in.tum.de/teaching/ws2425/codegen/submit.py?hw=1&matrno=<your-matrno>'`
- Write your solution in a single C++ file.
- Include answers to theory questions as comments at the top of the source file.
- Make sure your submission passes as many tests as possible from this test script.
- No external dependencies, build systems other than Makefile, etc.
- If you really, *really* need more than just the C++ file, need to write a Makefile, or want to use C instead, combine all files s.t. this command sequence works: `split-file <submission> somedir || cp <submission> somedir/brainfuck.cc; cd somedir; bash <hw1.txt>`

````````bash
set -euo pipefail

FAILED=0
testcase() { ./brainfuck "$2" | "${@:3}" || (echo "FAILED: $1"; FAILED=1); }

CC=gcc CXX=g++ CFLAGS="-O3 -std=gnu11" CXXFLAGS="-O3 -std=c++20" make brainfuck

# NOTE: test cases use cmp to compare stdout with expected output

testcase "+." <(echo "+.") cmp <(echo -ne '\x01')
testcase "+-." <(echo "+-.") cmp <(echo -ne '\x00')
testcase "+>." <(echo "+>.") cmp <(echo -ne '\x00')

testcase "3434" <(base64 -d <<EOF | gunzip
H4sIAAAAAAAAA9PWJhnYka5FW9tGz04PhLkAIcEna3EAAAA=
EOF
) cmp <(echo -n "3434")

testcase "scrub right" <(echo ">+>->->+<[[-]<].>.>.>.>.") cmp <(echo -ne '\x00\x00\x00\x00\x01')

testcase "helloworld" <(base64 -d <<EOF | gunzip
H4sIAAAAAAAAAzWLyQ0AIQwD/7QS2RWgaQTRfxvrLGDJzuSqOlq8bDiekTaFYmrN3S0GSb6PbjDO
abJZv24JkHXG4wMEkgyIawAAAA==
EOF
) cmp <(echo "Hello World!")

testcase "mandelbrot" <(base64 -d <<EOF | gunzip
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
EOF
) cmp <(base64 -d <<EOF | gunzip
H4sIAAAAAAAAA82YWXKrMBBF/1lFbyUmaAYxmRc7+9/HawkjJimAKxXpftmmSve4JyR9fKx1Wys/
oc9JBSWkQI3f3PPbj8p+Arji/onOhHI+Qkw/nqDIgv5X3K1/QQjhIBmllFHHcAiR+QGu2Y8ASEAZ
L3upnpqS4ixD5gN4yx9zQBnjsq4lZ+tczAweiGwPcNF+8jcBELID+MJMMGpL8gxDtgV427+p74Pu
HwDwbNR3yUYEB7FYIUBwzt9nT4wwA0I9YdJdvhIxQyyX8RG84e/c0R4BsAJglkkENc/8DAGCC/6u
9DhnRpxjDZTt5D+UUpgH9EXYjd3pDUPmAE7bz5Un0cx4CQ1QQWuK0Opx71qo1QDKUCCmFliZAYTs
FMDa/BV9ykAbv76CDvyqpJAtKNWUFSsCCBPBoX2x0BhajH0VcJ7VjIVZ/qNFACE7ANiaz9Vnul/p
/pDB6FtMI2q1+ILgtLsrfvYYoG3up/ybUpjG2Achnwj2AC7tBEuarETp2HviOAMLiXFKbhsi9xBs
250p29lm5Nt/Mn7E5m/bY1+newMKGNkT5Jbgtnd3HUeYbMpWVthxauhACMmthLoCYPSoXgTbIDiC
/awvbL6x3jSmW9vOq01rCeRQF/1Bm2x6CHJDsLFfNP2Y8HLzd9tSlZeKwKrqyW77NhOE/ZnUVffc
rtbUV3MAlaB+gjzDJOzmva15Xgs77BvPesPX5RiYJAQIluW/6Dgs96fWdR0auFcB6Lxp2hJMb1n0
dM2G2ReqevyOOb45xHrbtiH4LZugamVHYpggfgzi10H8Xog/DxKYifHfC/HfjfH3BwnskeLvExPY
K8c/LyRwZop/bkzg7JzA/UH8O5QE7pESuEtL4D4xgTvVBO6VNwx/dLf+H5YQgvpgGAAA
EOF
)

exit $FAILED

