#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler. Please make sure that you use the correct compile_all.sh file in the GitHub repo to compile."
#endif

/* This will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "Must be compiled with an ix86-elf compiler."
#endif

/* Hardware text mode color constants. */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) 
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;


void terminal_initialize(void) 
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) 
{
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_writestring(const char* data);

void terminal_putchar(char c) 
{
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
	    terminal_row = 0;
    } 
}

void terminal_write(const char* data, size_t size) 
{
    for (size_t i = 0; i < size; i++) {    
        if (data[i] == '\n') {
            terminal_row++;
            terminal_column = 0;
        } else {
            if (data[i] == '\x7F')
                terminal_putchar((char)219);
            else
                terminal_putchar(data[i]);
        }
    }
}

void terminal_writestring(const char* data) 
{
    terminal_write(data, strlen(data)); 
    if (terminal_row >= (VGA_HEIGHT / 3 * 2)) {
        terminal_initialize();
        terminal_write(data, strlen(data));
    }
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

enum {
      cmos_address = 0x70,
      cmos_data    = 0x71
};

unsigned char get_RTC_register(int reg) {
      outb (cmos_address, reg);
      return inb(cmos_data);
}

unsigned char getTimeUnit(int reg) {
    unsigned char unit = get_RTC_register(reg);
    unsigned char registerB = get_RTC_register(0x0B);
    if (!(registerB & 0x04)) {
        if (reg == 0x04) { // If it's the hour unit, it get's converted differently
            unit =  ((unit & 0x0F) + (((unit & 0x70) / 16) * 10) ) | (unit & 0x80);
        } else {
            unit = (unit & 0x0F) + ((unit / 16) * 10);
        }
    }
    return unit;
}

const char* wholeDate() {
    unsigned char day = getTimeUnit(0x07);
    unsigned char month = getTimeUnit(0x08);
    unsigned char year = getTimeUnit(0x09);
    static char toReturn[9];
    // Convert day to string
    toReturn[0] = '0' + day / 10;
    toReturn[1] = '0' + day % 10;
    toReturn[2] = '/';

    // Convert month to string
    toReturn[3] = '0' + month / 10;
    toReturn[4] = '0' + month % 10;
    toReturn[5] = '/';

    // Convert year to string
    toReturn[6] = '0' + year / 10;
    toReturn[7] = '0' + year % 10;
    toReturn[8] = '\0'; // Null-terminate the string
    return toReturn;
}

const char* wholeTime() {
    unsigned char hour = getTimeUnit(0x04);
    unsigned char minute = getTimeUnit(0x02);
    static char toReturn[6];
    toReturn[0] = '0' + hour / 10;
    toReturn[1] = '0' + hour % 10;
    toReturn[2] = ':';
    toReturn[3] = '0' + minute / 10;
    toReturn[4] = '0' + minute % 10;
    toReturn[5] = '\0';
    return toReturn;
}

// This whole next section is creating a GDT
// Segment descriptor structure
struct gdt_entry {
    uint16_t limit_low;     // The lower 16 bits of the limit
    uint16_t base_low;      // The lower 16 bits of the base
    uint8_t base_middle;    // The next 8 bits of the base
    uint8_t access;         // Access flags
    uint8_t granularity;
    uint8_t base_high;      // The last 8 bits of the base
} __attribute__((packed));

// Pointer to the GDT structure
struct gdt_ptr {
    uint16_t limit;         // The upper 16 bits of all selector limits
    uint32_t base;          // The address of the first gdt_entry_t struct
} __attribute__((packed));

// GDT array
struct gdt_entry gdt[3];

// GDT pointer
struct gdt_ptr gp;

// Function to set up a GDT entry
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    // Set the base address
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    // Set the limit
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    // Set flags
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

// Function to load the GDT
static inline void gdt_load() {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;

    // Load the GDT
    asm volatile("lgdt (%0)" : : "r"(&gp));
    asm volatile("mov $0x10, %%ax; \
                  mov %%ax, %%ds; \
                  mov %%ax, %%es; \
                  mov %%ax, %%fs; \
                  mov %%ax, %%gs; \
                  ljmp $0x08, $next; \
                  next:": : : "eax");
}

void init_gdt() {
    // Null segment
    gdt_set_gate(0, 0, 0, 0, 0);

    // Code segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // 0x9A = present, ring 0, code segment
                                                // 0xCF = 4 KB granularity, 32-bit mode, limit in 4 KB blocks

    // Data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // 0x92 = present, ring 0, data segment
                                                // 0xCF = 4 KB granularity, 32-bit mode, limit in 4 KB blocks

    // Load the GDT
    gdt_load();
}

// This next section is for initialising an IDT
#define IDT_MAX_DESCRIPTORS 32 // or whatever value you intend to use
bool vectors[IDT_MAX_DESCRIPTORS] = {false};
typedef struct {
    uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t     reserved;     // Set to zero
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10)))
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

