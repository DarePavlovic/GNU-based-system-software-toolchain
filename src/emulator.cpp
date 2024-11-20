#include "../inc/emulator.hpp"

Emulator::Emulator(string inputFileName)
{
  memory = map<uint32_t, uint8_t>();

  open_hex_file(inputFileName);
  is_running = true;
  mm_reg_Size = 256;
  mm_reg_startAddr = 0xffffff00;
  mem_startAddr = 0x40000000;
  for (int i = 0; i < 16; i++)
  {
    reg[i] = 0;
  }
  
  pc = mem_startAddr;
  
  status = 0;
  cause = 0;
}

void Emulator::print_output()
{
  cout << "-----------------------------------------------------------------\n";
  cout << "Emulated processor executed halt instruction\n";
  cout << "Emulated processor state:\n";
  for (int i = 0; i < 16; ++i)
  {
    cout << "r" << i << "=0x" << setfill('0') << setw(8) << hex << reg[i] << dec;
    cout << "\t";
    if (i % 4 == 3)
    {
      cout << endl;
    }
  }
}

void Emulator::open_hex_file(string inputFileName)
{
  ifstream inputFile;

  inputFile.open(inputFileName, ios::binary);
  if (inputFile.fail())
  {
    cerr << "Error opening file: " << inputFileName << endl;
  }

  uint32_t n;
  uint32_t address;
  int id = 0;
  char ch;
  uint32_t position;
  // while (!inputFile.eof())
  // {
  inputFile.read(reinterpret_cast<char *>(&n), sizeof(n));
  while (n != 0)
  {
    for (int i = 0; i < n; i++)
    {
      if (i % 4 == 0)
      {
        inputFile.read(reinterpret_cast<char *>(&address), sizeof(address));
        // inputFile.read(reinterpret_cast<char *>(&ch), sizeof(ch));
        position = address;
        //cout << "\n0x" << hex << setfill('0') << setw(8) << position;
      }
      char bt;
      inputFile.read(reinterpret_cast<char *>(&bt), sizeof(bt));
      memory[position++] = static_cast<uint8_t>(bt);
      uint32_t k = static_cast<uint8_t>(bt);
      //cout << " " << hex << setfill('0') << setw(2) << k;
    }
    inputFile.read(reinterpret_cast<char *>(&n), sizeof(n));
  }

  cout << endl;
  inputFile.close();
}

uint32_t Emulator::read_address(uint32_t address, bool jedan)
{
  if (jedan == 1)
    return memory[address];
  char first = memory[address];
  char second = memory[address + 1];
  char third = memory[address + 2];
  char four = memory[address + 3];
  uint32_t value = (uint32_t)((first & 0x000000ff) | ((second << 8) & 0x0000ff00) | ((third << 16) & 0x00ff0000) | ((four << 24) & 0xff000000));
  //uint32_t value = (uint32_t)(((first<<24) & 0xff000000) | ((second << 16) & 0x00ff0000) | ((third << 8) & 0x0000ff00) | ((four) & 0x000000ff));
  return value;
}

