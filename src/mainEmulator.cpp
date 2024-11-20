#include "../inc/emulator.hpp"

int main(int argc, char *argv[]){

  string fileName = argv[1];
  
  Emulator* e = new Emulator(fileName);

  e->emulation();

  return 0;
}