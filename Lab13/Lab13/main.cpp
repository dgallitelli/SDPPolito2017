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
#define TIMEMUL 10
#define ifmain _tmain
#define INPUT FileI
#define OUTPUT FileO

typedef struct _row {
	INT length;
	LPTSTR string;
} tRow;

typedef struct _buffer {
	tRow queue[LEN];
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
static INT partition(TCHAR *a, INT l, INT r);
static VOID quickSort(TCHAR *a, INT l, INT r);
static FLOAT frand();
static tRow *dequeue(tBuffer);
static void enqueue(tBuffer, tRow );

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
	INT i;
	DWORD *threadsIdA, *threadsIdB, *threadsIdC;

	HANDLE *threadsA, *threadsB, *threadsC;
	TCHAR buf[LEN];
	
	threadsQty = _ttoi(argv[2]);

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
		_stprintf(buf, _T("INPUT%d"), i + 1);
		lfInput[i].file = CreateFile(buf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		lfInput[i].sem = CreateSemaphore(NULL, 1, 1, NULL);
		_stprintf(buf, _T("OUTPUT%d"), i + 1);
		lfOutput[i].file = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		lfOutput[i].sem = CreateSemaphore(NULL, 1, 1, NULL);
		queuesA[i].emptySem = CreateSemaphore(NULL, threadsQty, threadsQty, NULL);
		queuesB[i].emptySem = CreateSemaphore(NULL, threadsQty, threadsQty, NULL);
		queuesA[i].fullSem = CreateSemaphore(NULL, 0, threadsQty, NULL);
		queuesB[i].fullSem = CreateSemaphore(NULL, 0, threadsQty, NULL);
		queuesA[i].producerSem = CreateSemaphore(NULL, 1, threadsQty, NULL);
		queuesB[i].producerSem = CreateSemaphore(NULL, 1, threadsQty, NULL);
		queuesA[i].consumerSem = CreateSemaphore(NULL, 1, threadsQty, NULL);
		queuesB[i].consumerSem = CreateSemaphore(NULL, 1, threadsQty, NULL);
	}

	for (i = 0; i < threadsQty; i++) {
		threadsA[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tJobA, NULL, 0, threadsIdA + i);
		threadsB[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tJobB, NULL, 0, threadsIdB + i);
		threadsC[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tJobC, NULL, 0, threadsIdC + i);
	}

	WaitForMultipleObjects(threadsQty, threadsA, TRUE, INFINITE);
	stackA = TRUE;
	WaitForMultipleObjects(threadsQty, threadsB, TRUE, INFINITE);
	stackB = TRUE;
	WaitForMultipleObjects(threadsQty, threadsC, TRUE, INFINITE);

	return 0;
}

DWORD WINAPI tJobA(LPVOID lpParam) {
	tFile file;
	INT length;
	DWORD nRead;
	TCHAR *string;
	TCHAR *c;
	TCHAR *output;
	INT i = 0;
	tRow row;
	BOOL terminated;

	while (1) {
		srand(time(NULL) + GetCurrentThreadId());
		Sleep(((INT)frand() * TIMEMUL) * 1000);

		do {
			for (i = 0; i < threadsQty; i++)
				if (!lfInput[i].terminated)
					break;
			if (threadsQty == i) {
				terminated = TRUE;
				break;
			}

			file = lfInput[(INT)(frand() * threadsQty)];
			if (file.terminated == TRUE)
				continue;

			WaitForSingleObject(file.sem, INFINITE);
			ReadFile(file.file, &length, sizeof(INT), &nRead, NULL);
			if (nRead == 0) {
				file.terminated = TRUE;
				ReleaseSemaphore(file.sem, 1, NULL);
			}
		} while (nRead == 0);
		string = (TCHAR*)malloc(sizeof(TCHAR) * (length));
		ReadFile(file.file, string, sizeof(TCHAR)*length, &nRead, NULL);
		ReleaseSemaphore(file.sem, 1, NULL);

		output = (TCHAR*)malloc(sizeof(TCHAR) * (length + 1));
		for (c = string; c - string < length; c++) {
			if (*c > 'A' && *c < 'Z' || *c > 'a' && *c < 'z')
				output[i++] = *c;
		}
		output[i] = '\0';
		row.length = length;
		row.string = output;

		Sleep(((INT)frand() * TIMEMUL) * 1000);

		enqueue(queuesA[(INT)(frand() * threadsQty)], row);
	}

	ExitThread(0);
}