void Emulator::emulation()
{
  uint32_t instruction;
  uint8_t oc;
  uint8_t mod;
  int id = 0;
  while (is_running)
  {

    //cout << "pc:" << hex << pc << endl;
    uint32_t instruction = read_address(pc, false);
    uint8_t oc = ((instruction >> 28) & 0xf);
    uint8_t mod = ((instruction >> 24) & 0xf);
    regA = ((instruction >> 20) & 0xf);
    regB = ((instruction >> 16) & 0xf);
    regC = ((instruction >> 12) & 0xf);
    disp = ((instruction) & 0xfff);

    if (disp & 0x800)
    {
      disp |= 0xfffff000;
    }

    pc += 4;
    //cout << "Instruction code: " << hex << instruction << endl;
    switch (oc)
    {
    case 0:
    { // halt
      if (mod != 0x00 | regA != 0x00 | regB != 0x00 | regC != 0x00 | disp != 0)
      {
        cout << "Bad instruction code " << hex << instruction << endl;
        exit(-1);
      }
      is_running = false;
      // print_output();
      break;
    }
    case 1:
    { // int
      pushCsr(0);
      pushReg(15);
      cause = 4;
      status = (status) & (~0x1);
      reg[15] = handler;
      break;
    }
    case 2:
    { // call
      if (mod == 0)
      {
        pushReg(15);
        reg[15] = reg[regA] + reg[regB] + disp;
      }
      else if (mod == 1)
      {
        pushReg(15);
        reg[15] = read_address((reg[regA] + reg[regB] + disp), false);
      }
      else
      {
        cout << "Bad instruction code " << hex << instruction << endl;
        exit(-1);
      }
      break;
    }
    case 3:
    { // jmp
      if (mod == 0)
      {
        reg[15] = reg[regA] + disp;
      }
      else if (mod == 1)
      {
        if (reg[regB] == reg[regC])
          reg[15] = reg[regA] + disp;
      }
      else if (mod == 2)
      {
        if (reg[regB] != reg[regC])
          reg[15] = reg[regA] + disp;
      }
      else if (mod == 3)
      {
        if (reg[regB] > reg[regC])
          reg[15] = reg[regA] + disp;
      }
      if (mod == 8)
      {
        uint32_t pom = reg[regA] + disp;
        reg[15] = read_address(pom, false);
      }
      else if (mod == 9)
      {
        if (reg[regB] == reg[regC])
        {
          uint32_t pom = reg[regA] + disp;
          reg[15] = read_address(pom, false);
        }
      }
      else if (mod == 10)
      {
        if (reg[regB] != reg[regC])
        {
          uint32_t pom = reg[regA] + disp;
          reg[15] = read_address(pom, false);
        }
      }
      else if (mod == 11)
      {
        if (reg[regB] > reg[regC])
        {
          uint32_t pom = reg[regA] + disp;
          reg[15] = read_address(pom, false);
        }
      }
      else
      {
        cout << "Bad instruction code " << hex << instruction << endl;
        exit(-1);
      }
      break;
    }
    case 4:
    { // xchg
      uint32_t temp = reg[regB];
      reg[regB] = reg[regC];
      reg[regC] = temp;
      break;
    }
    case 5:
    { // add/sub/mul/div
      if (mod == 0)
      {
        reg[regA] = (reg[regB] + reg[regC]);
      }
      else if (mod == 1)
      {
        reg[regA] = (reg[regC] - reg[regB]);
      }
      else if (mod == 2)
      {
        reg[regA] = (reg[regC] * reg[regB]);
      }
      else if (mod == 3)
      {
        reg[regA] = (reg[regC] / reg[regB]);
      }
      else
      {
        cout << "Bad instruction code " << hex << instruction << endl;
        exit(-1);
      }
      break;
    }
    case 6:
    { // not/and/or/xor
      if (mod == 0)
      {
        reg[regA] = ~reg[regB];
      }
      else if (mod == 1)
      {
        reg[regA] = (reg[regB] & reg[regC]);
      }
      else if (mod == 2)
      {
        reg[regA] = (reg[regB] | reg[regC]);
      }
      else if (mod == 3)
      {
        reg[regA] = (reg[regB] ^ reg[regC]);
      }
      else
      {
        cout << "Bad instruction code " << hex << instruction << endl;
        exit(-1);
      }
      break;
    }
    case 7:
    { // shl/shr
      if (mod == 0)
      {
        reg[regA] = (reg[regB] << reg[regC]);
      }
      else if (mod == 1)
      {
        reg[regA] = (reg[regB] >> reg[regC]);
      }
      else
      {
        cout << "Bad instruction code " << hex << instruction << endl;
        exit(-1);
      }
      break;
    }
    case 8:
    { // st
      if (mod == 0)
      { // memorijsko direktno
        uint32_t k = reg[regA] + reg[regB] + disp;
        uint32_t value = reg[regC];
        storeInMemory(value, k, false);
      }
      else if (mod == 1)
      { // push reg
        pushReg(regC);
      }
      else if (mod == 2)
      {                                            // mem ind
        uint32_t k = reg[regA] + reg[regB] + disp; //
        uint32_t pom = read_address(k, false);
        uint32_t value = reg[regC];
        storeInMemory(value, pom, false);
      }
      else
      {
        cout << "Bad instruction code k" << hex << instruction << endl;
        exit(-1);
      }
      break;
    }
    case 9:
    {               // ld
      if (mod == 0) // csrrd
      {
        uint8_t csrNumber = regB;
        if (csrNumber == 0)
        {
          reg[regA] = status;
        }
        else if (csrNumber == 1)
        {
          reg[regA] = handler;
        }
        else if (csrNumber == 2)
        {
          reg[regA] = cause;
        }
        else
        {
          cout << "Bad instruction code " << hex << instruction << endl;
          exit(-1);
        }
      }
      else if (mod == 1)
      { // neposredna vrednost u reg ili reg direktno
        reg[regA] = reg[regB] + disp;
        // cout << "Ucitavam neposredno u registar vrednost : " << disp << endl;
      }
      else if (mod == 2)
      {
        
          uint32_t k = reg[regB] + reg[regC] + disp;
          reg[regA] = read_address(k, false);
        
      }
      else if (mod == 3)
      { // popReg
        if (disp != 4)
        {
          reg[regA] = read_address(reg[regB], false);
          reg[regB] += disp;
        }
        else
        {
          popReg(regA);
        }
      }
      else if (mod == 4)
      { // csrwr
        uint8_t csrNumber = regA;
        if (csrNumber == 0)
        {
          status = reg[regB];
        }
        else if (csrNumber == 1)
        {
          handler = reg[regB];
          // cout << "Registar HANDLER: sada ima vrednost: 0x" << hex << handler << endl;
        }
        else if (csrNumber == 2)
        {
          cause = reg[regB];
        }
        else
        {
          cout << "Bad instruction code " << hex << instruction << endl;
          exit(-1);
        }
      }
      else if (mod == 5)
      { // neposredna vrednost u csr
        uint8_t csrNumber = regA;
        uint32_t pom = disp;
        uint8_t csrN2 = regB;
        if (regB == 0)
          pom += status;
        else if (regB == 1)
          pom += handler;
        else if (regB == 2)
          pom += handler;
        else
        {
          cout << "Bad instruction code " << hex << instruction << endl;
          exit(-1);
        }

        if (csrNumber == 0)
        {
          status = pom;
        }
        else if (csrNumber == 1)
        {
          handler = pom;
        }
        else if (csrNumber == 2)
        {
          cause = pom;
        }
        else
        {
          cout << "Bad instruction code " << hex << instruction << endl;
          exit(-1);
        }
      }
      else if (mod == 6)
      { // mem indirektno ili citanje iz pool-a i stavljanje u csr registar
        uint32_t k = reg[regB] + reg[regC] + disp;
        uint32_t pom = read_address(k, false);
        uint8_t csrNumber = regA;
        if (csrNumber == 0)
        {
          status = pom;
        }
        else if (csrNumber == 1)
        {
          handler = pom;
        }
        else if (csrNumber == 2)
        {
          cause = pom;
        }
        else
        {
          cout << "Bad instruction code " << hex << instruction << endl;
          exit(-1);
        }
      }
      else if (mod == 7)
      { // popCsr
        popCsr(regA);
      }
      break;
    }
    }

    // uint32_t r1 = 1;
    // cout << "Registar: " << r1 << " sada ima vrednost: 0x" << hex << reg[1] << endl;
    // uint32_t r2 = 2;
    // cout << "Registar: " << r2 << " sada ima vrednost: 0x" << hex << reg[2] << endl;
    // uint32_t r14 = read_address(reg[14], false);
    // cout << "Na steku poslednja vrednost: 0x" << hex << r14 << endl;
  }

  print_output();
  //  print_memory();
}

