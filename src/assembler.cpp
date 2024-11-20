#include "../inc/assembler.hpp"
#include <cstdio>
static int sectionId = 0;
bool trebaRelokacijaWord = false;

void print_symbol_entry(SymbolTable entry)
{
  const char *n = entry.symbolName.c_str();
  printf("| %-10s | %-12d | %-7x | %-8s | %-6d | %-6d |\n", n, entry.sectionId, entry.symbolValue,
         (entry.locGlob ? "Glob" : "Loc"), entry.symbolId, entry.size);
}
void print_symbol_entryFile(SymbolTable entry, FILE *file)
{
  const char *n = entry.symbolName.c_str();
  fprintf(file, "| %-10s | %-12d | %-7x | %-8s | %-6d | %-6d |\n", n, entry.sectionId, entry.symbolValue,
          (entry.locGlob ? "Glob" : "Loc"), entry.symbolId, entry.size);
}

void print_reloc_entry2(vector<RelocationTable> e)
{
  for (RelocationTable entry : e)
  {
    const char *nS = entry.symbolName.c_str();
    printf("   %x   :    %s    :   %d  \n", entry.offset, nS, entry.addend);
  }
}

void print_reloc_entryFile(const std::vector<RelocationTable> &e, FILE *file){
  for (const RelocationTable &entry : e)
  {
    const char *nS = entry.symbolName.c_str();
    fprintf(file, "   %x   :    %s    :   %d  \n", entry.offset, nS, entry.addend);
  }
}

void Assembler::print_symbol_table()
{
  printf("\n--------------------------------------------------------------------\n");
  printf("| %-10s | %-12s | %-7s | %-8s | %-6s | %-4s |\n", "SymbolName", "SectionNumber", "Value", "IsGlobal", "Number", "Size");
  printf("\n--------------------------------------------------------------------\n");

  for (auto entry : sym_table)
  {
    print_symbol_entry(entry.second);
  }
  printf("\n--------------------------------------------------------------------\n");
}

void Assembler::print_symbol_tableFile()
{
  string outputName = outputFileName +".txt";
  FILE *file = fopen(outputName.c_str(), "w");
  if (!file)
  {
    fprintf(stderr, "Failed to open file for writing.\n");
    return;
  }

  fprintf(file, "\n--------------------------------------------------------------------\n");
  fprintf(file, "| %-10s | %-12s | %-7s | %-8s | %-6s | %-4s |\n", "SymbolName", "SectionNumber", "Value", "IsGlobal", "Number", "Size");
  fprintf(file, "\n--------------------------------------------------------------------\n");

  for (auto entry : sym_table)
  {
    print_symbol_entryFile(entry.second, file);
  }
  fprintf(file, "\n--------------------------------------------------------------------\n");

  fprintf(file, "Section Realocation\n");
  fprintf(file, "--------------------\n");
  for (const auto &entry : section_table)
  {
    const char *secN = entry.first.c_str();
    fprintf(file, "Section: .%s with size: %x \n", secN, entry.second.size);
    fprintf(file, "----------------------------------------------\n");
    fprintf(file, " Offset  :  Symbol : Addend \n");
    fprintf(file, "----------------------------------------------\n");
    print_reloc_entryFile(entry.second.relocation_table, file);
    fprintf(file, "----------------------------------------------\n");
  }

  fclose(file);
}

void Assembler::print_relocation_table2()
{
  printf("Section Realocation\n");
  printf("--------------------\n");
  for (auto entry : section_table)
  {
    const char *secN = entry.first.c_str();
    printf("Section: .%s with size: %x \n", secN, entry.second.size);
    printf("----------------------------------------------\n");
    printf(" Offset  :  Symbol : Addend \n");
    printf("----------------------------------------------\n");
    print_reloc_entry2(entry.second.relocation_table);
    printf("----------------------------------------------\n");
  }
}

int Assembler::literalToNumber(string literal)
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

int Assembler::symbolNum = 0;

Assembler::Assembler()
{
  cur_section.idSection = sectionId++;
  cur_section.location_counter = 0;
  cur_section.name = "";
  cur_section.size = 0;
  cur_section.pool = vector<PoolEntry>();
  cur_section.relocation_table = vector<RelocationTable>();

  SymbolTable s;
  s.symbolId = symbolNum++;
  s.symbolValue = 0;
  s.size = 0;
  s.type = typeS::NoType;
  s.locGlob = reach::Loc;
  s.sectionId = 0; // definisano za UND
  s.sectionName = "";
  s.symbolName = "";
  s.defined = true;
  sym_table[""] = s;
}

void Assembler::addSymbolToSymTable(string symbol, reach r, typeS t, Section i, OccursSymbol occurs)
{
  SymbolTable p;
  p.locGlob = r;
  if (i.idSection != 0)
  {
    p.sectionId = i.idSection;
    p.symbolValue = i.location_counter; // ovde treba adresa da se upise
    p.sectionName = i.name;
  }
  else
  {
    p.sectionId = 0;   // UND
    p.symbolValue = 0; // ovde treba adresa da se upise
    p.sectionName = "";
  }
  p.defined = false;
  p.symbolId = symbolNum++;
  p.symbolName = symbol;
  p.type = t;
  p.size = 0;
  p.flink = vector<OccursSymbol>();
  if (occurs.instructionName != "")
  {
    p.flink.push_back(occurs);
  }
  putSymbolToSymTable(p);
}

