#pragma once
#include "../src/errorhandler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>

namespace core_file
{
    enum class FileCode : int {
        SUCCESS = 0,
        ERROR_CANTOPENFILE = 1,
        ERROR_STRINGNOTPROVIDED = 2,
    };

    class File {
    private:
        bool b_IsAvailable = false;
        std::vector<std::string> storedLines;
    public:
        std::string fileName;
        std::function<void(const std::string& line, const int& lineNumber, const bool &b_isLastLine)> funcToExecute;
        
        //File(const char* argFileName, std::function<void()> argLambda) : fileName(argFileName), funcToExecute(argLambda);
        File(const char* argFileName, std::function<void(const std::string&, const int& lineNumber, const bool &b_isLastLine)> argLambda);
        
        FileCode Init();
        
        void Execute();
    };
}