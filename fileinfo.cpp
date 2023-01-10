#include "StdAfx.h"
#include "fileinfo.h"
#include <sys/stat.h>

BOOL IsFolder2(const CString &path)
{
    WIN32_FIND_DATA fd;
    BOOL ret = FALSE;
    HANDLE hFind = FindFirstFile(path, &fd);

    if ((hFind != INVALID_HANDLE_VALUE) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        ret = TRUE;
    }

    FindClose(hFind);
    return ret;
}

/**
 * Get the size of a file.
 * @param filename The name of the file to check size for
 * @return The filesize, or 0 if the file does not exist.
 */
size_t getFilesize(const std::tstring &filename)
{
    struct _stat st;

    if (_tstat(filename.c_str(), &st) != 0) {
        return 0;
    }

    return st.st_size;
}


#include <fstream>
#include <vector>
#if 0
bool UpdateFileInfo(CString strFN, struct fileinfo *sfinfo)
{
    std::ifstream       file(strFN);

    if (file) {
        /*
         * Get the size of the file
         */
        file.seekg(0, std::ios::end);
        std::streampos          length = file.tellg();
        file.seekg(0, std::ios::beg);
        /*
         * Use a vector as the buffer.
         * It is exception safe and will be tidied up correctly.
         * This constructor creates a buffer of the correct length.
         * Because char is a POD data type it is not initialized.
         *
         * Then read the whole file into the buffer.
         */
        //std::vector<char>       buffer(length);
        //file.read(&buffer[0],length);
        sfinfo->filename = strFN.GetBuffer(0);
        sfinfo->st_size = length;
        sfinfo->vbuf.resize(length);
        file.read(&(sfinfo->vbuf[0]), length);

        if (file) {
            printf("all characters read successfully.\n");
        } else {
            printf("error: only %i could be read\n", file.gcount());
        }

        unsigned short cks = 0;

        for (size_t i = 0; i < length; i++) {
            cks += sfinfo->vbuf[i];
        }

        sfinfo->usCheckSum = cks;
        return true;
    }

    return false;
}
#else
#include <string>
#include <cstdio>
#include <cerrno>

bool UpdateFileInfo(CString strFN, struct fileinfo *sfinfo)
{
    if (IsFolder2(strFN)) {
        printf("THis is a folder\n");
        return false;
    }

    std::FILE *fp = _tfopen(strFN.GetBuffer(0), _T("rb"));

    if (fp) {
        std::fseek(fp, 0, SEEK_END);
        size_t length = std::ftell(fp);
        std::rewind(fp);
        sfinfo->filename = strFN;
        sfinfo->st_size = length;
        sfinfo->vbuf.resize(length);
        size_t result = 0;
        unsigned short cks = 0;

        if (length) {
            result = std::fread(&(sfinfo->vbuf[0]), 1, length, fp);
        }

        for (size_t i = 0; i < length; i++) {
            cks += sfinfo->vbuf[i];
        }

        sfinfo->usCheckSum = cks;
        //if(result == length)
        //	printf("all characters read successfully. (%d, %X)\n", length, cks);
        //else
        //	printf("error: only %i could be read\n", result);
        std::fclose(fp);
        return (result == length);
    }

    return false;
}

static unsigned char Ascii2Hex(char c)
{
	unsigned char ucVal;

	if ((c >= '0') && (c <= '9'))
	{
		ucVal = c - '0';
	}
	else if ((c >= 'a') && (c <= 'f'))
	{
		ucVal = c - 'a' + 10;
	}
	else if ((c >= 'A') && (c <= 'F'))
	{
		ucVal = c - 'A' + 10;
	}
	else
		ucVal = 0xFF;

	return ucVal;
}

