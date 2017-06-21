// Like in the version A,
// I'm assuming the file format is correct and thus that the first integer
// put in each row is always present and not leaping. So I'll use it again,
// this time to do some maths to correctly set the overlapped structure.
// When adding a student to the file, because of the will to keep the file 
// well formed, if the requested id is too high, it will be properly set to the
// correct one to be appended to the end of the file.
// For the locks I've opted not to use any flag while reading (shared lock),
// and using a LOCKFILE_EXCLUSIVE_LOCK while writing.

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define BUF_SIZE 256
#define LEN 256

typedef struct _access {
	TCHAR ip[15];
	TCHAR dude[LEN];
	TCHAR datetime[19];
	TCHAR length[8];
} student;

// righe da modificare: 67, 99, struct

INT _tmain(INT argc, LPTSTR argv[])
{
	INT id;
	HANDLE hIn;
	LARGE_INTEGER filePos;
	DWORD nRead, nWrite;
	student stud;
	TCHAR lpBuffer[BUF_SIZE];
	OVERLAPPED ov = { 0, 0, 0, 0, NULL }; //Internal, InternalHigh, Offset, OffsetHigh, hEvent

	if (argc != 2) {
		fprintf(stderr, "Usage: (%s) database\n", argv[0]);
		return 1;
	}


	hIn = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Cannot open input file. Error: %x\n", GetLastError());
		return 2;
	}

	while (1)
	{
		_tprintf("User choice: |R id| or |W id| or |E|: ");
		_fgetts(lpBuffer, BUF_SIZE + 1, stdin);

		_tprintf("Your input:\n%s\n", lpBuffer);

		id = _ttoi(&lpBuffer[2]);
		switch (lpBuffer[0])
		{
		/*case 'R':
			filePos.QuadPart = id * sizeof(student);
			ov.Offset = filePos.LowPart;
			ov.OffsetHigh = filePos.HighPart;
			LockFileEx(hIn, 0, 0, sizeof(student), 0, &ov);
			ReadFile(hIn, &stud, sizeof(student), &nRead, &ov);
			UnlockFileEx(hIn, 0, sizeof(student), 0, &ov);
			if (nRead && stud.id == id) {
				_tprintf("%s %s %s %s\n", stud.id, stud.register_number, stud.name, stud.surname, stud.mark);
				break;
			}
			if (nRead == 0)
				_tprintf("Your ID %d does not exist.\n", id);
			fflush(stdin); //if we do not put fflush there is an error with the CR
			break; //from case R
			*/
		case 'W':
			filePos.QuadPart = id * sizeof(student);
			ov.Offset = filePos.LowPart;
			ov.OffsetHigh = filePos.HighPart;
			LockFileEx(hIn, 0, 0, sizeof(student), 0, &ov);
			/*if (ReadFile(hIn, &stud, sizeof(student), &nRead, &ov) && stud.id == id) {
				UnlockFileEx(hIn, 0, sizeof(student), 0, &ov);
				_tprintf("%d already existing, you will overwrite the content of the line.\n", stud.id);
			}
			else { 
				do {
					UnlockFileEx(hIn, 0, sizeof(student), 0, &ov);
					filePos.QuadPart = --id * sizeof(student);
					ov.Offset = filePos.LowPart;
					ov.OffsetHigh = filePos.HighPart;
					LockFileEx(hIn, 0, 0, sizeof(student), 0, &ov);
				} while (!(ReadFile(hIn, &stud, sizeof(student), &nRead, &ov) && nRead > 0));
				UnlockFileEx(hIn, 0, sizeof(student), 0, &ov);
				id++;*/
				//setting offsets to 0xFFFFFFFF to append to file
				ov.Offset = 0xFFFFFFFF;
				ov.OffsetHigh = 0xFFFFFFFF;
			//}
			_tprintf("Insert: id register_number name surname mark\n");
			_tscanf("%s %s %s %s", &stud.ip, &stud.dude, &stud.datetime, &stud.length);

			LockFileEx(hIn, LOCKFILE_EXCLUSIVE_LOCK, 0, sizeof(student), 0, &ov);
			if (WriteFile(hIn, &stud, sizeof(student), &nWrite, &ov) && nWrite == sizeof(student))
				_tprintf("Line with id: %s stored correctly.\n", stud.ip);

			UnlockFileEx(hIn, 0, sizeof(student), 0, &ov);
			fflush(stdin);
			break;
		case 'E':
			_tprintf("The program will exit...\n");
			fflush(stdin);
			CloseHandle(hIn);
			system("pause");
			return 0;
		default:
			_tprintf("Command not supported.\n");
			break;
		}
	}
}