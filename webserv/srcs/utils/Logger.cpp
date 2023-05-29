#include "Logger.hpp"

LoggingLevel Logger::CurrentLevel = ERROR;

Logger::Logger()
{
}

Logger::Logger(LoggingLevel level)
{
	_msgLevel = level;
	if (level >= Logger::CurrentLevel)
		std::cout << "[" << getLabel(level) << "] ";
}

Logger::~Logger()
{
	if (_msgLevel >= Logger::CurrentLevel)
		std::cout << std::endl;
}

void Logger::setCurrentLevel(LoggingLevel level)
{
	Logger::CurrentLevel = level;
}

const std::string Logger::getLabel(LoggingLevel level) const
{
	switch (level)
	{
		case DEBUG :
			return "DEBUG";
			break;
		case INFO :
			return "INFO";
			break;
		case WARN :
			return "WARNING";
			break;
		case ERROR :
			return "ERROR";
			break;
	}
	return "";
}
