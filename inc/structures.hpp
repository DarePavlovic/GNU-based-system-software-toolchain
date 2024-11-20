#include <iostream>
using namespace std;
#include <string>
#include<vector>


struct OccursSymbol{//obracanje identifikatorima pre njihove definicije
  uint32_t positionValue;//adresa u programu za izmenu
  string instructionName;
};

struct symLitNum{
  int num;
  string sym;
  string lit;
};


enum reach { Loc, Glob };
enum typeS {NoType, Sect, File};

struct SymbolTable
{
  int symbolId;
  uint32_t symbolValue;
  string symbolName;
  reach locGlob;
  typeS type;
  int size;
  int sectionId;
  string sectionName;
  vector<OccursSymbol> flink;
  bool defined;
};

struct PoolEntry {
  long long address;
  vector<uint32_t> positionsValue;
  string symbol;
  bool notSymbol;//literal je
};

struct RelocationTable{
  //string sectionName;//ovo izbacujem
  //int sectionId;//ovo ubacujem
  uint32_t offset;
  string symbolName;
  int symbolId;
  int addend;
};

struct Section{
  int idSection;
  uint32_t location_counter;
  string name;
  int size;
  vector<char> mem_space;
  vector<PoolEntry> pool;
  vector<RelocationTable> relocation_table;
};


struct Operand{
  string symbol;//ako je symbol imace vrednost simbola
  int value;    //ako je literal ili reg imace vrednost njegovu
  int addrType;//0 direktno; 1 neposredno; 2 indirektno
  int argType;//0 symbol; 1 literal; 2 registar; 3 reg ind sa pomerajem literal; 4 reg ind sa pomerajem simbol
  string regNumb; //za slucaj kada je indirektno sa pomerajem 
};




