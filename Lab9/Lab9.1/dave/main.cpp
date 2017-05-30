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
#include <search.h>

DWORD WINAPI ordering_thread(LPVOID);
VOID ordered_merge(PINT, PINT);
VOID print_out(PTCHAR);
int compare_to_sort(const void*, const void*);

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
	PINT final_array;

	num_point = (PINT*)malloc(sizeof(PINT)*(argc - 2));
	myThread = (PHANDLE)malloc(sizeof(HANDLE)*(argc - 2));
	parameters = (param_struct*)malloc(sizeof(param_struct)*(argc - 2));

	for (i=0; i < argc - 2; i++){
		parameters[i].filename = argv[i+1];
		parameters[i].counter = i;
		myThread[i] = (HANDLE) _beginthreadex(0, 0, (unsigned(_stdcall*)(void*))ordering_thread, &parameters[i], 0, NULL);
		// myThread[i] = (HANDLE) CreateThread(0, 0, ordering_thread, (LPVOID)&parameters[i], 0, NULL);
	}
	
	WaitForMultipleObjects(argc - 2, myThread, TRUE, INFINITE);

	outHandle = CreateFile(argv[argc - 1], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (outHandle == INVALID_HANDLE_VALUE){
		_ftprintf(stderr, _T("Cannot open output file. Error: %x\n"), GetLastError());
		ExitThread(1);
	}

	tot_num = 0;
	for (INT i = 0; i < argc - 2; i++)
		tot_num += num_point[i][0];

	_tprintf(_T("\nThe final dimension will be: %d \n"), tot_num);
	
	final_array = (PINT) malloc(sizeof(INT)*tot_num);
	final_array[0] = tot_num;
	for (INT i = 1; i < tot_num; i++)
		final_array[i] = -1;

	for (INT j = 0; j < argc - 2; j++){
		_tprintf(_T("Merging array %d - with size %d\n"),j, num_point[j][0]); 
		ordered_merge(final_array, num_point[j]);
		_tprintf(_T("Final Array: \n")); 
		for (INT i = 0; i < tot_num; i++)
			_tprintf(_T("%d "),final_array[i]); 
		_tprintf(_T("\n")); 
	}

	WriteFile(outHandle, final_array, sizeof(INT)*(tot_num+1), &nWrite, NULL);

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
	// quickSort(temp_num, 0, n_num-1);
	qsort(temp_num, n_num, sizeof(INT), compare_to_sort);

	for (INT i=0; i < n_num; i++){
		num_point[parameters.counter][i + 1] = temp_num[i];
	}

	CloseHandle(fileHandle);

	/*_tprintf(_T("Printing my array - %d : \n"), parameters.counter);
	for (INT i = 0; i < n_num; i++){
		_tprintf(_T("%d "), num_point[parameters.counter][i + 1]);
	}*/

	ExitThread(0);
}

VOID ordered_merge(PINT arr_dest, PINT my_arr){
	if (arr_dest[1] == -1){
		// Case of merging with first array
		for (INT i = 1; i <= my_arr[0]; i++)
			arr_dest[i] = my_arr[i];
	} else {
		// Merging from the second array
		
		INT arr_dest_size = arr_dest[0];
		INT my_arr_size = my_arr[0];

		// Temp final array for merging safely
		PINT arr_dest_temp;
		arr_dest_temp = (PINT) malloc(sizeof(INT)*arr_dest_size);
		for (INT i = 0; i <= arr_dest_size; i++)
			arr_dest_temp[i] = arr_dest[i];

		// Declaring counters
		int arr_dest_count, arr_dest_temp_count, my_arr_count;
		arr_dest_count = 1; arr_dest_temp_count = 1; my_arr_count = 1;

		// Merge until one of the two arrays runs out of elements
		while (my_arr_count != my_arr_size+1){
			_tprintf(_T("my_arr[%d] = %d | arr_dest_temp[%d] = %d \n"), my_arr_count, my_arr[my_arr_count], arr_dest_temp_count, arr_dest_temp[arr_dest_temp_count]);
			if (my_arr[my_arr_count] < arr_dest_temp[ arr_dest_temp_count ] || arr_dest_temp[arr_dest_temp_count] == -1){
				arr_dest[arr_dest_count] = my_arr[my_arr_count];
				my_arr_count++;
			} else {
				arr_dest[arr_dest_count] = arr_dest_temp[arr_dest_temp_count];
				arr_dest_temp_count++;
			}
			arr_dest_count++;
		}

		// Check which one has not run out of elements, and parse them in the final array
		if (my_arr_count != my_arr_size+1){
			_tprintf(_T("Parsing directly elements from my_arr. \n"));
			// Elements left in my_arr, parse them in arr_dest
			for (INT i = my_arr_count; i < my_arr_size+1; i++){
				arr_dest[arr_dest_count] = my_arr[i];
				arr_dest_count++;
			}
		} else {
			_tprintf(_T("Parsing directly elements from arr_dest_temp. \n"));
			// There are elements left in arr_dest_temp
			for (INT i = arr_dest_temp_count; i <= arr_dest_size+1; i++){
				arr_dest[arr_dest_count] = arr_dest_temp[i];
				arr_dest_count++;
			}
		}

		// Here, arr_dest contains merged and ordered elements.
	}
}

VOID print_out(PTCHAR filename){
	INT n;
	DWORD nIn;

	HANDLE hIn = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Cannot open input file. Error: %x\n"),
			GetLastError());
		//return 2;
	}

	_tprintf(_T("Printing the final ASCII file:\n"));
	while (ReadFile(hIn, &n, sizeof(INT), &nIn, NULL) && nIn > 0) {
		_tprintf(_T("%d "), n);
	}
	_tprintf(_T("\n"));

	CloseHandle(hIn);

}

int compare_to_sort(const void* num1, const void* num2){
	return ( *(int*)num1 - *(int*)num2 );
}