ASSEMBLER=./assembler
LINKER=./linker
EMULATOR=./emulator
${ASSEMBLER} -o obj/main.o tests/nivo-a/main.s
${ASSEMBLER} -o obj/math.o tests/nivo-a/math.s
${ASSEMBLER} -o obj/handler.o tests/nivo-a/handler.s
${ASSEMBLER} -o obj/isr_timer.o tests/nivo-a/isr_timer.s
${ASSEMBLER} -o obj/isr_terminal.o tests/nivo-a/isr_terminal.s
${ASSEMBLER} -o obj/isr_software.o tests/nivo-a/isr_software.s
${LINKER} -hex \
  -place=my_code@0x40000000 -place=math@0xF0000000 \
  -o program.hex \
  obj/handler.o obj/math.o obj/main.o obj/isr_terminal.o obj/isr_timer.o obj/isr_software.o
${EMULATOR} program.hex
