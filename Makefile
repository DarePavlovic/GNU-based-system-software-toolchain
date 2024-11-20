all: assembler linker emulator

assembler: parser lexer src/main.cpp src/assembler.cpp
	g++ -g -o $@ src/main.cpp misc/bison.tab.cpp misc/flex.yy.cpp src/assembler.cpp

linker: src/mainLinker.cpp src/linker.cpp
	g++ -o $@ src/mainLinker.cpp src/linker.cpp

emulator: src/mainEmulator.cpp src/emulator.cpp
	g++ -o $@ src/mainEmulator.cpp src/emulator.cpp
	
parser: misc/bison.ypp
	bison -o misc/bison.tab.cpp -d $<

lexer: misc/flex.lpp
	flex -o misc/flex.yy.cpp $<

clean:
	rm -f misc/bison.tab.* misc/flex.yy.* assembler linker emulator glob_relocation_table.txt global_symbol_table.txt program.hex program.hex.txt

.PHONY: all clean parser lexer