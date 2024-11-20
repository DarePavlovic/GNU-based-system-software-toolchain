#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <algorithm>

using namespace std;

enum reach
{
  Loc,
  Glob
};
enum typeS
{
  NoType,
  Sect,
  File
};

struct SymbolTableGroup
{
  int symbolId;
  string symbolName;
  uint32_t symbolValue;
  int size;
  int sectionId;
  string sectionName;
  reach GlobLoc;
  typeS type;
  bool defined;
};

struct RelocationTable
{
  int offset;
  string symbolName;
  int symbolId;
  int addend;
};

struct SectionGroup
{
  int idSection;
  string name;
  int size;
  vector<char> mem_space;
  vector<RelocationTable> relocation_table;
  uint32_t startAddr;
  uint32_t endAddr;
};

struct FileEntry
{
  string fileName;
  unordered_map<string, SymbolTableGroup> sym_table;
  unordered_map<string, SectionGroup> section_table;
};

struct SectionMapEntry{
  string name;
  uint32_t startAddr;
  uint32_t sectionSize;
  vector<char>mem_space;
  int idSection;
  vector<RelocationTable> relocation_table;

};

class Linker
{
private:
  void readAssemblyFile(string inputFileName);
  uint32_t startAddr;
  vector<char>outputByteFile;
public:
  Linker();
  unordered_map<string, FileEntry> files_map;
  unordered_map<string, SectionMapEntry> section_table;
  unordered_map<string, SymbolTableGroup> glob_sym_table;

  void readAssemblyFiles(vector<string>, int);
  void setSectionStartAddress(vector< pair< string,  string>> place);
  void print_symbol_table();
  void print_symbol_tableFile();
  void print_relocation_table2File();
  void print_relocation_table2();
  void create_intern_section();
  void load_sections();
  void make_global_symbol_table();
  void update_reloc_tables();
  void checkUndefinedSymbols();
  void resolve_reloc_entries();
  void make_output_file(string outputName);
  void make_output_txt_file(string outputName);
  static int ID;
  inline void putSymbolToSymTableFile(FileEntry& fE, SymbolTableGroup j)
  {
    fE.sym_table[j.symbolName] = j;
  };
  inline void putSectionToSectionTableFile(FileEntry& fE, SectionGroup j)
  {
    fE.section_table[j.name] = j;
  };
  inline void putSectionToSectionTable(SectionMapEntry j)
  {
    section_table[j.name] = j;
  };
  inline void putFileToFileTable(FileEntry j)
  {
    files_map[j.fileName] = j;
  };
  inline void putSymbolToSGlobymTable(SymbolTableGroup j)
  {
    glob_sym_table[j.symbolName] = j;
  };
};
