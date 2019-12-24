//------------------------------------------------------------------------------
/// @brief Kernel Lab: Page Table Walk (test.c)
///
/// @section license_section License
/// Copyright (c) 2018-2019, Computer Systems and Platforms Laboratory, SNU
/// All rights reserved.
///
/// Redistribution and use in source and binary forms,  with or without modifi-
/// cation, are permitted provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice,
///   this list of conditions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER  OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE
/// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT
/// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

static volatile int terminate = 0;

void handler(int dummy) {
        terminate = 1;
}

int main(int argc, char *argv[]) {
        int input, i;
        unsigned long size;
        unsigned long *mem;

        if(argc != 2) {
                printf("Usage: ./test <memory_size>\n");
                exit(-1);
        }

        printf("My PID: %d\n", getpid());
        signal(SIGINT, handler);

        size = atoi(argv[1]);
        srand(time(NULL));
        mem = (unsigned long *)malloc(sizeof(unsigned long) * size);
        if(!mem) {
                printf("Error in memory allocation!\n");
                exit(-1);
        }

        printf("Allocated: %lu Bytes\n", sizeof(unsigned long) * size);
        while(1) {
                if(terminate)
                        break;
                for(i = 0; i < size; i++) {
                        mem[i] = rand() % 10000000000;
                }
                sleep(1);
        }

        printf("\nBye\n");

        free(mem);
        return 0;
}
