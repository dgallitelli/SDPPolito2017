#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LEN 256
#define MAXLEN 254
#define QUEUE_LEN 10

#define print(f_, ...) WaitForSingleObject(sStdOut, INFINITE); _tprintf((f_), __VA_ARGS__); ReleaseSemaphore(sStdOut, 1, NULL);
#define TIMEMUL 3
#define MAX_TIMEOUT 3
#define ifmain _tmain
#define INPUT FileI
#define OUTPUT FileO

typedef struct _row {
	INT length;
	LPTSTR string;
} tRow;

typedef struct _buffer {
	tRow queue[QUEUE_LEN];
	HANDLE fullSem;
	HANDLE emptySem;
	HANDLE producerSem;
	HANDLE consumerSem;
	INT pIndex;
	INT cIndex;
} tBuffer;

typedef struct _file {
	HANDLE file;
	HANDLE sem;
	BOOL terminated;
} tFile;

DWORD WINAPI tJobA(LPVOID);
DWORD WINAPI tJobB(LPVOID);
DWORD WINAPI tJobC(LPVOID);
static INT partition(TCHAR*, INT , INT);
static VOID quickSort(TCHAR*, INT, INT);
static FLOAT frand();
static BOOLEAN dequeue(tBuffer*, tRow*);
static void enqueue(tBuffer*, tRow);
static void EBSleep();
static BOOLEAN checkIfThereIsWorkToDo(tBuffer*);

/*
 * for syncing stdout
 */
HANDLE sStdOut;
tBuffer *queuesA, *queuesB;
tFile *lfOutput;
tFile *lfInput;
INT threadsQty;
BOOL stackA = FALSE, stackB = FALSE;


