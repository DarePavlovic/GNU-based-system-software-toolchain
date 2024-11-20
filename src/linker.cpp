#include "../inc/linker.hpp"

int Linker::ID = 0;

void Linker::readAssemblyFiles(vector<string> objectFiles, int c)
{
  string inputNames[c];
  for (int i = 0; i < c; i++)
  {
    readAssemblyFile(objectFiles[i]);
  }
}

void Linker::readAssemblyFile(string objectFileName)
{

  FileEntry fEntry;
  fEntry.fileName = objectFileName;

  uint64_t SymbolTableSize;

  ifstream inputFile;

  inputFile.open(objectFileName, ios::binary);
  if (inputFile.fail())
  {
    cerr << "Error opening file: " << objectFileName << endl;
  }
  inputFile.read(reinterpret_cast<char *>(&SymbolTableSize), sizeof(SymbolTableSize));
  while (SymbolTableSize > 0)
  {
    SymbolTableSize--;
    SymbolTableGroup s;
    inputFile.read(reinterpret_cast<char *>(&s.symbolId), sizeof(s.symbolId));

    int symNameLen;
    inputFile.read(reinterpret_cast<char *>(&symNameLen), sizeof(symNameLen));

    vector<char> buffer(symNameLen + 1);
    inputFile.read(buffer.data(), symNameLen);
    buffer[symNameLen] = '\0';
    s.symbolName = string(buffer.data());
    inputFile.read(reinterpret_cast<char *>(&s.symbolValue), sizeof(s.symbolValue));
    inputFile.read(reinterpret_cast<char *>(&s.size), sizeof(s.size));
    inputFile.read(reinterpret_cast<char *>(&s.sectionId), sizeof(s.sectionId));

    inputFile.read(reinterpret_cast<char *>(&symNameLen), sizeof(symNameLen));
    if (symNameLen == 0)
    {
      s.sectionName = "";
    }
    else
    {
      vector<char> buffer1(symNameLen + 1);
      inputFile.read(buffer1.data(), symNameLen);
      buffer1[symNameLen] = '\0';
      s.sectionName = string(buffer1.data());
    }

    inputFile.read(reinterpret_cast<char *>(&s.GlobLoc), sizeof(s.GlobLoc));
    inputFile.read(reinterpret_cast<char *>(&s.type), sizeof(s.type));
    int def;
    inputFile.read(reinterpret_cast<char *>(&def), sizeof(def));
    s.defined = (def == 1);

    putSymbolToSymTableFile(fEntry, s);
  }

  uint64_t SectionTableSize;
  inputFile.read(reinterpret_cast<char *>(&SectionTableSize), sizeof(SectionTableSize));
  while (SectionTableSize > 0)
  {
    SectionTableSize--;
    SectionGroup sec;
    inputFile.read(reinterpret_cast<char *>(&sec.idSection), sizeof(sec.idSection));

    int symNameLen;
    inputFile.read(reinterpret_cast<char *>(&symNameLen), sizeof(symNameLen));
    vector<char> buffer(symNameLen + 1);
    inputFile.read(buffer.data(), symNameLen);
    buffer[symNameLen] = '\0';
    sec.name = string(buffer.data());
    //cout << "SectionName: " << sec.name;

    inputFile.read(reinterpret_cast<char *>(&sec.size), sizeof(sec.size));
    //cout << " | size: " << hex << setw(8) << setfill('0') << sec.size << endl;
    int n;
    inputFile.read(reinterpret_cast<char *>(&n), sizeof(n));
    char c;
    for (uint64_t i = 0; i < n; i++)
    {
      // cout<<"Pokusam "<<i<<endl;
      inputFile.read(reinterpret_cast<char *>(&c), sizeof(char));
      sec.mem_space.push_back(c);
      // cout<<"Procitao: "<<i<<" "<< hex << setw(2) << setfill('0')<<c<<endl;
    }

    uint64_t rtSectionSize;
    inputFile.read(reinterpret_cast<char *>(&rtSectionSize), sizeof(rtSectionSize));
    while (rtSectionSize > 0)
    {
      rtSectionSize--;
      RelocationTable rtEntry;

      int symNameLen1;
      inputFile.read(reinterpret_cast<char *>(&symNameLen1), sizeof(symNameLen1));
      vector<char> buffer(symNameLen1 + 1);
      inputFile.read(buffer.data(), symNameLen1);
      buffer[symNameLen1] = '\0';
      rtEntry.symbolName = string(buffer.data());
      inputFile.read(reinterpret_cast<char *>(&rtEntry.offset), sizeof(rtEntry.offset));
      inputFile.read(reinterpret_cast<char *>(&rtEntry.addend), sizeof(rtEntry.addend));
      sec.relocation_table.push_back(rtEntry);
    }
    putSectionToSectionTableFile(fEntry, sec);
  }

  putFileToFileTable(fEntry);

  inputFile.close();
}

