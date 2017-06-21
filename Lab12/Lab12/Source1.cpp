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

#define BUF_SIZE 51
#define LEN 256
#define ifmain _tmain

typedef struct _access {
	TCHAR ip[15];
	TCHAR dude[LEN];
	TCHAR datetime[19];
	TCHAR length[8];
} student;

// righe da modificare: 67, 99, struct

INT ifmain(INT argc, LPTSTR argv[])
{
	INT id;
	HANDLE hIn, src, hOut;
	LARGE_INTEGER filePos;
	DWORD nWrite, nRead;
	student stud;
	TCHAR lpBuffer[BUF_SIZE];
	OVERLAPPED ov = { 0, 0, 0, 0, NULL }; //Internal, InternalHigh, Offset, OffsetHigh, hEvent
	OVERLAPPED ov1 = { 0,0,0,0, NULL };
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	TCHAR outName[256];

	if (argc != 2) {
		_ftprintf(stderr, _T("Usage: (%s) database\n"), argv[0]);
		return 1;
	}

	while (1)
	{
		_tprintf(_T("User choice: |R id| or |W id| or |E|: "));
		_fgetts(lpBuffer, BUF_SIZE + 1, stdin);

		_tprintf(_T("Your input:\n%s\n"), lpBuffer);

		id = _ttoi(&lpBuffer[2]);
		switch (lpBuffer[0])
		{
		case 'R':
			SearchHandle = FindFirstFile(argv[1], &FindData);
			do {
				src = CreateFile(FindData.cFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				_tprintf(_T("%s"), FindData.cFileName);
				_stprintf(outName, _T("%s.bin"), FindData.cFileName);
				_tprintf(_T("%s"), outName);
				hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				while (ReadFile(src, lpBuffer, sizeof(lpBuffer), &nRead, &ov) && nRead > 0) {
					_stscanf(lpBuffer, _T("%s\t%s\t%s\t%s"), &stud.ip, &stud.dude, &stud.datetime, &stud.length);
					_tprintf(_T("%s %s %s %s\n"), stud.ip, stud.dude, stud.datetime, stud.length);
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
			break; //from case R
		case 'E':
			_tprintf(_T("The program will exit...\n"));
			fflush(stdin);
			system("pause");
			return 0;
		default:
			_tprintf(_T("Command not supported.\n"));
			break;
		}
	}
}