void Emulator::storeInMemory(uint32_t a, uint32_t location, bool jedan)
{
  if (jedan)
  {
    memory[location] = a;
  }
  uint8_t first = a & 0xff;
  uint8_t second = ((a >> 8) & 0xff);
  uint8_t third = ((a >> 16) & 0xff);
  uint8_t four = ((a >> 24) & 0xff);
  // uint8_t four = a & 0xff;
  // uint8_t third = ((a >> 8) & 0xff);
  // uint8_t second = ((a >> 16) & 0xff);
  // uint8_t first = ((a >> 24) & 0xff);
  memory[location] = first;
  memory[location + 1] = second;
  memory[location + 2] = third;
  memory[location + 3] = four;
}

void Emulator::pushReg(uint8_t regNumber)
{
  uint32_t stack = reg[14] - 4;
  reg[14] -= 4;
  uint32_t value = reg[regNumber];
  storeInMemory(value, stack, false);
}
void Emulator::pushCsr(uint8_t csrNumber)
{
  reg[14] -= 4;
  if (csrNumber == 0)
  {
    storeInMemory(status, reg[14], false);
  }
  else if (csrNumber == 1)
  {
    storeInMemory(handler, reg[14], false);
  }
  else if (csrNumber == 2)
  {
    storeInMemory(cause, reg[14], false);
  }
  else
  {
    cout << "Bad instruction code push csr" << endl;
    exit(-1);
  }
}

void Emulator::popReg(uint8_t regNumber)
{
  uint32_t stack = reg[14];
  // reg[14] = stack;
  reg[regNumber] = read_address(stack, false);
  reg[14] += 4;
}

void Emulator::popCsr(uint8_t csrNumber)
{
  uint32_t stack = reg[14];
  if (csrNumber == 0)
  {
    status = read_address(stack, false);
  }
  else if (csrNumber == 1)
  {
    handler = read_address(stack, false);
  }
  else if (csrNumber == 2)
  {
    cause = read_address(stack, false);
  }
  else
  {
    cout << "Bad instruction code " << endl;
    exit(-1);
  }
  reg[14] += 4;
}

void Emulator::print_memory()
{
  int id = 0;
  for (auto m : memory)
  {
    if (id % 4 == 0)
    {
      cout << "\n0x" << hex << setfill('0') << setw(8) << m.first;
    }
    uint32_t k = m.second;
    cout << " " << hex << setfill('0') << setw(2) << k;
    id++;
  }
}