void print_symbol_entryFile(const SymbolTableGroup &entry, std::ofstream &file)
{
  file << entry.symbolName << "\t\t" << entry.symbolId << "\t" << "0x" << std::hex << entry.symbolValue << "\t" << entry.sectionId << "\t" 
       << entry.sectionName << "\t" << entry.GlobLoc << "\t" << entry.type << "\t" << entry.defined << std::endl;
  file << "\n--------------------------------------------------------------------\n";
} 

void Linker::print_symbol_tableFile()
{
  std::ofstream file("global_symbol_table.txt");
  if (!file.is_open())
  {
    std::cerr << "Failed to open file for writing.\n";
    return;
  }

  file << "\n--------------------------------------------------------------------\n";
  file << "\nSymbolName\tSymbolId\tSymbolValue\tSectionId\tSectionName\tGlobLoc\tType\tDefined\t";
  file << "\n--------------------------------------------------------------------\n";

  for (const auto &entry : glob_sym_table)
  {
    print_symbol_entryFile(entry.second, file);
  }

  file.close();
}

void print_symbol_entry(SymbolTableGroup entry)
{

  cout << entry.symbolName << "\t\t" << entry.symbolId << "\t" << entry.symbolValue << "\t" << entry.sectionId << "\t" << entry.sectionName << "\t" << entry.GlobLoc << "\t" << entry.type << "\t" << entry.defined << endl;
  cout << "\n"
       << "--------------------------------------------------------------------\n";
}

void Linker::print_symbol_table()
{
  cout << "\n"
       << "--------------------------------------------------------------------\n";
  cout << "\n"
       << "SymbolName\t" << "SymbolId \t" << "SymbolValue\t" << "SectionId\t" << "SectionName\t" << "GlobLoc\t" << "Type\t" << "Defined\t";
  cout << "\n"
       << "--------------------------------------------------------------------\n";

  for (auto entry : glob_sym_table)
  {
    print_symbol_entry(entry.second);
  }
}

void print_reloc_entry2(vector<RelocationTable> e)
{
  for (RelocationTable entry : e)
  {
    cout << "\t" << entry.offset << "\t" << entry.symbolName << "\t" << entry.addend << endl;
  }
}

void print_reloc_entry2File(const std::vector<RelocationTable> &e, std::ofstream &file)
{
  for (const RelocationTable &entry : e)
  {
    file << "\t" <<"0x"<<std::hex<< entry.offset << "\t" << entry.symbolName << "\t" << entry.addend << std::endl;
  }
}


void Linker::print_relocation_table2()
{
  cout << "Section Realocation\n";
  cout << "--------------------\n";
  for (auto entry : section_table)
  {
    cout << "Section: " << entry.first << endl;
    cout << "----------------------------------------------\n";
    cout << " Offset  :  Symbol : Addend \n";
    cout << "----------------------------------------------\n";
    print_reloc_entry2(entry.second.relocation_table);
    cout << "----------------------------------------------\n";
  }
}

void Linker::print_relocation_table2File()
{
  std::ofstream file("glob_relocation_table.txt");
  if (!file.is_open())
  {
    std::cerr << "Failed to open file for writing.\n";
    return;
  }

  file << "Section Realocation\n";
  file << "--------------------\n";
  for (const auto &entry : section_table)
  {
    file << "Section: " << entry.first << std::endl;
    file << "----------------------------------------------\n";
    file << " Offset  :  Symbol : Addend \n";
    file << "----------------------------------------------\n";
    print_reloc_entry2File(entry.second.relocation_table, file);
    file << "----------------------------------------------\n";
  }

  file.close();
}

Linker::Linker()
{
  startAddr = 0x40000000;
  outputByteFile = vector<char>();
}

void Linker::create_intern_section()
{
  uint64_t pom = startAddr;
  uint64_t next_addr = startAddr;
  for (auto entry : section_table)
  {
    if (entry.second.startAddr == next_addr)
      next_addr += entry.second.sectionSize;
  }
  for (auto &entry : section_table)
  {
    if (entry.second.startAddr == 0)
    {
      entry.second.startAddr = next_addr;
      next_addr += entry.second.sectionSize;
    }
  }

  //cout << endl;
  for (auto k : section_table)
  {
    //cout << "Sekcija: " << k.first << " sada pocinje";
    //cout << " na adresi: 0x" << hex << setw(8) << setfill('0') << k.second.startAddr << " i zavrsava se na adresi: " << (k.second.sectionSize + k.second.startAddr) << endl;
  }
}