void Assembler::setGlobal(vector<string> symbols)
{
  for (string i : symbols)
  {
    // da li se nalazi u tabeli simbola
    unordered_map<string, SymbolTable>::iterator got = sym_table.find(i);
    if (got != sym_table.end())
    {
      if (got->second.locGlob == reach::Glob && got->second.size == 2)
      {
        printf("Greska: nije moguce definisati globalni i externi simbol\n");
        exit(-1);
      }
      else
      {
        printf("Vec postoji takav globalni simbol\n");
        exit(-1);
      }
    }
    // ako nije definisan ranije da ga ubacis u tabelu simbola
    else
    {
      SymbolTable p;
      p.symbolName = i;
      p.symbolValue = 0;
      p.symbolId = symbolNum++;
      p.defined = false;
      p.locGlob = reach::Glob;
      p.size = 0;
      p.type = typeS::NoType;
      p.flink = vector<OccursSymbol>();
      if (cur_section.idSection != 0)
      {
        p.sectionId = cur_section.idSection;
        p.sectionName = cur_section.name;
      }
      else
      {
        p.sectionId = 0; // UND
        p.sectionName = "";
      }
      putSymbolToSymTable(p);
    }
  }
}

void Assembler::setExtern(vector<string> symbols)
{
  for (string i : symbols)
  {
    // da li se nalazi u tabeli simbola
    unordered_map<string, SymbolTable>::iterator got = sym_table.find(i);
    if (got != sym_table.end())
    {
      if (got->second.locGlob == reach::Glob && got->second.size != 2)
      {
        const char *st = got->second.symbolName.c_str();
        printf("Greska: nije moguce definisati externi i globalni simbol: %s \n", st);
        exit(-1);
      }
      else
      {
        printf("Vec postoji takav externi simbol\n");
        exit(-1);
      }
    }
    else
    {
      SymbolTable p;
      p.symbolName = i;
      p.symbolValue = 0;
      p.symbolId = symbolNum++;
      p.defined = false;
      p.locGlob = reach::Glob;
      p.size = 2;
      p.sectionName = "";
      p.type = typeS::NoType;
      p.sectionId = 0;
      putSymbolToSymTable(p);
    }
  }
}

void Assembler::addSection(string symbol)
{
  if (cur_section.idSection != 0)
  {
    cur_section.size = cur_section.location_counter;
    putSectionToSectionTable(cur_section);
  }
  unordered_map<string, Section>::iterator got = section_table.find(symbol);
  if (got != section_table.end())
  {
    cur_section.idSection = got->second.idSection;
    cur_section.location_counter = got->second.size;
    cur_section.mem_space = got->second.mem_space;
    cur_section.name = got->second.name;
    cur_section.size = got->second.size;
    cur_section.pool = got->second.pool;
    cur_section.relocation_table = got->second.relocation_table;
  }
  else
  {
    Section s;
    s.idSection = symbolNum++;
    s.size = 0;
    s.location_counter = 0;
    s.mem_space = std::vector<char>();
    s.name = symbol;
    s.pool = vector<PoolEntry>();
    s.relocation_table = vector<RelocationTable>();

    SymbolTable p;
    p.symbolId = s.idSection;
    p.symbolValue = s.location_counter;
    p.locGlob = reach::Loc;
    p.type = typeS::Sect;
    p.sectionId = s.idSection;
    p.sectionName = s.name;
    p.symbolName = symbol;
    p.size = 0;
    p.defined = true;
    cur_section = s;
    putSymbolToSymTable(p);
  }
}
void Assembler::setWord(vector<symLitNum *> sln)
{
  for (symLitNum *k : sln)
  {

    if (k->lit != "")
    {
      uint32_t number = literalToNumber(k->lit);
      cur_section.mem_space.push_back((char)0xFF & number);
      cur_section.mem_space.push_back((char)(0xFF & (number >> 8)));
      cur_section.mem_space.push_back((char)(0xFF & (number >> 16)));
      cur_section.mem_space.push_back((char)(0xFF & (number >> 24)));
      cur_section.location_counter += 4;
      // for (char byte : cur_section.mem_space)
      // {
      //   printf("%02X ", (unsigned char)byte);
      // }
    }
    else if (k->num != -1)
    {
      uint32_t number = k->num;

      cur_section.mem_space.push_back((char)0xFF & number);
      cur_section.mem_space.push_back((char)(0xFF & (number >> 8)));
      cur_section.mem_space.push_back((char)(0xFF & (number >> 16)));
      cur_section.mem_space.push_back((char)(0xFF & (number >> 24)));
      cur_section.location_counter += 4;

      // for (char byte : cur_section.mem_space)
      // {
      //   printf("%02X ", (unsigned char)byte);
      // }
    }
    else if (k->sym != "")
    {
      if (trebaRelokacijaWord == false)
      {
        unordered_map<string, SymbolTable>::iterator got = sym_table.find(k->sym);
        if (got != sym_table.end())
        {

          if (got->second.defined)
          { // simbol definisan

            uint32_t number = got->second.symbolValue;
            cur_section.mem_space.push_back((char)0xFF & number);
            cur_section.mem_space.push_back((char)(0xFF & (number >> 8)));
            cur_section.mem_space.push_back((char)(0xFF & (number >> 16)));
            cur_section.mem_space.push_back((char)(0xFF & (number >> 24)));
            cur_section.location_counter += 4;

            // for (char byte : cur_section.mem_space)
            // {
            //   printf("%02X ", (unsigned char)byte);
            // }
          }
          else
          { // nije definisan

            OccursSymbol o;
            o.positionValue = cur_section.location_counter;
            o.instructionName = "word";
            got->second.flink.push_back(o);    // cisto da bih posle izmenio addend u relokacionoj tabeli
            cur_section.location_counter += 4; // da bih ostavio mesta kasnije da se doda, a da se ne poremeti locationCounter
          }
        }
        else
        {
          OccursSymbol occurs;
          occurs.instructionName = "word";
          occurs.positionValue = cur_section.location_counter;
          addSymbolToSymTable(k->sym, Loc, NoType, cur_section, occurs);
          cur_section.location_counter += 4; // da bih ostavio mesta kasnije da se doda, a da se ne poremeti locationCounter
        }
      }
      else
      {

        unordered_map<string, SymbolTable>::iterator got = sym_table.find(k->sym);
        if (got != sym_table.end())
        {

          if (got->second.defined)
          { // simbol definisan

            uint32_t number = got->second.symbolValue;

            // Ako mora relokacioni zapis
            RelocationTable rt;
            rt.offset = cur_section.location_counter;
            if (got->second.locGlob == reach::Loc)
            {
              rt.symbolName = cur_section.name;
              rt.addend = number;
            }
            else
            {
              rt.symbolName = k->sym;
              rt.addend = 0;
            }
            cur_section.relocation_table.push_back(rt);
          }
          else
          { // nije definisan

            OccursSymbol o;
            o.positionValue = cur_section.location_counter;
            o.instructionName = "word";
            got->second.flink.push_back(o); // cisto da bih posle izmenio addend u relokacionoj tabeli

            RelocationTable rt;
            rt.offset = cur_section.location_counter;
            if (got->second.locGlob == reach::Loc)
            { // lokalni nedefinisan
              rt.symbolName = cur_section.name;
              rt.addend = 0;
              OccursSymbol o;
              o.positionValue = cur_section.location_counter;
              o.instructionName = "word";
              got->second.flink.push_back(o); // cisto da bih posle izmenio addend u relokacionoj tabeli
            }
            else
            { // globalni nedefinisan
              rt.symbolName = k->sym;
              rt.addend = 0;
            }
            cur_section.relocation_table.push_back(rt);
          }
          cur_section.mem_space.push_back((char)(0x00));
          cur_section.mem_space.push_back((char)(0x00));
          cur_section.mem_space.push_back((char)(0x00));
          cur_section.mem_space.push_back((char)(0x00));
          cur_section.location_counter += 4;
        }
        else
        { // nije pronadjen u tabeli simbola
          cur_section.mem_space.push_back((char)(0x00));
          cur_section.mem_space.push_back((char)(0x00));
          cur_section.mem_space.push_back((char)(0x00));
          cur_section.mem_space.push_back((char)(0x00));
          OccursSymbol occurs;
          occurs.instructionName = "word";
          occurs.positionValue = cur_section.location_counter;
          addSymbolToSymTable(k->sym, Loc, NoType, cur_section, occurs);
          RelocationTable rt;
          rt.symbolName = cur_section.name;
          rt.offset = cur_section.location_counter;
          rt.addend = 0;

          cur_section.relocation_table.push_back(rt);
          cur_section.location_counter += 4;
        }
      }
    }
  }
}

