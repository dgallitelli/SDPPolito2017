// Like in the version A,
// I'm assuming the file format is correct and thus that the first integer
// put in each row is always present and not leaping. So I'll use it again,
// this time to do some maths to correctly set the overlapped structure.
// When adding a student to the file, because of the will to keep the file 
// well formed, if the requested id is too high, it will be properly set to the
// correct one to be appended to the end of the file.
// For the locks I've opted not to use any flag while reading (shared lock),
// and using a LOCKFILE_EXCLUSIVE_LOCK while writing.

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <cstdlib>

#define BUF_SIZE 54
#define LEN 256
#define ifmain _tmain

typedef struct _access {
	TCHAR ip[16];
	TCHAR dude[LEN+1];
	TCHAR datetime[19+1];
	TCHAR length[8+1];
} student;

// righe da modificare: 67, 99, struct

INT ifmain(INT argc, LPTSTR argv[])
{
	INT id;
	HANDLE hIn, src, hOut;
	LARGE_INTEGER filePos;
	DWORD nWrite, nRead;
	student stud;
	TCHAR lpBuffer[BUF_SIZE +1];
	OVERLAPPED ov1 = { 0,0,0,0, NULL }, ov = { 0, 0, 0, 0, NULL };
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	TCHAR outName[256];
	TCHAR oversize[BUF_SIZE];
	char clpBuffer[BUF_SIZE + 1];

	filePos.QuadPart = 0;

	if (argc != 2) {
		_ftprintf(stderr, _T("Usage: (%s) database\n"), argv[0]);
		return 1;
	}

	SetCurrentDirectory(argv[1]);
	SearchHandle = FindFirstFile(_T("*.txt"), &FindData);
	do {
		filePos.QuadPart = 0;
		ov.Offset = filePos.LowPart;
		ov.OffsetHigh = filePos.HighPart;
		_tprintf(_T("%s"), FindData.cFileName);
		if (_tcscmp(_T("."), FindData.cFileName) == 0 || _tcscmp(_T(".."), FindData.cFileName) == 0)
			continue;
		src = CreateFile(FindData.cFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		_stprintf(outName, _T("%s.bin"), FindData.cFileName);
		_tprintf(_T("\nCreated file: %s\n\n"), outName);
		hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		while (ReadFile(src, clpBuffer, BUF_SIZE -1, &nRead, &ov) && nRead > 0) {
			mbstowcs(lpBuffer, clpBuffer, BUF_SIZE +1);
			_stscanf(lpBuffer, _T("%s\t%s\t%s\t%s\r\n%s"), stud.ip, stud.dude, stud.datetime, stud.length, oversize);
			filePos.QuadPart += _tcslen(lpBuffer) - _tcslen(oversize);
			ov.Offset = filePos.LowPart;
			ov.OffsetHigh = filePos.HighPart;
			_tprintf(_T("Stud data:\n%s\t%s\t%s\t\t%s\n"), stud.ip, stud.dude, stud.datetime, stud.length);
			ov1.Offset = 0xFFFFFFFF;
			ov1.OffsetHigh = 0xFFFFFFFF;
			if (WriteFile(hOut, &stud, sizeof(student), &nWrite, &ov1) && nWrite == sizeof(student))
				_tprintf(_T("Line with id: %s stored correctly.\n"), stud.ip);
			else
				_tprintf(_T("Error storing line"));
		}
		CloseHandle(hOut);
		CloseHandle(src);
	} while (FindNextFile(SearchHandle, &FindData));
	FindClose(SearchHandle);
	system("pause");
	return 1;
}