typedef struct {
    uint16_t    limit;
    uint32_t    base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr;

void (*isr_stub_table[34])() = {
    NULL
};

void idt_set_descriptor(uint8_t vector, void (*isr)(), uint8_t flags) {
    isr_stub_table[vector] = isr;
    idt[vector].isr_low = (uint32_t)isr & 0xFFFF;
    idt[vector].kernel_cs = 0x08;
    idt[vector].reserved = 0;
    idt[vector].attributes = flags | 0x60;
    idt[vector].isr_high = ((uint32_t)isr >> 16) & 0xFFFF;
}

void isr_divide_by_zero() {
    terminal_setcolor(VGA_COLOR_RED);
    terminal_writestring("EXCEPTION: Divide by zero\n");
    __asm__("cli; hlt");
}

void isr_stub_divide_by_zero() {
    // Push error code and call the actual ISR
    asm volatile("push $0");
    asm volatile("jmp isr_divide_by_zero");
}

void idt_init() {
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint32_t)&idt;
    idt_set_descriptor(0, isr_divide_by_zero, 0x8E);
    __asm__("lidt %0" : : "m"(idtr));
}

void remap_PIC() {
    // ICW1: Start initialization of PIC
    outb(0x20, 0x11); // Master PIC
    outb(0xA0, 0x11); // Slave PIC

    // ICW2: Set interrupt vector offsets
    outb(0x21, 0x20); // Master PIC vector offset
    outb(0xA1, 0x28); // Slave PIC vector offset

    // ICW3: Tell Master PIC there is a slave PIC at IRQ2 (0000 0100)
    outb(0x21, 0x04);
    // Tell Slave PIC its cascade identity (0000 0010)
    outb(0xA1, 0x02);

    // ICW4: Set PIC to x86 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask interrupts on both PICs
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void init_IRQ();

bool shifted = false;
bool capslock = false;

bool inScanf = false;

char wholeInput[100] = "";

void addCharToString(char *str, char c) {
    // Iterate to find the end of the string
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    // Append the character
    str[i] = c;
    // Null-terminate the string
    str[i + 1] = '\0';
}

unsigned int inputLength = 0;

unsigned char convertScancode(unsigned char scancode) {
   char characterTable[] = {
    0,    0,    '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',
    '-',  '=',  0,    0x09, 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
    'o',  'p',  '[',  ']',  0,    0,    'a',  's',  'd',  'f',  'g',  'h',
    'j',  'k',  'l',  ';',  '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',  0,    ' ',  0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0x1B, 0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0x0E, 0x1C, 0,    0,    0,
    0,    0,    0,    0,    0,    '/',  0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0,
    0,    0,    0,    0,    0,    0,    0,    0x2C
    };
    char shiftedCharacterTable[] = {
    0,    0,    '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',
    '_',  '+',  0,    0x09, 'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
    'O',  'P',  '{',  '}',  0,    0,    'A',  'S',  'D',  'F',  'G',  'H',
    'J',  'K',  'L',  ':',  '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',  0,    ' ',  0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0x1B, 0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0x0E, 0x1C, 0,    0,    0,
    0,    0,    0,    0,    0,    '?',  0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0,
    0,    0,    0,    0,    0,    0,    0,    0x2C,
    };
    if (scancode == 0x3A) {
        capslock = !capslock;
        return '\0';
    }
    if (scancode == 0x2A || scancode == 0x36) {
        shifted = true;
        return '\0';
    }
    if (scancode == 0x0E && inputLength != 0) {
        terminal_column--;
        terminal_putentryat(' ', VGA_COLOR_BLACK, terminal_column, terminal_row);
        inputLength--;
        wholeInput[inputLength] = '\0';
        return '\0';
    }
    if (scancode == 0x1C) {
        inScanf = false;
        return '\0';
    }

    inputLength++;
    if (capslock || shifted) {
        shifted = false;
        return shiftedCharacterTable[scancode];
    }
    unsigned char toReturn = characterTable[scancode];
    shifted = false;
    return toReturn;
}

void strcpy(char* dest, const char* src) {
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0'; // Null-terminate the destination string
}

void scanf(char* inp) {
    shifted = false;
    capslock = false;
    inScanf = true;
    // Reset wholeInput
    int i = 0;
    while (1) {
        if (wholeInput[i] == '\0')
            break;
        wholeInput[i] = '\0';
        i++;
    }
    while (inScanf) {
        __asm__("hlt");
        // This next part will done only when the next interrupt is called.
        if (inb(0x60) == 0x1C)
            break;
    }
    terminal_write("\0", 1); // I'm not really sure why, but if I don't write something to the terminal after the device crashes.
    strcpy(inp, wholeInput);
}

void isr_keyboard() {
    unsigned char status = inb(0x64); // Read the status register of the keyboard controller
    if (status & 0x01) { // Check if the least significant bit (bit 0) is set, indicating data is available
        unsigned char scan_code = inb(0x60); // Read the scan code from the keyboard controller
        // Check if it's a key press event (high bit not set)
        if (!(scan_code & 0x80) && inScanf) {
            // Handle the key press event
            char combined = convertScancode(scan_code);
            if (combined != '\0')
                addCharToString(wholeInput, combined);
            terminal_putchar(combined);
        }
    }
    outb(0x20, 0x20); // Acknowledge interrupt from master PIC
    outb(0xa0, 0x20); // Acknowledge interrupt from slave PIC
    init_IRQ();
}


void init_IRQ() {
    remap_PIC();
    idt_set_descriptor(33, isr_keyboard, 0x8E); // Map IRQ 1 (keyboard) to vector 33
    outb(0x21, ~(1 << 1)); // Unmask IRQ 1 (keyboard)
    __asm__("sti");
}

void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Function to convert size_t to string
void size_t_to_str(size_t num, char* buffer) {
    int i = 0;
    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }
    while (num != 0) {
        int rem = num % 10;
        buffer[i++] = rem + '0';
        num = num / 10;
    }
    buffer[i] = '\0';
    reverse(buffer, i);
}

