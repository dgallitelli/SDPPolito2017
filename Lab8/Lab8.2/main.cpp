#include <windows.h>
#include <tchar.h>
#include <stdio.h>

struct student{
	INT id;
	DWORD register_number;
	CHAR surname[30];
	CHAR name[30];
	INT mark;
};

INT _tmain(INT argc, LPTSTR argv[])
{
	FILE *stream;
	HANDLE hOut, hTest;
	DWORD wr_bytes, nr_bytes;
	student stud;

	if (argc != 3) {
		fprintf(stderr, "Usage: (%s) file1 file2\n", argv[0]);
		return 1;
	}

	if ((stream = _tfopen(argv[1], "r")) == NULL){
		fprintf(stderr, "Cannot open output file. Error: %x\n", GetLastError());
		return 2;
	}


	hOut = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Cannot open output file. Error: %x\n", GetLastError());
		fclose(stream);
		return 3;
	}

	while (1)
	{
		if (_ftscanf(stream, "%d %ld %s %s %d", &stud.id, &stud.register_number, &stud.name, &stud.surname, &stud.mark) != 5)
			break;

	WriteFile(hOut, &stud, sizeof(student), &wr_bytes, NULL);
	}

	CloseHandle(hOut);

	hTest = CreateFile(argv[2], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hTest == INVALID_HANDLE_VALUE) {
		fclose(stream);
		fprintf(stderr, "Cannot open output file. Error: %x\n", GetLastError());
		return 4;
	}

	while (ReadFile(hTest, &stud, sizeof(student), &nr_bytes, NULL) && nr_bytes > 0)
	{
		_tprintf("%d %ld %s %s %d\n", stud.id, stud.register_number, stud.name, stud.surname, stud.mark);
	}
	
	CloseHandle(hTest);
	system("pause");
	return 0;
}