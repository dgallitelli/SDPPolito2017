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

// righe da modificare: 67, 99, struct

INT ifmain(INT argc, LPTSTR argv[])
{
	INT id;
	HANDLE hIn, src, hOut;
	LARGE_INTEGER filePos;
	DWORD nWrite, nRead;
	TCHAR stud[LEN];
	TCHAR lpBuffer[BUF_SIZE + 1];
	OVERLAPPED ov1 = { 0,0,0,0, NULL }, ov = { 0, 0, 0, 0, NULL };
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	TCHAR outName[256];
	TCHAR oversize[BUF_SIZE];
	char clpBuffer[BUF_SIZE + 1];
	INT qty;

	filePos.QuadPart = 0;

	src = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	_stprintf(outName, _T("%s.bin"), argv[1]);
	hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	qty = _ttoi(argv[2]);
	WriteFile(hOut, &qty, sizeof(INT), &nWrite, NULL);

	filePos.QuadPart = 0;
	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;
	_tprintf(_T("\nCreated file: %s\n\n"), outName);
	while (ReadFile(src, clpBuffer, BUF_SIZE - 1, &nRead, &ov) && nRead > 0) {
		mbstowcs(lpBuffer, clpBuffer, BUF_SIZE + 1);
		_stscanf(lpBuffer, _T("%s\r\n%s"), stud, oversize);
		filePos.QuadPart += _tcslen(stud) + _tcslen(_T("\r\n"));
		ov.Offset = filePos.LowPart;
		ov.OffsetHigh = filePos.HighPart;
		_tprintf(_T("Stud data:\n%s\n"), stud);
		ov1.Offset = 0xFFFFFFFF;
		ov1.OffsetHigh = 0xFFFFFFFF;
		if (WriteFile(hOut, &stud, LEN * sizeof(TCHAR), &nWrite, &ov1) && nWrite == LEN * sizeof(TCHAR))
			_tprintf(_T("Line with id: %s stored correctly.\n"), stud);
		else
			_tprintf(_T("Error storing line"));
	}
	CloseHandle(hOut);
	CloseHandle(src);
	system("pause");
	return 1;
}