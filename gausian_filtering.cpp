
#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <stdio.h>

#define R 640
#define C 480

DWORD WINAPI threadFunction1(LPVOID);
DWORD WINAPI threadFunction2(LPVOID);

typedef struct syn_object {
	INT count;
	HANDLE mt;
	HANDLE barrier1, barrier2;
}syn_object_t;

typedef struct files {
	INT count;
	HANDLE mt;
}files_t;
typedef struct threads {
	LPTSTR file1, file2;
	files_t files;
	syn_object_t syn_obj;
}threads_t;


int n, m;
LPTSTR name1, name2, name3;
threads_t group1, group2;

int _tmain(int argc, LPTSTR argv[]) {
	name1 = argv[1];
	name2 = argv[2];
	name3 = argv[3];
	n = _ttoi(argv[4]);
	m = _ttoi(argv[5]);

	group1.file1 = name1;
	group1.file2 = name2;
	group1.files.count = 0;
	group1.files.mt = CreateMutex(NULL, FALSE, NULL);
	group1.syn_obj.count = 0;
	group1.syn_obj.mt = CreateMutex(NULL, FALSE, NULL);
	group1.syn_obj.barrier1 = CreateSemaphore(NULL, n, n, NULL);
	group1.syn_obj.barrier2 = CreateSemaphore(NULL, 0, n, NULL);

	group2.file1 = name2;
	group2.file2 = name3;
	group2.files.count = 0;
	group2.files.mt = CreateMutex(NULL, FALSE, NULL);
	group2.syn_obj.count = 0;
	group2.syn_obj.mt = CreateMutex(NULL, FALSE, NULL);
	group2.syn_obj.barrier1 = CreateSemaphore(NULL, n, n, NULL);
	group2.syn_obj.barrier2 = CreateSemaphore(NULL, 0, n, NULL);

	HANDLE *hThreads1, * hThreads2;
	hThreads1 = (HANDLE*)malloc(n * sizeof(HANDLE));
	hThreads2 = (HANDLE*)malloc(m * sizeof(HANDLE));
	
	INT i;
	/*create the threads*/
	for (i = 0; i < n; i++) 
		hThreads1[i] = CreateThread(NULL, 0 , (PTHREAD_START_ROUTINE)threadFunction1, &group1, 0, NULL);
	
	WaitForMultipleObjects(n, hThreads1, TRUE, INFINITE);
	for (i = 0; i < n; i++) {
		CloseHandle(hThreads1[i]);
	}

	WaitForMultipleObjects(m, hThreads2, TRUE, INFINITE);
	for (i = 0; i < m; i++) {
		CloseHandle(hThreads2[i]);
	}
	return 0;
}

DWORD WINAPI threadFunction1(LPVOID lpParam) {
	threads_t* data = (threads_t*)lpParam;
	HANDLE hFile1, hFile2;
	hFile1 = CreateFile(data->file1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ| FILE_SHARE_WRITE, NULL,0, FILE_ATTRIBUTE_NORMAL, NULL);
	hFile2 = CreateFile(data->file2, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 0, FILE_ATTRIBUTE_NORMAL, NULL);
	INT i;
	for (i = 0; i < 3; i++) {
		WaitForSingleObject(data->syn_obj.barrier1, INFINITE);
		
		while (1) {
			WaitForSingleObject(data->files.mt, INFINITE);
				data->files.count++;
				if (data->files.count > (R * C)) break;
				_tprintf(_T("Doing filter3\n"));
			ReleaseMutex(data->files.mt);
		}
	
		WaitForSingleObject(data->syn_obj.mt, INFINITE);
			data->syn_obj.count++;
			if (data->syn_obj.count == n) {
				/*I am the last group 1 thread*/
				INT j;
				for (j = 0; j < m;j++) {
					ReleaseSemaphore(group2.syn_obj.barrier1, 1, NULL);
				}
				data->syn_obj.count = 0;
				data->files.count = 0;
				for (j = 0; j < n; j++) {
					ReleaseSemaphore(data->syn_obj.barrier2, 1, NULL);
				}
			}
		ReleaseMutex(data->syn_obj.mt);
		WaitForSingleObject(data->syn_obj.barrier2, INFINITE);

	}
	CloseHandle(hFile1);
	CloseHandle(hFile2);
	ExitThread(0);
}

DWORD WINAPI threadFunction2(LPVOID lpParam) {
	threads_t* data = (threads_t*)lpParam;
	HANDLE hFile2, hFile3;
	hFile2 = CreateFile(data->file1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 0, FILE_ATTRIBUTE_NORMAL, NULL);
	hFile3 = CreateFile(data->file2, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 0, FILE_ATTRIBUTE_NORMAL, NULL);
	INT i;
	for (i = 0; i < 3; i++) {
		WaitForSingleObject(data->syn_obj.barrier1, INFINITE);

		while (1) {
			WaitForSingleObject(data->files.mt, INFINITE);
			data->files.count++;
			if (data->files.count > (R * C)) break;
			_tprintf(_T("Done filter5\n"));
			ReleaseMutex(data->files.mt);
		}

		WaitForSingleObject(data->syn_obj.mt, INFINITE);
		data->syn_obj.count++;
		if (data->syn_obj.count == n) {
			/*I am the last group 1 thread*/
			INT j;
			for (j = 0; j < n; j++) {
				ReleaseSemaphore(group1.syn_obj.barrier1, 1, NULL);
			}
			data->syn_obj.count = 0;
			data->files.count = 0;
			for (j = 0; j < m; j++) {
				ReleaseSemaphore(data->syn_obj.barrier2, 1, NULL);
			}
		}
		ReleaseMutex(data->syn_obj.mt);
		WaitForSingleObject(data->syn_obj.barrier2, INFINITE);

	}
	CloseHandle(hFile2);
	CloseHandle(hFile3);
	ExitThread(0);
}