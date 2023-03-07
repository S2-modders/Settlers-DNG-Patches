/*
 * simple header only logger class
 *
 * @author zocker_160
 * @license GPLv3
 *
 */
#pragma once

#include <Windows.h>
#include <iostream>

namespace Logging {

typedef std::ostream& (*Manip1)(std::ostream&);

class Logger {
public:
    explicit Logger(char* module, bool console = false) {
        this->module = module;

        if (console)
            setupConsole();
    }

    explicit Logger(char* module, char* logfile, bool console = false) {
        this->module = module;

        if (console)
            setupConsole();
        else
            setupLogfile(logfile);
    }

    template<typename T>
    Logger& operator<<(T t) {
        std::cout << t;
        return *this;
    }
    Logger& operator<<(Manip1 fp) {
        std::cout << fp;
        return *this;
    }

    Logger& debug() {
        return printSelf("DEBUG");
    }
    void debug(char* msg) {
        debug() << msg << std::endl;
    }

    Logger& info() {
        return printSelf("INFO");
    }
    void info(char* msg) {
        info() << msg << std::endl;
    }

    Logger& warn() {
        return printSelf("WARN");
    }
    void warn(char* msg) {
        warn() << msg << std::endl;
    }

    Logger& error() {
        return printSelf("ERROR");
    }
    void error(char* msg) {
        error() << msg << std::endl;
    }

    Logger& naked() {
        return *this;
    }
    void naked(char* msg) {
        naked() << msg << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& out, const Logger& logger) {
        out << "[" << logger.module << "] " << logger.level << ":";
        return out;
    }

private:
    char* module;
    char* level = "DEBUG";

    FILE* fp;
    FILE* flog;

    Logger& printSelf(char* level) {
        this->level = level;
        std::cout << *this << "\t";
        return *this;
    }

    void setupConsole() {
        AllocConsole();
        freopen_s(&fp, "CONOUT$", "w", stdout);
    }

    void setupLogfile(char* logfile) {
        freopen_s(&flog, logfile, "w", stdout);
    }
};

}