#pragma once
#include <iostream>
#include <string>
#include <cstdlib>

class ErrorHandler {
public:
    static void report(int lineNumber, const std::string& message)
    {
        std::cerr << "[Error] Line " << lineNumber + 1 << ": " << message << "\n";
        // lineNumber plus one because thats what an IDE/code editor shows.
        
        // Optional: If you want errors to be fatal immediately:
        // std::exit(EXIT_FAILURE); 
    }

    static void report(const std::string& message) {
        std::cerr << "[Error]: " << message << "\n";
    }
};