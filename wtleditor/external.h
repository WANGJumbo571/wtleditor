#pragma once

extern WCHAR* utf8ToUnicode(const char* zFilename);
extern char* unicodeToUtf8(const WCHAR* zWideFilename);
extern void reportError(int location, HRESULT hr);
extern byte* FReadData(std::string filename, LONG* len);
extern HINSTANCE g_hInstance;