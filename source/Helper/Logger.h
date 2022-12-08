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
	explicit Logger(char* module, bool debug = true) {
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
		this->level = "DEBUG";
		std::cout << *this << "\t";
		return *this;
	}
	void debug(char* msg) {
		debug() << msg << std::endl;
	}

	Logger& info() {
		this->level = "INFO";
		std::cout << *this << "\t";
		return *this;
	}
	void info(char* msg) {
		info() << msg << std::endl;
	}

	Logger& warn() {
		this->level = "WARN";
		std::cout << *this << "\t";
		return *this;
	}
	void warn(char* msg) {
		warn() << msg << std::endl;
	}

	Logger& error() {
		this->level = "ERROR";
		std::cout << *this << "\t";
		return *this;
	}
	void error(char* msg) {
		error() << msg << std::endl;
	}

	friend std::ostream& operator<<(std::ostream& out, const Logger& logger) {
		out << "[" << logger.module << " | " << logger.level << "]";
		return out;
	}

private:
	char* module;
	char* level = "DEBUG";

	FILE* fp;
};
