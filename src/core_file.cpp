#include "core_file.h"

core_file::File::File(const char* argFileName, std::function<void(const std::string&, const int& lineNumber, const bool &b_isLastLine)> argLambda) : fileName(argFileName), funcToExecute(argLambda)
{   // std::function<void(const std::string& line, const int& lineNumber, bool b_isLastLine)> funcToExecute;
    //
    FileCode code = Init();
    if (code == FileCode::SUCCESS)
    {
        printf("File named \"%s\" was loaded successfully.\n", fileName.c_str());
        b_IsAvailable = true;
    }
    else
    {
        if (code == FileCode::ERROR_CANTOPENFILE)
        {
            // std::cerr << "Error: Could not open file named \" " << fileName << "\"." << std::endl;
            std::string msg = std::string{ "Could not open file named \" " }.append(fileName).append("\".");
            ErrorHandler::report(msg);
            b_IsAvailable = false;
        }
        if (code == FileCode::ERROR_STRINGNOTPROVIDED)
        {
            // std::cerr << "Error: String not provided to File()'s constructor." << std::endl;
            std::string msg = std::string{ "String not provided to File()'s constructor." };
            ErrorHandler::report(msg);
        }
    }
};

core_file::FileCode core_file::File::Init()
{
    bool b_WasStringProvided = (this->fileName.length());
    if (b_WasStringProvided)
    {
        std::ifstream file(fileName);

        if (!file.is_open())
        {
            return FileCode::ERROR_CANTOPENFILE;
        }
        std::string line;
        while (std::getline(file, line))
        {
            storedLines.push_back(line + "\n");
        }
        return FileCode::SUCCESS;
    }
    else
    {
        return FileCode::ERROR_STRINGNOTPROVIDED;
    }
}

void core_file::File::Execute()
{
    if (funcToExecute == nullptr)
    {
        std::string msg = std::string{ "Could not execute Lambda in File class. Did you check if it a nullptr? " };
        ErrorHandler::report(msg);
    }
    else
    {
        if (!b_IsAvailable)
        {
            // std::cerr << "(In File::Execute() -> ) Lambda exists, but can't open file." << std::endl;
            std::string msg = std::string{ "(In File::Execute() ->) Lambda exists, but can't open file." };
            ErrorHandler::report(msg);
        }
        else // it IS available.
        {
            int idx = 0;
            for (const auto& line : storedLines)
            {
                bool b_isLastLine = (idx == storedLines.size() - 1);
                funcToExecute(line, idx, b_isLastLine);
                idx++;
            }
            
        }
    }
}