int compareDifferentLengths(const char *longer, const char *shorter) {
    char binmap[10];
    size_t num = 0;
    while (num < strlen(shorter) - 1) {
        if (longer[num] != shorter[num]) {
            terminal_writestring(binmap);
            return 0;
        }
        binmap[num] = '1';
        num++;
    }
    return 1;
}


void test_userspace() {
    char inp[100];
    while(1) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring(">> ");
        terminal_setcolor(VGA_COLOR_WHITE);
        scanf(inp);
        if (compareDifferentLengths(inp, "echo") != 0) {
            terminal_writestring("\nArgument: ");
            scanf(inp);
            terminal_writestring("\n");
            terminal_setcolor(VGA_COLOR_LIGHT_GREY);
            terminal_writestring(inp);
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "timedate") != 0) {
            terminal_writestring("\nTime: ");
            terminal_writestring(wholeTime());
            terminal_writestring("\nDate: ");
            terminal_writestring(wholeDate());
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "colours") || compareDifferentLengths(inp, "colors")) {
            terminal_writestring("\n");
            for (enum vga_color colour = VGA_COLOR_BLACK; colour <= VGA_COLOR_WHITE; colour++) {
                terminal_setcolor(colour);
                terminal_writestring("\x7F");
            }
            terminal_writestring("\n");
        } else if (compareDifferentLengths(inp, "poweroff")) {
            terminal_writestring("\nAre you sure you would like to power off your device? (y/N)");
            scanf(inp);
            if (compareDifferentLengths(inp, "y") != 0) {
                terminal_writestring("\nBeware that this only works on emulators such as Qemu, Bochs, VirtualBox, and Cloud \nHypervisor.\nIf you are running on real hardware, you may need to disconnect your device from power or press the physical power button.\nTrying to power off...\n");
                    outw(0xB004, 0x2000);
                    outw(0x604, 0x2000);
                    outw(0x4004, 0x3400);
                    outw(0x600, 0x34);
            }
        }
        else {
            terminal_setcolor(VGA_COLOR_RED);
            terminal_writestring("\nCommand not found.\n");
        }
    }
}


void init_kernel() 
{
    terminal_initialize();
    terminal_setcolor(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("Successful boot!\n");
    terminal_writestring("\n\n");
    terminal_writestring("Initialising GDT...\n");
    init_gdt();
    terminal_writestring("Initialising IDT...\n");
    idt_init();
    terminal_writestring("Initialising IRQs...\n");
    init_IRQ();
    test_userspace();
    while(1);
}

void kernel_main() {
    init_kernel();
}