void Assembler::setSkip(vector<symLitNum *> sln)
{
  if (cur_section.name == "")
  {
    printf("Error: Skip must be in section\n");
    exit(-4);
  }
  for (auto k : sln)
  {
    uint32_t number;
    if (k->lit != "")
    {
      number = literalToNumber(k->lit);
      // int l = k.lit;
    }
    else if (k->num != -1)
    {
      number = k->num;
    }
    for (int i = 0; i < number; i++)
      cur_section.mem_space.push_back((char)(0x00));

    cur_section.location_counter += number;
  }
}

void Assembler::setAscii(string asci)
{ 
  for (int i = 0; i < asci.size(); i++)
  {
    int ascint = (int)asci[i];
    cur_section.mem_space.push_back((char)0xff & ascint);
  }
  cur_section.location_counter += asci.size();
}

void Assembler::setEQU(string symbol, int izraz)
{ 
  unordered_map<string, SymbolTable>::iterator got = sym_table.find(symbol);
  if (got != sym_table.end())
  {
    cout << "Error:simbol already in symtable" << endl;
  }
  else
  {
    // definisemo novi simbol
    SymbolTable p;
    p.locGlob = reach::Glob;
    p.size = 0;
    p.defined = true;
    p.symbolId = symbolNum++;
    p.symbolName = symbol;
    p.type = typeS::NoType;
    p.symbolValue = izraz;
    putSymbolToSymTable(p);
  }
}

void Assembler::addLabel(string nameLabel)
{
  // prodjem celu tabelu simbola i ako nije u tabeli ubacim je kao nov simbol
  unordered_map<string, SymbolTable>::iterator got = sym_table.find(nameLabel);
  if (got != sym_table.end())
  {
    if (got->second.defined == true)
    {
      printf("Greska: Simbol vec definisan, cao!\n");
      exit(-1);
    }
    else
    {
      if (got->second.locGlob == reach::Glob)
      {
        if (got->second.size == 2)
        {
          printf("Greska: Ne mozes definisati externi simbol, cao!\n");
          exit(-1);
        }
      }
      // postavim prave vrednosti u tabelu simbola
      got->second.sectionId = cur_section.idSection;
      got->second.sectionName = cur_section.name;
      got->second.symbolValue = cur_section.location_counter;
      got->second.defined = true;
      // definisan je prvi put, push u bazen literala za tu sekciju
      PoolEntry pe;
      pe.symbol = nameLabel;
      pe.positionsValue = vector<uint32_t>();
      pe.address = 0;
      pe.notSymbol = false;
      if (!got->second.flink.empty())
      {
        int idF = 0;
        for (OccursSymbol o : got->second.flink)
        {
          if (o.instructionName == "word")
          {
            // izmeni memoriju
            uint32_t number = cur_section.location_counter;
            if (trebaRelokacijaWord == false)
            {
              cur_section.mem_space.insert(cur_section.mem_space.begin() + o.positionValue++, (char)0xFF & number);
              cur_section.mem_space.insert(cur_section.mem_space.begin() + o.positionValue++, (char)0xFF & (number >> 8));
              cur_section.mem_space.insert(cur_section.mem_space.begin() + o.positionValue++, (char)0xFF & (number >> 16));
              cur_section.mem_space.insert(cur_section.mem_space.begin() + o.positionValue++, (char)0xFF & (number >> 24));
            }
            else
            { // slucaj da treba relokacija
              for (auto &c : cur_section.relocation_table)
              {
                if (c.offset == o.positionValue)
                {
                  c.addend = cur_section.location_counter;
                }
              }
            }

            got->second.flink.erase(got->second.flink.begin() + idF);
          }
          else
          {
            pe.positionsValue.push_back(o.positionValue); // postavim ranije definisane pozicije
          }
          idF++;
        }
      }
      cur_section.pool.push_back(pe);
    }
  }
  // ako nije definisan ranije da ga ubacis u tabelu simbola
  else
  {
    SymbolTable newLabel;
    newLabel.symbolName = nameLabel;
    newLabel.defined = true;
    newLabel.locGlob = reach::Loc;
    newLabel.sectionId = cur_section.idSection;
    newLabel.sectionName = cur_section.name;
    newLabel.symbolId = symbolNum++;
    newLabel.symbolValue = cur_section.location_counter; // mozda mi nece ni trebati vec samo da koristim rastojanje od pool-a
    newLabel.type = typeS::NoType;
    newLabel.size = 0;
    putSymbolToSymTable(newLabel);

    PoolEntry pe;
    pe.symbol = nameLabel;
    pe.positionsValue = vector<uint32_t>();
    pe.address = 0;
    pe.notSymbol = false;
    cur_section.pool.push_back(pe);
  }
}

