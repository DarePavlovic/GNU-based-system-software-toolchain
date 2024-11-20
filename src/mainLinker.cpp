#include <iostream>
#include <fstream>
#include <string>
#include "../inc/linker.hpp"
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{

    bool hexFlag = false;
    string outputFile;
    vector<pair<string, string>> places;
    vector<string> objectFiles;

    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];

        if (arg == "-hex")
        {
            hexFlag = true;
        }
        else if (arg.rfind("-place=", 0) == 0)
        {
            string placeArg = arg.substr(7);
            size_t atPos = placeArg.find('@');
            if (atPos != string::npos)
            {
                string name = placeArg.substr(0, atPos);
                string address = placeArg.substr(atPos + 1);
                places.push_back({name, address});
            }
        }
        else if (arg == "-o")
        {
            if (i + 1 < argc)
            { // Make sure the output file argument is present
                outputFile = argv[++i];
            }
            else
            {
                cerr << "Greska: Ne dostaje ime izlaznog fajla nakon -o" << endl;
                return -1;
            }
        }
        else
        {
            objectFiles.push_back(arg);
        }
    }
    //std::cout << "Places: " << std::endl;
    // for (const auto &place : places)
    // {
    //     std::cout << "  " << place.first << " at " << place.second << std::endl;
    // }

    Linker *linker = new Linker();
    if (linker == nullptr)
    {
        cout << "Lose se pravi linker\n";
    }

    linker->readAssemblyFiles(objectFiles, objectFiles.size());
    linker->load_sections();
    linker->setSectionStartAddress(places);
    linker->create_intern_section();
    linker->make_global_symbol_table();
    linker->checkUndefinedSymbols();
    linker->update_reloc_tables();
    linker->resolve_reloc_entries();

    if (outputFile != "")
    {
        linker->make_output_file(outputFile);
        linker->make_output_txt_file(outputFile);
    }
    linker->print_symbol_tableFile();
    linker->print_relocation_table2File();

    delete linker;

    return 0;
}
