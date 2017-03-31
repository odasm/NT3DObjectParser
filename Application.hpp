#pragma once
#ifndef _NOSTALE_3D_OBJECT_PARSER_APPLICATION
#define _NOSTALE_3D_OBJECT_PARSER_APPLICATION

#include <iostream>
#include <string>
#include <cstdio> //strcmp

#include "NTFile.hpp"

#define APPLICATION_SHOW_HELP		(1 << 0)
#define APPLICATION_LIST_FILES		(1 << 1)
#define APPLICATION_EXTRACT_FILE	(1 << 2)
#define APPLICATION_EXTRACT_FILES	(1 << 3)
#define APPLICATION_EXPORT_OBJECT	(1 << 4)
#define APPLICATION_EXPORT_OBJECTS	(1 << 5)

class CApplication {
private:
	char m_Flags;
	std::string m_szInputFileName;
	std::string m_szInput;
	std::string m_szOutputDestination;

public:
	CApplication() noexcept { ; }
	virtual ~CApplication() noexcept { ; }

	bool bootStrap(int argc, char *argv[]) noexcept {
		m_Flags = 0;
		for (auto i = 0; i < argc; i++) {
			if ((!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input")) && i + 1 < argc)
				m_szInputFileName = argv[++i];
			else if ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) && i + 1 < argc)
				m_szOutputDestination = argv[++i];
			else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
				m_Flags |= APPLICATION_SHOW_HELP;
			else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list"))
				m_Flags |= APPLICATION_LIST_FILES;
			else if (!strcmp(argv[i], "-xa") || !strcmp(argv[i], "--extractall"))
				m_Flags |= APPLICATION_EXTRACT_FILES;
			else if (!strcmp(argv[i], "-ea") || !strcmp(argv[i], "--exportall"))
				m_Flags |= APPLICATION_EXPORT_OBJECTS;
			else if ((!strcmp(argv[i], "-x") || !strcmp(argv[i], "--extract")) && i + 1 < argc) {
				m_Flags |= APPLICATION_EXTRACT_FILE;
				m_szInput = argv[++i];
			} else if ((!strcmp(argv[i], "-e") || !strcmp(argv[i], "--export")) && i + 1 < argc) {
				m_Flags |= APPLICATION_EXPORT_OBJECT;
				m_szInput = argv[++i];
			}
		}

		if (m_Flags == 0) //Application executed without action args
			return false;

		return true;
	}

	bool Run() noexcept(false) {
		if (m_Flags & APPLICATION_SHOW_HELP) {
			showHelp();
			return false;
		}
		
		CNTFile NTFile;
		if (!NTFile.Open(m_szInputFileName)) {
			std::cout
				<< "Couldn't process file \"" << m_szInputFileName << "\"." << std::endl;
			return false;
		}
		if (m_Flags & APPLICATION_LIST_FILES)
			NTFile.List();
		if (m_Flags & APPLICATION_EXTRACT_FILE)
			NTFile.Extract(std::stoi(m_szInput), m_szOutputDestination);
		if (m_Flags & APPLICATION_EXTRACT_FILES)
			NTFile.ExtractAll(m_szOutputDestination);

		return true;
	}
	void showUsage() noexcept {
		std::cout
			<< "Usage: Use -h / --help for further information." << std::endl;
	}
	void showHelp() noexcept {
		std::cout
			<< "NT3DObjectParser" << std::endl
			<< std::endl
			<< "Required arguments:" << std::endl
			<< "\t-i, --input\tINPUT\tInput NosTale file." << std::endl
			<< std::endl
			<< "May be required arguments:" << std::endl
			<< "\t-o, --output\tOUTPUT\tOuput destination." << std::endl
			<< std::endl
			<< "Action arguments:" << std::endl
			<< "\t-h, --help\t\tShow this help message and exit." << std::endl
			<< "\t-l, --list\t\tLists files." << std::endl
			<< "\t-x, --extract\tFILE_ID\tExtracts specified FILE_ID, requires --output" << std::endl
			<< "\t-xa, --extractall\tExtract all files into directory, requires --output" << std::endl
			<< "\t-e, --export\tFILE_ID\tExports specified FILE_ID 3D Model to .obj, requires --output" << std::endl
			<< "\t-ea, -exportall\t\tExports all fils to .obj, requires --output" << std::endl
			<< std::endl;
	}
};

#endif // !_NOSTALE_3D_OBJECT_PARSER_APPLICATION