void Assembler::setOutputFileName(string o)
{
  this->outputFileName = o;
}

void Assembler::setEnd()
{
  cur_section.size = cur_section.location_counter;
  putSectionToSectionTable(cur_section);
  int address;

  for (auto &sec : section_table)
  {
    address = sec.second.size;
    for (PoolEntry pe : sec.second.pool)
    {
      pe.address = address;
      // za adrese iz positionValue PoolEntry-ja izracunaj pomeraj i upisi ga u memoriju
      // nacin kako upisati u memoriju jeste da mem_space.push na lokaciju positionValue-a, tako "izmenimo memoriju", a ustvari samo je
      // dopunimo na odgovarajuce mesto
      for (int position : pe.positionsValue)
      { // ispravi upise u memoriju gde god se trazi vrednost iz pool-a
        int razlika = pe.address - position;
        razlika -= 4; // zbog pozicije pc-ja
        //printf("Razlika je za simbol %s : %x\n", pe.symbol.c_str(), razlika);
        sec.second.mem_space[position] |= (char)(0xff & razlika);
        sec.second.mem_space[position +1] |= (char)(0xF00 & razlika);
      }

      if (pe.notSymbol)
      {
        uint32_t number = literalToNumber(pe.symbol);
        sec.second.mem_space.push_back((char)0xFF & number);
        sec.second.mem_space.push_back((char)(0xFF & (number >> 8)));
        sec.second.mem_space.push_back((char)(0xFF & (number >> 16)));
        sec.second.mem_space.push_back((char)(0xFF & (number >> 24)));
      }
      else
      {
        // postavi relokacioni zapis za adresu da se azurira
        RelocationTable rte;
        rte.symbolName = pe.symbol;
        rte.offset = address;
        rte.addend = 0;
        sec.second.relocation_table.push_back(rte);

        sec.second.mem_space.push_back((char)(0xFF & 0));
        sec.second.mem_space.push_back((char)(0xFF & 0));
        sec.second.mem_space.push_back((char)(0xFF & 0));
        sec.second.mem_space.push_back((char)(0xFF & 0));
      }
      address += 4;
    }
    sec.second.size = address;
  }

  for (auto &ste : sym_table)
  {
    if (ste.second.locGlob == reach::Glob && ste.second.size != 0 && ste.second.defined == false)
    {
      ste.second.size = 2; // postaju extern globalni koji nisu definisani
      ste.second.sectionId = 0;
      ste.second.sectionName = "";
    }
  }
  // ONDA PRIPREMA IZLAZNIH PODATAKA

  // printf("\n");
  // for (auto sec : section_table)
  // {
  //   printf("%s:\n", sec.first.c_str());
  //   for (auto k : sec.second.mem_space)
  //   {
  //     printf("%hhx", k);
  //   }
  //   printf("\n");
  // }
  print_symbol_tableFile();
  writeSymTable(this->outputFileName);
  writeSectionTable(this->outputFileName);
  
}

void pushToSection(Section& s, uint32_t code){
  // s.mem_space.push_back((char)(0xFF & (code >> 24)));
  // s.mem_space.push_back((char)(0xFF & (code >> 16)));
  // s.mem_space.push_back((char)(0xFF & (code >> 8)));
  s.mem_space.push_back((char)0xFF & code);
  s.mem_space.push_back((char)(0xFF & (code >> 8)));
  s.mem_space.push_back((char)(0xFF & (code >> 16)));
  s.mem_space.push_back((char)(0xFF & (code >> 24)));
  s.location_counter+=4;
}

void Assembler::instructionHALT()
{
  pushToSection(cur_section,0);
}

void Assembler::instructionINT()
{
  cur_section.mem_space.push_back((char)(0x00));
  cur_section.mem_space.push_back((char)(0x00));
  cur_section.mem_space.push_back((char)(0x00));
  cur_section.mem_space.push_back((char)(0x10));
  cur_section.location_counter += 4;
}

void Assembler::instructionIRET()
{
  Operand *o = new Operand;
  o->regNumb = "r14";
  o->argType = 3;
  o->addrType = 2;
  o->value = 4;
  loadOperation(o, "status"); // pokupi vrednost statusa
  o->value = 8;
  o->regNumb = "r14";
  o->argType = 4;
  o->addrType = 2;

  loadOperation(o, "r15"); // ubaci vrednost sa steka u pc
  // i stek uvecaj za 8
}

