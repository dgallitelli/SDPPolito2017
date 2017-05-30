#include <windows.h>
#include <tchar.h>
#include <stdio.h>

struct student{
	INT id;
	DWORD register_number;
	CHAR name[30];
	CHAR surname[30];
	INT mark;
};

INT _tmain(INT argc, LPTSTR argv[])
{
	CHAR cmd[3];
	INT id, i;
	HANDLE hIn;
	DWORD wr_bytes, nr_bytes;
	student stud;

	if (argc != 2) {
		fprintf(stderr, "Usage: (%s) database\n", argv[0]);
		return 1;
	}

	/*if ((stream = _tfopen(argv[1], "r")) == NULL){
		fprintf(stderr, "Cannot open output file. Error: %x\n", GetLastError());
		return 2;
		}*/


	hIn = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Cannot open input file. Error: %x\n", GetLastError());
		return 2;
	}

	while (1)
	{
		_tprintf("User choice: |R id| or |W id| or |E|: ");
		_fgetts(cmd, sizeof(cmd)+1, stdin);
		//_tscanf("%c", &cmd);
		

		_tprintf("Hai inserito %s\n", cmd);

		id = atoi(&cmd[2]);
		switch (cmd[0])
		{
		case 'R':
			//_tscanf(" %d", &id);
			SetFilePointer(hIn, NULL, NULL, FILE_BEGIN);
			while (ReadFile(hIn, &stud, sizeof(student), &nr_bytes, NULL) && nr_bytes > 0){
				if (stud.id == id){
					_tprintf("%d %ld %s %s %d\n", stud.id, stud.register_number, stud.name, stud.surname, stud.mark);
					break;
				}
				//SetFilePointer(hIn, sizeof(student), NULL, FILE_CURRENT);
			}
			if (nr_bytes == 0)
				_tprintf("Your ID %d does not exist.\n", id);
			fflush(stdin); //if we do not put fflush there is an error with the CR
			break; //from case R

		case 'W':
			//_tscanf(" %d", &id);
			SetFilePointer(hIn, NULL, NULL, FILE_BEGIN);
			i = 0;
			while (ReadFile(hIn, &stud, sizeof(student), &nr_bytes, NULL) && nr_bytes > 0){
				if (stud.id == id){
					_tprintf("%d already existing, you will overwrite the content of the line", stud.id);
					SetFilePointer(hIn, i*sizeof(student), NULL, FILE_BEGIN);
					break;
				}
				else i++;
				// SetFilePointer(hIn, NULL, NULL, FILE_END);
			}
			_tprintf("Insert: id register_number name surname mark\n");
			_tscanf("%d %ld %s %s %d", &stud.id, &stud.register_number, &stud.name, &stud.surname, &stud.mark);

			if (WriteFile(hIn, &stud, sizeof(student), &wr_bytes, NULL) && wr_bytes == sizeof(student))
				_tprintf("Line with id: %d stored correctly.\n", stud.id);
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