#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>

using namespace std;

class Emulator
{
private:
  
  //polja za citanje instrukcije
  uint8_t regA;
  uint8_t regB;
  uint8_t regC;
  int disp;
  //registri masine
  uint32_t reg[16];
  uint32_t &pc = reg[15];
  uint32_t &sp = reg[14];
  uint32_t status;
  uint32_t cause;
  uint32_t handler;

  uint32_t mm_reg_Size;
  uint32_t mm_reg_startAddr;
  uint32_t mem_startAddr;
  map<uint32_t, uint8_t> memory;

  void open_hex_file(string inputFile);

  bool is_running;

public:
  Emulator(string inputFile);
  void emulation();
  void print_output();
  void print_memory();
  uint32_t read_address(uint32_t address, bool jedan);
  void storeInMemory(uint32_t a, uint32_t location, bool jedan);

  void pushReg(uint8_t regNumber);
  void pushCsr(uint8_t csrNumber);
  void popReg(uint8_t regNumber);
  void popCsr(uint8_t regNumber);
};