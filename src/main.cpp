#include <iostream>
#include <fstream>
#include <string>
#include "../inc/assembler.hpp"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

extern void parser_main(Assembler *k, char *fileName);





int main(int argc, char *argv[])
{
    char *inputFile;
    string outputFileName;

    int k = argc;
   
    inputFile = argv[3];
    outputFileName = argv[2];
    // printf("outputFIleName: %s\n",outputFileName.c_str());
    // printf("inputFileName: %s", inputFile);

    ifstream inFile(inputFile);
    if (!inFile)
    {
        printf("Unable to open input file: %s\n", inputFile);
        return 1;
    }
    

    Assembler *asembl = new Assembler();
    asembl->setOutputFileName(outputFileName);
    if (asembl == nullptr)
    {
        printf("Lose se pravi asembler\n");
        
    }
    parser_main(asembl, inputFile);

    // asembl->print_symbol_table();


    inFile.close();

    delete asembl;


    return 0;
}
