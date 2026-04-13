#pragma once

#include <exception>
#include <string>
#include "../../inc/Client.hpp"

class ExceptionError : std::exception
{
	std::string	errMsg;
	Client		errSubject;
	int			errCode;
	const char *what() const throw();
};

const char *ExceptionError::what() const throw()
{
	// return message with errMsg and errCode	
}