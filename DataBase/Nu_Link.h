#ifndef INC__NU_LINK_H__
#define INC__NU_LINK_H__

#include "resource.h"		// main symbols
#include <string>
#include <vector>

void inline GetDllInstallFolder(TCHAR szPath[MAX_PATH])
{
	::GetModuleFileName(NULL, szPath, MAX_PATH);

	TCHAR *pEnd0 = _tcsrchr(szPath, '\\');
	TCHAR *pEnd1 = _tcsrchr(szPath, '/');
	if(pEnd0 < pEnd1)
		pEnd0 = pEnd1;

	if(pEnd0 != NULL)
		*pEnd0 = '\0';
}

template<class T>
inline T my_min(const T &l, const T &r)
{
	return (l < r ? l : r);
}

template<class T>
inline T my_max(const T &l, const T &r)
{
	return (l > r ? l : r);
}

inline unsigned int my_rand()
{
	union
	{
		unsigned char ch[4];
		unsigned int integer;
	} u;
	/* The max value of rand() is RAND_MAX, not full 32 bits value */
	u.ch[0] = (unsigned char)rand();
	u.ch[1] = (unsigned char)rand();
	u.ch[2] = (unsigned char)rand();
	u.ch[3] = (unsigned char)rand();
	return u.integer;
}

inline std::string size_str(unsigned int size)
{
	char buf[128];
	if(size <= 1)
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d byte", size);
	else
	if(size < 1024)
		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d bytes", size);
	else
	if(((size / 1024.) + 0.005) < 1024)
	{
		double f = (size / 1024.) + 0.005;
		unsigned int i = (unsigned int)f;
		unsigned int j = (unsigned int)((f - i) * 100.);

		if(j == 0)
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%dK", i);
		else
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d.%02dK", i, j);
	}
	else
	{
		double f = (size / 1024. / 1024.) + 0.005;
		unsigned int i = (unsigned int)f;
		unsigned int j = (unsigned int)((f - i) * 100.);

		if(j == 0)
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%dM", i);
		else
			_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d.%02dM", i, j);
	}

	return buf;
}

template <class T>
inline T *vector_ptr(std::vector<T> &v)
{
	if(v.size() > 0)
		return &v[0];
	else
		return NULL;
}

template <class T>
inline const T *vector_ptr(const std::vector<T> &v)
{
	if(v.size() > 0)
		return &v[0];
	else
		return NULL;
}

#endif