uint32_t literalToNumber(string literal)
{
  int base = 10;
  string whitoutPrefix = literal;
  if (literal.substr(0, 2) == "0x" || literal.substr(0, 2) == "0X")
  {
    base = 16;
    whitoutPrefix = literal.substr(2);
  }
  else if (literal.substr(0, 1) == "0")
  {
    base = 8;
  }
  uint32_t number = stoul(whitoutPrefix, nullptr, base);
  return number;
}

void Linker::setSectionStartAddress(vector<pair<string, string>> place)
{

  for (auto p : place)
  {
    string section_name = p.first;
    string adresa = p.second;
    uint32_t address = literalToNumber(adresa);
    //cout << "Adresa je: " << address << endl;
    for (auto &section : section_table)
    {
      if (section.first == section_name)
      {
        section.second.startAddr = address;
      }
    }
  }
}

void Linker::load_sections()
{
  for (auto &file : files_map)
  {

    for (auto &sec : file.second.section_table)
    {

      unordered_map<string, SectionMapEntry>::iterator got = section_table.find(sec.second.name);
      if (got != section_table.end())
      {
        for (auto c : sec.second.mem_space)
        {
          got->second.mem_space.push_back(c); // dodavanje bytekoda za sekcije nakon njih sto postaju
        }
        for (auto &sym : file.second.sym_table)
        {
          if (sym.second.sectionName == got->first)
          {
            sym.second.symbolValue += got->second.sectionSize;
          }
        }
        for (auto &rt : sec.second.relocation_table)
        {
          rt.offset += got->second.sectionSize;
          got->second.relocation_table.push_back(rt);
        }

        got->second.sectionSize += sec.second.size;
      }
      else
      {
        SectionMapEntry sme;
        SectionGroup section = sec.second;
        sme.name = section.name;
        sme.sectionSize = section.size;
        sme.startAddr = 0;
        for (auto k : section.mem_space)
        {
          sme.mem_space.push_back(k);
        }
        for (auto r : section.relocation_table)
        {
          sme.relocation_table.push_back(r);
        }
        putSectionToSectionTable(sme);
      }
    }
  }
}

void Linker::make_global_symbol_table()
{
  // ubacimo UND prvo
  SymbolTableGroup s;
  s.symbolId = ID++;
  s.symbolValue = 0;
  s.size = 0;
  s.type = typeS::NoType;
  s.GlobLoc = reach::Loc;
  s.sectionId = 0; // definisano za UND
  s.sectionName = "";
  s.symbolName = "";
  s.defined = true;
  glob_sym_table[""] = s;
  // zatim sve sekcije
  for (auto &section : section_table)
  {
    SymbolTableGroup stg;
    stg.symbolId = ID++;
    stg.symbolName = section.second.name;
    stg.size = 0;
    stg.type = typeS::Sect;
    stg.GlobLoc = reach::Loc;
    stg.defined = true;
    stg.sectionName = section.second.name;
    stg.sectionId = stg.symbolId;
    stg.symbolValue = section.second.startAddr;
    section.second.idSection = stg.symbolId;
    putSymbolToSGlobymTable(stg);
  }

  for (auto file : files_map)
  {
    for (auto st : file.second.sym_table)
    {

      if (st.first != "" && st.second.type != typeS::Sect && st.second.size != 2)
      {

        unordered_map<string, SymbolTableGroup>::iterator got = glob_sym_table.find(st.second.symbolName);
        if (got != glob_sym_table.end())
        {
          if (got->first == st.first)
          {
            if (got->second.GlobLoc == reach::Glob && st.second.GlobLoc == reach::Glob)
            {
              cout << "Error: Visestruka definisanost simbola:" << st.second.symbolName << endl;
              exit(-1);
            }
          }
        }
        
        SymbolTableGroup stg;
        stg.symbolId = ID++;
        stg.symbolName = st.second.symbolName;
        stg.size = st.second.size;
        stg.type = st.second.type;
        stg.GlobLoc = st.second.GlobLoc;
        stg.defined = st.second.defined;
        stg.sectionName = st.second.sectionName;
       
        for (auto q : section_table)
        {
          if (q.second.name == stg.sectionName)
          {
            stg.sectionId = q.second.idSection;
            stg.symbolValue = st.second.symbolValue;
            //cout << "Simbol " << stg.symbolName << " pre symValue: " << stg.symbolValue << endl;
            stg.symbolValue += q.second.startAddr;
            //cout << "Simbol " << stg.symbolName << " posle symValue: " << stg.symbolValue << endl;
          }
        }
        putSymbolToSGlobymTable(stg);
        
      }
    }
  }
}