void Assembler::instructionRET()
{
  Operand *o = new Operand;
  o->value = 4;
  o->regNumb = "r14";
  o->argType = 3;
  o->addrType = 2;
  // loadOperation(o, "r15");
  popOperation("r15");
}
void Assembler::instructionJumpsOrCall(string what, vector<symLitNum *> sln)
{

  uint32_t code;
  uint32_t pomeraj;

  for (auto k : sln)
  {
    if (k->sym != "")
    {
      if (what == "call")
        code = 0b00100001111100000000000000000000;
      else if (what == "jmp")
        code = 0b00111000111100000000000000000000;

      setPoolSymbol(k->sym, "jmpOrCall");
    }
    else
    {
      uint32_t number;
      if (k->lit != "")
      {
        number = literalToNumber(k->lit);
      }
      else if (k->num != -1)
      {
        number = k->num;
      }

      if (number > 4096 || number < 0)
      {
        bool flag2 = false;
        if (what == "call")
          code = 0b00100001111100000000000000000000;
        else if (what == "jmp")
          code = 0b00111000111100000000000000000000;
        string lit;
        if (k->lit != "")
          lit = k->lit;
        else
          lit = to_string(k->num);
        setPoolLiteral(lit);
        
      }
      else
      {
        if (what == "call")
          code = 0b00100000000000000000000000000000;
        else if (what == "jmp")
          code = 0b00110000111100000000000000000000;
        pomeraj = (0xFFF) & (number);
        code |= pomeraj;
      }
    }
    //push to memory
    pushToSection(cur_section, code);
  }
}

void Assembler::instructionBJMP(string what, string r1, string r2, vector<symLitNum *> sln)
{
  if (r1 == "sp")
  {
    r1 = "r14";
  }
  if (r1 == "pc")
  {
    r1 = "r15";
  }
  if (r2 == "sp")
  {
    r2 = "r14";
  }
  if (r2 == "pc")
  {
    r2 = "r15";
  }
  uint32_t reg1 = stoi(r1.substr(1)) << 16;
  uint32_t reg2 = stoi(r2.substr(1)) << 12;

  uint32_t code;
  uint32_t pomeraj;

  for (auto k : sln)
  {
    if (k->sym != "")
    {
      if (what == "beq")
        code = 0b00111001111100000000000000000000;
      else if (what == "bne")
        code = 0b00111010111100000000000000000000;
      else if (what == "bgt")
        code = 0b00111011111100000000000000000000;
      setPoolSymbol(k->sym, "bjmp");
    }
    else
    {
      uint32_t number;
      if (k->lit != "")
      {
        number = literalToNumber(k->lit);
      }
      else if (k->num != -1)
      {
        number = k->num;
      }

      if (number > 4096 || number < 0)
      {

        if (what == "beq")
          code = 0b00111001111100000000000000000000;
        else if (what == "bne")
          code = 0b00111010111100000000000000000000;
        else if (what == "bgt")
          code = 0b00111011111100000000000000000000;
        string lit;
        if (k->lit != "")
          lit = k->lit;
        else
          lit = to_string(k->num);
        setPoolLiteral(lit);
      }
      else
      {
        if (what == "beq")
          code = 0b00110001111100000000000000000000;
        else if (what == "bne")
          code = 0b00110010111100000000000000000000;
        else if (what == "bgt")
          code = 0b00110011111100000000000000000000;
        pomeraj = (0xFFF) & (number);
        code |= pomeraj;
      }
    }

    code |= reg1;
    code |= reg2;

    //push to memory
    pushToSection(cur_section, code);
  }
}

void Assembler::setXCHG(string reg1, string reg2)
{

  if (reg1 == "sp")
  {
    reg1 = "r14";
  }
  if (reg1 == "pc")
  {
    reg1 = "r15";
  }
  if (reg2 == "sp")
  {
    reg2 = "r14";
  }
  if (reg2 == "pc")
  {
    reg2 = "r15";
  }
  uint32_t code = 0b01000000000000000000000000000000;
  uint32_t rg1 = stoi(reg1.substr(1)) << 16;
  uint32_t rg2 = stoi(reg2.substr(1)) << 12;
  code |= rg1;
  code |= rg2;

  //push to memory
  pushToSection(cur_section, code);
}

void Assembler::setAritmeticOperation(string reg1, string reg2, int op)
{

  if (reg1 == "sp")
  {
    reg1 = "r14";
  }
  if (reg1 == "pc")
  {
    reg1 = "r15";
  }
  if (reg2 == "sp")
  {
    reg2 = "r14";
  }
  if (reg2 == "pc")
  {
    reg2 = "r15";
  }
  uint32_t code;
  uint32_t rg1 = stoi(reg1.substr(1)) << 16;
  uint32_t rg2 = stoi(reg2.substr(1)) << 12;
  uint32_t rg3 = stoi(reg2.substr(1)) << 20;
  switch (op)
  {
  case 1:
    code = 0b01010000000000000000000000000000;
    break;
  case 2:
    code = 0b01010001000000000000000000000000;
    break;
  case 3:
    code = 0b01010010000000000000000000000000;
    break;
  case 4:
    code = 0b01010011000000000000000000000000;
    break;

  default:
    break;
  }
  code |= rg1;
  code |= rg2;
  code |= rg3;

  //push to memory
  pushToSection(cur_section, code);
}

void Assembler::setLogicalOperations(string reg1, string reg2, int op)
{

  if (reg1 == "sp")
  {
    reg1 = "r14";
  }
  if (reg1 == "pc")
  {
    reg1 = "r15";
  }
  if (reg2 == "sp")
  {
    reg2 = "r14";
  }
  if (reg2 == "pc")
  {
    reg2 = "r15";
  }
  uint32_t code;
  switch (op)
  {
  case 1:
    code = 0b01100000000000000000000000000000;
     
    break;
  case 2:
    code = 0b01100001000000000000000000000000;
    break;
  case 3:
    code = 0b01100010000000000000000000000000;
    break;
  case 4:
    code = 0b01100011000000000000000000000000;
    break;

  default:
    break;
  }

  uint32_t rg1 = stoi(reg1.substr(1)) << 16;
  code |= rg1;
  if (op != 1)
  {
    int32_t rg2 = stoi(reg2.substr(1)) << 12;
    uint32_t rg3 = stoi(reg2.substr(1)) << 20;
    code |= rg2;
    code |= rg3;
  }

  //push to memory
  pushToSection(cur_section, code);
}

