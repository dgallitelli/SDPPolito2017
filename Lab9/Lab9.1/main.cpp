#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#define MAX_LEN 256

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <io.h>

DWORD WINAPI ordering_thread(LPVOID);
VOID quickSort(PINT, INT, INT);
INT partition(PINT, INT, INT);
VOID ordered_merge(PINT, PINT);
VOID print_out(LPTSTR);

struct param_struct{
	PTCHAR filename;
	INT counter;
};

PINT* num_point;

int _tmain(int argc, LPTSTR argv[]){
	INT i, tot_num, tot_num_index;
	PHANDLE myThread;
	HANDLE outHandle;
	DWORD  nWrite;
	param_struct* parameters;
	PINT tot_num_array;

	num_point = (PINT*)malloc(sizeof(PINT)*(argc - 2));
	myThread = (PHANDLE)malloc(sizeof(HANDLE)*(argc - 2));
	parameters = (param_struct*)malloc(sizeof(param_struct)*(argc - 2));

	for (i=0; i < argc - 2; i++){
		parameters[i].filename = argv[i+1];
		parameters[i].counter = i;
		//myThread[i] = (HANDLE) _beginthreadex(0, 0, (unsigned(_stdcall*)(void*))ordering_thread, &parameters[i], 0, NULL);
		myThread[i] = (HANDLE) CreateThread(0, 0, ordering_thread, (LPVOID)&parameters[i], 0, NULL);
	}
	
	WaitForMultipleObjects(argc - 2, myThread, TRUE, INFINITE);

	outHandle = CreateFile(argv[argc - 1], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (outHandle == INVALID_HANDLE_VALUE){
		_ftprintf(stderr, _T("Cannot open output file. Error: %x\n"), GetLastError());
		ExitThread(1);
	}

	tot_num = 0;
	for (INT i = 0; i < argc - 2; i++)
		tot_num = num_point[i][0];

	for (INT j = 1; j < argc - 2; j++)
		ordered_merge(num_point[0], num_point[j]);

	WriteFile(outHandle, num_point[0], sizeof(INT)*(tot_num+1), &nWrite, NULL);

	CloseHandle(outHandle);

	print_out(argv[argc - 1]);

	system("pause");
	return 0;


}

DWORD WINAPI ordering_thread(LPVOID param){
	param_struct *temp = (param_struct*)param;
	param_struct parameters;
	parameters.filename = temp->filename;
	parameters.counter = temp->counter;

	HANDLE fileHandle;
	INT n_num;
	DWORD nRead;
	PINT temp_num;

	fileHandle = CreateFile(parameters.filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE){
		_ftprintf(stderr, _T("Cannot open input file. Error: %x\n"), GetLastError());
		ExitThread(1);
	}
	if (!(ReadFile(fileHandle, &n_num, sizeof(int), &nRead, NULL) && nRead > 0)){
		_ftprintf(stderr, _T("Error while reading the file. %x\n"), GetLastError());
		ExitThread(2);
	}
	num_point[parameters.counter] = (PINT)malloc(sizeof(PINT)*(n_num));
	num_point[parameters.counter][0] = n_num;
	temp_num = (PINT)malloc(sizeof(INT)*n_num);

	for (INT i=0; i < n_num; i++){
		if (!(ReadFile(fileHandle, &temp_num[i], sizeof(int), &nRead, NULL) && nRead > 0)){
			_ftprintf(stderr, _T("Error while reading the number in the file. %x\n"), GetLastError());
			ExitThread(3);
		}
	}
	quickSort(temp_num, 0, n_num-1);

	for (INT i=0; i < n_num; i++){
		num_point[parameters.counter][i + 1] = temp_num[i];
	}

	CloseHandle(fileHandle);
	ExitThread(0);
}

VOID quickSort(PINT a, INT l, INT r){

	INT j;

	if (l < r)
	{
		// divide and conquer
		j = partition(a, l, r);
		quickSort(a, l, j - 1);
		quickSort(a, j + 1, r);
	}
}

INT partition(PINT a, INT l, INT r){
	int pivot, i, j, t;
	pivot = a[l];
	i = l; j = r + 1;

	while (1)
	{
		do ++i; while (a[i] <= pivot && i <= r);
		do --j; while (a[j] > pivot);
		if (i >= j) break;
		t = a[i]; a[i] = a[j]; a[j] = t;
	}
	t = a[l]; a[l] = a[j]; a[j] = t;
	return j;
}

VOID ordered_merge(PINT arr1, PINT arr2)
{
	INT n1 = arr1[0];
	INT n2 = arr2[0];
	INT count = 1, count1 = 1, count2 = 1;

	num_point[0][0] = n1 + n2;

	while (count1 < n1 && count < n2)
	{
		if (arr1[count1] < arr2[count2])
		{
			num_point[0][count] = arr1[count1];
			count1++;
		}
		else
		{
			num_point[0][count] = arr2[count2];
			count2++;
		}
		count++;
	}

	if (count1 < n1)
	{
		for (; count1 < n1; count1++)
			num_point[0][count] = arr1[count1];
	}
	else
	{
		for (; count2 < n2; count2++)
			num_point[0][count] = arr2[count2];
	}

}

VOID print_out(LPTSTR filename){
	INT n;
	DWORD nIn;

	HANDLE hIn = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Cannot open input file. Error: %x\n"),
			GetLastError());
		//return 2;
	}

	_tprintf(_T("Dedug Printing 2 (FROM BINARY TO ASCII - backward):\n"));
	while (ReadFile(hIn, &n, sizeof(INT), &nIn, NULL) && nIn > 0) {
		_tprintf(_T("%d "), n);
	}
	_tprintf(_T("\n"));

	CloseHandle(hIn);

#if 0
	_tprintf(_T("Go on? "));
	_tscanf(_T("%d"), &i);
#else
	Sleep(5000);
#endif
}