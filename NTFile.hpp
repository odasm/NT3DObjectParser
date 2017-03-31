#pragma once
#ifndef _NOSTALE_3D_OBJECT_PARSER_NOSTALE_FILE
#define _NOSTALE_3D_OBJECT_PARSER_NOSTALE_FILE

#include <mutex>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <iomanip>

class CNTFile {
private:
	FILE *m_fHandle;
	std::once_flag m_onceFlag;

	std::string m_szFileHeader;
	std::string m_szLiteralDate;
	std::vector<std::pair<int, int>> m_vecFiles;

	std::string convertToLiteralDate(int iDate) noexcept(false) {
		char *pszLiteralDate = new (std::nothrow) char[11];
		if (pszLiteralDate == nullptr)
			return "0000/00/00";
		sprintf_s(pszLiteralDate, 11, "%04X/%02X/%02X\0", (iDate & 0xFFFF0000) >> 16, (iDate & 0xFF00) >> 8, iDate & 0xFF);
		std::string szLiteralDate(pszLiteralDate);
		delete[] pszLiteralDate;
		return szLiteralDate;
	}
	void writeLine(int &iIdentifier, int &iPosition, int &iSize, bool &bCompressed, std::string szLiteralDate) noexcept {
		std::cout
			<< std::setw(12) << iIdentifier
			<< std::setw(4) << "0x" << std::hex << std::setfill('0') << std::setw(8) << iPosition << std::setfill(' ')
			<< std::setw(12) << std::dec << iSize
			<< std::setw(12) << (bCompressed ?  "True" : "False")
			<< std::setw(12) << szLiteralDate
			<< std::endl;
	}
protected:
	struct SFileHeader {
		int iDate;
		int iUncompressedSize;
		int iCompressedSize;
		bool bCompressed;
	};

public:
	bool Open(std::string szFileName) noexcept(false) {
		std::call_once(m_onceFlag, [this]() { this->m_fHandle = nullptr; });
		if (m_fHandle != nullptr)
			Close();

		fopen_s(&m_fHandle, szFileName.c_str(), "rb");
		if (m_fHandle == nullptr)
			return false;
		
		char *pszFileHeader = new (std::nothrow) char[13];
		if (pszFileHeader == nullptr)
			return false;
		fread_s(pszFileHeader, 13, 12, 1, m_fHandle);
		pszFileHeader[12] = '\0';


		//fread_s shouldn't use a 4 as hardcoded _BufferSize as some machines might interpret an int as a 2bytes variable.
		int iFilesCount,
			iDate;
		fread_s(&iDate, 4, 4, 1, m_fHandle);
		fread_s(&iFilesCount, 4, 4, 1, m_fHandle);
		fseek(m_fHandle, 1, SEEK_CUR); 
		
		m_szFileHeader = std::string(pszFileHeader); //Not sure how copy works
		m_szLiteralDate = convertToLiteralDate(iDate);

		m_vecFiles.reserve(iFilesCount);

		for (auto i = 0; i < iFilesCount; i++) {
			int iIdentifier,
				iFileOffset;
			fread_s(&iIdentifier, 4, 4, 1, m_fHandle);
			fread_s(&iFileOffset, 4, 4, 1, m_fHandle);
			m_vecFiles.push_back(std::make_pair(iIdentifier, iFileOffset));
		}

		delete[] pszFileHeader;

		return true;
	}
	void Close() noexcept(false) {
		fclose(m_fHandle);
	}
	void List() noexcept(false) {
		std::cout
			<< std::setw(12) << "Identifier"
			<< std::setw(12) << "Position "
			<< std::setw(12) << "Size "
			<< std::setw(12) << "Compressed "
			<< std::setw(12) << "Date "
			<< std::endl
			<< " +---------+ +---------+ +---------+ +---------+ +---------+"
			<< std::endl;

		for (auto it = std::begin(m_vecFiles); it != std::end(m_vecFiles); it++) {
			SFileHeader fileHeader;

			fseek(m_fHandle, (*it).second, SEEK_SET);
			fread_s(&fileHeader, sizeof(SFileHeader), 13, 1, m_fHandle);

			writeLine((*it).first, (*it).second, fileHeader.iUncompressedSize, fileHeader.bCompressed, convertToLiteralDate(fileHeader.iDate));
		}
	}
	void ExtractAll(std::string szOuputDirectory) noexcept {
		//O(n**2) function, aka, very time expensive to extract all files 
		for (auto it = std::begin(m_vecFiles); it != std::end(m_vecFiles); it++) {
			std::string szActualOut = szOuputDirectory;
			szActualOut.append("/").append(std::to_string((*it).first));
			if (!Extract((*it).first, szActualOut))
				std::cout << __FUNCTION__ << " Unable to extract " << (*it).first << " to " << szActualOut << std::endl;
		}

	}
	bool Extract(int iIdentifier, std::string szOutputFileName) noexcept(false) {
		bool bFound = false;
		for (auto it = std::begin(m_vecFiles); it != std::end(m_vecFiles) && bFound == false; it++) {
			if ((*it).first == iIdentifier) {
				bFound = true;
				fseek(m_fHandle, (*it).second, SEEK_SET);
			}
		}
		if (!bFound)
			return false;

		FILE *fOutHandle = nullptr;
		fopen_s(&fOutHandle, szOutputFileName.c_str(), "wb");
		if (fOutHandle == nullptr)
			return false;

		SFileHeader fileHeader;
		fread_s(&fileHeader, sizeof(SFileHeader), 13, 1, m_fHandle);
		if (fileHeader.bCompressed) {
			std::cout << "Compressed files aren't supported." << std::endl; //It's just ZLib standart compression
			return false;
		}

		std::vector<char> fileBuffer(fileHeader.iUncompressedSize, '\0');
		fread_s(&fileBuffer.front(), fileBuffer.capacity(), fileHeader.iCompressedSize, 1, m_fHandle);
		fwrite(&fileBuffer.front(), fileBuffer.size(), 1, fOutHandle);
		fflush(fOutHandle);
		fclose(fOutHandle);

		std::cout << "Extracted " << iIdentifier << " to " << szOutputFileName << std::endl;

		return true;
	}
};

#endif // !_NOSTALE_3D_OBJECT_PARSER_NOSTALE_FILE