DWORD WINAPI tJobB(LPVOID lpParam) {
	TCHAR *c;
	TCHAR *output;
	INT i;
	tRow *row;

	while (1) {
		if (stackA) {
			for (i = 0; i < threadsQty; i++) {
				if (queuesA[i].cIndex != queuesA[i].pIndex)
					break;
			}
			if (i == threadsQty)
				break;
		}
		srand(time(NULL) + GetCurrentThreadId());
		Sleep(((INT)frand() * TIMEMUL) * 1000);

		row = dequeue(queuesA[(INT)(frand() * threadsQty)]);
		if (row == NULL)
			continue;

		//CONSUME
		output = (TCHAR*)malloc(sizeof(TCHAR) * (row->length + 1));
		for (c = row->string; c - row->string < row->length; c++) {
			if (*c > 'a' && *c < 'z')
				output[i++] = *c + 'a' - 'A';
			else
				output[i++] = *c;
		}
		output[i] = '\0';

		enqueue(queuesB[(INT)(frand() * threadsQty)], *row);
	}

	ExitThread(0);
}

DWORD WINAPI tJobC(LPVOID lpParam) {
	INT length;
	DWORD nWrited;
	TCHAR *c;
	TCHAR *output;
	INT i = 0;
	tRow *row;
	tFile file;
	OVERLAPPED ov = { 0, 0, 0, 0, NULL }; //Internal, InternalHigh, Offset, OffsetHigh, hEvent

	while (1) {
		if (stackB) {
			for (i = 0; i < threadsQty; i++) {
				if (queuesB[i].cIndex != queuesB[i].pIndex)
					break;
			}
			if (i == threadsQty)
				break;
		}
		srand(time(NULL) + GetCurrentThreadId());
		Sleep(((INT)frand() * TIMEMUL) * 1000);
		
		row = dequeue(queuesB[(INT)(frand() * threadsQty)]);
		if (row == NULL) {
			continue;
		}

		//CONSUME
		output = (TCHAR*)malloc(sizeof(TCHAR) * (row->length + 1));
		for (c = row->string; c - row->string < row->length; c++) {
			output[i++] = *c;
		}
		output[i] = '\0';
		quickSort(output, 0, row->length);

		file = lfOutput[(INT)(frand() * threadsQty)];

		WaitForSingleObject(file.sem, INFINITE);
		ov.Offset = 0xFFFFFFFF;
		ov.OffsetHigh = 0xFFFFFFFF;
		WriteFile(file.file, &length, sizeof(INT), &nWrited, &ov);
		ov.Offset = 0xFFFFFFFF;
		ov.OffsetHigh = 0xFFFFFFFF;
		WriteFile(file.file, output, sizeof(TCHAR)*row->length, &nWrited, &ov);
		ReleaseSemaphore(file.sem, 1, NULL);
	}

	ExitThread(0);
}

static tRow *dequeue(tBuffer queue) {
	tRow *row;
	DWORD fullSem;
	fullSem = WaitForSingleObject(queue.fullSem, 100);
	if (fullSem == WAIT_TIMEOUT) {
		return NULL;
	}
	WaitForSingleObject(queue.consumerSem, INFINITE);
	//DEQUEUE
	row = &queue.queue[queue.cIndex++];
	queue.cIndex = queue.cIndex % QUEUE_LEN;

	ReleaseSemaphore(queue.consumerSem, 1, NULL);
	ReleaseSemaphore(queue.emptySem, 1, NULL);
	return row;
}

static void enqueue(tBuffer queue, tRow row) {
	WaitForSingleObject(queue.emptySem, INFINITE);
	WaitForSingleObject(queue.producerSem, INFINITE);
	//ENQUEUE
	queue.queue[queue.pIndex++] = row;
	queue.pIndex = queue.pIndex % QUEUE_LEN;

	ReleaseSemaphore(queue.producerSem, 1, NULL);
	ReleaseSemaphore(queue.fullSem, 1, NULL);
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