void Assembler::setShiftOperation(string reg1, string reg2, int op)
{

  if (reg1 == "sp")
  {
    reg1 = "r14";
  }
  if (reg1 == "pc")
  {
    reg1 = "r15";
  }
  if (reg2 == "sp")
  {
    reg2 = "r14";
  }
  if (reg2 == "pc")
  {
    reg2 = "r15";
  }
  uint32_t code;
  switch (op)
  {
  case 1:
    code = 0b01110000000000000000000000000000;
    
    break;
  case 2:
    code = 0b01110001000000000000000000000000;
    break;

  default:
    break;
  }

  uint32_t rg1 = stoi(reg1.substr(1)) << 16;
  code |= rg1;
  if (op != 1)
  {
    int32_t rg2 = stoi(reg2.substr(1)) << 12;
    uint32_t rg3 = stoi(reg2.substr(1)) << 20;
    code |= rg2;
    code |= rg3;
  }

  //push to memory
  pushToSection(cur_section, code);
}

void Assembler::loadOperation(Operand *oper, string reg)
{
  bool csrReg = false;
  if (reg == "sp")
  {
    reg = "r14";
  }
  else if (reg == "pc")
  {
    reg = "r15";
  }
  else if (reg == "status" | reg == "cause" || reg == "handler")
  {
    csrReg = true;
  }

  uint32_t code;
  uint32_t rg3;
  if (!csrReg)
  {
    rg3 = stoi(reg.substr(1)) << 20;
    // printf("Load operation: reg: %d\n",rg3);
  }

  if (oper->addrType == 0)
  { // registarsko direktno adresiranje
    string k = oper->regNumb;
    if (k == "sp")
    {
      k = "r14";
    }
    if (k == "pc")
    {
      k = "r15";
    }
    uint32_t rg2 = stoi(k.substr(1)) << 16;
    code = 0b10010001000000000000000000000000;
    code |= rg2;
    code |= rg3;
  }

  else if (oper->addrType == 1)
  {
    code = 0b10010001000000000000000000000000;
    if (csrReg)
    {
      if (reg == "status")
      {
        code = 0b10010101000000000000000000000000;
      }
      else if (reg == "cause")
      {
        code = 0b10010101001000000000000000000000;
      }
      else if (reg == "handler")
      {
        code = 0b10010101000100000000000000000000;
      }
    }

    if (oper->argType == 0)
    {
      unordered_map<string, SymbolTable>::iterator got = sym_table.find(oper->symbol);
      bool f1 = false;
      if (got != sym_table.end())
      {
        if (got->second.defined)
        {
          if (got->second.symbolValue <= 4096)
          {
            code |= ((char)0xFFF & got->second.symbolValue);
            f1 = true;
          }
        }
      }
      if (!f1)
      {
        setPoolSymbol(oper->symbol, "load");
        code = 0b10010010000011110000000000000000; // memorijsko direktno
      }
    }
    if (oper->argType == 1)
    {
      if (oper->value > 4096 || oper->value < 0)
      {
        //printf("Literal %d je ubacen u load", oper->value);
        setPoolLiteral(std::to_string(oper->value));
        code = 0b10010010000011110000000000000000; // memorijsko direktno
      }
      else
      {
        int d = oper->value;
        code |= ((char)0xFFF & d);
      }
    }

    if (!csrReg)
    {
      code |= rg3;
    }
  }
  if (oper->addrType == 2)
  {
    code = 0b10010010000000000000000000000000;

    if (oper->argType == 0)
    { // vrednost iz memorije na adresi symbol
      setPoolSymbol(oper->symbol, "load");
      code = 0b10010010000011110000000000000000; // memorijsko indirektno
      code |= rg3;
      //push to memory
      pushToSection(cur_section, code);
      code = 0b10010010000000000000000000000000;
      uint32_t pom = stoi(reg.substr(1)) << 16;
      code |=pom;
    }
    if (oper->argType == 1)
    {
      setPoolLiteral(oper->symbol);
      code = 0b10010010000011110000000000000000; // memorijsko indirektno
      code |= rg3;
      //push to memory
      pushToSection(cur_section, code);
      code = 0b10010010000000000000000000000000;
      uint32_t pom = stoi(reg.substr(1)) << 16;
      code |=pom;
    }
    code |= rg3;
    if (oper->argType == 3)
    {

      string k = oper->regNumb;
      if (k == "sp")
      {
        k = "r14";
      }
      if (k == "pc")
      {
        k = "r15";
      }
      uint32_t rg2 = stoi(k.substr(1)) << 16;
      code |= rg2;

      if (csrReg)
      {
        if (reg == "status")
        {
          code = 0b10010110000011100000000000000000;
        }
        else if (reg == "cause")
        {
          code = 0b10010110001011100000000000000000;
        }
        else if (reg == "handler")
        {
          code = 0b10010110000111100000000000000000;
        }
      }
      int d = oper->value;
      code |= ((char)0xFFF & d);
    }
    if (oper->argType == 4)
    {
      code = 0b10010011000000000000000000000000;
      string k = oper->regNumb;
      if (k == "sp")
      {
        k = "r14";
      }
      if (k == "pc")
      {
        k = "r15";
      }
      uint32_t rg2 = stoi(k.substr(1)) << 16;
      code |= rg2;
      code |= rg3;
      int d = oper->value;
      code |= ((char)0xFFF & d);
      // setPoolSymbol(oper->symbol, "load");
    }
  }
  //push to memory
  pushToSection(cur_section, code);
}
void Assembler::setPoolLiteral(string lit)
{
  bool flag2 = false;
  for (auto &r : cur_section.pool)
  {
    if (r.notSymbol)
    {
      uint32_t val = literalToNumber(r.symbol);
      if (val == literalToNumber(lit))
      {
        flag2 = true;
        r.positionsValue.push_back(cur_section.location_counter);
        break;
      }
    }
  }
  if (flag2 == false)
  {
    PoolEntry pentry;
    pentry.positionsValue = vector<uint32_t>();
    pentry.symbol = lit;
    pentry.address = 0;
    pentry.notSymbol = true;
    pentry.positionsValue.push_back(cur_section.location_counter);
    cur_section.pool.push_back(pentry);
  }
}

