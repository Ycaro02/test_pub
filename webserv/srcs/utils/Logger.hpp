#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

/* #include <string> */
/* #include <iostream> */
#include "webserv.hpp"

enum LoggingLevel {DEBUG, INFO, WARN, ERROR};

class Logger {
	private :
		static LoggingLevel CurrentLevel;
		LoggingLevel _msgLevel;

		const std::string getLabel(LoggingLevel lvl) const;
		/* const std::string& getColor(LoggingLevel lvl) const; */
	public :
		Logger();
		Logger(LoggingLevel mode);
		~Logger();

		template <class T>
		Logger &operator<<(const T msg);

		static void setCurrentLevel(LoggingLevel mode);
};

template <class T>
Logger &Logger::operator<<(const T msg)
{
	if (_msgLevel >= Logger::CurrentLevel)
	{
		if (_msgLevel >= WARN)
			std::cerr << msg;
		else
			std::cout << msg;
	}
	return (*this);
}

#endif
