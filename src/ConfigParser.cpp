#include "ConfigParser.hpp"

/* Constructors & Destructors */
ConfigParser::ConfigParser(const char* path)
{
	std::string line;
	std::ifstream inputFile(path);
	std::vector<std::string> tokens;

	if (inputFile.fail())
		throw; // todo: exception
	while (std::getline(inputFile, line, '\n'))
	{
		for (size_t i = 0; i < line.size(); i++)
		{
			if (line[i] == '#')
				line = line.substr(0, i);
		}
		std::stringstream ssa(line);
		std::string tmpa;
		while (std::getline(ssa, tmpa, ' '))
		{
			std::stringstream ss(tmpa);
			std::string tmp;
			while (std::getline(ss, tmp, '\t'))
			{
				size_t lastSplit = 0;
				for (size_t i = 0; i < tmp.size(); i++)
				{
					if (tmp[i] == '{' || tmp[i] == '}' || tmp[i] == ';')
					{
						std::string check = tmp.substr(lastSplit, (i - lastSplit));
						if (check != "")
							tokens.push_back(check);
						tokens.push_back(tmp.substr(i, 1));
						lastSplit = i + 1;
					}
				}
				if (tmp == "")
					continue;
				if (lastSplit == 0)
					tokens.push_back(tmp);
				if (lastSplit != 0)
				{
					std::string check = tmp.substr(lastSplit);
					if (check != "")
						tokens.push_back(check);
				}
			}
		}
	}
	if (!inputFile.eof())
		throw; // todo: exception
	for (size_t i = 0; i < tokens.size(); i++)
	{
		std::cout << "\"" << tokens[i] << "\", ";
	}
}

ConfigParser::~ConfigParser()
{
}

/* Member Functions */

/* Basic Operators */

/* Getters & Setters */
