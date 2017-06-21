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
#define ifmain _tmainCCC2

typedef struct _access {
	TCHAR ip[15+1];
	TCHAR dude[LEN + 1];
	TCHAR datetime[19 + 1];
	TCHAR length[8 + 1];
} tAccess;

typedef struct _file {
	TCHAR dir[LEN];
	HANDLE writerSem;
	HANDLE readerSem;
	INT readcount;
} tFile;

typedef struct threadParam {
	INT numberOfServers;
	tFile* dirs;
} tParam;

DWORD WINAPI tJob(LPVOID);
static void threadJob(tParam*);
static void reader(HANDLE);
static void writer(HANDLE);
static FLOAT frand();

INT ifmain(INT argc, LPTSTR argv[]) {
	INT qty, threadsQty, i;
	HANDLE server_list;
	DWORD nr_bytes;
	DWORD *threadId;
	HANDLE *threadHandle;
	TCHAR buf[LEN];
	tParam param;

	threadsQty = _ttoi(argv[2]);
	threadHandle = (HANDLE*)malloc((threadsQty) * sizeof(HANDLE));
	threadId = (DWORD*)malloc((threadsQty) * sizeof(DWORD));

	server_list = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ReadFile(server_list, &qty, sizeof(INT), &nr_bytes, NULL);
	param.numberOfServers = qty;
	param.dirs = (tFile*)malloc(qty * sizeof(tFile));
	i = 0;
	while (ReadFile(server_list, buf, sizeof(TCHAR) * LEN, &nr_bytes, NULL) && nr_bytes > 0) {
		_stscanf(param.dirs[i].dir, _T("%s"), buf);
		param.dirs[i].writerSem = CreateSemaphore(NULL, 1, qty, NULL);
		param.dirs[i].readerSem = CreateSemaphore(NULL, 1, qty, NULL);
		i++;
	}
	for (i = 0; i < threadsQty; i++) {
		threadHandle[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tJob,
			&param, 0, threadId + i);
	}

	return 0;
}

DWORD WINAPI tJob(LPVOID lpParam) {
	threadJob(((tParam*)lpParam));

	ExitThread(0);
}

