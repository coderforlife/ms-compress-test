#include "stdafx.h"

#include <wimgapi.h>
#ifdef _WIN64
#pragma comment(lib, "wimgapi.lib")
#else
#pragma comment(lib, "wimgapi.lib")
#endif

#include "wim.h"
#include "timer.h"

//static bool StartsWith(PWSTR str, PWSTR sub)
//{
//	size_t len1 = wcslen(str), len2 = wcslen(sub);
//	return len2 <= len1 && wcsncmp(str, sub, len2) == 0;
//}

static bool Contains(PWSTR str, PWSTR sub)
{
	size_t len1 = wcslen(str), len2 = wcslen(sub);
	return len2 <= len1 && wcsstr(str, sub) != NULL;
}

static bool EndsWith(PWSTR str, PWSTR sub)
{
	size_t len1 = wcslen(str), len2 = wcslen(sub);
	return len2 <= len1 && wcsncmp(str + (len1 - len2), sub, len2) == 0;
}

DWORD WINAPI SampleCaptureCallback(
	DWORD dwMsgId,   // Message ID
	WPARAM wParam,   // Usually a file name
	LPARAM lParam,   // Usually an error code
	PVOID pvIgnored  // Used to maintain caller context. Not used in this code sample.
	)
{
	PWSTR filename = NULL;
	double time = 0;

	UNREFERENCED_PARAMETER(pvIgnored);

	switch (dwMsgId)
	{
		case WIM_MSG_PROGRESS:

			// Prints out the current percentage.
			//wprintf(L"%d %%\n", (DWORD)wParam);
			break;

		case WIM_MSG_PROCESS:

			// This message is sent for each file. It reports whether the calling client is trying to capture the file.
			filename = (PWSTR)wParam;
			if (!Contains(filename, L"\\test.") || EndsWith(filename, L".lznt1") || EndsWith(filename, L".xpress") || EndsWith(filename, L".xpress_huff") || EndsWith(filename, L".wim"))
			{
				// Cancel processing on this file
				*((PBOOL)lParam) = FALSE;
			}
			else
			{
				// Print out the file name being processed
				//wprintf(L"File Path: %s\n", filename);
			}
			break;

		// TODO: this only works for XPRESS compression and not LZX
		case WIM_MSG_ALIGNMENT: wprintf(L"Compressing: %s... ", wcsrchr((PWSTR)wParam, L'\\') + 1); timer_reset(); break;
		case WIM_MSG_STEPIT:    time = timer_get(); wprintf(L"%f sec\n", time); break;

		case WIM_MSG_RETRY:

			// This message is sent when the file is being reapplied because of a
			// network timeout. Retry is attempted up to five times.
			wprintf(L"RETRY: %s [err = %d]\n", (PWSTR)wParam, (DWORD)lParam);
			break;

		case WIM_MSG_ERROR:   wprintf(L"ERROR:   %s [err = %d]\n", (PWSTR)wParam, (DWORD)lParam); break;
		case WIM_MSG_WARNING: wprintf(L"WARNING: %s [err = %d]\n", (PWSTR)wParam, (DWORD)lParam); break;
		case WIM_MSG_INFO:    wprintf(L"INFO:    %s [err = %d]\n", (PWSTR)wParam, (DWORD)lParam); break;
		//default: wprintf(L"Skipped Message: %x\n", dwMsgId);
	}

	// To abort image processing, return WIM_MSG_ABORT_IMAGE. Not all message types
	// check for this status. When you decide to cancel an image, it may take repeated
	// returns of this status before WIMGAPI can perform the cancelation.
	return WIM_MSG_SUCCESS;
}

int create_wim(PWSTR pszWimFile, PWSTR pszCaptureDir, DWORD dwCompressionType) // WIM_COMPRESS_NONE, WIM_COMPRESS_XPRESS, or WIM_COMPRESS_LZX
{
	BOOL bRet = TRUE;
	HANDLE hWim = NULL, hImage = NULL;
	DWORD dwCreationResult = 0, dwError = 0;

	//PWSTR pszTmpDir   = L"C:\\tmp";      // Temporary directory: OPTIONAL

	// Register the Callback function.
	if (WIMRegisterMessageCallback(NULL, (FARPROC)SampleCaptureCallback, NULL) == INVALID_CALLBACK_VALUE)
	{
		bRet = FALSE;
		dwError = GetLastError();
		wprintf(L"Cannot set callback\n");
	}

	// Open the .wim file
	if (bRet)
	{
		hWim = WIMCreateFile(pszWimFile, WIM_GENERIC_WRITE, WIM_CREATE_ALWAYS, 0, dwCompressionType, &dwCreationResult);
		if (!hWim)
		{
			bRet = FALSE;
			dwError = GetLastError();
			wprintf(L"Cannot open WIM file\n");
		}
	}

	// WIMGAPI uses temporary files. You must specify where to store the files.
	//if (bRet)
	//{
	//	bRet = WIMSetTemporaryPath(hWim, pszTmpDir);
	//	if (!bRet)
	//	{
	//		dwError = GetLastError();
	//		wprintf(L"Cannot set temporary directory\n");
	//	}
	//}
	// Finally, perform the capture operation.
	if (bRet)
	{
		hImage = WIMCaptureImage(hWim, pszCaptureDir, 0);
		if (!hImage)
		{
			dwError = GetLastError();
			wprintf(L"Cannot capture/append image\n");
		}
	}

	// When you are finished, close the handles that you created in the previous steps.
	// Note: you must close image handles before you close WIM handles.
	if (hImage) WIMCloseHandle(hImage);
	if (hWim)   WIMCloseHandle(hWim);

	WIMUnregisterMessageCallback(NULL, (FARPROC)SampleCaptureCallback);

	wprintf(L"Returning status: 0x%x\n", dwError);
	return dwError;
}