void Assembler::setPoolSymbol(string symbol, string instructionName)
{
  unordered_map<string, SymbolTable>::iterator got = sym_table.find(symbol);
  if (got != sym_table.end())
  {
    bool flag = false;
    for (auto &q : cur_section.pool)
    {
      if (q.symbol == symbol)
      {
        q.positionsValue.push_back(cur_section.location_counter);
        flag = true;
        // ne treba mi realokacija jer je postavljena vrednost u bazenu literala a meni treba u ovom momentu samo pomeraj
        break;
      }
    }
    if (flag != true)
    {
      
      PoolEntry pentry;
      pentry.positionsValue = vector<uint32_t>();
      pentry.symbol = symbol;
      pentry.address = 0;
      pentry.notSymbol = false;
      pentry.positionsValue.push_back(cur_section.location_counter);
      cur_section.pool.push_back(pentry);
    }
  }
  else
  { // nije pronadjen u tabeli simbola
    OccursSymbol o;
    o.instructionName = instructionName;
    o.positionValue = cur_section.location_counter;
    addSymbolToSymTable(symbol, Loc, NoType, cur_section, o);
  }
}

void Assembler::storeOperation(Operand *oper, string reg)
{
  if (reg == "sp")
  {
    reg = "r14";
  }
  if (reg == "pc")
  {
    reg = "r15";
  }

  uint32_t code;

  uint32_t rg3 = stoi(reg.substr(1)) << 12; // regC

  if (oper->addrType == 0)
  {
    code = 0b10000000000000000000000000000000;

    if (oper->argType == 3)
    {
      string k = oper->regNumb;
      if (k == "sp")
      {
        k = "r14";
      }
      if (k == "pc")
      {
        k = "r15";
      }
      uint32_t rg2 = stoi(k.substr(1)) << 16;
      code |= rg2;
      int d = oper->value;
      code |= ((char)0xFFF & d);
    }
    if (oper->argType == 4)
    {
      string k = oper->regNumb;
      if (k == "sp")
      {
        k = "r14";
      }
      if (k == "pc")
      {
        k = "r15";
      }
      uint32_t rg2 = stoi(k.substr(1)) << 16;
      code |= rg2;
      // setPoolSymbol(oper->symbol, "store");
      // ovde treba da se pronadje u tabeli simbola i izvrsi
      // code |= ((char)0xFFF & symbolValue);
    }
    code |= rg3;
  }

  if (oper->addrType == 1) // registarsko direktno ali sa literalima i simbolima
  {
    code = 0b10000000000000000000000000000000;

    if (oper->argType == 0)
    {
      bool f1 = false;
      unordered_map<string, SymbolTable>::iterator got = sym_table.find(oper->symbol);
      if (got != sym_table.end())
      {
        if (got->second.defined)
        {
          if (got->second.symbolValue <= 4096)
          {
            code |= ((char)0xFFF & got->second.symbolValue);
            f1 = true;
          }
        }
      }
      if (!f1)
      {
        setPoolSymbol(oper->symbol, "store");
        code = 0b10000010000011110000000000000000; // memorijsko direktno
      }
    }
    if (oper->argType == 1)
    {
      if (oper->value > 4096 || oper->value < 0)
      {
        setPoolLiteral(std::to_string(oper->value));
        code = 0b10000010000011110000000000000000; // memorijsko direktno
      }
      else
      {
        int d = oper->value;
        code |= ((char)0xFFF & d);
      }
    }
    code |= rg3;
  }
  if (oper->addrType == 2)
  {
    code = 0b10000010000011110000000000000000; // memorijsko indirektno

    if (oper->argType == 0)
    { // vrednost iz memorije na adresi symbol
      setPoolSymbol(oper->symbol, "store");
    }
    if (oper->argType == 1)
    {
      setPoolLiteral(oper->symbol);
    }

    code |= rg3;
  }
  //push to memory
  pushToSection(cur_section, code);
}

void Assembler::pushOperation(string reg)
{
  if (reg == "sp")
  {
    reg = "r14";
  }
  if (reg == "pc")
  {
    reg = "r15";
  }
  uint32_t code;
  uint32_t rg1 = 14 << 20;
  if (reg == "status")
  {
    code = 0b10000011000000000000000000000000;
  }
  else if (reg == "cause")
  {
    code = 0b10000011000000000010000000000000;
  }
  else if (reg == "handler")
  {
    code = 0b10000011000000000001000000000000;
  }
  else
  {
    uint32_t rg3 = stoi(reg.substr(1)) << 12;
    code = 0b10000001000000000000000000000000;
    code |= rg3;
  }
  code |= rg1;
  //push to memory
  pushToSection(cur_section, code);
}
void Assembler::popOperation(string reg)
{
  if (reg == "sp")
  {
    reg = "r14";
  }
  if (reg == "pc")
  {
    reg = "r15";
  }
  uint32_t code;
  if (reg == "status")
  {
    code = 0b10010111000011100000000000000100;
  }
  else if (reg == "cause")
  {
    code = 0b10010111001011100000000000000100;
  }
  else if (reg == "handler")
  {
    code = 0b10010111000111100000000000000100;
  }
  else
  {
    uint32_t rg1 = stoi(reg.substr(1)) << 20;
    uint32_t rg3 = 14 << 16;
    code = 0b10010011000000000000000000000100;
    code |= rg3;
    code |= rg1;
  }
  //push to memory
  pushToSection(cur_section, code);
}