void Linker::update_reloc_tables()
{
  for (auto &section : section_table)
  {
    for (auto &rtEntry : section.second.relocation_table)
    {
      rtEntry.offset += section.second.startAddr;
      for (auto gst : glob_sym_table)
      {
        if (gst.second.symbolName == rtEntry.symbolName)
        {
          rtEntry.symbolId = gst.second.symbolId;
        }
      }
    }
  }
}

void Linker::checkUndefinedSymbols()
{
  bool flagUndefined = false;
  for (auto gst : glob_sym_table)
  {
    if ((gst.second.defined == false))
    {
      cout << "Greskaa!!!:::Nedefinisan simbol: " << gst.second.symbolName << endl;
      flagUndefined = true;
    }
  }
  if (flagUndefined)
  {
    exit(-2);
  }
  else
  {
    //cout << "Bravo, nema nedefinisanih simbola!!!" << endl;
  }
}

void Linker::resolve_reloc_entries()
{
  for (auto &section : section_table)
  {
    for (auto rtEntry : section.second.relocation_table)
    {
      // uint32_t refaddr = section.second.startAddr + rtEntry.offset;
      uint32_t refaddr = rtEntry.offset - section.second.startAddr;
      uint32_t refvalue = 0;
      unordered_map<string, SymbolTableGroup>::iterator got = glob_sym_table.find(rtEntry.symbolName);
      if (got != glob_sym_table.end())
      {
        refvalue = (unsigned)(got->second.symbolValue + rtEntry.addend);
      }

      section.second.mem_space[refaddr++] = (char)(0xFF & refvalue);
      section.second.mem_space[refaddr++] = (char)(0xFF & (refvalue >> 8));
      section.second.mem_space[refaddr++] = (char)(0xFF & (refvalue >> 16));
      section.second.mem_space[refaddr++] = (char)(0xFF & (refvalue >> 24));

      // section.second.mem_space[refaddr++] = (char)(0xFF & (refvalue >> 24));
      // section.second.mem_space[refaddr++] = (char)(0xFF & (refvalue >> 16));
      // section.second.mem_space[refaddr++] = (char)(0xFF & (refvalue >> 8));
      // section.second.mem_space[refaddr++] = (char)(0xFF & refvalue);

      // u slucaju da postoji pc_32
      //  if(typeRel==pc_32){
      //    refvalue = refvalue+rtEntry.addend - refaddr;
      //  }
    }
  }
}

void Linker::make_output_file(string outputName)
{

  ofstream outputFile;

  outputFile.open(outputName, ios::trunc | ios::binary);
  uint32_t si = 0;
  for (auto &sec : section_table)
  {
    SectionMapEntry s = sec.second;
    uint32_t n = s.mem_space.size();

    outputFile.write((char *)&n, sizeof(n));

    uint32_t position = s.startAddr;
    //cout << "Position: " << position << endl;

    char c = ':';
    for (int i = 0; i < n; i++)
    {
      if (i % 4 == 0)
      {
        outputFile.write((char *)&(position), sizeof(position));
        // outputFile.write((char *)&(c), sizeof(c));
      }
      position++;
      outputFile.write((char *)&s.mem_space[i], sizeof(char));
    }

    si += n;
  }
  // cout << "Velicina hex fajla je: " << si << endl;
  uint32_t n = 0;
  outputFile.write((char *)&n, sizeof(n));
  outputFile.close();
}

void Linker::make_output_txt_file(string outputName){
  ofstream outputFile;
  string name = outputName+".txt";
  outputFile.open(name, ios::trunc | ios::binary);
  uint32_t si = 0;
  for (auto &sec : section_table)
  {
    SectionMapEntry s = sec.second;
    uint32_t n = s.mem_space.size();

    uint32_t position = s.startAddr;
    char c = ':';
    for (int i = 0; i < n; i++)
    {
      if (i % 4 == 0)
      {
        outputFile.write((char *)&(position), sizeof(position));
      }
      position++;
      outputFile.write((char *)&s.mem_space[i], sizeof(char));
    }

    si += n;
  }
  outputFile.close();
}