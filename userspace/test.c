/* Test userspace program for the kernel to try and run.
 * It simply calls a kernel defined syscall which is set to display some text, then hangs.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

void main() {
    asm("int $0x80");
    for (;;);
}
