/*
       
    TO BUILD

    cl.exe /I src /D DEBUG /EHsc main.cpp /link user32.lib Gdi32.lib Gdiplus.lib /OUT:snorkelog.exe
*/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <time.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <gdiplus.h>
#include <AtlBase.h>
#include <AtlConv.h>

#include <GdiPlusImageCodec.h>

using namespace std;


#define CAPTURE_THRESHOLD_SECS 60*20

HWND lastwindow = NULL;
time_t lastcapture;

void mkdir(string dir)
{
#ifdef _WIN32
    string cmd("if not exist \"");
    cmd.append(dir.c_str());
    cmd.append("\" mkdir \"");
    cmd.append(dir.c_str());
    cmd.append("\"");
    system(cmd.c_str());
#endif
}
const string curr_date_time() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

const string curr_date() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);

    return buf;
}
string curr_timestamp()
{
    stringstream ss;
    time_t now = time(0);
    ss << (long)now; 
    return ss.str();
}

void mreplace(string& str, const string& oldStr, const string& newStr)
{
  size_t pos = 0;
  while((pos = str.find(oldStr, pos)) != string::npos)
  {
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}

std::wstring strtowstr(const std::string &str)
{
    // Convert an ASCII string to a Unicode String
    std::wstring wstrTo;
    wchar_t *wszTo = new wchar_t[str.length() + 1];
    wszTo[str.size()] = L'\0';
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wszTo, (int)str.length());
    wstrTo = wszTo;
    delete[] wszTo;
    return wstrTo;
}

// In case I need it... see 
// http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte/3999597#3999597
std::string wstrtostr(const std::wstring &wstr)
{
    // Convert a Unicode string to an ASCII string
    std::string strTo;
    char *szTo = new char[wstr.length() + 1];
    szTo[wstr.size()] = '\0';
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
    strTo = szTo;
    delete[] szTo;
    return strTo;
}



int GetEncoderClsid2(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return 0;
} // GetEncoderClsid2()



string gdiscreen2(HDC monDC, wstring pngPath, LPRECT lprcMonitor)
{
	using namespace Gdiplus;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	{
		HDC scrdc, memdc;
		HBITMAP membit;
		int Height = GetSystemMetrics(SM_CYSCREEN);
		int Width = GetSystemMetrics(SM_CXSCREEN);

        cout << "monitor handle = " << monDC << ", " <<
             "height = " << Height << ", width = " << Width << endl;
		memdc = CreateCompatibleDC(monDC);
		membit = CreateCompatibleBitmap(monDC, Width, Height);
		HBITMAP hOldBitmap =(HBITMAP) SelectObject(memdc, membit);
		BitBlt(memdc, 0, 0, Width, Height, monDC, lprcMonitor->left, lprcMonitor->top, SRCCOPY);

		Gdiplus::Bitmap bitmap(membit, NULL);
		CLSID clsid;
		//GetEncoderClsid2(L"image/jpeg", &clsid);
		GetEncoderClsid2(L"image/png", &clsid);
		bitmap.Save(pngPath.c_str(), &clsid);

		SelectObject(memdc, hOldBitmap);

		DeleteObject(memdc);

		DeleteObject(membit);

		::ReleaseDC(0,monDC);
	}

	GdiplusShutdown(gdiplusToken);
    return "";
} // gdiscreen()


// used by EnumDisplayMonitors
BOOL CALLBACK captureMonitor(
  HMONITOR hMonitor,
  HDC hdcMonitor,
  LPRECT lprcMonitor,
  LPARAM dwData
)
{
    MONITORINFOEX moninf;
    moninf.cbSize = sizeof(moninf);
    GetMonitorInfo(hMonitor, &moninf);
    cout << "monitor name = " << moninf.szDevice << endl;
    HDC h = ::CreateDC(NULL, moninf.szDevice, NULL, NULL);

    mkdir(curr_date());
    string fn = curr_date();
    fn.append("\\");

    string tm(curr_date_time());
    mreplace(tm, ":", "_");
    fn.append(tm.c_str());
    fn.append("   ");

    string mn(moninf.szDevice);
    mreplace(mn, "\\", "");
    mreplace(mn, ".", "");
    fn.append(mn.c_str());

    fn.append(".png");
    cout << "image filename = " << fn << endl;

    wstring wfn = strtowstr(fn);
    gdiscreen2(h, wfn, lprcMonitor);

    return TRUE;
}


void savescreens()
{
    EnumDisplayMonitors(NULL, NULL, captureMonitor, 0);  
}

int main()
{
    lastcapture = time(0);
    
    cout << "lastcapture = " << (long)lastcapture << endl;
    cout << "lastwindow = " << (long)lastwindow << endl;
    while (true)
    {
        HWND cw = GetForegroundWindow();
        time_t now = time(0);
        cout << "current window id = " << (long)cw << endl;
        cout << "current timestamp = " << (long)now << endl;

        if (lastwindow == NULL || lastwindow != cw)
        {
            lastwindow = cw;
            lastcapture = now;
            savescreens();    
        }
        else 
        {
            cout << "(current timestamp - lastcapture) = " <<
                (now - lastcapture) << " (seconds)" << endl;
            if ((now - lastcapture) > CAPTURE_THRESHOLD_SECS)
            {
                lastcapture = now;
                savescreens();
            }
        }
         
        Sleep(5000);
        //TODO capture ctrl+c signal
    }
    /*
    EnumDisplayMonitors(NULL, NULL, my_MonitorEnumProc, 0);  
    cout << gdiscreen2() << endl;
    */
    return 0;
}
