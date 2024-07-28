# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

override KERNEL := SpecOS

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

override DEFAULT_KCC := cc
$(eval $(call DEFAULT_VAR,KCC,$(DEFAULT_KCC)))

# user controllable C flags.
override DEFAULT_KCFLAGS := -g -O2 -pipe
$(eval $(call DEFAULT_VAR,KCFLAGS,$(DEFAULT_KCFLAGS)))

# user controllable C preprocessor flags, set none by default.
override DEFAULT_KCPPFLAGS :=
$(eval $(call DEFAULT_VAR,KCPPFLAGS,$(DEFAULT_KCPPFLAGS)))

# user controllable nasm flags.
override DEFAULT_KNASMFLAGS := -F dwarf -g
$(eval $(call DEFAULT_VAR,KNASMFLAGS,$(DEFAULT_KNASMFLAGS)))

# user controllable linker flags, set none by default.
override DEFAULT_KLDFLAGS :=
$(eval $(call DEFAULT_VAR,KLDFLAGS,$(DEFAULT_KLDFLAGS)))

override KCFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fPIE \
    -m64 \
    -march=x86-64 \
    -mno-80387 \
    -mno-mmx \
	-g \
    -mno-sse \
    -mno-sse2 \
	-fno-omit-frame-pointer \
    -mno-red-zone

override KCPPFLAGS := \
    -I src \
    $(KCPPFLAGS) \
    -MMD \
    -MP

override KLDFLAGS += \
    -Wl,-g \
    -Wl,-T,linker.ld \
	-nostdlib \
	-Wl,-nostdlib

override KNASMFLAGS += \
    -Wall \
    -f elf64

override CFILES := $(shell find -L * -type f -name '*.c' -not -path '32/*')
override ASFILES := $(shell find -L * -type f -name '*.S' -not -path '32/*')
override NASMFILES := $(shell find -L * -type f -name '*.asm' -not -path '32/*')
override OBJ := $(addprefix bin/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix bin/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Default target.
.PHONY: all
all: bin/$(KERNEL)

bin/$(KERNEL): linker.ld $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(OBJ) $(KLDFLAGS) -o $@
	printf '\003' | dd of=$@ bs=1 count=1 seek=16 conv=notrunc 2>/dev/null

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
bin/%.c.o: %.c
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
bin/%.S.o: %.S
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
bin/%.asm.o: %.asm
	mkdir -p "$$(dirname $@)"
	nasm $(KNASMFLAGS) $< -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf bin