static void threadJob(tParam *param) {
	FLOAT r1, r2, r3;
	srand(time(NULL));
	LPTSTR server;
	INT isReader;
	HANDLE SearchHandle;
	WIN32_FIND_DATA FindData;
	HANDLE handle;
	DWORD fAccess;
	tFile f;

	while (1) {
		r1 = frand();
		r2 = frand();
		r3 = frand();
		Sleep(r1 * 100 * 1000);
		f = param->dirs[(int)(r3 * param->numberOfServers)];
		server = f.dir;
		isReader = r2 < 0.5;
		SearchHandle = FindFirstFile(server, &FindData);
		fAccess = isReader ? (GENERIC_WRITE | GENERIC_READ) : GENERIC_READ;
		do {
			handle = CreateFile(FindData.cFileName, fAccess, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (isReader) {
				WaitForSingleObject(f.readerSem, INFINITE);
				f.readcount++;
				if (f.readcount == 1)
					WaitForSingleObject(f.writerSem, INFINITE);
				reader(handle);
				ReleaseSemaphore(f.readerSem, 1, NULL);

				WaitForSingleObject(f.readerSem, INFINITE);
				f.readcount--;
				if (f.readcount == 0)
					ReleaseSemaphore(f.writerSem, 1, NULL);
				ReleaseSemaphore(f.readerSem, 1, NULL);
			}
			else {
				WaitForSingleObject(f.writerSem, INFINITE);
				writer(handle);
				ReleaseSemaphore(f.writerSem, 1, NULL);
			}
			CloseHandle(handle);
		} while (FindNextFile(SearchHandle, &FindData));
	}
}

static void reader(HANDLE handle) {
	tAccess access;
	DWORD nr_bytes;
	INT yyyy, MM, dd, HH, mm, ss;
	struct tm *tm1;
	time_t time;
	time_t last_time = -1;
	TCHAR strtime[LEN];
	LONG totLength = 0;

	while (ReadFile(handle, &access, sizeof(tAccess), &nr_bytes, NULL) && nr_bytes > 0) {
		_stscanf(access.datetime, _T("%d/%d/%d:%d:%d:%d"), &yyyy, &MM, &dd, &HH, &mm, &ss);
		tm1 = (struct tm*) malloc(sizeof(struct tm));
		tm1->tm_sec = ss;
		tm1->tm_min = mm;
		tm1->tm_hour = HH;
		tm1->tm_mday = dd;
		tm1->tm_mon = MM - 1;
		tm1->tm_year = yyyy - 1900;
		tm1->tm_isdst = -1;
		time = mktime(tm1);
		if (last_time < 0 || time > last_time)
			last_time = time;
		_stscanf(access.length, _T("%d:%d:%d"), &HH, &mm, &ss);
		tm1->tm_sec = ss;
		tm1->tm_min = mm;
		tm1->tm_hour = HH;
		tm1->tm_mday = 0;
		tm1->tm_mon = 0;
		tm1->tm_year = 0;
		tm1->tm_isdst = -1;
		time = mktime(tm1);
		totLength += (LONG)time;
	}
	tm1 = gmtime(&last_time);
	_tcsftime(strtime, 128, _T("%F:%T"), tm1);
	_tprintf(_T("\nLast modified time: %s"), strtime);
	_tprintf(_T("\nTotal connection time: %ld"), totLength);
}

static void writer(HANDLE handle) {
	tAccess access;
	DWORD nr_bytes, nWrite;
	INT yyyy, MM, dd, HH, mm, ss;
	struct tm *tm1;
	time_t time;
	time_t last_time = -1;
	TCHAR strtime[LEN];
	INT done = 0;
	int pos = 0;
	OVERLAPPED ov = { 0, 0, 0, 0, NULL }; //Internal, InternalHigh, Offset, OffsetHigh, hEvent
	LARGE_INTEGER filePos;

	while (ReadFile(handle, &access, sizeof(tAccess), &nr_bytes, NULL) && nr_bytes > 0) {
		done = 0;
		filePos.QuadPart = pos++ * sizeof(tAccess);
		ov.Offset = filePos.LowPart;
		ov.OffsetHigh = filePos.HighPart;
		if (frand() > 0.5) {
			done = 1;
			_stscanf(access.datetime, _T("%d/%d/%d:%d:%d:%d"), &yyyy, &MM, &dd, &HH, &mm, &ss);
			tm1 = (struct tm*) malloc(sizeof(struct tm));
			tm1->tm_sec = ss;
			tm1->tm_min = mm;
			tm1->tm_hour = HH;
			tm1->tm_mday = dd;
			tm1->tm_mon = MM - 1;
			tm1->tm_year = yyyy - 1900;
			tm1->tm_isdst = -1;
			time = mktime(tm1);
			time *= ((frand() > 0.5) ? 1 : -1)*(1 + .2*frand());
			tm1 = gmtime(&time);
			_tcsftime(strtime, 128, _T("%F:%T"), tm1);

			_stprintf(access.datetime, _T("%s"), strtime);

		}
		if (!done || (frand() > 0.5)) {
			_stscanf(access.length, _T("%d:%d:%d"), &HH, &mm, &ss);
			tm1 = (struct tm*) malloc(sizeof(struct tm));
			tm1->tm_sec = ss;
			tm1->tm_min = mm;
			tm1->tm_hour = HH;
			tm1->tm_mday = 0;
			tm1->tm_mon = 0;
			tm1->tm_year = 0;
			tm1->tm_isdst = -1;
			time = mktime(tm1);
			tm1 = gmtime(&time);
			_tcsftime(strtime, 128, _T("%T"), tm1);

			_stprintf(access.length, _T("%s"), strtime);
		}
		if (!(WriteFile(handle, &access, sizeof(tAccess), &nWrite, &ov) && nWrite == sizeof(tAccess)))
			exit(1);
	}
}

static FLOAT frand() {
	return ((FLOAT)rand() - 1) / (FLOAT)(RAND_MAX);
}