bool UpdateFileInfo3(CString strFN, std::string str, std::vector<unsigned char> &vecROTPK)
{
	if (IsFolder2(strFN))
	{
		printf("This is a folder\n");
		return false;
	}

	FILE *file;
	file = _tfopen(strFN, _T("r"));
	if (file == NULL)
		return false;

	CString ext = strFN.Mid(strFN.ReverseFind('.') + 1);

	unsigned int uSize;

	if (!str.empty())
	{
		uSize = str.size();
	}
	else
	{
		fseek(file, 0, SEEK_END);
		uSize = ftell(file);
		fseek(file, SEEK_SET, 0);
	}

	std::vector<unsigned char> ROTPK;
	ROTPK.reserve(uSize);

	bool bError = false;

	if (ext == "pem")
	{
		char ch, ch0, ch1;
		bool bConvert = false;
		unsigned int i, uIndex = 0;

		for (i = 0; i < uSize; i++)
		{
			ch = str[i];

			if (ch == '{')
				bConvert = true;

			if (ch == '}')
				bConvert = false;

			if (bConvert)
			{
				switch(uIndex)
				{
				case 0:
					if (ch == '0')
						uIndex++;
					break;
				case 1:
					if ((ch == 'x') || (ch == 'X'))
						uIndex++;
					else
					{
						uIndex = 0;
						bConvert = false;
					}
					break;
				case 2:
					ch1 = Ascii2Hex(ch);

					if ((ch1 >= 0x0) && (ch1 <= 0xF))
						uIndex++;
					else
					{
						uIndex = 0;
						bConvert = false;
					}
					break;
				case 3:
					ch0 = Ascii2Hex(ch);

					if ((ch0 >= 0x0) && (ch0 <= 0xF))
						ROTPK.push_back((ch1 << 4) | ch0);
					else
						bConvert = false;

					uIndex = 0;
					break;
				default:
					uIndex = 0;
					bConvert = false;
				}
			}
		}

		if (ROTPK.size() == 91)
		{
			ROTPK.erase(ROTPK.begin(), ROTPK.begin() + (ROTPK.size() - 64));
		}
		else
			bError = true;
	}
	else if (ext == "bin")
	{
		if (uSize != 64)
		{
			bError = true;
		}
		else
		{
			ROTPK.resize(uSize);
			fread(&ROTPK[0], uSize, 1, file);
		}
	}
	else if (ext == "c")
	{
		char ch, ch0, ch1;
		bool bConvert = false;
		unsigned int uIndex = 0;

		while (!feof(file))
		{
			ch = fgetc(file);

			if (ch == '{')
				bConvert = true;

			if (ch == '}')
				bConvert = false;

			if (bConvert)
			{
				switch(uIndex)
				{
				case 0:
					if (ch == '0')
						uIndex++;
					break;
				case 1:
					if ((ch == 'x') || (ch == 'X'))
						uIndex++;
					else
					{
						uIndex = 0;
						bConvert = false;
					}
					break;
				case 2:
					ch1 = Ascii2Hex(ch);

					if ((ch1 >= 0x0) && (ch1 <= 0xF))
						uIndex++;
					else
					{
						uIndex = 0;
						bConvert = false;
					}
					break;
				case 3:
					ch0 = Ascii2Hex(ch);

					if ((ch0 >= 0x0) && (ch0 <= 0xF))
						ROTPK.push_back((ch1 << 4) | ch0);
					else
						bConvert = false;

					uIndex = 0;
					break;
				default:
					uIndex = 0;
					bConvert = false;
				}
			}
		}

		if ((ROTPK.size() != 64) && (ROTPK.size() != 91))
		{
			bError = true;
		}

		if (ROTPK.size() == 91)
			ROTPK.erase(ROTPK.begin(), ROTPK.begin() + (ROTPK.size() - 64));
	}
	else if (ext == "txt")
	{
		char line[100];
		unsigned int i, uLen, uCount = 0;

		while (fgets(line, sizeof(line), file))
		{
			if (uCount >= 2)
			{
				bError = true;
				break;
			}

			uLen = strlen(line);

			for (i = 0; i < uLen; i++)
			{
				if ((line[i] == '\n') || (line[i] == '\r'))
					line[i] = '\0';
			}

			uLen = strlen(line);

			if (uLen != 64)
			{
				bError = true;
				break;
			}

			char ch0, ch1;

			for (i = 0; i < 64; i+=2)
			{
				ch1 = Ascii2Hex(line[i + 0]);
				ch0 = Ascii2Hex(line[i + 1]);

				if (!((ch1 >= 0x0) && (ch1 <= 0xF)))
				{
					bError = true;
					break;
				}

				if (!((ch0 >= 0x0) && (ch0 <= 0xF)))
				{
					bError = true;
					break;
				}

				ROTPK.push_back((ch1 << 4) | ch0);
			}

			if (bError)
				break;

			uCount++;
		}
	}
	else
		bError = true;

	fclose(file);

	if (bError)
		return false;

	if (ROTPK.size() == 64)
	{
		vecROTPK.resize(64);

		unsigned int i, j;

		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 32; j++)
			{
				vecROTPK[(i * 32) + j] = ROTPK[((i + 1) * 32) - j - 1];
			}
		}

		return true;
	}

	return false;
}
#endif

size_t mfwrite(const void *ptr, size_t len, _TCHAR *_FileName)
{
    size_t ret = 0;
    std::FILE *fp = _tfopen(_FileName, _T("wb"));
    ret = std::fwrite(ptr, 1, len, fp);
    std::fclose(fp);
    return ret;
}

size_t mfread(void *ptr, size_t len, _TCHAR *_FileName)
{
    size_t ret = 0;
    std::FILE *fp = _tfopen(_FileName, _T("rb"));
    ret = std::fread(ptr, 1, len, fp);
    std::fclose(fp);
    return ret;
}