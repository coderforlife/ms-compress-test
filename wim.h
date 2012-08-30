#ifndef WIM_H
#define WIM_H

#ifndef _WIMGAPI_H_
typedef enum
{
    WIM_COMPRESS_NONE = 0,
    WIM_COMPRESS_XPRESS,
    WIM_COMPRESS_LZX
};
#endif

int create_wim(PWSTR pszWimFile, PWSTR pszCaptureDir, DWORD dwCompressionType);

#endif