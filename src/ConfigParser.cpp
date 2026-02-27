#include "ConfigParser.hpp"

/* Constructors & Destructors */
ConfigParser::ConfigParser(const char* path)
{
	std::ifstream configFile(path);
	if (configFile.fail())
		throw; // todo: exception
	std::vector<std::string> tokens = extractTokens(configFile);
}

ConfigParser::~ConfigParser()
{
}

/* Member Functions */
std::vector<std::string> ConfigParser::extractTokens(std::ifstream& configFile)
{
	std::string line;
	std::vector<std::string> tokens;

	while (std::getline(configFile, line, '\n'))
	{
		for (size_t i = 0; i < line.size(); i++)
		{
			if (line[i] == '#')
				line = line.substr(0, i);
		}
		std::stringstream ssLine(line);
		std::string splitBySpace;
		while (std::getline(ssLine, splitBySpace, ' '))
		{
			std::stringstream ssSplitBySpace(splitBySpace);
			std::string splitByTab;
			while (std::getline(ssSplitBySpace, splitByTab, '\t'))
			{
				size_t lastSplit = 0;
				for (size_t i = 0; i < splitByTab.size(); i++)
				{
					if (splitByTab[i] == '{' || splitByTab[i] == '}' || splitByTab[i] == ';')
					{
						std::string tmp = splitByTab.substr(lastSplit, (i - lastSplit));
						if (tmp != "")
							tokens.push_back(tmp);
						tokens.push_back(splitByTab.substr(i, 1));
						lastSplit = i + 1;
					}
				}
				if (splitByTab == "")
					continue;
				if (lastSplit == 0)
					tokens.push_back(splitByTab);
				if (lastSplit != 0)
				{
					std::string tmp = splitByTab.substr(lastSplit);
					if (tmp != "")
						tokens.push_back(tmp);
				}
			}
		}
	}
	if (!configFile.eof())
		throw; // todo: exception
	for (size_t i = 0; i < tokens.size(); i++)
	{
		std::cout << "\"" << tokens[i] << "\", ";
	}
	return (tokens);
}

/* Basic Operators */

/* Getters & Setters */
