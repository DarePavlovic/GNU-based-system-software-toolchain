#include "structures.hpp"
#include <unordered_map>
#include <iostream>
#include <string>
#include <iomanip>

#include <sstream>
#include <bitset>
#include <fstream>
class Assembler
{
private:
  string outputFileName;
public:
  unordered_map<string, SymbolTable> sym_table;
  unordered_map<string, Section> section_table;
  static int symbolNum;
  int location_counter;
  Section cur_section;
  Assembler();
  void setOutputFileName(string o);
  void addSymbolToSymTable(string symbol, reach r, typeS t, Section i, OccursSymbol occurs);
  void setGlobal(vector<string> symbols);
  void setExtern(vector<string> symbols);
  void addSection(string symbol);
  int literalToNumber(string literal);
  void setWord(vector<symLitNum *> sln);
  void setSkip(vector<symLitNum *> sln);
  void addLabel(string nameLabel);
  void setEnd();
  void print_symbol_table();
  void print_symbol_tableFile();
  // void print_relocation_table();
  void print_relocation_table2();
  void instructionHALT();
  void instructionINT();
  void instructionJumpsOrCall(string what, vector<symLitNum *> sln);
  void instructionBJMP(string what, string r1, string r2, vector<symLitNum *> sln);
  void setXCHG(string reg1, string reg2);
  void setAritmeticOperation(string reg1, string reg2, int op);
  void setLogicalOperations(string reg1, string reg2, int op);
  void setShiftOperation(string reg1, string reg2, int op);
  void setPoolLiteral(string lit);
  void setPoolSymbol(string symbol, string instructionName);
  void loadOperation(Operand *oper, string reg);
  void storeOperation(Operand *oper, string reg);
  void pushOperation(string reg);
  void popOperation(string reg);
  void csrrdOperation(string csr, string reg);
  void csrwrOperation(string csr, string reg);
  void setAscii(string asci);
  void setEQU(string symbol, int izraz);
  void writeSymTable(string outputFileName);
  void writeSectionTable(string outputFileName);
  void writeRelocationTable(string outputFileName);
  inline void putSymbolToSymTable(SymbolTable j)
  {
    sym_table[j.symbolName] = j;
  };
  inline void putSectionToSectionTable(Section j)
  {
    section_table[j.name] = j;
  };

  // nastavi sa instrukcijama i vrednosti operanada
  void instructionIRET(); 
  void instructionRET();  
};

