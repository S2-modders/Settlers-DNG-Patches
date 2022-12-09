#pragma once

/*
 * simple header only logger class
 *
 * @author zocker_160
 * @license GPLv3
 *
 */

#include <Windows.h>
#include <iostream>

typedef std::ostream& (*Manip1)(std::ostream&);

class Logger {
public:
	explicit Logger(char* module, bool debug = false) {
		this->module = module;

		if (debug) {
			AllocConsole();
			freopen_s(&fp, "CONOUT$", "w", stdout);
		}
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

	friend std::ostream& operator<<(std::ostream& out, const Logger& logger) {
		out << "[" << logger.module << "] " << logger.level << ":";
		return out;
	}

private:
	char* module;
	char* level = "DEBUG";

	FILE* fp;

	Logger& printSelf(char* level) {
		this->level = level;
		std::cout << *this << "\t";
		return *this;
	}
};
