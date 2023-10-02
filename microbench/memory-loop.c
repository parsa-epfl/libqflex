#include <stdio.h>
#include <stdlib.h>

void fillMemory(int* memory, size_t size) {
    for (size_t i = 0; i < size / sizeof(int); i++) {
        memory[i] = i;
    }
}

void loopMemory(int* memory, size_t size) {
    int iteration = 0;
    while(1) {
        for (size_t i = 0; i < size / sizeof(int); i++) {
            memory[i] = memory[i-1]*2;
        }
        if (iteration % 10000 == 0) {
            printf("Ran %d iterations\n", iteration);
        }
        iteration++;
    }
    for (size_t i = 0; i < size / sizeof(int); i++) {
        printf("%d ", memory[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <memory_size_in_kilobytes>\n", argv[0]);
        return 1;
    }

    size_t memorySize = atoi(argv[1]) * 1000;
    int* dynamicMemory = (int*) malloc(memorySize);

    if (dynamicMemory == NULL) {
        perror("Memory allocation failed");
        return 1;
    }

    fillMemory(dynamicMemory, memorySize);
    loopMemory(dynamicMemory, memorySize);

    free(dynamicMemory);

    return 0;
}