INT ifmain(INT argc, LPTSTR argv[]) {
	INT i, j;
	DWORD *threadsIdA, *threadsIdB, *threadsIdC;

	HANDLE *threadsA, *threadsB, *threadsC;
	TCHAR buf[LEN];
	
	threadsQty = _ttoi(argv[1]);
	print(_T("\nThreads are %d\n"), threadsQty);

	threadsIdA = (DWORD*)malloc((threadsQty) * sizeof(DWORD));
	threadsIdB = (DWORD*)malloc((threadsQty) * sizeof(DWORD));
	threadsIdC = (DWORD*)malloc((threadsQty) * sizeof(DWORD));
	lfInput = (tFile*)malloc(threadsQty * sizeof(tFile));
	queuesA = (tBuffer*)malloc(threadsQty * sizeof(tBuffer));
	queuesB = (tBuffer*)malloc(threadsQty * sizeof(tBuffer));
	lfOutput = (tFile*)malloc(threadsQty * sizeof(tFile));
	threadsA = (HANDLE*)malloc(threadsQty * sizeof(HANDLE));
	threadsB = (HANDLE*)malloc(threadsQty * sizeof(HANDLE));
	threadsC = (HANDLE*)malloc(threadsQty * sizeof(HANDLE));

	sStdOut = CreateSemaphore(NULL, 1, threadsQty, NULL);

	for (i = 0; i < threadsQty; i++) {
		_stprintf(buf, _T("FileI%d"), i + 1);
		print(_T("\nFile input #%d is %s\n"), i, buf);
		lfInput[i].file = CreateFile(buf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		lfInput[i].sem = CreateSemaphore(NULL, 1, 1, NULL);
		lfInput[i].terminated = 0;
		_stprintf(buf, _T("FileO%d"), i + 1);
		print(_T("\nFile output #%d is %s\n"), i, buf);
		lfOutput[i].file = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		lfOutput[i].terminated = 0;
		lfOutput[i].sem = CreateSemaphore(NULL, 1, 1, NULL);
		queuesA[i].emptySem = CreateSemaphore(NULL, threadsQty, threadsQty, NULL);
		queuesB[i].emptySem = CreateSemaphore(NULL, threadsQty, threadsQty, NULL);
		queuesA[i].fullSem = CreateSemaphore(NULL, 0, threadsQty, NULL);
		queuesB[i].fullSem = CreateSemaphore(NULL, 0, threadsQty, NULL);
		queuesA[i].producerSem = CreateSemaphore(NULL, 1, threadsQty, NULL);
		queuesB[i].producerSem = CreateSemaphore(NULL, 1, threadsQty, NULL);
		queuesA[i].consumerSem = CreateSemaphore(NULL, 1, threadsQty, NULL);
		queuesB[i].consumerSem = CreateSemaphore(NULL, 1, threadsQty, NULL);
		queuesA[i].pIndex = 0;
		queuesB[i].pIndex = 0;
		queuesA[i].cIndex = 0;
		queuesB[i].cIndex = 0;
		for (j = 0; j < QUEUE_LEN; j++) {
			queuesA[i].queue[j].length = 0;
			queuesA[i].queue[j].string = NULL;
		}

	}

	for (i = 0; i < threadsQty; i++) {
		threadsA[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tJobA, NULL, 0, threadsIdA + i);
		threadsB[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tJobB, NULL, 0, threadsIdB + i);
		threadsC[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tJobC, NULL, 0, threadsIdC + i);
		print(_T("\nThread A #%d is %d\n"), i, threadsIdA[i]);
		print(_T("\nThread B #%d is %d\n"), i, threadsIdB[i]);
		print(_T("\nThread C #%d is %d\n"), i, threadsIdC[i]);
	}

	WaitForMultipleObjects(threadsQty, threadsA, TRUE, INFINITE);
	print(_T("\nAll A threads finished"));
	stackA = TRUE;
	for (i = 0; i < threadsQty; i++)
		CloseHandle(lfInput[i].file);
	WaitForMultipleObjects(threadsQty, threadsB, TRUE, INFINITE);
	print(_T("\nAll B threads finished"));
	stackB = TRUE;
	WaitForMultipleObjects(threadsQty, threadsC, TRUE, INFINITE);
	for (i = 0; i < threadsQty; i++)
		CloseHandle(lfOutput[i].file);
	free(threadsIdA);
	free(threadsIdB);
	free(threadsIdC);
	free(lfInput);
	free(queuesA);
	free(queuesB);
	free(lfOutput);
	free(threadsA);
	free(threadsB);
	free(threadsC);
	system("pause");
	return 0;
}

DWORD WINAPI tJobA(LPVOID lpParam) {
	tFile *file = NULL;
	INT length;
	DWORD nRead;
	TCHAR *string;
	TCHAR *c;
	TCHAR *output;
	INT i = 0;
	tRow row;
	BOOL terminated = FALSE;
	srand(time(NULL) + GetCurrentThreadId());
	INT fnum, qnum;

	while (1) {
		EBSleep();

		do {
			for (i = 0; i < threadsQty && lfInput[i].terminated; i++);
			if (threadsQty == i) {
				print(_T("\nThread %d: input files are over\n"), GetCurrentThreadId());
				terminated = TRUE;
				break;
			}
			fnum = (INT)(frand() * threadsQty);
			file = &(lfInput[fnum]);

			WaitForSingleObject(file->sem, INFINITE);
			nRead = 0;
			if (file->terminated == FALSE)
				ReadFile(file->file, &length, sizeof(INT), &nRead, NULL);
			if (nRead == 0) {
				file->terminated = TRUE;
				ReleaseSemaphore(file->sem, 1, NULL);
			}
		} while (nRead == 0);
		if (terminated)
			break;
		i = 0;
		string = (TCHAR*)malloc(sizeof(TCHAR) * (length));
		ReadFile(file->file, string, sizeof(TCHAR)*length, &nRead, NULL);
		print(_T("\nThreadA %d-File %d: Read: [%d] %.*s\n"), GetCurrentThreadId(), fnum, length, length, string);

		ReleaseSemaphore(file->sem, 1, NULL);

		output = (TCHAR*)malloc(sizeof(TCHAR) * (length));
		for (c = string; c - string < length; c++)
			if (_istalpha(*c))
				output[i++] = *c;
		row.string = (TCHAR*)malloc(sizeof(TCHAR) * (i));
		for (c = output; c - output < i; c++)
			row.string[c-output] = *c;
		row.length = i;
		free(output);
		free(string);

		EBSleep();
		qnum = (INT)(frand() * threadsQty);

		print(_T("\nThreadA %d-File %d: Output Queue%d: [%d] %.*s\n"), GetCurrentThreadId(), fnum, qnum, i, i, row.string);
		enqueue(&(queuesA[qnum]), row);
	}

	print(_T("\nExiting A thread %d"), GetCurrentThreadId());
	ExitThread(0);
}

DWORD WINAPI tJobB(LPVOID lpParam) {
	TCHAR *c;
	INT i;
	tRow row, rOutput;
	srand(time(NULL) + GetCurrentThreadId());
	INT qnum;

	while (1) {
		if (stackA && checkIfThereIsWorkToDo(queuesA) == FALSE)
			break;
		EBSleep();
		qnum = (INT)(frand() * threadsQty);
		if (dequeue(&(queuesA[qnum]), &row) == FALSE) {
			print(_T("\nQueue %d is empty."), qnum);
			continue;
		}
		print(_T("\nThreadB %d-Queue input %d: Read: [%d] %.*s\n"), GetCurrentThreadId(), qnum, row.length, row.length, row.string);

		//CONSUME
		i = 0;
		rOutput.string = (TCHAR*)malloc(sizeof(TCHAR) * (row.length));
		for (c = row.string; c - row.string < row.length; c++)
			rOutput.string[i++] = _totupper(*c);
		rOutput.length = i;
		EBSleep();
		qnum = (INT)(frand() * threadsQty);
		print(_T("\nThreadB %d: Output queue %d: [%d] %.*s\n"), GetCurrentThreadId(), qnum, i, i, rOutput.string);
		enqueue(&(queuesB[qnum]), rOutput);
	}

	print(_T("\nExiting B thread %d"), GetCurrentThreadId());
	ExitThread(0);
}

DWORD WINAPI tJobC(LPVOID lpParam) {
	DWORD nWrited;
	TCHAR *c;
	TCHAR *output;
	INT i = 0;
	tRow row;
	tFile *file;
	OVERLAPPED ov = { 0, 0, 0, 0, NULL }; //Internal, InternalHigh, Offset, OffsetHigh, hEvent
	srand(time(NULL) + GetCurrentThreadId());
	INT fnum, qnum;

	while (1) {
		if (stackB && checkIfThereIsWorkToDo(queuesB) == FALSE)
			break;
		EBSleep();
		qnum = (INT)(frand() * threadsQty);
		if (dequeue(&(queuesB[qnum]), &row) == FALSE) {
			print(_T("Queue %d is empty."), qnum);
			continue;
		}
		//CONSUME
		i = 0;
		output = (TCHAR*)malloc(sizeof(TCHAR) * (row.length));
		for (c = row.string; c - row.string < row.length; c++)
			output[i++] = *c;
		quickSort(output, 0, row.length);

		EBSleep();
		fnum = (INT)(frand() * threadsQty);
		print(_T("\nThreadC %d: Output file %d: [%d] %.*s\n"), GetCurrentThreadId(), fnum, row.length, row.length, output);
		file = &(lfOutput[fnum]);
		
		WaitForSingleObject(file->sem, INFINITE);
		ov.Offset = 0xFFFFFFFF;
		ov.OffsetHigh = 0xFFFFFFFF;
		WriteFile(file->file, &(row.length), sizeof(INT), NULL, &ov);
		ov.Offset = 0xFFFFFFFF;
		ov.OffsetHigh = 0xFFFFFFFF;
		WriteFile(file->file, output, sizeof(TCHAR)*row.length, NULL, &ov);
		ReleaseSemaphore(file->sem, 1, NULL);
		free(output);
	}

	print(_T("\nExiting C thread %d"), GetCurrentThreadId());
	ExitThread(0);
}

static BOOLEAN dequeue(tBuffer *queue, tRow *row) {
	DWORD wres = WaitForSingleObject(queue->fullSem, 1000*MAX_TIMEOUT);
	if (wres == WAIT_TIMEOUT)
		return FALSE;
	WaitForSingleObject(queue->consumerSem, INFINITE);
	//DEQUEUE
	*row = queue->queue[queue->cIndex++];
	queue->cIndex = queue->cIndex % QUEUE_LEN;

	ReleaseSemaphore(queue->consumerSem, 1, NULL);
	ReleaseSemaphore(queue->emptySem, 1, NULL);
	return TRUE;
}

static void enqueue(tBuffer *queue, tRow row) {
	WaitForSingleObject(queue->emptySem, INFINITE);
	WaitForSingleObject(queue->producerSem, INFINITE);
	//ENQUEUE
	queue->queue[queue->pIndex++] = row;
	queue->pIndex = queue->pIndex % QUEUE_LEN;

	ReleaseSemaphore(queue->producerSem, 1, NULL);
	ReleaseSemaphore(queue->fullSem, 1, NULL);
}

static FLOAT frand() {
	return ((FLOAT)rand() - 1) / (FLOAT)(RAND_MAX);
}

static VOID quickSort(TCHAR *a, INT l, INT r) {
	INT j;

	if (l < r){
		// divide and conquer
		j = partition(a, l, r);
		quickSort(a, l, j - 1);
		quickSort(a, j + 1, r);
	}
}

static INT partition(TCHAR *a, INT l, INT r) {
	INT pivot, i, j, t;
	pivot = a[l];
	i = l; j = r + 1;

	while (1) {
		do ++i; while (a[i] <= pivot && i <= r);
		do --j; while (a[j] > pivot);
		if (i >= j) break;
		t = a[i]; a[i] = a[j]; a[j] = t;
	}
	t = a[l]; a[l] = a[j]; a[j] = t;
	return j;
}

static void EBSleep() {
	INT wtime;
	wtime = ((INT)(frand() * TIMEMUL)) * 1000;
	print(_T("\nThread %d will wait %d s\n"), GetCurrentThreadId(), wtime/1000);
	Sleep(wtime);
}

static BOOLEAN checkIfThereIsWorkToDo(tBuffer *queues) {
	INT i;
	for (i = 0; i < threadsQty; i++) {
		/*if (WaitForSingleObject(queues[i].fullSem, 1) != WAIT_TIMEOUT) {
			ReleaseSemaphore(queues[i].fullSem, 1, NULL);
			return TRUE;
		}*/
		if (queues[i].cIndex != queues[i].pIndex)
			return TRUE;
	}
	return FALSE;
}