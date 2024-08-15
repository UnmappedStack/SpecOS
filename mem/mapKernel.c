/* Maps kernel into virtual memory, as part of the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "include/mapKernel.h"
#include "../include/kernel.h"
#include "../utils/include/printf.h"
#include "include/paging.h"
#include "../limine.h"

// get all the labels defined in the linker file
extern uint64_t readonly_start;
extern uint64_t readonly_end;
extern uint64_t nxe_enabled_start;
extern uint64_t nxe_enabled_end;
extern uint64_t writeallowed_start;
extern uint64_t writeallowed_end;

void mapKernel() {
    printf("In the kernel mapping function :D\nRead only section start: 0x%x\n", &readonly_start);
}