void Assembler::csrrdOperation(string csr, string reg)
{
  if (reg == "sp")
  {
    reg = "r14";
  }
  if (reg == "pc")
  {
    reg = "r15";
  }
  uint32_t rg1 = stoi(reg.substr(1)) << 20;
  uint32_t code = 0b10010000000000000000000000000000;
  code |= rg1;
  if (csr == "status")
  {
    uint32_t rg2 = (0x0 << 16);
    code |= rg2;
  }
  else if (csr == "handler")
  {
    uint32_t rg2 = (0x1 << 16);
    code |= rg2;
  }
  else if (csr == "cause")
  {
    uint32_t rg2 = (0x2 << 16);
    code |= rg2;
  }
  else
  {
    printf("Greska, lose unet csr registar!\n");
    exit(-5);
  }

  //push to memory
  pushToSection(cur_section, code);
}
void Assembler::csrwrOperation(string csr, string reg)
{
  if (reg == "sp")
  {
    reg = "r14";
  }
  if (reg == "pc")
  {
    reg = "r15";
  }
  uint32_t rg1 = stoi(reg.substr(1)) << 16;
  uint32_t rg2;
  uint32_t code = 0b10010100000000000000000000000000;
  code |= rg1;
  if (csr == "status")
  {
    uint32_t rg2 = (0 << 20);
    code |= rg2;
  }
  else if (csr == "handler")
  {
    uint32_t rg2 = (1 << 20);
    code |= rg2;
  }
  else if (csr == "cause")
  {
    uint32_t rg2 = (2 << 20);
    code |= rg2;
  }
  else
  {
    printf("Greska, lose unet csr registar!\n");
    exit(-5);
  }

  //push to memory
  pushToSection(cur_section, code);
}

void Assembler::writeSymTable(string outputFileName)
{
  uint64_t SymbolTableSize = sym_table.size();

  //printf("Velicina tabele simbola: %lu", SymbolTableSize);

  ofstream outputFile;
  outputFileName = outputFileName;

  outputFile.open(outputFileName, ios::trunc | ios::binary);
  if (outputFile.fail())
  {
    printf("Ne radi ti ovo!\n");
  }
  outputFile.write((char *)&SymbolTableSize, sizeof(SymbolTableSize));
  for (auto entry : sym_table)
  {
    SymbolTable s = entry.second;
    outputFile.write((char *)&s.symbolId, sizeof(s.symbolId));

    int symNameLen = s.symbolName.capacity();
    outputFile.write((char *)&symNameLen, sizeof(symNameLen));
    for (int i = 0; i < symNameLen; i++)
    {
      outputFile << s.symbolName[i];
    }
    outputFile << "\0";
    outputFile.write((char *)&s.symbolValue, sizeof(s.symbolValue));
    outputFile.write((char *)&s.size, sizeof(s.size));
    outputFile.write((char *)&s.sectionId, sizeof(s.sectionId));

    int symNameLen2 = s.sectionName.capacity();
    outputFile.write((char *)&symNameLen2, sizeof(symNameLen2));
    for (int i = 0; i < symNameLen2; i++)
    {
      outputFile << s.sectionName[i];
    }
    outputFile << "\0";

    outputFile.write((char *)&s.locGlob, sizeof(s.locGlob));
    outputFile.write((char *)&s.type, sizeof(s.type));
    int def = 0;
    if (s.defined)
    {
      def = 1;
    }
    outputFile.write((char *)&def, sizeof(def));
  }
  outputFile.close();
}
void Assembler::writeSectionTable(string outputFileName)
{
  uint64_t SectionTableSize = section_table.size();

  //printf("\nVelicina tabele sekcija: %lu \n", SectionTableSize);

  ofstream outputFile;
  outputFileName = outputFileName;

  outputFile.open(outputFileName, ios::binary | ios::app);
  if (outputFile.fail())
  {
    printf("Ne radi ti ovo!\n");
  }
  outputFile.write((char *)&SectionTableSize, sizeof(SectionTableSize));
  for (auto entry : section_table)
  {
    Section s = entry.second;

    outputFile.write((char *)&s.idSection, sizeof(s.idSection));

    int symNameLen = s.name.capacity();
    outputFile.write((char *)&symNameLen, sizeof(symNameLen));

    for (int i = 0; i < symNameLen; i++)
    {
      outputFile << s.name[i];
    }
    outputFile << "\0";
    outputFile.write((char *)&s.size, sizeof(s.size));
    int n = s.mem_space.size();
    //printf("Mem_space size: %d, a section size: %d\n", n, s.size);
    outputFile.write((char *)&n, sizeof(n));
    // outputFile.write(s.mem_space.data(), n);
    for (int i = 0; i < n; i++)
    {
      outputFile.write((char *)&s.mem_space[i], sizeof(char));
    }
    // outputFile<<"\0";
    uint64_t relEntrySize = s.relocation_table.size();
    outputFile.write((char *)&relEntrySize, sizeof(relEntrySize));
    for (auto rtEntry : s.relocation_table)
    {
      int symLen = rtEntry.symbolName.capacity();
      outputFile.write((char *)&symLen, sizeof(symLen));

      for (int i = 0; i < symNameLen; i++)
      {
        outputFile << rtEntry.symbolName[i];
      }
      outputFile << "\0";
      outputFile.write((char *)&rtEntry.offset, sizeof(rtEntry.offset));
      outputFile.write((char *)&rtEntry.addend, sizeof(rtEntry.addend));
    }
  }
  outputFile.close();
}