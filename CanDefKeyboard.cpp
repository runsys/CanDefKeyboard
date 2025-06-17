// CanDefKeyboard.cpp : 定义应用程序的入口点。
// author:iwlb@outlook.com; site:github.com/runsys/;

#include "stdafx.h"
#include "CanDefKeyboard.h"
#include <cstdio>
#include <ctime>
#include <string.h>
#include "atlbase.h"
#include "atlstr.h"
#include <shellapi.h>
#include <tlhelp32.h>
#include <WinInet.h>
#include <string>

#pragma   comment(lib,   "shell32.lib")
#pragma   comment(lib,   "imm32.lib")
#pragma   comment(lib, "wininet.lib")
#pragma comment( lib, "urlmon.lib" )

//Candef Keybord Change Log
//1.1.6 处理托盘图标消息
//1.1.8 箭头下右键是Num_lock键，mstsc远程连接方向键变数字键有用；启动拖动杀tabtip.exe;
//1.2.0 shift ctrl win alt按下标记红色；按下shift显示2键；
//1.3.0 添加caps状态显示；推荐ctrl,shift,alt,win右键点击长按功能；
//1.4.0 shift+back修改窗口大小为2倍；
//1.4.1 ctrl+back隐藏窗口
//1.4.4 只允许一个实例运行;
//1.5.4 添加了箭头上下右键模拟鼠标滚轮;num_lock变更到caps的右键;
//1.5.6 修改Caps状态错误;
//1.5.8 修复右alt点击无反应错误;
//1.5.9 修改num_lock说明
//1.6.0 添加alt+back自动关闭TabTip.exe;
//1.6.2 优化上下箭头的模拟滚轮
//1.7.3 优化了按下五角星移动准确度；
//1.8.0 添加鼠标按下或移动提升当前鼠标或者触摸笔手指正下方字符，方便手机或触摸设备观察触摸点按键；
//！！！1.8.1 无法完成测试！微软安卓Windows远程桌面没有按下消息，实现不了按下显示当前字符；
//1.9.0 添加显示面板热键Ctrl+Alt+8,显示隐藏开关热键Ctrl+Alt+9;
//1.9.0 添加程序AutoKillTabTip以系统权限结束TabTip程序,调用psexec -i -d -s exe提权运行;
//2.0.1 20250407-20250408添加键盘隐藏快捷键Shift+Alt+H;添加键盘退出快捷键Shift+Alt+Q;添加虚拟鼠标功能:Alt+方向键 移动鼠标;Shift+Alt+方向键 45移动鼠标;Ctrl+Alt+方向键 鼠标拖动;Alt+Del 左键单击,End 中键单击,PageDown 右键键单击;Alt+Ins 左键双击,Home 中键双击,PageUp 右键双击;Shift+Alt+Del 左键按下,End 中键按下,PageDown 右键按下;Shift+Alt+Ins 左键松开,Home 中键松开,PageUp 右键松开;Alt+Z 一格大小10点,X 一格大小20点,C 一格大小40点;Alt+Back滚轮上滚(Scroll up),Slash滚轮下滚(scroll down),cTRL+AlthOME滚轮上滚(Scroll up),\r\neND滚轮下滚(scroll down);
//2.0.2 20250408添加win+ins延迟左键双击,del延迟左键单击,pageup延迟右键双击,pagedown延迟右键单击;win+c开启关闭虚拟键盘;
//2.1.0 20250415添加alt+shift+方向键2倍数;alt+'<','>'慢速左右移动;':','/'慢速上下移动;  alt+'+',']','\',BACK, 左上,左下,右下,右上2倍数移动;
//2.2.0 ALT+1,2,3,4移动步长2,13,26,39;去除ALT+Z,X,C;ALY+'+' scroll  up, ']' scroll down;Alt+Ctrl+Back Fast scroll up, '|' fast scroll down;
//2.3.0 ALT+句号 替换剪切板文本/到\, / 替换\为/
//2.4.0 ALT0-9= conf文件自定义粘贴
//2.4.5 20250522T120332 fix ALT5-9,0= conf文件自定义粘贴
//2.5.0 20250526T091923 Alt5-9,0 添加{cd}替换为当前剪切板内容,修改显示隐藏窗口Win+`;
//2.6.0 20250617T185431 alt+'"' 替换剪切板内容为编程字符串;

//字符串替换函数
/********************************************************************
*  Function：  my_strstr()
*  Description: 在一个字符串中查找一个子串;
*  Input：      ps: 源;      pd：子串
*  Return :    0：源字符串中没有子串; 1：源字符串中有子串;
*********************************************************************/
wchar_t * wmy_strstr(wchar_t * ps, wchar_t *pd)
{
	wchar_t *pt = pd;
	int c = 0;
	while (*ps != '\0')
	{
		if (*ps == *pd)
		{
			while (*ps == *pd && *pd != '\0')
			{
				ps++;
				pd++;
				c++;
			}
		}
		else
		{
			ps++;
		}
		if (*pd == '\0')
		{
			return (ps - c);
		}
		c = 0;
		pd = pt;
	}
	return 0;
}

/********************************************************************
*  Function：  wmemcpy()
*  Description: 复制一个内存区域到另一个区域;
*  Input：      src: 源;
count: 复制字节数.
*  Output：      dest: 复制目的地;
*  Return :      dest;
*********************************************************************/
void * wmemcpy(void * dest, const void *src, size_t count)
{
	wchar_t *tmp = (wchar_t *)dest, *s = (wchar_t *)src;
	while (count--)
		*tmp++ = *s++;
	return dest;
}

/********************************************************************
*  Function：  wstr_replace()
*  Description: 在一个字符串中查找一个子串，并且把所有符合的子串用
另一个替换字符串替换。
*  Input：      p_source:要查找的母字符串； p_seach要查找的子字符串;
p_repstr：替换的字符串;
*  Output：      p_result:存放结果;
*  Return :      返回替换成功的子串数量;
*  Others:      p_result要足够大的空间存放结果，所以输入参数都要以\0结束;
*********************************************************************/
int wstr_replace(wchar_t *p_result, wchar_t* p_source, wchar_t* p_seach, wchar_t *p_repstr)
{
	int c = 0;
	int repstr_leng = 0;
	int searchstr_leng = 0;

	wchar_t *p1;
	wchar_t *presult = p_result;
	wchar_t *psource = p_source;
	wchar_t *prep = p_repstr;
	wchar_t *pseach = p_seach;
	int nLen = 0;

	repstr_leng = wcslen(prep);
	searchstr_leng = wcslen(pseach);

	do {
		p1 = wmy_strstr(psource, p_seach);

		if (p1 == 0)
		{
			wcscpy(presult, psource);
			return c;
		}
		c++;  //匹配子串计数加1;
		wprintf(L"结果:%s\r\n", p_result);
		wprintf(L"源字符:%s\r\n", p_source);

		// 拷贝上一个替换点和下一个替换点中间的字符串
		nLen = p1 - psource;
		wmemcpy(presult, psource, nLen);

		// 拷贝需要替换的字符串
		wmemcpy(presult + nLen, p_repstr, repstr_leng);

		psource = p1 + searchstr_leng;
		presult = presult + nLen + repstr_leng;
	} while (p1);

	return c;
}



DWORD KillProcessidByName(LPCTSTR name)
{
	PROCESSENTRY32 pe;
	DWORD id = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
		return 0;
	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE)
			break;
		
		if ((wcscmp(name ,pe.szExeFile)==0/* || L"hvkcm64.exe" == program_info.szExeFile*/) && GetCurrentProcessId() != pe.th32ProcessID)
		{
			// 根据进程id打开进程句柄
			auto h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
			if (h)
			{
				::TerminateProcess(h, 0);
			}
			break;
		}
	}
	CloseHandle(hSnapshot);
	return id;
}

double xdpi = 96.0, ydpi = 96.0;
//double xdpi = 274, ydpi = 274;
bool DPIInit() {
	SetProcessDPIAware();
	auto dc = ::GetDC(0);
	xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
	ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
	auto val = ::ReleaseDC(0, dc);
	return true;
}


struct SheellInfo {
	HWND hWnd;
	int x;
	int y;
};


void dp(const char* strOutputString, ...)
{
#ifdef _DEBUG
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugString(CA2W(strBuffer));
#endif
}

void dpw(const wchar_t* strOutputString, ...)
{
#ifdef _DEBUG
	wchar_t strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	wprintf(strBuffer, vlArgs);
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugString(strBuffer);
#endif
}


int GStepBase = 13;
int GStep = GStepBase;

DWORD WINAPI winupdragthrd(LPVOID p) {
	Sleep(333);
	char tit[32] = { 0 };
	auto hwnd = GetTopWindow(0);
	GetWindowTextA(hwnd, tit, 32);
	dp("tit%s\n", tit);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_MOVE, 0, -GStep, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	return 0;
}


DWORD WINAPI windowndragthrd(LPVOID p) {
	Sleep(333);
	char tit[32] = { 0 };
	auto hwnd = GetTopWindow(0);
	GetWindowTextA(hwnd, tit, 32);
	dp("tit%s\n", tit);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_MOVE, 0, GStep, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	return 0;
}
DWORD WINAPI pastethrd(LPVOID p) {
	Sleep(500);
	keybd_event(VK_LCONTROL, 0, 0, 0);
	keybd_event('V', 0, 0, 0);
	keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
	return 0;
}

DWORD WINAPI typethrd(LPVOID p) {
	Sleep(500); 
	wchar_t *p1 = (wchar_t*)p;
	for (int i = 0; i < wcslen(p1); i += 1) {
		if (p1[i] >= 65 && p1[i] <= 90) {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event(p1[i], 0, 0, 0);
			keybd_event(p1[i], 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
		}
		else {
			keybd_event(p1[i], 0, 0, 0);
			keybd_event(p1[i], 0, KEYEVENTF_KEYUP, 0);
		}
	}
	return 0;
}

BOOL SetClipboardTextA(const char* pszData, const int nDataLen)
{
	while (::OpenClipboard(NULL) == FALSE) {
		Sleep(1000);
	}
	if (::EmptyClipboard() == FALSE) {
		CloseClipboard();
		return FALSE;
	}
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, (nDataLen + 1) * sizeof(char));
	if (hMem == 0) {
		CloseClipboard();
		return FALSE;
	}
	char *bufferbuffer = (char *)::GlobalLock(hMem);
	if (bufferbuffer == 0) {
		CloseClipboard();
		return FALSE;
	}
	strncpy(bufferbuffer, pszData, nDataLen + 1);
	::GlobalUnlock(hMem);
	if (::SetClipboardData(CF_TEXT, hMem) == FALSE) {
		return FALSE;
	}
	if (::CloseClipboard() == FALSE) {
		return FALSE;
	}
	return TRUE;
}




BOOL cppast(HWND hwnd,const char* pszData, const int nDataLen)
{
	while (::OpenClipboard(hwnd) == FALSE) {
		Sleep(1000);
	}
	if (::EmptyClipboard() == FALSE) {
	}
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, (nDataLen + 1) * sizeof(char));
	if (hMem == 0) {
		CloseClipboard();
		return FALSE;
	}
	char *bufferbuffer = (char *)::GlobalLock(hMem);
	if (bufferbuffer == 0) {
		CloseClipboard();
		return FALSE;
	}
	strncpy(bufferbuffer, pszData, nDataLen + 1);
	::GlobalUnlock(hMem);
	if (::SetClipboardData(CF_TEXT, hMem) == FALSE) {
		return FALSE;
	}
	if (::CloseClipboard() == FALSE) {
		return FALSE;
	}
	return TRUE;
}


wchar_t * GetClipboardText()
{
	auto txth1 = IsClipboardFormatAvailable(CF_UNICODETEXT);
	auto txthw = IsClipboardFormatAvailable(CF_TEXT);
	while (::OpenClipboard(NULL) == FALSE) {
		Sleep(1000);
	}
	auto hGlobal = GetClipboardData(CF_UNICODETEXT);
	if (hGlobal != 0) // is equal "NULL" condition
	{
		auto pGlobal = (wchar_t*)GlobalLock(hGlobal);
		if (pGlobal == 0) {
			CloseClipboard();
			return NULL;
		}
		wchar_t *texty = (wchar_t *)malloc(wcslen(pGlobal) * 4 + 1);
		wcscpy(texty, pGlobal);
		texty[wcslen(pGlobal)] = 0;
		if (GlobalUnlock(hGlobal) == FALSE) {
			CloseClipboard();
			return NULL;
		}
		if (CloseClipboard() == FALSE) {
			return NULL;
		}
		return texty;
	}
	else {
		hGlobal = GetClipboardData(CF_TEXT);
		if (hGlobal != 0) // is equal "NULL" condition
		{
			auto pGlobal = (char*)GlobalLock(hGlobal);
			if (pGlobal == 0) {
				CloseClipboard();
				return NULL;
			}
			char *texty2 = (char *)malloc(strlen(pGlobal)*4 + 1);
			strcpy(texty2, pGlobal);
			texty2[strlen(pGlobal)] = 0;
			wchar_t *texty = (wchar_t *)malloc(strlen(texty2) + 1);
			auto n = MultiByteToWideChar(CP_ACP, 0, texty2, strlen(texty2), texty, strlen(texty2) + 1);
			texty[n] = 0;
			free(texty2);
			if (GlobalUnlock(hGlobal) == FALSE) {
				CloseClipboard();
				return NULL;
			}
			if (CloseClipboard() == FALSE) {
				return NULL;
			}
			return texty;
		}
	}
	auto eele = GetLastError();
	return NULL;
}


BOOL SetClipboardText(const wchar_t* pszData, const int nDataLen)
{
	while (::OpenClipboard(NULL) == FALSE) {
		Sleep(1000);
	}
	if (::EmptyClipboard() == FALSE) {
		CloseClipboard();
		return FALSE;
	}
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, (nDataLen + 1) * sizeof(wchar_t));
	if (hMem == 0) {
		CloseClipboard();
		return FALSE;
	}
	wchar_t *bufferbuffer = (wchar_t *)::GlobalLock(hMem);
	if (bufferbuffer == 0) {
		CloseClipboard();
		return FALSE;
	}
	wcscpy_s(bufferbuffer, nDataLen + 1, pszData);
	::GlobalUnlock(hMem);
	if (::SetClipboardData(CF_UNICODETEXT, hMem) == FALSE) {
		return FALSE;
	}
	if (::CloseClipboard() == FALSE) {
		return FALSE;
	}
	return TRUE;
}


char *p1 = 0;
char *p2 = 0;

DWORD WINAPI cppastethrd(LPVOID p) {
	auto aa = GetClipboardText(); 
	Sleep(800);
	auto p0=(char*)p;
	if (strlen(p0) > 10240) {
		return 0;
	}
	if (p1 == 0) {
		p1 = (char*)malloc(10240);
	}
	if (p2 == 0) {
		p2 = (char*)malloc(10240);
	}
	//auto p1 = (char*)malloc(strlen(p0)*4 * 1);
	//for (int i = 0; i < strlen(p0) * 4+1; i += 1){
	//	p1[i] = 0;
	//}
	dp(p0);
	int p1i = 0;
	for (int i = 0; i < strlen(p0); i += 1) {
		if (i + 1<strlen(p0) && p0[i] == '\\' && p0[i + 1] == 'n') {
			p1[p1i] = '\n';
			p1i += 1;
			i += 1;
		}
		else if (i + 1<strlen(p0) && p0[i] == '\\' && p0[i + 1] == 'r') {
			p1[p1i] = '\r';
			p1i += 1;
			i += 1;
		}
		else if (i + 1<strlen(p0) && p0[i] == '\\' && p0[i + 1] == 't') {
			p1[p1i] = '\t';
			p1i += 1;
			i += 1;
		}
		else {
			p1[p1i] = p0[i];
			p1i += 1;
		}
	}
	dp(", %d\n", p1i);
	p1[p1i] = 0;

	//replace {cd} 为现剪切板内容"{cd}",{cd}
	if (strstr(p1, "{cd}") != 0&&wcslen(aa)>0 &&wcslen(aa)<2500) {
		//dp("strlen(p1)", strlen(p1), wcslen(aa) * 4 + 1); 
		
		char aa2[10240];
		for (int j = 0; j < 10240; j += 1) {
			aa2[j] = 0;
		}
		WideCharToMultiByte(CP_UTF8, 0, aa, wcslen(aa), aa2, wcslen(aa) * 4 + 1, 0, 0);
		//wcstombs(aa2, aa, wcslen(aa));
		dp("aa2 %d  p1 %d\n", aa2, strlen(p1));
		auto p2i = 0;
		auto cnt = 0;
		for (int i = 0; i < p1i; i += 1) {
			if (i+3<strlen(p1)&&  p1[i] == '{'&&p1[i + 1] == 'c'&&p1[i + 2] == 'd'&&p1[i + 3] == '}') {
				for (int j = 0; j < strlen(aa2); j += 1) {
					//dp("aa2 j %d %d %d\n", j, strlen(p1), strlen(aa2));
					p2[p2i] = aa2[j];
					//dp("aa2 j %d %d %d\n", j, strlen(p1), strlen(aa2));
					p2i += 1;
					//dp("aa2 j %d %d %d\n", j, strlen(p1), strlen(aa2));
				}
				//strcat(p2, aa2);
				
				i += 3;
				cnt += 1;
				//dp("p1 i %d %d %d\n",i, strlen(p1), p2i);
				if (cnt > 9) {
					break;
				}
			}
			else {
				p2[p2i] = p1[i];
				p2i += 1;
				//dp("p2i %d %d %d\n", p2i, i, strlen(p1));
			}
		}
		p2[p2i] = 0;
		//dp("end");
		//delete[]p1;
		//delete[]aa2;
		SetClipboardTextA(p2, strlen(p2));
	}
	else {
		SetClipboardTextA(p1, strlen(p1));
	}
	keybd_event(VK_LCONTROL, 0, 0, 0);
	keybd_event('V', 0, 0, 0);
	keybd_event('V', 0, KEYEVENTF_KEYUP, 0); 
	keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
	Sleep(800);
	if (aa != 0) {
		SetClipboardText(aa, wcslen(aa));
	}
	//free(p1);
	return 0;
}

DWORD WINAPI typecthrd(LPVOID p) {
	Sleep(500);
	char *p1 = (char*)p;
	for (int i = 0; i < strlen(p1); i += 1) {
		if (p1[i] >= 65 && p1[i] <= 90) {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event(p1[i], 0, 0, 0);
			keybd_event(p1[i], 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
		}else if (p1[i] >= 97 && p1[i] <= 122) {
			keybd_event(p1[i]-32, 0, 0, 0);
			keybd_event(p1[i]-32, 0, KEYEVENTF_KEYUP, 0);
		}
		else if (p1[i] == '=') {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event(VK_OEM_PLUS, 0, 0, 0);
			keybd_event(VK_OEM_PLUS, 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
		}
		else if (p1[i] == '(') {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event('9', 0, 0, 0);
			keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
		}
		else if (p1[i] == ')') {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event('0', 0, 0, 0);
			keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
		}
		else if (p1[i] == '{') {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event(VK_OEM_4, 0, 0, 0);
			keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
		}
		else if (p1[i] == '}') {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event(VK_OEM_6, 0, 0, 0);
			keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
		}
		else if (p1[i] == '.') {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event(VK_OEM_PERIOD, 0, 0, 0);
			keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
		}
		else if (p1[i]=='\\' && p1[i+1] == 'n') {
			keybd_event(VK_RETURN, 0, 0, 0);
			keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
			i += 1;
		}else if (p1[i] == '\\' && p1[i + 1] == 'E' && p1[i + 2] == 'S') {
			keybd_event(VK_ESCAPE, 0, 0, 0);
			keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '1') {
			keybd_event(VK_F1, 0, 0, 0);
			keybd_event(VK_F1, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '2') {
			keybd_event(VK_F2, 0, 0, 0);
			keybd_event(VK_F2, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '3') {
			keybd_event(VK_F3, 0, 0, 0);
			keybd_event(VK_F3, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '4') {
			keybd_event(VK_F4, 0, 0, 0);
			keybd_event(VK_F4, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '5') {
			keybd_event(VK_F5, 0, 0, 0);
			keybd_event(VK_F5, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '6') {
			keybd_event(VK_F6, 0, 0, 0);
			keybd_event(VK_F6, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '7') {
			keybd_event(VK_F7, 0, 0, 0);
			keybd_event(VK_F7, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '8') {
			keybd_event(VK_F8, 0, 0, 0);
			keybd_event(VK_F8, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == '9') {
			keybd_event(VK_F9, 0, 0, 0);
			keybd_event(VK_F9, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == 'A') {
			keybd_event(VK_F10, 0, 0, 0);
			keybd_event(VK_F10, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == 'B') {
			keybd_event(VK_F11, 0, 0, 0);
			keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'F' && p1[i + 2] == 'C') {
			keybd_event(VK_F12, 0, 0, 0);
			keybd_event(VK_F12, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'S' && p1[i + 2] == 'S') {
			keybd_event(VK_SNAPSHOT, 0, 0, 0);
			keybd_event(VK_SNAPSHOT, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'P' && p1[i + 2] == 'A') {
			keybd_event(VK_PAUSE, 0, 0, 0);
			keybd_event(VK_PAUSE, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'I' && p1[i + 2] == 'N') {
			keybd_event(VK_INSERT, 0, 0, 0);
			keybd_event(VK_INSERT, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'D' && p1[i + 2] == 'E') {
			keybd_event(VK_DELETE, 0, 0, 0);
			keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'G' && p1[i + 2] == 'A') {
			keybd_event(VK_OEM_3, 0, 0, 0);
					keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'M' && p1[i + 2] == 'I') {
			keybd_event(VK_OEM_MINUS, 0, 0, 0);
					keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'P' && p1[i + 2] == 'L') {
			keybd_event(VK_OEM_PLUS, 0, 0, 0);
					keybd_event(VK_OEM_PLUS, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'B' && p1[i + 2] == 'A') {
			keybd_event(VK_BACK, 0, 0, 0);
						keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
						i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'H' && p1[i + 2] == 'O') {
			keybd_event(VK_HOME, 0, 0, 0);
					keybd_event(VK_HOME, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\t' || p1[i] == '\\' && p1[i + 1] == 'T' && p1[i + 2] == 'A') {
			keybd_event(VK_TAB, 0, 0, 0);
					keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
					if(p1[i] != '\t')i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'L' && p1[i + 2] == 'S') {
			keybd_event(VK_OEM_4, 0, 0, 0);
					keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'R' && p1[i + 2] == 'S') {
			keybd_event(VK_OEM_6, 0, 0, 0);
					keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'L' && p1[i + 2] == 'S') {
			keybd_event(VK_OEM_5, 0, 0, 0);
					keybd_event(VK_OEM_5, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'P' && p1[i + 2] == 'U') {
			keybd_event(VK_PRIOR, 0, 0, 0);
					keybd_event(VK_PRIOR, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'C' && p1[i + 2] == 'A') {
			keybd_event(VK_CAPITAL, 0, 0, 0);
					keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'C' && p1[i + 2] == 'N') {
			keybd_event(VK_OEM_1, 0, 0, 0);
					keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'Q' && p1[i + 2] == 'U') {
			keybd_event(VK_OEM_7, 0, 0, 0);
					keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'R' && p1[i + 2] == 'E') {
			keybd_event(VK_RETURN, 0, 0, 0);
					keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'P' && p1[i + 2] == 'D') {
			keybd_event(VK_NEXT, 0, 0, 0);
					keybd_event(VK_NEXT, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'L' && p1[i + 2] == 'S') {
			keybd_event(VK_LSHIFT, 0, 0, 0);
			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'C' && p1[i + 2] == 'A') {
			keybd_event(VK_OEM_COMMA, 0, 0, 0);
					keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'D' && p1[i + 2] == 'O') {
			keybd_event(VK_OEM_PERIOD, 0, 0, 0);
					keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'R' && p1[i + 2] == 'S') {
			keybd_event(VK_OEM_2, 0, 0, 0);
					keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'R' && p1[i + 2] == 'S') {
			keybd_event(VK_RSHIFT, 0, 0, 0);
						keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
						i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'U' && p1[i + 2] == 'P') {
			keybd_event(VK_UP, 0, 0, 0);
						keybd_event(VK_UP, 0, KEYEVENTF_KEYUP, 0);
						i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'E' && p1[i + 2] == 'D') {
			keybd_event(VK_END, 0, 0, 0);
					keybd_event(VK_END, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'R' && p1[i + 2] == 'S') {
			keybd_event(VK_LCONTROL, 0, 0, 0);
			keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'L' && p1[i + 2] == 'W') {
			keybd_event(VK_LWIN, 0, 0, 0);
						keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
						i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'L' && p1[i + 2] == 'A') {
			keybd_event(VK_LMENU, 0, 0, 0);
			keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'S' && p1[i + 2] == 'P') {
			keybd_event(VK_SPACE, 0, 0, 0);
					keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == ' ' || p1[i] == '\\' && p1[i + 1] == 'S' && p1[i + 2] == 'P') {
			keybd_event(VK_RMENU, 0, 0, 0);
			keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
			if (p1[i] != ' ')i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'A' && p1[i + 2] == 'P') {
			keybd_event(VK_APPS, 0, 0, 0);
					keybd_event(VK_APPS, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'R' && p1[i + 2] == 'C') {
			keybd_event(VK_RCONTROL, 0, 0, 0);
			keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
			i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'L' && p1[i + 2] == 'T') {
			keybd_event(VK_LEFT, 0, 0, 0);
					keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
					i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'D' && p1[i + 2] == 'N') {
			keybd_event(VK_DOWN, 0, 0, 0);
						keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0);
						i += 2;
		}
		else if (p1[i] == '\\' && p1[i + 1] == 'R' && p1[i + 2] == 'T') {
			keybd_event(VK_RIGHT, 0, 0, 0);
					keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
			}
		else {
			keybd_event(p1[i], 0, 0, 0);
			keybd_event(p1[i], 0, KEYEVENTF_KEYUP, 0);
		}
		Sleep(10);
	}
	return 0;
}



DWORD WINAPI winleftdragthrd(LPVOID p) {
	Sleep(333);
	char tit[32] = { 0 };
	auto hwnd = GetTopWindow(0);
	GetWindowTextA(hwnd, tit, 32);
	dp("tit%s %x\n", tit, hwnd);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_MOVE, -GStep, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	return 0;
	//CHAR tit[32] = { 0 };
	//POINT pt;
	//::GetCursorPos(&pt);
	//auto hwnd = WindowFromPoint(pt);
	//hwnd = GetParent(hwnd);
	//hwnd = GetParent(hwnd);
	//GetWindowTextA(hwnd, tit, 32);
	//RealGetWindowClassA(hwnd, tit, 32);
	//dp("tit to left %s %x %d %d\n", tit, hwnd,pt.x, pt.y);
	//if(strcmp("TaskManagerWindow", tit) == 0) {
	//	RECT  rect;
	//	GetWindowRect(hwnd, &rect);
	//	dp("titlll to left %s %x %d %d\n", tit, hwnd, rect.left, rect.top);
	//	MoveWindow(hwnd, rect.left - GStep, rect.right, 0, 0, FALSE);
	//	SetWindowPos(hwnd, 0, 0, rect.right, 0, 0, SWP_NOSIZE| SWP_SHOWWINDOW);
	//	SendMessage(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x-GStep, pt.y));

	//}
	//else {
	//	//Sleep(50);
	//	//SendMessage(hwnd, WM_LBUTTONDOWN, 0, MAKELPARAM(pt.x,pt.y));
	//	//Sleep(50);
	//	//SendMessage(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x-GStep, pt.y));
	//	//Sleep(100);
	//	//SendMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
	//	//return 0;
	//	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	//	Sleep(10);
	//	mouse_event(MOUSEEVENTF_MOVE, -GStep, 0, 0, 0);
	//	Sleep(10);
	//	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	//}
	//return 0;
}


DWORD WINAPI winrightdragthrd(LPVOID p) {
	Sleep(333);
	char tit[32] = { 0 };
	auto hwnd = GetTopWindow(0);
	GetWindowTextA(hwnd, tit, 32);
	dp("tit%s %x\n", tit, hwnd);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_MOVE, GStep, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	return 0;
}






DWORD WINAPI winaltins_ldblclick(LPVOID p) {
	Sleep(333);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	return 0;
}

DWORD WINAPI winaltdel_lclick(LPVOID p) {
	Sleep(333); 
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	return 0;
}

DWORD WINAPI ctrlalthm_mdblclick(LPVOID p) {
	Sleep(333); 
	mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
	return 0;
}

DWORD WINAPI ctrlalted_mclick(LPVOID p) {
	Sleep(333); 
	mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
	return 0;
}


DWORD WINAPI winaltpgup_rdblclick(LPVOID p) {
	Sleep(333); 
	mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	return 0;
}
DWORD WINAPI winaltpgdn_rclick(LPVOID p) {
	Sleep(333);
	mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
	Sleep(10);
	mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	return 0;
}

			
			
bool middleupscrollleft = false;
bool middleupscrollright = false;
DWORD WINAPI wheelupscrollthrd(LPVOID p) {
	return 0;
	auto whin = (POINT*)p; 
	//Sleep(200);
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, whin->x - 10, whin->y - 10, 0, 0);
	//Sleep(20);
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, whin->x - 10, whin->y - 10, 0, 0);
	//Sleep(120);
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, whin->x + 10, whin->y + 10, 0, 0);
	//Sleep(120);
	//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, whin->x + 10, whin->y + 10, 0, 0);
	Sleep(500);
	for (; true;) {
		if (middleupscrollleft || middleupscrollright) {
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, WHEEL_DELTA, 0);
			Sleep(2000);
		}
		else {
			break;
		}
	}
	delete(whin);
	return 0;
}

bool middledownscrollleft = false;
bool middledownscrollright = false;
DWORD WINAPI wheeldownscrollthrd(LPVOID p) {
	return 0;
	auto whin = (SheellInfo*)p;
	//Sleep(200);
	//mouse_event(MOUSEEVENTF_MOVE, whin->x - 10, whin->y - 10, 0, 0);
	//Sleep(20);
	//mouse_event(MOUSEEVENTF_MOVE, whin->x - 10, whin->y - 10, 0, 0);
	//Sleep(120);
	//mouse_event(MOUSEEVENTF_MOVE, whin->x + 10, whin->y + 10, 0, 0);
	//Sleep(120);
	//mouse_event(MOUSEEVENTF_MOVE, whin->x + 10, whin->y + 10, 0, 0);
	Sleep(500);
	for (; true;) {
		if (middledownscrollleft || middledownscrollright) {
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, -WHEEL_DELTA, 0);
			Sleep(2000);
		}
		else {
			break;
		}
	}
	delete(whin);
	return 0;
}

NOTIFYICONDATA nid; 
HANDLE checkgithubthrdh=0;
DWORD WINAPI checkgithubthrd(LPVOID p) {
	auto hWnd = (HWND)p;
	for (; true;) {
		auto rl = URLDownloadToFile(NULL, L"https://GitHub.com/", L"test_website_can_visit.out", 0, 0);
		dp("test download github rl %d\n",rl);
		if ( rl== S_OK) {
			auto f = _wfopen(L"test_website_can_visit.out", L"rb");
			auto fsize = fseek(f, 0, SEEK_END);
			fclose(f);
			dp("test download github middle\n");
			if (fsize > 1024) {
				Shell_NotifyIcon(NIM_DELETE, &nid);
				nid.cbSize = sizeof(nid);
				nid.hWnd = hWnd;
				nid.uID = 86450;
				nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
				nid.uCallbackMessage = WM_USER;
				nid.hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON3));
				lstrcpy(nid.szTip, L"CanDefKeyboard");
				Shell_NotifyIcon(NIM_ADD, &nid);
			}
			else {
				Shell_NotifyIcon(NIM_DELETE, &nid);
				nid.cbSize = sizeof(nid);
				nid.hWnd = hWnd;
				nid.uID = 86450;
				nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
				nid.uCallbackMessage = WM_USER;
				nid.hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));
				lstrcpy(nid.szTip, L"CanDefKeyboard");
				Shell_NotifyIcon(NIM_ADD, &nid);
			}
		}
		else {
			Shell_NotifyIcon(NIM_DELETE, &nid);
			nid.cbSize = sizeof(nid);
			nid.hWnd = hWnd;
			nid.uID = 86450;
			nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			nid.uCallbackMessage = WM_USER;
			nid.hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));
			lstrcpy(nid.szTip, L"CanDefKeyboard");
			Shell_NotifyIcon(NIM_ADD, &nid);
		}
		dp("test download github end\n");
		Sleep(30000);
	}
	return 0;
}





DWORD WINAPI altscrollupthrd(LPVOID p) {
	auto whin = (SheellInfo*)p;
	Sleep(100);
	mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, GStep, 0);
	delete(whin);
	return 0;
}

DWORD WINAPI altscrolldownthrd(LPVOID p) {
	auto whin = (SheellInfo*)p;
	Sleep(100);
	mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, -GStep, 0);// -WHEEL_DELTA, 0);
	delete(whin);
	return 0;
}





#define MAX_LOADSTRING 100



// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	DPIInit();
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CANDEFKEYBOARD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CANDEFKEYBOARD));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;// | WS_EX_NOACTIVATE | WS_EX_TOPMOST;// WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_EX_NOACTIVATE | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
	wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CANDEFKEYBOARD));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = 0;// MAKEINTRESOURCEW(IDC_CANDEFKEYBOARD);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_CANDEFKEYBOARD));

    return RegisterClassExW(&wcex);
}
int msx, msy;
bool mouseupMoveWin=false;
int repeatvk = -1;
bool brepeat = false;
BOOL bCaps;
bool autokilltabtip = true;
wchar_t g_LDownKeyName[10] = { 0 };
bool showkeytip = 1;
VOID CALLBACK TimeProc(HWND hWnd, UINT msg, UINT_PTR tid, DWORD)
{
	if (tid == 1988) {
		if (IsWindowVisible(hWnd)) {
			RECT rt;
			GetClientRect(hWnd, &rt);
			POINT pt;
			pt.x = 0;
			pt.y = 0;
			ClientToScreen(hWnd, &pt);
			if (pt.x < 0)pt.x = 0;
			if (pt.y < 0)pt.y = 0;
			SetWindowPos(hWnd, NULL, pt.x + 1, pt.y + 1, 0, 0, SWP_NOSIZE);
			SetWindowPos(hWnd, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE);
			int   cx = GetSystemMetrics(SM_CXFULLSCREEN); //得到宽度
			int   cy = GetSystemMetrics(SM_CYFULLSCREEN); //得到高度
			POINT point;
			::GetCursorPos(&point);
			if (pt.x + (rt.right - rt.left) > cx) {
				SetWindowPos(hWnd, NULL, cx- (rt.right - rt.left), pt.y, 0, 0, SWP_NOSIZE);
			}else if (pt.y + (rt.bottom - rt.top- ydpi*0.75) > cy) {
				SetWindowPos(hWnd, NULL, pt.x, cy- (rt.bottom - rt.top- ydpi*0.75), 0, 0, SWP_NOSIZE);
			}
			else if (mouseupMoveWin) {
				SetWindowPos(hWnd, NULL, point.x - msx, point.y - msy, 0, 0, SWP_NOSIZE);
			}
			if (!Shell_NotifyIcon(NIM_MODIFY, &nid)) {
				Shell_NotifyIcon(NIM_ADD, &nid);
			}
			auto bCaps2 = GetKeyState(VK_CAPITAL) == 1 ? TRUE : FALSE;
			if (bCaps2 != bCaps) {
				bCaps = bCaps2;
				InvalidateRect(hWnd, &rt, TRUE);
			}
			//if (showkeytip == true) {
			//	if (point.x > rt.left && point.x<rt.right && point.y>rt.top && point.y < rt.bottom) {

			//	}
			//	else if (g_LDownKeyName[0] != 0) {
			//		g_LDownKeyName[0] = 0;
			//		InvalidateRect(hWnd, &rt, TRUE);
			//	}
			//}
		}
		else {
			if (autokilltabtip) {
				KillProcessidByName(L"TabTip.exe");
			}
		}
	}
	else if (tid == 1987) {
		KillTimer(hWnd, 1987);
		SetTimer(hWnd, 1986, 50, TimeProc);
	}
	else if (tid == 1986) {
		//dp("timer2 %i\n", repeatvk);
		if (repeatvk != -1) {
			brepeat = true;
			keybd_event(repeatvk, 0, 0, 0);
			keybd_event(repeatvk, 0, KEYEVENTF_KEYUP, 0);
		}
	}
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
wchar_t *f4v=NULL;
wchar_t *f5v = NULL;
wchar_t *f6v = NULL;
HWND hWnd = 0;
float width = 3.8;
float height = 1.5; //应该是英寸
wchar_t  zyh[64]=L"S";//shift;
UINT restartmsg;
int dblsize = 1;
int lastposx=0, lastposy=0;
#define KBHOTKEYID 44
#define HK_ALTSHIFT_K_KILLTIP 45
#define HK_AUP_ID 46
#define HK_ADN_ID 47
#define HK_ALT_ID 48
#define HK_ART_ID 49


#define HK_SAH_ID 50
#define HK_SAU_ID 51
#define HK_SAD_ID 52
#define HK_SAL_ID 53
#define HK_SAR_ID 54
#define HK_SAQ_ID 55


#define HK_SAHM_ID 56
#define HK_SAED_ID 57
#define HK_SAPU_ID 58
#define HK_SAPD_ID 59

#define HK_CAUP_ID 60
#define HK_CADN_ID 61
#define HK_CALT_ID 62
#define HK_CART_ID 63


#define HK_ALT_1 64
#define HK_ALT_2 65
#define HK_ALT_3 66

#define HK_AINS_ID 67
#define HK_ADEL_ID 68


#define HK_SALU_ID 69
#define HK_SAMU_ID 70
#define HK_SARU_ID 71
#define HK_SALD_ID 72
#define HK_SAMD_ID 73
#define HK_SARD_ID 74

#define HK_ALTBACK_SCROLLUP 75
#define HK_ALTOR_SCROLLDN 76

#define HK_SAGITHUB_ID 77

#define HK_CAHM_WHEELUP 78
#define HK_CAED_WHEELDOWN 79


#define HK_WINS_LDBLCLK_ID 80
#define HK_WDEL_LCLK_ID 81
#define HK_WPGUP_RDBLCLK_ID 82
#define HK_WPGUP_RCLK_ID 83



#define HK_WL_DRAGL_ID 84
#define HK_WR_DRAGR_ID 85
#define HK_WU_DRAGU_ID 86
#define HK_WD_DRAGD_ID 87

#define HK_WINC_SHOWHIDEKB_ID 88

//#define HK_ALTCOMMA_MB2P_ID 89
//#define HK_ALTPRERIOD_MB2P_ID 90
//#define HK_ALTLB_MB2P_ID 91
//#define HK_ALTCONON_MB2P_ID 92


#define HK_SAPLUS_LU_ID 93
#define HK_SARB_LD_ID 94
#define HK_SAB_RU_ID 95
#define HK_SABS_RD_ID 96

//#define HK_CTRLSHIFT_U_ID 97
//#define HK_CTRLSHIFT_D_ID 98
//#define HK_CTRLSHIFT_L_ID 99
//#define HK_CTRLSHIFT_R_ID 100

#define HK_CTRLSHIFT_BACK_FASTSU 101
#define HK_CTRLSHIFT_OR_FASTSD 102
#define HK_ALT_4 103
#define HK_ALTSHIFT_PLUS_SU 104
#define HK_ALTSHIFT_BRACKET_SD 105


#define HK_ALTDOT_TOLEFTSLASH 106
#define HK_ALTRIGHTSLASH_TORIGHTSLASH 107
#define HK_ALTCOMMA_TOMINGWPATH 108

#define HK_ALT0 109
//#define HK_ALT1 110
//#define HK_ALT2 111
//#define HK_ALT3 112
//#define HK_ALT4 113
#define HK_ALT5 114
#define HK_ALT6 115
#define HK_ALT7 116
#define HK_ALT8 117
#define HK_ALT9 118
#define HK_ALTQUOT 119

char *Alt0=0, *Alt1 = 0, *Alt2 = 0, *Alt3 = 0, *Alt4 = 0, *Alt5 = 0, *Alt6 = 0, *Alt7 = 0, *Alt8 = 0, *Alt9 = 0;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HANDLE m_hMutex = CreateMutex(NULL, TRUE, L"Candefkeyboard");
	DWORD dwRet = GetLastError();
	if (m_hMutex)
	{
		if (ERROR_ALREADY_EXISTS == dwRet)
		{
			keybd_event(VK_CONTROL, 0, 0, 0);
			keybd_event(VK_MENU, 0, 0, 0);
			keybd_event('8', 0, 0, 0);
			keybd_event('8', 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
			keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
			

			printf("程序已经在运行中了!\n");
			CloseHandle(m_hMutex);
			return FALSE;
		}
	}
   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindowEx(WS_EX_NOACTIVATE| WS_EX_LAYERED|WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,//WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ::SetLayeredWindowAttributes(hWnd, RGB(251, 255, 242), 160, LWA_ALPHA|LWA_COLORKEY);

   ShowWindow(hWnd, SW_HIDE);// nCmdShow);
   UpdateWindow(hWnd);


   restartmsg = RegisterWindowMessage(L"TaskbarCreated");
   HKEY pk;
   RegOpenKey(HKEY_CURRENT_USER, L"Software\\CanDefKeyboard", &pk);
   TCHAR wText[100] = { 0 };
   DWORD dwSize = sizeof(wText);
   DWORD dwType = REG_SZ;
   auto rrr=RegQueryValueEx(pk, L"lastpos", NULL, &dwType, (LPBYTE)&wText, &dwSize);
   dwSize = 64;
   auto rrr2 = RegQueryValueEx(pk, L"CnEnHotKey", NULL, &dwType, (LPBYTE)&zyh, &dwSize);
   if (rrr2 != 0) {
	   wcscpy(zyh, L"S+");
	   RegSetValueEx(pk, L"CnEnHotKey", 0, REG_SZ, (BYTE*)zyh, sizeof(wchar_t)*wcslen(zyh));
   }
   wchar_t widthw[32];
   dwSize = 32;
   auto rrr3 = RegQueryValueEx(pk, L"width", NULL, &dwType, (LPBYTE)&widthw, &dwSize);
   if (rrr3 != 0) {
	   wcscpy(widthw, L"3.8");
	   RegSetValueEx(pk, L"width", 0, REG_SZ, (BYTE*)widthw, sizeof(wchar_t)*wcslen(widthw));
   }
   else {
	   width = _wtof(widthw);
   }
   wchar_t heightw[32];
   dwSize = 32;
   auto rrr4 = RegQueryValueEx(pk, L"height", NULL, &dwType, (LPBYTE)&heightw, &dwSize);
   if (rrr4 != 0) {
	   wcscpy(heightw, L"1.5");
	   RegSetValueEx(pk, L"height", 0, REG_SZ, (BYTE*)heightw, sizeof(wchar_t)*wcslen(heightw));
   }
   else {
	   height = _wtof(heightw);
   }



   wchar_t showkeytipw[32] = { 0 };
   auto rrr5 = RegQueryValueEx(pk, L"showkeytip", NULL, &dwType, (LPBYTE)&showkeytipw, &dwSize);
   if (rrr5 != 0) {
	   showkeytip = 1;
	   RegSetValueEx(pk, L"showkeytip", 0, REG_SZ, (BYTE*)L"1", sizeof(wchar_t)*1);
   }
   else {
	   showkeytip = _wtof(showkeytipw);
   }
   RegCloseKey(pk);
   int   cx = GetSystemMetrics(SM_CXFULLSCREEN); //得到宽度
   int   cy = GetSystemMetrics(SM_CYFULLSCREEN); //得到高度


   DPIInit();
   dp("dpi:%f,%f\n", xdpi, ydpi);
   if (wcschr(wText, ',') != 0 && _wtoi(wText)>=0&& _wtoi(wText)<=cx-width*xdpi && _wtoi(wcschr(wText, ',') + 1)>=0 && _wtoi(wcschr(wText, ',') + 1)<=cy-(height*ydpi-ydpi*0.75)) {
	   lastposx = _wtoi(wText);
	   lastposy = _wtoi(wcschr(wText, ',') + 1);
	   MoveWindow(hWnd, lastposx,lastposy, width*xdpi, height*ydpi, TRUE);
   }
   else {
	   MoveWindow(hWnd,0,0, width*xdpi, height*ydpi, TRUE);
   }

   SetTimer(hWnd, 1988, 2000, TimeProc);
   //SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
   //reset key state
   if (GetKeyState(VK_LSHIFT) < 0) {
	   keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
   }
   if (GetKeyState(VK_LCONTROL) < 0) {
	   keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
   }
   if (GetKeyState(VK_LWIN) < 0) {
	   keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
   }
   if (GetKeyState(VK_LMENU) < 0) {
	   keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
   }
   if (GetKeyState(VK_RSHIFT) < 0) {
	   keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
   }
   if (GetKeyState(VK_RCONTROL) < 0) {
	   keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
   }
   if (GetKeyState(VK_RMENU) < 0) {
	   keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
   }
   KillProcessidByName(L"TabTip.exe");
   bCaps = GetKeyState(VK_CAPITAL) == 1 ? TRUE : FALSE;


   auto ff = fopen("CandefKeyboard.conf", "rb");
   if (ff != 0) {
	   fseek(ff, 0, SEEK_END);
	   auto ffend = ftell(ff);
	   fseek(ff, 0, SEEK_SET);
	   auto ffbuf = new char[ffend + 1];
	   fread(ffbuf, 1, ffend, ff);
	   ffbuf[ffend] = 0;
	   fclose(ff);
	   auto pfb = ffbuf;
	   for (; true;) {
		   if (strncmp(pfb, "GStepBase=", strlen("GStepBase="))==0) {
			   GStepBase = strtol(pfb + strlen("GStepBase="), 0, 10);
			   if (GStepBase < 1)GStepBase = 13;
		   }else if (strncmp(pfb, "Alt0=", strlen("Alt0=")) == 0) {
			   Alt0 = pfb + strlen("Alt0=");
		   }
		   else if (strncmp(pfb, "Alt5=", strlen("Alt5=")) == 0) {
			   Alt5 = pfb + strlen("Alt5=");
		   }
		   else if (strncmp(pfb, "Alt6=", strlen("Alt6=")) == 0) {
			   Alt6 = pfb + strlen("Alt6=");
		   }
		   else if (strncmp(pfb, "Alt7=", strlen("Alt7=")) == 0) {
			   Alt7 = pfb + strlen("Alt7=");
		   }
		   else if (strncmp(pfb, "Alt8=", strlen("Alt8=")) == 0) {
			   Alt8 = pfb + strlen("Alt8=");
		   }
		   else if (strncmp(pfb, "Alt9=", strlen("Alt9=")) == 0) {
			   Alt9 = pfb + strlen("Alt9=");
		   }
		   pfb = strchr(pfb, '\n');
		   if(pfb == 0){
			   break;
		   }
		   *pfb=0;
		   pfb += 1;
	   }
	   
   }
   else {
	   auto ff = fopen("CandefKeyboard.conf", "wb");
	   fwrite("GStepBase=13\n",1,strlen("GStepBase=13\n"),ff);
	   fclose(ff);
   }
   int kk = 343;
   auto aa = [kk](int a) {return GStepBase*a; };
   
   dp(Alt5);

   //BOOL bregok=RegisterHotKey(hWnd, KBHOTKEYID, MOD_CONTROL | MOD_ALT, '9');
   //dp("RegisterHotKey 1 Result:%d\n", bregok);
   BOOL bregok = RegisterHotKey(hWnd, HK_ALTSHIFT_K_KILLTIP, MOD_SHIFT | MOD_ALT, 'K');
   dp("RegisterHotKey 2 Result:%d\n", bregok);

   bregok = RegisterHotKey(hWnd, HK_AUP_ID, MOD_ALT, VK_UP);
   dp("RegisterHotKey win_up Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ADN_ID, MOD_ALT, VK_DOWN);
   dp("RegisterHotKey win_down Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT_ID, MOD_ALT, VK_LEFT);
   dp("RegisterHotKey win_left Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ART_ID, MOD_ALT, VK_RIGHT);
   dp("RegisterHotKey win_right Result:%d\n", bregok);

   bregok = RegisterHotKey(hWnd, HK_SAH_ID, MOD_SHIFT|MOD_ALT, 'H');
   dp("RegisterHotKey shift_alt_h Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAU_ID, MOD_SHIFT | MOD_ALT, VK_UP);
   dp("RegisterHotKey shift_alt_up Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAD_ID, MOD_SHIFT | MOD_ALT, VK_DOWN);
   dp("RegisterHotKey shift_alt_down Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAL_ID, MOD_SHIFT | MOD_ALT, VK_LEFT);
   dp("RegisterHotKey shift_alt_left Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAR_ID, MOD_SHIFT | MOD_ALT, VK_RIGHT);
   dp("RegisterHotKey shift_alt_right Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAQ_ID, MOD_SHIFT | MOD_ALT, 'Q');
   dp("RegisterHotKey shift_alt_q Result:%d\n", bregok);


   bregok = RegisterHotKey(hWnd, HK_SAHM_ID, MOD_ALT,VK_HOME);
   dp("RegisterHotKey alt_home left double click Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAED_ID, MOD_ALT, VK_END);
   dp("RegisterHotKey alt_end left click Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAPU_ID, MOD_ALT, VK_PRIOR);
   dp("RegisterHotKey alt_pageup double right click Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAPD_ID, MOD_ALT, VK_NEXT);
   dp("RegisterHotKey alt_pagedown right click Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CAUP_ID, MOD_CONTROL | MOD_ALT, VK_UP);
   dp("RegisterHotKey ctrl_alt_up drag to up Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CADN_ID, MOD_CONTROL | MOD_ALT, VK_DOWN);
   dp("RegisterHotKey ctrl_alt_down drag to down Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CALT_ID, MOD_CONTROL | MOD_ALT, VK_LEFT);
   dp("RegisterHotKey ctrl_alt_l drag to left Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CART_ID, MOD_CONTROL | MOD_ALT, VK_RIGHT);
   dp("RegisterHotKey ctrl_alt_r drag to right Result:%d\n", bregok);


   bregok = RegisterHotKey(hWnd, HK_ALT_1, MOD_ALT, '1');
   dp("RegisterHotKey alt_z step 10 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT_2, MOD_ALT, '2');
   dp("RegisterHotKey alt_x step 20 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT_3, MOD_ALT, '3');
   dp("RegisterHotKey alt_c step 40 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT_4, MOD_ALT, '4');
   dp("RegisterHotKey alt_c step 40 Result:%d\n", bregok);

   bregok = RegisterHotKey(hWnd, HK_AINS_ID, MOD_ALT, VK_INSERT);
   dp("RegisterHotKey alt_del wheel up Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ADEL_ID, MOD_ALT, VK_DELETE);
   dp("RegisterHotKey alt_ins wheel down Result:%d\n", bregok);


   bregok = RegisterHotKey(hWnd, HK_SALU_ID, MOD_SHIFT | MOD_ALT, VK_INSERT);
   dp("RegisterHotKey shift_alt_ins left up Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAMU_ID, MOD_SHIFT | MOD_ALT, VK_HOME);
   dp("RegisterHotKey shift_alt_del middle up Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SARU_ID, MOD_SHIFT | MOD_ALT, VK_PRIOR);
   dp("RegisterHotKey shift_alt_pgup right up Result:%d\n", bregok);


   bregok = RegisterHotKey(hWnd, HK_SALD_ID, MOD_SHIFT | MOD_ALT, VK_DELETE);
   dp("RegisterHotKey shift_alt_del left  down Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAMD_ID, MOD_SHIFT | MOD_ALT, VK_END);
   dp("RegisterHotKey shift_alt_end middle down Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SARD_ID, MOD_SHIFT | MOD_ALT, VK_NEXT);
   dp("RegisterHotKey shift_alt_pagedown  right down Result:%d\n", bregok);

   bregok = RegisterHotKey(hWnd, HK_ALTBACK_SCROLLUP, MOD_ALT, VK_BACK);
   dp("RegisterHotKey shift_alt_end middle down Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALTOR_SCROLLDN, MOD_ALT, VK_OEM_5);
   dp("RegisterHotKey shift_alt_pagedown  right down Result:%d\n", bregok);

   //bregok = RegisterHotKey(hWnd, HK_SAGITHUB_ID,MOD_SHIFT| MOD_ALT, 'G');
   //dp("RegisterHotKey shift_alt_g github visitable  right down Result:%d\n", bregok);

   bregok = RegisterHotKey(hWnd, HK_CAHM_WHEELUP, MOD_CONTROL | MOD_ALT, VK_HOME);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CAED_WHEELDOWN, MOD_CONTROL | MOD_ALT, VK_END);
   dp("RegisterHotKey 2 Result:%d\n", bregok);
   
   bregok = RegisterHotKey(hWnd, HK_WINS_LDBLCLK_ID, MOD_WIN , VK_INSERT);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_WDEL_LCLK_ID, MOD_WIN , VK_DELETE);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_WPGUP_RDBLCLK_ID, MOD_WIN , VK_PRIOR);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_WPGUP_RCLK_ID, MOD_WIN , VK_NEXT);
   dp("RegisterHotKey 1 Result:%d\n", bregok);



   bregok = RegisterHotKey(hWnd, HK_WU_DRAGU_ID, MOD_WIN, VK_HOME);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_WD_DRAGD_ID, MOD_WIN, VK_END);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_WL_DRAGL_ID, MOD_WIN, VK_HOME);
   //dp("RegisterHotKey 1 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_WR_DRAGR_ID, MOD_WIN, VK_END);
   //dp("RegisterHotKey 1 Result:%d\n", bregok);

   bregok = RegisterHotKey(hWnd, HK_WINC_SHOWHIDEKB_ID, MOD_ALT, VK_OEM_3);
   dp("RegisterHotKey 1 Result:%d\n", bregok);


   //bregok = RegisterHotKey(hWnd, HK_ALTCOMMA_MB2P_ID, MOD_ALT, VK_OEM_COMMA);
   //dp("RegisterHotKey 1 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_ALTPRERIOD_MB2P_ID, MOD_ALT, VK_OEM_PERIOD);
   //dp("RegisterHotKey 1 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_ALTLB_MB2P_ID, MOD_ALT, VK_OEM_1);
   //dp("RegisterHotKey 1 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_ALTCONON_MB2P_ID, MOD_ALT, VK_OEM_2);
   //dp("RegisterHotKey 1 Result:%d\n", bregok);



   bregok = RegisterHotKey(hWnd, HK_SAPLUS_LU_ID, MOD_SHIFT|MOD_ALT, VK_OEM_PLUS);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SARB_LD_ID, MOD_SHIFT | MOD_ALT, VK_OEM_6);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SAB_RU_ID, MOD_SHIFT | MOD_ALT, VK_BACK);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_SABS_RD_ID, MOD_SHIFT | MOD_ALT, VK_OEM_5);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
/*
   bregok = RegisterHotKey(hWnd, HK_CTRLSHIFT_U_ID, MOD_SHIFT | MOD_CONTROL, VK_OEM_6);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CTRLSHIFT_D_ID, MOD_SHIFT | MOD_CONTROL, VK_OEM_6);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CTRLSHIFT_L_ID, MOD_SHIFT | MOD_CONTROL, VK_OEM_6);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CTRLSHIFT_R_ID, MOD_SHIFT | MOD_CONTROL, VK_OEM_6);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   */

   bregok = RegisterHotKey(hWnd, HK_CTRLSHIFT_BACK_FASTSU, MOD_ALT | MOD_CONTROL, VK_BACK);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_CTRLSHIFT_OR_FASTSD, MOD_ALT | MOD_CONTROL, VK_OEM_5);
   dp("RegisterHotKey 1 Result:%d\n", bregok);


   bregok = RegisterHotKey(hWnd, HK_ALTSHIFT_PLUS_SU, MOD_ALT , VK_OEM_PLUS);
   dp("RegisterHotKey 1 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALTSHIFT_BRACKET_SD, MOD_ALT, VK_OEM_6);
   dp("RegisterHotKey 1 Result:%d\n", bregok);


   bregok = RegisterHotKey(hWnd, HK_ALTDOT_TOLEFTSLASH, MOD_ALT, VK_OEM_PERIOD);
   dp("RegisterHotKey HK_ALTDOT_TOLEFTSLASH Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALTRIGHTSLASH_TORIGHTSLASH, MOD_ALT, VK_OEM_2);
   dp("RegisterHotKey HK_ALTRIGHTSLASH_TORIGHTSLASH Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALTCOMMA_TOMINGWPATH, MOD_ALT, VK_OEM_COMMA);
   dp("RegisterHotKey HK_ALTRIGHTSLASH_TORIGHTSLASH Result:%d\n", bregok);

   
   bregok = RegisterHotKey(hWnd, HK_ALT0, MOD_ALT, '0');
   dp("RegisterHotKey HK_ALT0 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_ALT1, MOD_ALT, '1');
   //dp("RegisterHotKey HK_ALT1 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_ALT2, MOD_ALT, '2');
   //dp("RegisterHotKey HK_ALT2 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_ALT3, MOD_ALT, '3');
   //dp("RegisterHotKey HK_ALT3 Result:%d\n", bregok);
   //bregok = RegisterHotKey(hWnd, HK_ALT4, MOD_ALT, '4');
   //dp("RegisterHotKey HK_ALT4 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT5, MOD_ALT, '5');
   dp("RegisterHotKey HK_ALT5 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT6, MOD_ALT, '6');
   dp("RegisterHotKey HK_ALT6 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT7, MOD_ALT, '7');
   dp("RegisterHotKey HK_ALT7 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT8, MOD_ALT, '8');
   dp("RegisterHotKey HK_ALT8 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALT9, MOD_ALT, '9');
   dp("RegisterHotKey HK_ALT9 Result:%d\n", bregok);
   bregok = RegisterHotKey(hWnd, HK_ALTQUOT, MOD_ALT, VK_OEM_7);
   dp("RegisterHotKey HK_ALT9 Result:%d\n", bregok);


   return TRUE;
}



//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//


RECT startRect;
HMENU hmenu;//菜单句柄
#define IDR_QUIT 12
#define IDR_ABOUT 13
bool lshiftdown, lctrldown, lwindown, laltdown;
bool rshiftdown, rctrldown, raltdown;
bool lshiftlongdown, rshiftlongdown, lctrllongdown, rctrllongdown, laltlongdown, raltlongdown, lwinlongdown;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//dp("msg %i\n", message);
	if (message == restartmsg) {
		Shell_NotifyIcon(NIM_ADD, &nid);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
    switch (message)
    {
	case WM_POINTERUP:
		KillTimer(hWnd, 1987);
		KillTimer(hWnd,1986);
		repeatvk = -1;
		dp("brepeat %i\n", brepeat);
		if (brepeat) {
			if (lshiftdown) {
				keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				lshiftdown = false;
			}
			if (rshiftdown) {
				keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
				rshiftdown = false;
			}
			if (lctrldown) {
				keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				lctrldown = false;
			}
			if (rctrldown) {
				keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
				rctrldown = false;
			}
			if (laltdown) {
				keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
				laltdown = false;
			}
			if (raltdown) {
				keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
				raltdown = false;
			}
			if (lwindown) {
				keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
				lwindown = false;
			}
			brepeat = false;
			break;
		}
		else {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	case WM_POINTERDOWN:
	{
		brepeat = false;
		repeatvk = -1;
		int mouse_x = (int)LOWORD(lParam);//取低位
		int mouse_y = (int)HIWORD(lParam);//取高位
		dp("WM_POINTERDOWN pop %i,%i", mouse_x, mouse_y);
		POINT pt;
		pt.x = mouse_x;
		pt.y = mouse_y;
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);
		mouse_x = pt.x;
		mouse_y = pt.y;
		dp("WM_POINTERDOWN pop %i,%i", mouse_x, mouse_y);
										  //DPIInit();
		RECT rect;
		GetClientRect(hWnd, &rect);
		dp("WM_POINTERDOWN p enter %i,%i, %i,%i,%i,%i\n", mouse_x, mouse_y, rect.right, rect.bottom, rect.left, rect.top);
		if (mouse_x > width*xdpi / 16 * 12 && mouse_x<width*xdpi / 16 * 13 && mouse_y > rect.bottom - height*ydpi / 6) {
		}
		else {
			auto mx = mouse_x;
			auto my = mouse_y;

			if (my >= 0 && my <= height*ydpi / 6) {
				if (mx >= (width*xdpi) / 17 * 0 && mx <= (width*xdpi) / 17 * 1) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"ESC");
					}
					else {
						wcscpy(g_LDownKeyName, L"esc");
					}
					repeatvk = VK_ESCAPE;
				}
				else if (mx >= (width*xdpi) / 17 * 1 && mx <= (width*xdpi) / 17 * 2) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F1");
					}
					else {
						wcscpy(g_LDownKeyName, L"f1");
					}
					repeatvk = VK_F1;
				}
				else if (mx >= (width*xdpi) / 17 * 2 && mx <= (width*xdpi) / 17 * 3) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F2");
					}
					else {
						wcscpy(g_LDownKeyName, L"f2");
					}
					repeatvk = VK_F2;
				}
				else if (mx >= (width*xdpi) / 17 * 3 && mx <= (width*xdpi) / 17 * 4) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F3");
					}
					else {
						wcscpy(g_LDownKeyName, L"f3");
					}
					repeatvk = VK_F3;
				}
				else if (mx >= (width*xdpi) / 17 * 4 && mx <= (width*xdpi) / 17 * 5) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F4");
					}
					else {
						wcscpy(g_LDownKeyName, L"f4");
					}
					repeatvk = VK_F4;
				}
				else if (mx >= (width*xdpi) / 17 * 5 && mx <= (width*xdpi) / 17 * 6) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F5");
					}
					else {
						wcscpy(g_LDownKeyName, L"f5");
					}
					repeatvk = VK_F5;
				}
				else if (mx >= (width*xdpi) / 17 * 6 && mx <= (width*xdpi) / 17 * 7) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F6");
					}
					else {
						wcscpy(g_LDownKeyName, L"f6");
					}
					repeatvk = VK_F6;
				}
				else if (mx >= (width*xdpi) / 17 * 7 && mx <= (width*xdpi) / 17 * 8) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F7");
					}
					else {
						wcscpy(g_LDownKeyName, L"f7");
					}
					repeatvk = VK_F7;
				}
				else if (mx >= (width*xdpi) / 17 * 8 && mx <= (width*xdpi) / 17 * 9) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F8");
					}
					else {
						wcscpy(g_LDownKeyName, L"f8");
					}
					repeatvk = VK_F8;
				}
				else if (mx >= (width*xdpi) / 17 * 9 && mx <= (width*xdpi) / 17 * 10) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F9");
					}
					else {
						wcscpy(g_LDownKeyName, L"f9");
					}
					repeatvk = VK_F9;
				}
				else if (mx >= (width*xdpi) / 17 * 10 && mx <= (width*xdpi) / 17 * 11) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F10");
					}
					else {
						wcscpy(g_LDownKeyName, L"f10");
					}
					repeatvk = VK_F10;
				}
				else if (mx >= (width*xdpi) / 17 * 11 && mx <= (width*xdpi) / 17 * 12) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F11");
					}
					else {
						wcscpy(g_LDownKeyName, L"f11");
					}
					repeatvk = VK_F11;
				}
				else if (mx >= (width*xdpi) / 17 * 12 && mx <= (width*xdpi) / 17 * 13) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F12");
					}
					else {
						wcscpy(g_LDownKeyName, L"f12");
					}
					repeatvk = VK_F12;
				}
				else if (mx >= (width*xdpi) / 17 * 13 && mx <= (width*xdpi) / 17 * 14) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"PRI");
					}
					else {
						wcscpy(g_LDownKeyName, L"pri");
					}
					repeatvk = VK_SNAPSHOT;
				}
				else if (mx >= (width*xdpi) / 17 * 14 && mx <= (width*xdpi) / 17 * 15) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"PAU");
					}
					else {
						wcscpy(g_LDownKeyName, L"pau");
					}
					repeatvk = VK_PAUSE;
				}
				else if (mx >= (width*xdpi) / 17 * 15 && mx <= (width*xdpi) / 17 * 16) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"INS");
					}
					else {
						wcscpy(g_LDownKeyName, L"ins");
					}
					repeatvk = VK_INSERT;
				}
				else if (mx >= (width*xdpi) / 17 * 16 && mx <= (width*xdpi) / 17 * 17) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"INS");
					}
					else {
						wcscpy(g_LDownKeyName, L"ins");
					}
					repeatvk = VK_DELETE;
				}
			}
			else if (my >= height*ydpi / 6 && my <= height*ydpi / 6 * 2) {
				if (mx >= (width*xdpi) / 16 * 0 && mx <= (width*xdpi) / 16 * 1) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"TAB");
					}
					else {
						wcscpy(g_LDownKeyName, L"tab");
					}
					repeatvk = VK_OEM_3;
				}
				else if (mx >= (width*xdpi) / 16 * 1 && mx <= (width*xdpi) / 16 * 2) {
					wcscpy(g_LDownKeyName, L"1");
					repeatvk = '1';
				}
				else if (mx >= (width*xdpi) / 16 * 2 && mx <= (width*xdpi) / 16 * 3) {
					wcscpy(g_LDownKeyName, L"2");
					repeatvk = '2';
				}
				else if (mx >= (width*xdpi) / 16 * 3 && mx <= (width*xdpi) / 16 * 4) {
					wcscpy(g_LDownKeyName, L"3");
					repeatvk = '3';
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 5) {
					wcscpy(g_LDownKeyName, L"4");
					repeatvk = '4';
				}
				else if (mx >= (width*xdpi) / 16 * 5 && mx <= (width*xdpi) / 16 * 6) {
					wcscpy(g_LDownKeyName, L"5");
					repeatvk = '5';
				}
				else if (mx >= (width*xdpi) / 16 * 6 && mx <= (width*xdpi) / 16 * 7) {
					wcscpy(g_LDownKeyName, L"6");
					repeatvk = '6';
				}
				else if (mx >= (width*xdpi) / 16 * 7 && mx <= (width*xdpi) / 16 * 8) {
					wcscpy(g_LDownKeyName, L"7");
					repeatvk = '7';
				}
				else if (mx >= (width*xdpi) / 16 * 8 && mx <= (width*xdpi) / 16 * 9) {
					wcscpy(g_LDownKeyName, L"8");
					repeatvk = '8';
				}
				else if (mx >= (width*xdpi) / 16 * 9 && mx <= (width*xdpi) / 16 * 10) {
					wcscpy(g_LDownKeyName, L"9");
					repeatvk = '9';
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					wcscpy(g_LDownKeyName, L"0");
					repeatvk = '0';
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"_");
					}
					else {
						wcscpy(g_LDownKeyName, L"-");
					}
					repeatvk = VK_OEM_MINUS;
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"+");
					}
					else {
						wcscpy(g_LDownKeyName, L"=");
					}
					repeatvk = VK_OEM_PLUS;
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 15) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"BAC");
					}
					else {
						wcscpy(g_LDownKeyName, L"bac");
					}
					repeatvk = VK_BACK;
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"HOM");
					}
					else {
						wcscpy(g_LDownKeyName, L"hom");
					}
					repeatvk = VK_HOME;
				}
			}
			else if (my >= height*ydpi / 6 * 2 && my <= height*ydpi / 6 * 3) {
				float delta = 0;
				dp("tab %i %f\n", mx, (width*xdpi) / 16 * 15);
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16 / 2;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"TAB");
					}
					else {
						wcscpy(g_LDownKeyName, L"tab");
					}
					repeatvk = VK_TAB;
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"Q");
					}
					else {
						wcscpy(g_LDownKeyName, L"q");
					}
					repeatvk = 'Q';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"W");
					}
					else {
						wcscpy(g_LDownKeyName, L"w");
					}
					repeatvk = 'W';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"E");
					}
					else {
						wcscpy(g_LDownKeyName, L"e");
					}
					repeatvk = 'E';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"R");
					}
					else {
						wcscpy(g_LDownKeyName, L"r");
					}
					repeatvk = 'R';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"T");
					}
					else {
						wcscpy(g_LDownKeyName, L"t");
					}
					repeatvk = 'T';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"Y");
					}
					else {
						wcscpy(g_LDownKeyName, L"y");
					}
					repeatvk = 'Y';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"U");
					}
					else {
						wcscpy(g_LDownKeyName, L"u");
					}
					repeatvk = 'U';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"I");
					}
					else {
						wcscpy(g_LDownKeyName, L"i");
					}
					repeatvk = 'I';
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"O");
					}
					else {
						wcscpy(g_LDownKeyName, L"o");
					}
					repeatvk = 'O';
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"P");
					}
					else {
						wcscpy(g_LDownKeyName, L"p");
					}
					repeatvk = 'P';
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 12 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"{");
					}
					else {
						wcscpy(g_LDownKeyName, L"[");
					}
					repeatvk = VK_OEM_4;
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta&& mx <= (width*xdpi) / 16 * 13 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"}");
					}
					else {
						wcscpy(g_LDownKeyName, L"]");
					}
					repeatvk = VK_OEM_6;
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta && mx <= (width*xdpi) / 16 * 14 + delta + (width*xdpi) / 16 / 2) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"|");
					}
					else {
						wcscpy(g_LDownKeyName, L"\\");
					}
					repeatvk = VK_OEM_5;
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"PUP");
					}
					else {
						wcscpy(g_LDownKeyName, L"pup");
					}
					repeatvk = VK_PRIOR;
				}
			}
			else if (my >= height*ydpi / 6 * 3 && my <= height*ydpi / 6 * 4) {
				float delta = 0; dp("pgdn %i %f\n", mx, (width*xdpi) / 16 * 12);
				if (mx > (width*xdpi) / 16 * 1 + delta) {
					delta = (width*xdpi) / 16 * 0.75;
				}
				if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"CAP");
					}
					else {
						wcscpy(g_LDownKeyName, L"cap");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"A");
					}
					else {
						wcscpy(g_LDownKeyName, L"a");
					}
					repeatvk = 'A';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"S");
					}
					else {
						wcscpy(g_LDownKeyName, L"s");
					}
					repeatvk = 'S';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"D");
					}
					else {
						wcscpy(g_LDownKeyName, L"d");
					}
					repeatvk = 'D';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F");
					}
					else {
						wcscpy(g_LDownKeyName, L"f");
					}
					repeatvk = 'F';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"G");
					}
					else {
						wcscpy(g_LDownKeyName, L"g");
					}
					repeatvk = 'G';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"H");
					}
					else {
						wcscpy(g_LDownKeyName, L"h");
					}
					repeatvk = 'H';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"J");
					}
					else {
						wcscpy(g_LDownKeyName, L"j");
					}
					repeatvk = 'J';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"K");
					}
					else {
						wcscpy(g_LDownKeyName, L"k");
					}
					repeatvk = 'K';
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"L");
					}
					else {
						wcscpy(g_LDownKeyName, L"l");
					}
					repeatvk = 'L';
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L":");
					}
					else {
						wcscpy(g_LDownKeyName, L";");
					}
					repeatvk = VK_OEM_1;
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta&& mx <= (width*xdpi) / 16 * 12 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"\"");
					}
					else {
						wcscpy(g_LDownKeyName, L"'");
					}

					repeatvk = VK_OEM_7;
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta && mx <= (width*xdpi) / 16 * 13 + delta + (width*xdpi) / 16 * 1.25) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RET");
					}
					else {
						wcscpy(g_LDownKeyName, L"ret");
					}
					repeatvk = VK_RETURN;
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"PDO");
					}
					else {
						wcscpy(g_LDownKeyName, L"pdo");
					}
					repeatvk = VK_NEXT;
				}
			}
			else if (my >= height*ydpi / 6 * 4 && my <= height*ydpi / 6 * 5) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16;
				}
				if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LSHI");
					}
					else {
						wcscpy(g_LDownKeyName, L"lsh");
					}
					repeatvk = 'Z';
				}else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"Z");
					}
					else {
						wcscpy(g_LDownKeyName, L"z");
					}
					repeatvk = 'Z';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"X");
					}
					else {
						wcscpy(g_LDownKeyName, L"x");
					}
					repeatvk = 'X';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"C");
					}
					else {
						wcscpy(g_LDownKeyName, L"c");
					}
					repeatvk = 'C';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"V");
					}
					else {
						wcscpy(g_LDownKeyName, L"v");
					}
					repeatvk = 'V';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"B");
					}
					else {
						wcscpy(g_LDownKeyName, L"b");
					}
					repeatvk = 'B';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"N");
					}
					else {
						wcscpy(g_LDownKeyName, L"n");
					}
					repeatvk = 'N';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"M");
					}
					else {
						wcscpy(g_LDownKeyName, L"m");
					}
					repeatvk = 'M';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"<");
					}
					else {
						wcscpy(g_LDownKeyName, L",");
					}
					repeatvk = VK_OEM_COMMA;
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L">");
					}
					else {
						wcscpy(g_LDownKeyName, L".");
					}
					repeatvk = VK_OEM_PERIOD;
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"?");
					}
					else {
						wcscpy(g_LDownKeyName, L"/");
					}
					repeatvk = VK_OEM_2;
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 13 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RSH");
					}
					else {
						wcscpy(g_LDownKeyName, L"rsh");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta&& mx <= (width*xdpi) / 16 * 14 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"UP");
					}
					else {
						wcscpy(g_LDownKeyName, L"up");
					}
					repeatvk = VK_UP;
				}
				else if (mx >= (width*xdpi) / 16 * 14 + delta && mx <= (width*xdpi) / 16 * 15 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"END");
					}
					else {
						wcscpy(g_LDownKeyName, L"end");
					}
					repeatvk = VK_END;
				}
			}
			else if (my >= height*ydpi / 6 * 5 && my <= height*ydpi / 6 * 6) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 9) {
					delta = (width*xdpi) / 16 * 4;
				}

				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LCT");
					}
					else {
						wcscpy(g_LDownKeyName, L"lct");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					wcscpy(g_LDownKeyName, L"C/E");
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LWI");
					}
					else {
						wcscpy(g_LDownKeyName, L"lwi");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LAL");
					}
					else {
						wcscpy(g_LDownKeyName, L"lal");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 9) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"SPA");
					}
					else {
						wcscpy(g_LDownKeyName, L"spa");
					}
					repeatvk = VK_SPACE;
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RAL");
					}
					else {
						wcscpy(g_LDownKeyName, L"ral");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"APP");
					}
					else {
						wcscpy(g_LDownKeyName, L"app");
					}

				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RCT");
					}
					else {
						wcscpy(g_LDownKeyName, L"rct");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"★");
					}
					else {
						wcscpy(g_LDownKeyName, L"★");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 14) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LEF");
					}
					else {
						wcscpy(g_LDownKeyName, L"lef");
					}
					repeatvk = VK_LEFT;
				}
				else if (mx >= (width*xdpi) / 16 * 14 && mx <= (width*xdpi) / 16 * 15) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"DOW");
					}
					else {
						wcscpy(g_LDownKeyName, L"dow");
					}
					repeatvk = VK_DOWN;
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RIG");
					}
					else {
						wcscpy(g_LDownKeyName, L"rig");
					}
					repeatvk = VK_RIGHT;
				}
			}
			if (repeatvk != -1) {
				SetTimer(hWnd, 1987, 1000, TimeProc);
				dp("getvk %i\n", repeatvk);
			}

			if (showkeytip == true) {

				//if (g_LDownKeyName[0] != 0) {
					//auto hdc = GetDC(hWnd);
					//RECT rt;
					//if (!(wcsicmp(g_LDownKeyName, L"esc") == 0 || wcsicmp(g_LDownKeyName, L"f1") == 0 || wcsicmp(g_LDownKeyName, L"f2") == 0 || wcsicmp(g_LDownKeyName, L"f3") == 0
					//	|| wcsicmp(g_LDownKeyName, L"`") == 0 || wcsicmp(g_LDownKeyName, L"~") == 0 || wcsicmp(g_LDownKeyName, L"1") == 0 || wcsicmp(g_LDownKeyName, L"!") == 0 || wcsicmp(g_LDownKeyName, L"2") == 0 || wcsicmp(g_LDownKeyName, L"@") == 0 || wcsicmp(g_LDownKeyName, L"3") == 0 || wcsicmp(g_LDownKeyName, L"#") == 0
					//	|| wcsicmp(g_LDownKeyName, L"tab") == 0 || wcsicmp(g_LDownKeyName, L"q") == 0 || wcsicmp(g_LDownKeyName, L"w") == 0 || wcsicmp(g_LDownKeyName, L"e") == 0
					//	|| wcsicmp(g_LDownKeyName, L"cap") == 0 || wcsicmp(g_LDownKeyName, L"a") == 0 || wcsicmp(g_LDownKeyName, L"s") == 0 || wcsicmp(g_LDownKeyName, L"d") == 0
					//	)) {
					//	rt.left = 10;
					//	rt.right = height*ydpi / 2;
					//	rt.top = 10;
					//	rt.bottom = height*ydpi / 2;
					//}
					//else {
					//	rt.left = width*xdpi - height*ydpi / 2 - 10;
					//	rt.right = width*xdpi - 10;
					//	rt.top = 10;
					//	rt.bottom = height*ydpi / 2;
					//}
					////修改颜色
					//if (GetKeyState(VK_CONTROL) < 0) {
					//	SetTextColor(hdc, RGB(0, 0, 255)); //设置字体颜色
					//}
					//else if (GetKeyState(VK_MENU) < 0) {
					//	SetTextColor(hdc, RGB(0, 255, 255)); //设置字体颜色
					//}
					//else {
					//	SetTextColor(hdc, RGB(0, 255, 0)); //设置字体颜色
					//}
					//SetBkColor(hdc, RGB(255, 255, 255)); //设置背景色
					//SetBkMode(hdc, OPAQUE); //非透明模式


					//HFONT hFont;
					//LOGFONT lFont;
					//memset(&lFont, 0, sizeof(LOGFONT));
					////计算字体高度与磅值 
					//int nNumerator = ::GetDeviceCaps(hdc, LOGPIXELSY);
					//lFont.lfHeight = height*ydpi / 4;
					//lFont.lfWeight = height*ydpi / 2;
					////创建字体 
					//hFont = ::CreateFontIndirect(&lFont);
					////选择字体 
					//HGDIOBJ hOldFont = ::SelectObject(hdc, hFont);
					//DrawText(hdc, g_LDownKeyName, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
					////释放GDI资源 
					//::SelectObject(hdc, hOldFont);
					//BOOL bRes = ::DeleteObject(hFont);
					//ReleaseDC(hWnd, hdc);

					//SendMessage(hWnd, WM_MOUSEMOVE, wParam, lParam);
					//POINT cpt2;
					//GetCursorPos(&cpt2);
					//cpt2.x += 1;
					//SetCursorPos(cpt2.x, cpt2.y);
					
					InvalidateRect(hWnd, &rect, TRUE);

					//POINT pt2;
					//pt2.x = rect.left;
					//pt2.y = rect.top;
					//ClientToScreen(hWnd, &pt2);
					//SetWindowPos(hWnd, NULL, pt2.x + 1, pt2.y + 1, 0, 0, SWP_NOSIZE);
					//SetWindowPos(hWnd, NULL, pt2.x, pt2.y, 0, 0, SWP_NOSIZE);


				//}
			}

		}
	}
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_CREATE://窗口创建时候的消息.
		nid.cbSize = sizeof(nid);
		nid.hWnd = hWnd;
		nid.uID = 86450;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_USER;
		nid.hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CANDEFKEYBOARD));
		lstrcpy(nid.szTip, L"CanDefKeyboard");
		Shell_NotifyIcon(NIM_ADD, &nid);
		hmenu = CreatePopupMenu();//生成菜单
		AppendMenu(hmenu, MF_STRING, IDR_ABOUT, L"关于");//为菜单添加两个选项
		AppendMenu(hmenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hmenu, MF_STRING, IDR_QUIT, L"退出");
		break;
	case WM_USER://连续使用该程序时候的消息.
		if (lParam == WM_LBUTTONDOWN)
			if (IsWindowVisible(hWnd)) {
				ShowWindow(hWnd, SW_HIDE);
			}else{
				bCaps = GetKeyState(VK_CAPITAL) == 1 ? TRUE : FALSE;
				ShowWindow(hWnd, SW_SHOW);
				KillProcessidByName(L"TabTip.exe");
			}
		if (lParam == WM_LBUTTONDBLCLK){
			ShowWindow(hWnd, SW_SHOW);
		}
		if (lParam == WM_RBUTTONDOWN)
		{
			POINT pt;
			GetCursorPos(&pt);//取鼠标坐标
			::SetForegroundWindow(hWnd);//解决在菜单外单击左键菜单不消失的问题
			//EnableMenuItem(hmenu, WM_, MF_GRAYED);//让菜单中的某一项变灰
			auto xx = TrackPopupMenu(hmenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd, NULL);//显示菜单并获取选项ID
			if (xx == IDR_QUIT) {
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			else if (xx == IDR_ABOUT) {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				ShowWindow(hWnd, SW_HIDE);
				ShowWindow(hWnd, SW_SHOW);
			}
			else {
				ShowWindow(hWnd, SW_HIDE);
				ShowWindow(hWnd, SW_SHOW);
			}
			//if (xx == 0) PostMessage(hWnd, WM_LBUTTONDOWN, NULL, NULL);
			//MessageBox(hwnd, TEXT("右键"), szAppName, MB_OK);
		}
		break;
	case WM_MOUSEACTIVATE:
		//return MA_NOACTIVATE;
		return MA_NOACTIVATEANDEAT;
	case WM_MOUSEMOVE:
	{
		int mouse_x = (int)LOWORD(lParam);//取低位
		int mouse_y = (int)HIWORD(lParam);//取高位
		RECT rect;
		GetClientRect(hWnd, &rect);

		if (mouseupMoveWin) {
			dp("mmove:%i,%i\n", mouse_x, mouse_y);

			POINT point;
			::GetCursorPos(&point);
			SetWindowPos(hWnd, NULL, point.x- msx, point.y- msy, 0, 0, SWP_NOSIZE);
			break;

			::ScreenToClient(hWnd, &point);
			


			int Dx = point.x - msx;
			int Dy = point.y -msy;
			startRect.left -= point.x;
			startRect.right -= point.y;
			startRect.top -= point.y;
			startRect.bottom += Dy;             //获取新的位置  
			SetWindowPos(hWnd, NULL, startRect.left, startRect.top, 0, 0, SWP_NOSIZE);

			break;
			POINT pt;
			pt.x = 0;
			pt.y = 0;
			ClientToScreen(hWnd,&pt);
			MoveWindow(hWnd, pt.x+msx- mouse_x, pt.y+msy- mouse_y, width*xdpi, height*ydpi, true);
			msx=mouse_x, msy=mouse_y;
		}
		else {
			wchar_t g_LDownKeyName2[10] = { 0 };
			auto mx = mouse_x;
			auto my = mouse_y;

			bool slshiftdown = lshiftdown, slctrldown = lctrldown, slwindown = lwindown, slaltdown = laltdown;
			bool srshiftdown = rshiftdown, srctrldown = rctrldown, sraltdown = raltdown;

			if (my >= 0 && my <= height*ydpi / 6) {
				if (mx >= (width*xdpi) / 17 * 0 && mx <= (width*xdpi) / 17 * 1) {
					//keybd_event(VK_ESCAPE, 0, 0, 0);
					//keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"ESC");
					}
					else {
						wcscpy(g_LDownKeyName2, L"esc");
					}
					repeatvk = VK_ESCAPE;
				}
				else if (mx >= (width*xdpi) / 17 * 1 && mx <= (width*xdpi) / 17 * 2) {
					//keybd_event(VK_F1, 0, 0, 0);
					//keybd_event(VK_F1, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F1");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f1");
					}
					repeatvk = VK_F1;
				}
				else if (mx >= (width*xdpi) / 17 * 2 && mx <= (width*xdpi) / 17 * 3) {
					//keybd_event(VK_F2, 0, 0, 0);
					//keybd_event(VK_F2, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F2");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f2");
					}
					repeatvk = VK_F2;
				}
				else if (mx >= (width*xdpi) / 17 * 3 && mx <= (width*xdpi) / 17 * 4) {
					//keybd_event(VK_F3, 0, 0, 0);
					//keybd_event(VK_F3, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F3");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f3");
					}
					repeatvk = VK_F3;
				}
				else if (mx >= (width*xdpi) / 17 * 4 && mx <= (width*xdpi) / 17 * 5) {
					//keybd_event(VK_F4, 0, 0, 0);
					//keybd_event(VK_F4, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F4");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f4");
					}
					repeatvk = VK_F4;
				}
				else if (mx >= (width*xdpi) / 17 * 5 && mx <= (width*xdpi) / 17 * 6) {
					//keybd_event(VK_F5, 0, 0, 0);
					//keybd_event(VK_F5, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F5");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f5");
					}
					repeatvk = VK_F5;
				}
				else if (mx >= (width*xdpi) / 17 * 6 && mx <= (width*xdpi) / 17 * 7) {
					//keybd_event(VK_F6, 0, 0, 0);
					//keybd_event(VK_F6, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F6");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f6");
					}
					repeatvk = VK_F6;
				}
				else if (mx >= (width*xdpi) / 17 * 7 && mx <= (width*xdpi) / 17 * 8) {
					//keybd_event(VK_F7, 0, 0, 0);
					//keybd_event(VK_F7, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F7");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f7");
					}
					repeatvk = VK_F7;
				}
				else if (mx >= (width*xdpi) / 17 * 8 && mx <= (width*xdpi) / 17 * 9) {
					//keybd_event(VK_F8, 0, 0, 0);
					//keybd_event(VK_F8, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F8");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f8");
					}
					repeatvk = VK_F8;
				}
				else if (mx >= (width*xdpi) / 17 * 9 && mx <= (width*xdpi) / 17 * 10) {
					//keybd_event(VK_F9, 0, 0, 0);
					//keybd_event(VK_F9, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F9");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f9");
					}
					repeatvk = VK_F9;
				}
				else if (mx >= (width*xdpi) / 17 * 10 && mx <= (width*xdpi) / 17 * 11) {
					//keybd_event(VK_F10, 0, 0, 0);
					//keybd_event(VK_F10, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F10");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f10");
					}
					repeatvk = VK_F10;
				}
				else if (mx >= (width*xdpi) / 17 * 11 && mx <= (width*xdpi) / 17 * 12) {
					//keybd_event(VK_F11, 0, 0, 0);
					//keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F11");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f11");
					}
					repeatvk = VK_F11;
				}
				else if (mx >= (width*xdpi) / 17 * 12 && mx <= (width*xdpi) / 17 * 13) {
					//keybd_event(VK_F12, 0, 0, 0);
					//keybd_event(VK_F12, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F12");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f12");
					}
					repeatvk = VK_F12;
				}
				else if (mx >= (width*xdpi) / 17 * 13 && mx <= (width*xdpi) / 17 * 14) {
					//keybd_event(VK_SNAPSHOT, 0, 0, 0);
					//keybd_event(VK_SNAPSHOT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"PRT");
					}
					else {
						wcscpy(g_LDownKeyName2, L"prt");
					}
					repeatvk = VK_SNAPSHOT;
				}
				else if (mx >= (width*xdpi) / 17 * 14 && mx <= (width*xdpi) / 17 * 15) {
					//keybd_event(VK_PAUSE, 0, 0, 0);
					//keybd_event(VK_PAUSE, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"PAU");
					}
					else {
						wcscpy(g_LDownKeyName2, L"pau");
					}
					repeatvk = VK_PAUSE;
				}
				else if (mx >= (width*xdpi) / 17 * 15 && mx <= (width*xdpi) / 17 * 16) {
					//keybd_event(VK_INSERT, 0, 0, 0);
					//keybd_event(VK_INSERT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"INS");
					}
					else {
						wcscpy(g_LDownKeyName2, L"ins");
					}
					repeatvk = VK_INSERT;
				}
				else if (mx >= (width*xdpi) / 17 * 16 && mx <= (width*xdpi) / 17 * 17) {
					//keybd_event(VK_DELETE, 0, 0, 0);
					//keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"DEL");
					}
					else {
						wcscpy(g_LDownKeyName2, L"del");
					}
					repeatvk = VK_DELETE;
				}
			}
			else if (my >= height*ydpi / 6 && my <= height*ydpi / 6 * 2) {
				if (mx >= (width*xdpi) / 16 * 0 && mx <= (width*xdpi) / 16 * 1) {
					//keybd_event(VK_OEM_3, 0, 0, 0);
					//keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"~");
					}
					else {
						wcscpy(g_LDownKeyName2, L"`");
					}
					repeatvk = VK_OEM_3;
				}
				else if (mx >= (width*xdpi) / 16 * 1 && mx <= (width*xdpi) / 16 * 2) {
					//keybd_event('1', 0, 0, 0);
					//keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"!");
					}
					else {
						wcscpy(g_LDownKeyName2, L"1");
					}
					repeatvk = '1';
				}
				else if (mx >= (width*xdpi) / 16 * 2 && mx <= (width*xdpi) / 16 * 3) {
					//keybd_event('2', 0, 0, 0);
					//keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"@");
					}
					else {
						wcscpy(g_LDownKeyName2, L"2");
					}
					repeatvk = '2';
				}
				else if (mx >= (width*xdpi) / 16 * 3 && mx <= (width*xdpi) / 16 * 4) {
					//keybd_event('3', 0, 0, 0);
					//keybd_event('3', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"#");
					}
					else {
						wcscpy(g_LDownKeyName2, L"3");
					}
					repeatvk = '3';
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 5) {
					//keybd_event('4', 0, 0, 0);
					//keybd_event('4', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"$");
					}
					else {
						wcscpy(g_LDownKeyName2, L"4");
					}
					repeatvk = '4';
				}
				else if (mx >= (width*xdpi) / 16 * 5 && mx <= (width*xdpi) / 16 * 6) {
					//keybd_event('5', 0, 0, 0);
					//keybd_event('5', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"%");
					}
					else {
						wcscpy(g_LDownKeyName2, L"5");
					}
					repeatvk = '5';
				}
				else if (mx >= (width*xdpi) / 16 * 6 && mx <= (width*xdpi) / 16 * 7) {
					//keybd_event('6', 0, 0, 0);
					//keybd_event('6', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"^");
					}
					else {
						wcscpy(g_LDownKeyName2, L"6");
					}
					repeatvk = '6';
				}
				else if (mx >= (width*xdpi) / 16 * 7 && mx <= (width*xdpi) / 16 * 8) {
					//keybd_event('7', 0, 0, 0);
					//keybd_event('7', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"&");
					}
					else {
						wcscpy(g_LDownKeyName2, L"7");
					}
					repeatvk = '7';
				}
				else if (mx >= (width*xdpi) / 16 * 8 && mx <= (width*xdpi) / 16 * 9) {
					//keybd_event('8', 0, 0, 0);
					//keybd_event('8', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"*");
					}
					else {
						wcscpy(g_LDownKeyName2, L"8");
					}
					repeatvk = '8';
				}
				else if (mx >= (width*xdpi) / 16 * 9 && mx <= (width*xdpi) / 16 * 10) {
					//keybd_event('9', 0, 0, 0);
					//keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"(");
					}
					else {
						wcscpy(g_LDownKeyName2, L"9");
					}
					repeatvk = '9';
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					//keybd_event('0', 0, 0, 0);
					//keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L")");
					}
					else {
						wcscpy(g_LDownKeyName2, L"0");
					}
					repeatvk = '0';
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					//keybd_event(VK_OEM_MINUS, 0, 0, 0);
					//keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"_");
					}
					else {
						wcscpy(g_LDownKeyName2, L"-");
					}
					repeatvk = VK_OEM_MINUS;
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					//keybd_event(VK_OEM_PLUS, 0, 0, 0);
					//keybd_event(VK_OEM_PLUS, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"+");
					}
					else {
						wcscpy(g_LDownKeyName2, L"=");
					}
					repeatvk = VK_OEM_PLUS;
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 15) {
					if (lshiftdown) {
						if (dblsize == 1) {
							//dblsize = 2;
							//width *= 2;
							//height *= 2;
							//MoveWindow(hWnd, lastposx, lastposy, width*xdpi, height*ydpi, TRUE);
							wcscpy(g_LDownKeyName2, L"SX2");
						}
						else {
							//dblsize = 1;
							//width /= 2;
							//height /= 2;
							//MoveWindow(hWnd, lastposx, lastposy, width*xdpi, height*ydpi, TRUE);
							wcscpy(g_LDownKeyName2, L"S/2");
						}
						InvalidateRect(hWnd, &rect, TRUE);
					}
					else if (lctrldown) {
						//ShowWindow(hWnd,SW_HIDE);
						wcscpy(g_LDownKeyName2, L"CKH");
					}
					else if (laltdown) {
						//autokilltabtip = !autokilltabtip;
						//if (autokilltabtip) {
						//	MessageBoxW(hWnd, L"已经开", L"自动关闭TabTip.exe设置", 0);
						//}
						wcscpy(g_LDownKeyName2, L"AKT");
					}
					else {
						//keybd_event(VK_BACK, 0, 0, 0);
						//keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
						wcscpy(g_LDownKeyName2, L"BAS");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//keybd_event(VK_HOME, 0, 0, 0);
					//keybd_event(VK_HOME, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"HOM");
					}
					else {
						wcscpy(g_LDownKeyName2, L"hom");
					}
					repeatvk = VK_HOME;
				}
			}
			else if (my >= height*ydpi / 6 * 2 && my <= height*ydpi / 6 * 3) {
				float delta = 0;
				dp("tab %i %f\n", mx, (width*xdpi) / 16 * 15);
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16 / 2;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//keybd_event(VK_TAB, 0, 0, 0);
					//keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"TAB");
					}
					else {
						wcscpy(g_LDownKeyName2, L"tab");
					}
					repeatvk = VK_TAB;
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					//keybd_event('Q', 0, 0, 0);
					//keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"Q");
					}
					else {
						wcscpy(g_LDownKeyName2, L"q");
					}
					repeatvk = 'Q';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//keybd_event('W', 0, 0, 0);
					//keybd_event('W', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"W");
					}
					else {
						wcscpy(g_LDownKeyName2, L"w");
					}
					repeatvk = 'W';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//keybd_event('E', 0, 0, 0);
					//keybd_event('E', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"E");
					}
					else {
						wcscpy(g_LDownKeyName2, L"e");
					}
					repeatvk = 'E';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					//keybd_event('R', 0, 0, 0);
					//keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"R");
					}
					else {
						wcscpy(g_LDownKeyName2, L"r");
					}
					repeatvk = 'R';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					//keybd_event('T', 0, 0, 0);
					//keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"T");
					}
					else {
						wcscpy(g_LDownKeyName2, L"t");
					}
					repeatvk = 'T';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					//keybd_event('Y', 0, 0, 0);
					//keybd_event('Y', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"Y");
					}
					else {
						wcscpy(g_LDownKeyName2, L"y");
					}
					repeatvk = 'Y';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					//keybd_event('U', 0, 0, 0);
					//keybd_event('U', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"U");
					}
					else {
						wcscpy(g_LDownKeyName2, L"u");
					}
					repeatvk = 'U';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					//keybd_event('I', 0, 0, 0);
					//keybd_event('I', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"I");
					}
					else {
						wcscpy(g_LDownKeyName2, L"i");
					}
					repeatvk = 'I';
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					//keybd_event('O', 0, 0, 0);
					//keybd_event('O', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"O");
					}
					else {
						wcscpy(g_LDownKeyName2, L"o");
					}
					repeatvk = 'O';
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					//keybd_event('P', 0, 0, 0);
					//keybd_event('P', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"P");
					}
					else {
						wcscpy(g_LDownKeyName2, L"p");
					}
					repeatvk = 'P';
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 12 + delta) {
					//keybd_event(VK_OEM_4, 0, 0, 0);
					//keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"{");
					}
					else {
						wcscpy(g_LDownKeyName2, L"[");
					}
					repeatvk = VK_OEM_4;
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta&& mx <= (width*xdpi) / 16 * 13 + delta) {
					//keybd_event(VK_OEM_6, 0, 0, 0);
					//keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"}");
					}
					else {
						wcscpy(g_LDownKeyName2, L"]");
					}
					repeatvk = VK_OEM_6;
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta && mx <= (width*xdpi) / 16 * 14 + delta + (width*xdpi) / 16 / 2) {
					//keybd_event(VK_OEM_5, 0, 0, 0);
					//keybd_event(VK_OEM_5, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"|");
					}
					else {
						wcscpy(g_LDownKeyName2, L"\\");
					}
					repeatvk = VK_OEM_5;
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//keybd_event(VK_PRIOR, 0, 0, 0);
					//keybd_event(VK_PRIOR, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"PUP");
					}
					else {
						wcscpy(g_LDownKeyName2, L"pup");
					}
					repeatvk = VK_PRIOR;
				}
			}
			else if (my >= height*ydpi / 6 * 3 && my <= height*ydpi / 6 * 4) {
				float delta = 0; dp("pgdn %i %f\n", mx, (width*xdpi) / 16 * 12);
				if (mx > (width*xdpi) / 16 * 1 + delta) {
					delta = (width*xdpi) / 16 * 0.75;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"CAP");
					}
					else {
						wcscpy(g_LDownKeyName2, L"cap");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					//keybd_event('A', 0, 0, 0);
					//keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"A");
					}
					else {
						wcscpy(g_LDownKeyName2, L"a");
					}
					repeatvk = 'A';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//keybd_event('S', 0, 0, 0);
					//keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"S");
					}
					else {
						wcscpy(g_LDownKeyName2, L"s");
					}
					repeatvk = 'S';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//keybd_event('D', 0, 0, 0);
					//keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"D");
					}
					else {
						wcscpy(g_LDownKeyName2, L"d");
					}
					repeatvk = 'D';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					//keybd_event('F', 0, 0, 0);
					//keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"F");
					}
					else {
						wcscpy(g_LDownKeyName2, L"f");
					}
					repeatvk = 'F';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					//keybd_event('G', 0, 0, 0);
					//keybd_event('G', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"G");
					}
					else {
						wcscpy(g_LDownKeyName2, L"g");
					}
					repeatvk = 'G';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					//keybd_event('H', 0, 0, 0);
					//keybd_event('H', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"H");
					}
					else {
						wcscpy(g_LDownKeyName2, L"h");
					}
					repeatvk = 'H';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					//keybd_event('J', 0, 0, 0);
					//keybd_event('J', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"J");
					}
					else {
						wcscpy(g_LDownKeyName2, L"j");
					}
					repeatvk = 'J';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					//keybd_event('K', 0, 0, 0);
					//keybd_event('K', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"K");
					}
					else {
						wcscpy(g_LDownKeyName2, L"k");
					}
					repeatvk = 'K';
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					//keybd_event('L', 0, 0, 0);
					//keybd_event('L', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"L");
					}
					else {
						wcscpy(g_LDownKeyName2, L"l");
					}
					repeatvk = 'L';
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					//keybd_event(VK_OEM_1, 0, 0, 0);
					//keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L":");
					}
					else {
						wcscpy(g_LDownKeyName2, L";");
					}
					repeatvk = VK_OEM_1;
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta&& mx <= (width*xdpi) / 16 * 12 + delta) {
					//keybd_event(VK_OEM_7, 0, 0, 0);
					//keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"\"");
					}
					else {
						wcscpy(g_LDownKeyName2, L"'");
					}
					repeatvk = VK_OEM_7;
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta && mx <= (width*xdpi) / 16 * 13 + delta + (width*xdpi) / 16 * 1.25) {
					//keybd_event(VK_RETURN, 0, 0, 0);
					//keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"RET");
					}
					else {
						wcscpy(g_LDownKeyName2, L"ret");
					}
					repeatvk = VK_RETURN;
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//keybd_event(VK_NEXT, 0, 0, 0);
					//keybd_event(VK_NEXT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"PDO");
					}
					else {
						wcscpy(g_LDownKeyName2, L"pdo");
					}
					repeatvk = VK_NEXT;
				}
			}
			else if (my >= height*ydpi / 6 * 4 && my <= height*ydpi / 6 * 5) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//if (lshiftlongdown) {
					//	keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	lshiftlongdown = false;
					//}else if (lshiftdown == false) {
					//	keybd_event(VK_LSHIFT, 0, 0, 0);
					//	//keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	lshiftdown = true;
					//}
					//else {
					//	keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	lshiftdown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					wcscpy(g_LDownKeyName2, L"LSH");
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					//keybd_event('Z', 0, 0, 0);
					//keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"Z");
					}
					else {
						wcscpy(g_LDownKeyName2, L"z");
					}
					repeatvk = 'Z';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//keybd_event('X', 0, 0, 0);
					//keybd_event('X', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"X");
					}
					else {
						wcscpy(g_LDownKeyName2, L"x");
					}
					repeatvk = 'X';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//keybd_event('C', 0, 0, 0);
					//keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"C");
					}
					else {
						wcscpy(g_LDownKeyName2, L"c");
					}
					repeatvk = 'C';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					//keybd_event('V', 0, 0, 0);
					//keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"V");
					}
					else {
						wcscpy(g_LDownKeyName2, L"v");
					}
					repeatvk = 'V';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					//keybd_event('B', 0, 0, 0);
					//keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"B");
					}
					else {
						wcscpy(g_LDownKeyName2, L"b");
					}
					repeatvk = 'B';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					//keybd_event('N', 0, 0, 0);
					//keybd_event('N', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"N");
					}
					else {
						wcscpy(g_LDownKeyName2, L"n");
					}
					repeatvk = 'N';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					//keybd_event('M', 0, 0, 0);
					//keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"M");
					}
					else {
						wcscpy(g_LDownKeyName2, L"m");
					}
					repeatvk = 'M';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					//keybd_event(VK_OEM_COMMA, 0, 0, 0);
					//keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"<");
					}
					else {
						wcscpy(g_LDownKeyName2, L",");
					}
					repeatvk = VK_OEM_COMMA;
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					//keybd_event(VK_OEM_PERIOD, 0, 0, 0);
					//keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L">");
					}
					else {
						wcscpy(g_LDownKeyName2, L".");
					}
					repeatvk = VK_OEM_PERIOD;
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					//keybd_event(VK_OEM_2, 0, 0, 0);
					//keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"?");
					}
					else {
						wcscpy(g_LDownKeyName2, L"/");
					}
					repeatvk = VK_OEM_2;
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 13 + delta) {
					//if (rshiftlongdown) {
					//	keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	rshiftlongdown = false;
					//}
					//else if(rshiftdown==false){
					//	keybd_event(VK_RSHIFT, 0, 0, 0);
					//	rshiftdown = true;
					//}
					//else {
					//	keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	rshiftdown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					wcscpy(g_LDownKeyName2, L"RSH");
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta&& mx <= (width*xdpi) / 16 * 14 + delta) {
					////keybd_event(VK_RSHIFT, 0, 0, 0);
					//keybd_event(VK_UP, 0, 0, 0);
					//keybd_event(VK_UP, 0, KEYEVENTF_KEYUP, 0);
					////keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"UP");
					}
					else {
						wcscpy(g_LDownKeyName2, L"up");
					}
					repeatvk = VK_UP;
				}
				else if (mx >= (width*xdpi) / 16 * 14 + delta && mx <= (width*xdpi) / 16 * 15 + delta) {
					//keybd_event(VK_END, 0, 0, 0);
					//keybd_event(VK_END, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"END");
					}
					else {
						wcscpy(g_LDownKeyName2, L"end");
					}
					repeatvk = VK_END;
				}
			}
			else if (my >= height*ydpi / 6 * 5 && my <= height*ydpi / 6 * 6) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 9) {
					delta = (width*xdpi) / 16 * 4;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//if (lctrllongdown) {
					//	keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	lctrllongdown = false;
					//}
					//else if(lctrldown==false){
					//	keybd_event(VK_LCONTROL, 0, 0, 0);
					//	//keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	lctrldown = true;
					//}
					//else {
					//	keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	lctrldown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"LCT");
					}
					else {
						wcscpy(g_LDownKeyName2, L"lct");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					////auto kl = GetKeyboardLayout(0);
					////if (ImmIsIME(kl)) {
					////	ImmSimulateHotKey(GetFocus(), IME_THOTKEY_IME_NONIME_TOGGLE);
					////}
					////else {
					////	ImmSimulateHotKey(GetFocus(), IME_THOTKEY_IME_NONIME_TOGGLE);
					////}
					//for (int i = 0; i < wcslen(zyh);i+=1 ) {
					//	if (zyh[i] == 'S') {
					//		keybd_event(VK_LSHIFT, 0, 0, 0);
					//	}else if (zyh[i] == 'C') {
					//		keybd_event(VK_LCONTROL, 0, 0, 0);
					//	}
					//	else if (zyh[i] == 'A') {
					//		keybd_event(VK_LCONTROL, 0, 0, 0);
					//	}
					//	else if (zyh[i] == '+') {
					//		i += 1;
					//		if (i+2<=wcslen(zyh)&&wcsncmp(zyh + i, L"SPACE", 2) == 0) {
					//			keybd_event(VK_SPACE, 0, 0, 0);
					//			keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
					//		}
					//		else if(i+1<=wcslen(zyh)){
					//			keybd_event(zyh[i], 0, 0, 0);
					//			keybd_event(zyh[i], 0, KEYEVENTF_KEYUP, 0);
					//		}
					//		break;
					//	}
					//}
					//for (int i = 0; i < wcslen(zyh); i += 1) {
					//	if (zyh[i] == 'S') {
					//		keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	}
					//	else if (zyh[i] == 'C') {
					//		keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	}
					//	else if (zyh[i] == 'A') {
					//		keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	}
					//	else if (zyh[i] == '+') {
					//		break;
					//	}
					//}
					wcscpy(g_LDownKeyName2, L"C/E");
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//if (lwinlongdown) {
					//	keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
					//	lwinlongdown = false;
					//}
					//else if(lwindown==false){
					//	keybd_event(VK_LWIN, 0, 0, 0);
					//	//keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
					//	lwindown = true;
					//}
					//else {
					//	keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
					//	lwindown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"LWI");
					}
					else {
						wcscpy(g_LDownKeyName2, L"lwi");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//if (laltlongdown) {
					//	keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
					//	laltlongdown = false;
					//}
					//else if(laltdown==false){
					//	keybd_event(VK_LMENU, 0, 0, 0);
					//	//keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
					//	laltdown = true;
					//}
					//else {
					//	keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
					//	laltdown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"LAL");
					}
					else {
						wcscpy(g_LDownKeyName2, L"lal");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 9) {
					//keybd_event(VK_SPACE, 0, 0, 0);
					//keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"SPA");
					}
					else {
						wcscpy(g_LDownKeyName2, L"spa");
					}
					repeatvk = VK_SPACE;
				}
				else if (mx >= (width*xdpi) / 16 * 9 && mx <= (width*xdpi) / 16 * 10) {
					//if (raltlongdown) {
					//	keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
					//	raltlongdown = false;
					//}
					//else if(raltdown==false){
					//	keybd_event(VK_RMENU, 0, 0, 0);
					//	//keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
					//	raltdown = true;
					//}
					//else {
					//	keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
					//	raltdown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"RAL");
					}
					else {
						wcscpy(g_LDownKeyName2, L"ral");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					//keybd_event(VK_APPS, 0, 0, 0);
					//keybd_event(VK_APPS, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"APP");
					}
					else {
						wcscpy(g_LDownKeyName2, L"app");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					//if (rctrllongdown) {
					//	keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	rctrllongdown = false;
					//}
					//else if(rctrldown==false){
					//	keybd_event(VK_RCONTROL, 0, 0, 0);
					//	//keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	rctrldown = true;
					//}
					//else {
					//	keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	rctrldown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"RCT");
					}
					else {
						wcscpy(g_LDownKeyName2, L"rct");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					wcscpy(g_LDownKeyName2, L"★");
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 14) {
					////keybd_event(VK_RSHIFT, 0, 0, 0);
					//keybd_event(VK_LEFT, 0, 0, 0);
					//keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
					////keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"LEF");
					}
					else {
						wcscpy(g_LDownKeyName2, L"lef");
					}
					repeatvk = VK_LEFT;
				}
				else if (mx >= (width*xdpi) / 16 * 14 && mx <= (width*xdpi) / 16 * 15) {
					////keybd_event(VK_RSHIFT, 0, 0, 0);
					//keybd_event(VK_DOWN, 0, 0, 0);
					//keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0);
					////keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"DOW");
					}
					else {
						wcscpy(g_LDownKeyName2, L"dow");
					}
					repeatvk = VK_DOWN;
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					////keybd_event(VK_RSHIFT, 0, 0, 0);
					//keybd_event(VK_RIGHT, 0, 0, 0);
					//keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
					////keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName2, L"RIG");
					}
					else {
						wcscpy(g_LDownKeyName2, L"rig");
					}
					repeatvk = VK_RIGHT;
				}
			}

			//if (slshiftdown != lshiftdown || slctrldown != lctrldown || slwindown != lwindown || slaltdown != laltdown
			//	|| srshiftdown != rshiftdown || srctrldown != rctrldown || sraltdown != raltdown
			//	) {

			//}
			//else {
			//	if (slshiftdown == true) {
			//		if (lshiftdown) {
			//			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
			//			lshiftdown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (slctrldown == true) {
			//		if (lctrldown) {
			//			keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
			//			lctrldown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (slwindown == true) {
			//		if (lwindown) {
			//			keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
			//			lwindown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (slaltdown == true) {
			//		if (laltdown) {
			//			keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
			//			laltdown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (srshiftdown == true) {
			//		if (rshiftdown) {
			//			keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
			//			rshiftdown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (srctrldown == true) {
			//		if (rctrldown) {
			//			keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
			//			rctrldown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (sraltdown == true) {
			//		if (raltdown) {
			//			keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
			//			raltdown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//}

			//if (repeatvk != -1) {
			//	SetTimer(hWnd, 1987, 1000, TimeProc);
			//	dp("getvk %i\n", repeatvk);
			//}

			if (showkeytip == true) {

				//if (g_LDownKeyName2[0] != 0) {
					//auto hdc = GetDC(hWnd);
					//RECT rt;
					//if (!(wcsicmp(g_LDownKeyName2, L"esc") == 0 || wcsicmp(g_LDownKeyName2, L"f1") == 0 || wcsicmp(g_LDownKeyName2, L"f2") == 0 || wcsicmp(g_LDownKeyName2, L"f3") == 0
					//	|| wcsicmp(g_LDownKeyName2, L"`") == 0 || wcsicmp(g_LDownKeyName2, L"~") == 0 || wcsicmp(g_LDownKeyName2, L"1") == 0 || wcsicmp(g_LDownKeyName2, L"!") == 0 || wcsicmp(g_LDownKeyName2, L"2") == 0 || wcsicmp(g_LDownKeyName2, L"@") == 0 || wcsicmp(g_LDownKeyName2, L"3") == 0 || wcsicmp(g_LDownKeyName2, L"#") == 0
					//	|| wcsicmp(g_LDownKeyName2, L"tab") == 0 || wcsicmp(g_LDownKeyName2, L"q") == 0 || wcsicmp(g_LDownKeyName2, L"w") == 0 || wcsicmp(g_LDownKeyName2, L"e") == 0
					//	|| wcsicmp(g_LDownKeyName2, L"cap") == 0 || wcsicmp(g_LDownKeyName2, L"a") == 0 || wcsicmp(g_LDownKeyName2, L"s") == 0 || wcsicmp(g_LDownKeyName2, L"d") == 0
					//	)) {
					//	rt.left = 10;
					//	rt.right = height*ydpi / 2;
					//	rt.top = 10;
					//	rt.bottom = height*ydpi / 2;
					//}
					//else {
					//	rt.left = width*xdpi - height*ydpi / 2 - 10;
					//	rt.right = width*xdpi - 10;
					//	rt.top = 10;
					//	rt.bottom = height*ydpi / 2;
					//}
					////修改颜色
					//if (GetKeyState(VK_CONTROL) < 0) {
					//	SetTextColor(hdc, RGB(0, 0, 255)); //设置字体颜色
					//}
					//else if (GetKeyState(VK_MENU) < 0) {
					//	SetTextColor(hdc, RGB(0, 255, 255)); //设置字体颜色
					//}
					//else {
					//	SetTextColor(hdc, RGB(0, 255, 0)); //设置字体颜色
					//}
					//SetBkColor(hdc, RGB(255, 255, 255)); //设置背景色
					//SetBkMode(hdc, OPAQUE); //非透明模式


					//HFONT hFont;
					//LOGFONT lFont;
					//memset(&lFont, 0, sizeof(LOGFONT));
					////计算字体高度与磅值 
					//int nNumerator = ::GetDeviceCaps(hdc, LOGPIXELSY);
					//lFont.lfHeight = height*ydpi / 4;
					//lFont.lfWeight = height*ydpi / 2;
					////创建字体 
					//hFont = ::CreateFontIndirect(&lFont);
					////选择字体 
					//HGDIOBJ hOldFont = ::SelectObject(hdc, hFont);
					//DrawText(hdc, g_LDownKeyName, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
					////释放GDI资源 
					//::SelectObject(hdc, hOldFont);
					//BOOL bRes = ::DeleteObject(hFont);
					//ReleaseDC(hWnd, hdc);

					if (wcscmp(g_LDownKeyName, g_LDownKeyName2) != 0) {
						KillTimer(hWnd, 1987);
						KillTimer(hWnd, 1986);
						wcscpy(g_LDownKeyName, g_LDownKeyName2);
						InvalidateRect(hWnd, &rect, TRUE);
					}


					//POINT pt2;
					//pt2.x = rect.left;
					//pt2.y = rect.top;r
					//ClientToScreen(hWnd, &pt2);h
					//SetWindowPos(hWnd, NULL, pt2.x + 1, pt2.y + 1, 0, 0, SWP_NOSIZE);tf
					//SetWindowPos(hWnd, NULL, pt2.x, pt2.y, 0, 0, SWP_NOSIZE);bbbbbbbbbbccc


				}
			//}
		}

	}
		break;
	case WM_LBUTTONDOWN:
	{
		//g_LDownKeyName[0] = 0;
		int mouse_x = (int)LOWORD(lParam);//取低位
		int mouse_y = (int)HIWORD(lParam);//取高位
		
		//DPIInit();
		RECT rect;
		GetClientRect(hWnd, &rect);
		dp("WM_LBUTTONDOWN ldown %i,%i, %i,%i,%i,%i\n", mouse_x, mouse_y,rect.right, rect.bottom,rect.left,rect.top);
		if (mouse_x > width*xdpi/16*12 && mouse_x<width*xdpi / 16*13 && mouse_y > rect.bottom - height*ydpi / 6) {
			startRect = rect;
			POINT pt;
			pt.x = 0;
			pt.y = 0;
			ClientToScreen(hWnd, &pt);
			startRect.left = pt.x;
			startRect.right = pt.x+ startRect.right;
			startRect.top = pt.y;
			startRect.bottom = pt.y + startRect.bottom;
			msx = mouse_x;
			msy = mouse_y;
			mouseupMoveWin = true;
		}
		else {
			auto mx = mouse_x;
			auto my = mouse_y;

			bool slshiftdown= lshiftdown, slctrldown= lctrldown, slwindown= lwindown, slaltdown= laltdown;
			bool srshiftdown= rshiftdown, srctrldown= rctrldown, sraltdown= raltdown;

			if (my >= 0 && my <= height*ydpi / 6) {
				if (mx >= (width*xdpi) / 17 * 0 && mx <= (width*xdpi) / 17 * 1) {
					//keybd_event(VK_ESCAPE, 0, 0, 0);
					//keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"ESC");
					}
					else {
						wcscpy(g_LDownKeyName, L"esc");
					}
					repeatvk = VK_ESCAPE;
				}
				else if (mx >= (width*xdpi) / 17 * 1 && mx <= (width*xdpi) / 17 * 2) {
					//keybd_event(VK_F1, 0, 0, 0);
					//keybd_event(VK_F1, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F1");
					}
					else {
						wcscpy(g_LDownKeyName, L"f1");
					}
					repeatvk = VK_F1;
				}
				else if (mx >= (width*xdpi) / 17 * 2 && mx <= (width*xdpi) / 17 * 3) {
					//keybd_event(VK_F2, 0, 0, 0);
					//keybd_event(VK_F2, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F2");
					}
					else {
						wcscpy(g_LDownKeyName, L"f2");
					}
					repeatvk = VK_F2;
				}
				else if (mx >= (width*xdpi) / 17 * 3 && mx <= (width*xdpi) / 17 * 4) {
					//keybd_event(VK_F3, 0, 0, 0);
					//keybd_event(VK_F3, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F3");
					}
					else {
						wcscpy(g_LDownKeyName, L"f3");
					}
					repeatvk = VK_F3;
				}
				else if (mx >= (width*xdpi) / 17 * 4 && mx <= (width*xdpi) / 17 * 5) {
					//keybd_event(VK_F4, 0, 0, 0);
					//keybd_event(VK_F4, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F4");
					}
					else {
						wcscpy(g_LDownKeyName, L"f4");
					}
					repeatvk = VK_F4;
				}
				else if (mx >= (width*xdpi) / 17 * 5 && mx <= (width*xdpi) / 17 * 6) {
					//keybd_event(VK_F5, 0, 0, 0);
					//keybd_event(VK_F5, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F5");
					}
					else {
						wcscpy(g_LDownKeyName, L"f5");
					}
					repeatvk = VK_F5;
				}
				else if (mx >= (width*xdpi) / 17 * 6 && mx <= (width*xdpi) / 17 * 7) {
					//keybd_event(VK_F6, 0, 0, 0);
					//keybd_event(VK_F6, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F6");
					}
					else {
						wcscpy(g_LDownKeyName, L"f6");
					}
					repeatvk = VK_F6;
				}
				else if (mx >= (width*xdpi) / 17 * 7 && mx <= (width*xdpi) / 17 * 8) {
					//keybd_event(VK_F7, 0, 0, 0);
					//keybd_event(VK_F7, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F7");
					}
					else {
						wcscpy(g_LDownKeyName, L"f7");
					}
					repeatvk = VK_F7;
				}
				else if (mx >= (width*xdpi) / 17 * 8 && mx <= (width*xdpi) / 17 * 9) {
					//keybd_event(VK_F8, 0, 0, 0);
					//keybd_event(VK_F8, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F8");
					}
					else {
						wcscpy(g_LDownKeyName, L"f8");
					}
					repeatvk = VK_F8;
				}
				else if (mx >= (width*xdpi) / 17 * 9 && mx <= (width*xdpi) / 17 * 10) {
					//keybd_event(VK_F9, 0, 0, 0);
					//keybd_event(VK_F9, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F9");
					}
					else {
						wcscpy(g_LDownKeyName, L"f9");
					}
					repeatvk = VK_F9;
				}
				else if (mx >= (width*xdpi) / 17 * 10 && mx <= (width*xdpi) / 17 * 11) {
					//keybd_event(VK_F10, 0, 0, 0);
					//keybd_event(VK_F10, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F10");
					}
					else {
						wcscpy(g_LDownKeyName, L"f10");
					}
					repeatvk = VK_F10;
				}
				else if (mx >= (width*xdpi) / 17 * 11 && mx <= (width*xdpi) / 17 * 12) {
					//keybd_event(VK_F11, 0, 0, 0);
					//keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F11");
					}
					else {
						wcscpy(g_LDownKeyName, L"f11");
					}
					repeatvk = VK_F11;
				}
				else if (mx >= (width*xdpi) / 17 * 12 && mx <= (width*xdpi) / 17 * 13) {
					//keybd_event(VK_F12, 0, 0, 0);
					//keybd_event(VK_F12, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F12");
					}
					else {
						wcscpy(g_LDownKeyName, L"f12");
					}
					repeatvk = VK_F12;
				}
				else if (mx >= (width*xdpi) / 17 * 13 && mx <= (width*xdpi) / 17 * 14) {
					//keybd_event(VK_SNAPSHOT, 0, 0, 0);
					//keybd_event(VK_SNAPSHOT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"PRT");
					}
					else {
						wcscpy(g_LDownKeyName, L"prt");
					}
					repeatvk = VK_SNAPSHOT;
				}
				else if (mx >= (width*xdpi) / 17 * 14 && mx <= (width*xdpi) / 17 * 15) {
					//keybd_event(VK_PAUSE, 0, 0, 0);
					//keybd_event(VK_PAUSE, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"PAU");
					}
					else {
						wcscpy(g_LDownKeyName, L"pau");
					}
					repeatvk = VK_PAUSE;
				}
				else if (mx >= (width*xdpi) / 17 * 15 && mx <= (width*xdpi) / 17 * 16) {
					//keybd_event(VK_INSERT, 0, 0, 0);
					//keybd_event(VK_INSERT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"INS");
					}
					else {
						wcscpy(g_LDownKeyName, L"ins");
					}
					repeatvk = VK_INSERT;
				}
				else if (mx >= (width*xdpi) / 17 * 16 && mx <= (width*xdpi) / 17 * 17) {
					//keybd_event(VK_DELETE, 0, 0, 0);
					//keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"DEL");
					}
					else {
						wcscpy(g_LDownKeyName, L"del");
					}
					repeatvk = VK_DELETE;
				}
			}else if (my >= height*ydpi / 6 && my <= height*ydpi / 6*2 ) {
				if (mx >= (width*xdpi) / 16 * 0 && mx <= (width*xdpi) / 16 * 1) {
					//keybd_event(VK_OEM_3, 0, 0, 0);
					//keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"~");
					}
					else {
						wcscpy(g_LDownKeyName, L"`");
					}
					repeatvk = VK_OEM_3;
				}
				else if (mx >= (width*xdpi) / 16 * 1 && mx <= (width*xdpi) / 16 * 2) {
					//keybd_event('1', 0, 0, 0);
					//keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"!");
					}
					else {
						wcscpy(g_LDownKeyName, L"1");
					}
					repeatvk = '1';
				}
				else if (mx >= (width*xdpi) / 16 * 2 && mx <= (width*xdpi) / 16 * 3) {
					//keybd_event('2', 0, 0, 0);
					//keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"@");
					}
					else {
						wcscpy(g_LDownKeyName, L"2");
					}
					repeatvk = '2';
				}
				else if (mx >= (width*xdpi) / 16 * 3 && mx <= (width*xdpi) / 16 * 4) {
					//keybd_event('3', 0, 0, 0);
					//keybd_event('3', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"#");
					}
					else {
						wcscpy(g_LDownKeyName, L"3");
					}
					repeatvk = '3';
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 5) {
					//keybd_event('4', 0, 0, 0);
					//keybd_event('4', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"$");
					}
					else {
						wcscpy(g_LDownKeyName, L"4");
					}
					repeatvk = '4';
				}
				else if (mx >= (width*xdpi) / 16 * 5 && mx <= (width*xdpi) / 16 * 6) {
					//keybd_event('5', 0, 0, 0);
					//keybd_event('5', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"%");
					}
					else {
						wcscpy(g_LDownKeyName, L"5");
					}
					repeatvk = '5';
				}
				else if (mx >= (width*xdpi) / 16 * 6 && mx <= (width*xdpi) / 16 * 7) {
					//keybd_event('6', 0, 0, 0);
					//keybd_event('6', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"^");
					}
					else {
						wcscpy(g_LDownKeyName, L"6");
					}
					repeatvk = '6';
				}
				else if (mx >= (width*xdpi) / 16 * 7 && mx <= (width*xdpi) / 16 * 8) {
					//keybd_event('7', 0, 0, 0);
					//keybd_event('7', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"&");
					}
					else {
						wcscpy(g_LDownKeyName, L"7");
					}
					repeatvk = '7';
				}
				else if (mx >= (width*xdpi) / 16 * 8 && mx <= (width*xdpi) / 16 * 9) {
					//keybd_event('8', 0, 0, 0);
					//keybd_event('8', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"*");
					}
					else {
						wcscpy(g_LDownKeyName, L"8");
					}
					repeatvk = '8';
				}
				else if (mx >= (width*xdpi) / 16 * 9 && mx <= (width*xdpi) / 16 * 10) {
					//keybd_event('9', 0, 0, 0);
					//keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"(");
					}
					else {
						wcscpy(g_LDownKeyName, L"9");
					}
					repeatvk = '9';
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					//keybd_event('0', 0, 0, 0);
					//keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L")");
					}
					else {
						wcscpy(g_LDownKeyName, L"0");
					}
					repeatvk = '0';
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					//keybd_event(VK_OEM_MINUS, 0, 0, 0);
					//keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"_");
					}
					else {
						wcscpy(g_LDownKeyName, L"-");
					}
					repeatvk = VK_OEM_MINUS;
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					//keybd_event(VK_OEM_PLUS, 0, 0, 0);
					//keybd_event(VK_OEM_PLUS, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"+");
					}
					else {
						wcscpy(g_LDownKeyName, L"=");
					}
					repeatvk = VK_OEM_PLUS;
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 15) {
					if (lshiftdown) {
						if (dblsize == 1) {
							//dblsize = 2;
							//width *= 2;
							//height *= 2;
							//MoveWindow(hWnd, lastposx, lastposy, width*xdpi, height*ydpi, TRUE);
							wcscpy(g_LDownKeyName, L"SX2");
						}
						else {
							//dblsize = 1;
							//width /= 2;
							//height /= 2;
							//MoveWindow(hWnd, lastposx, lastposy, width*xdpi, height*ydpi, TRUE);
							wcscpy(g_LDownKeyName, L"S/2");
						}
						InvalidateRect(hWnd, &rect, TRUE);
					}else if (lctrldown) {
						//ShowWindow(hWnd,SW_HIDE);
						wcscpy(g_LDownKeyName, L"CKH");
					}
					else if (laltdown) {
						//autokilltabtip = !autokilltabtip;
						//if (autokilltabtip) {
						//	MessageBoxW(hWnd, L"已经开", L"自动关闭TabTip.exe设置", 0);
						//}
						wcscpy(g_LDownKeyName, L"AKT");
					}
					else {
						//keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
						wcscpy(g_LDownKeyName, L"BAS");
						repeatvk = VK_BACK;
					}
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//keybd_event(VK_HOME, 0, 0, 0);
					//keybd_event(VK_HOME, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"HOM");
					}
					else {
						wcscpy(g_LDownKeyName, L"hom");
					}
					repeatvk = VK_HOME;
				}
			}
			else if (my >= height*ydpi / 6*2 && my <= height*ydpi / 6 * 3) {
				float delta = 0;
				dp("tab %i %f\n", mx, (width*xdpi) / 16 * 15);
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16 / 2;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//keybd_event(VK_TAB, 0, 0, 0);
					//keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"TAB");
					}
					else {
						wcscpy(g_LDownKeyName, L"tab");
					}
					repeatvk = VK_TAB;
				}
				else if (mx >= (width*xdpi) / 16 * 1+delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					//keybd_event('Q', 0, 0, 0);
					//keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"Q");
					}
					else {
						wcscpy(g_LDownKeyName, L"q");
					}
					repeatvk = 'Q';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//keybd_event('W', 0, 0, 0);
					//keybd_event('W', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"W");
					}
					else {
						wcscpy(g_LDownKeyName, L"w");
					}
					repeatvk = 'W';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//keybd_event('E', 0, 0, 0);
					//keybd_event('E', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"E");
					}
					else {
						wcscpy(g_LDownKeyName, L"e");
					}
					repeatvk = 'E';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					//keybd_event('R', 0, 0, 0);
					//keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"R");
					}
					else {
						wcscpy(g_LDownKeyName, L"r");
					}
					repeatvk = 'R';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					//keybd_event('T', 0, 0, 0);
					//keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"T");
					}
					else {
						wcscpy(g_LDownKeyName, L"t");
					}
					repeatvk = 'T';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					//keybd_event('Y', 0, 0, 0);
					//keybd_event('Y', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"Y");
					}
					else {
						wcscpy(g_LDownKeyName, L"y");
					}
					repeatvk = 'Y';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					//keybd_event('U', 0, 0, 0);
					//keybd_event('U', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"U");
					}
					else {
						wcscpy(g_LDownKeyName, L"u");
					}
					repeatvk = 'U';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					//keybd_event('I', 0, 0, 0);
					//keybd_event('I', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"I");
					}
					else {
						wcscpy(g_LDownKeyName, L"i");
					}
					repeatvk = 'I';
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					//keybd_event('O', 0, 0, 0);
					//keybd_event('O', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"O");
					}
					else {
						wcscpy(g_LDownKeyName, L"o");
					}
					repeatvk = 'O';
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					//keybd_event('P', 0, 0, 0);
					//keybd_event('P', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"P");
					}
					else {
						wcscpy(g_LDownKeyName, L"p");
					}
					repeatvk = 'P';
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 12 + delta) {
					//keybd_event(VK_OEM_4, 0, 0, 0);
					//keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"{");
					}
					else {
						wcscpy(g_LDownKeyName, L"[");
					}
					repeatvk = VK_OEM_4;
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta&& mx <= (width*xdpi) / 16 * 13 + delta) {
					//keybd_event(VK_OEM_6, 0, 0, 0);
					//keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"}");
					}
					else {
						wcscpy(g_LDownKeyName, L"]");
					}
					repeatvk = VK_OEM_6;
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta && mx <= (width*xdpi) / 16 * 14 + delta+ (width*xdpi) / 16/2) {
					//keybd_event(VK_OEM_5, 0, 0, 0);
					//keybd_event(VK_OEM_5, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"|");
					}
					else {
						wcscpy(g_LDownKeyName, L"\\");
					}
					repeatvk = VK_OEM_5;
				}
				else if (mx >= (width*xdpi) / 16 * 15&& mx <= (width*xdpi) / 16 * 16 ) {
					//keybd_event(VK_PRIOR, 0, 0, 0);
					//keybd_event(VK_PRIOR, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"PUP");
					}
					else {
						wcscpy(g_LDownKeyName, L"pup");
					}
					repeatvk = VK_PRIOR;
				}
			}
			else if (my >= height*ydpi / 6 * 3 && my <= height*ydpi / 6 * 4) {
				float delta = 0; dp("pgdn %i %f\n", mx, (width*xdpi) / 16 * 12);
				if (mx > (width*xdpi) / 16 * 1 + delta) {
					delta = (width*xdpi) / 16 * 0.75;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"CAP");
					}
					else {
						wcscpy(g_LDownKeyName, L"cap");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					//keybd_event('A', 0, 0, 0);
					//keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"A");
					}
					else {
						wcscpy(g_LDownKeyName, L"a");
					}
					repeatvk = 'A';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//keybd_event('S', 0, 0, 0);
					//keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"S");
					}
					else {
						wcscpy(g_LDownKeyName, L"s");
					}
					repeatvk = 'S';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//keybd_event('D', 0, 0, 0);
					//keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"D");
					}
					else {
						wcscpy(g_LDownKeyName, L"d");
					}
					repeatvk = 'D';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					//keybd_event('F', 0, 0, 0);
					//keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"F");
					}
					else {
						wcscpy(g_LDownKeyName, L"f");
					}
					repeatvk = 'F';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					//keybd_event('G', 0, 0, 0);
					//keybd_event('G', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"G");
					}
					else {
						wcscpy(g_LDownKeyName, L"g");
					}
					repeatvk = 'G';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					//keybd_event('H', 0, 0, 0);
					//keybd_event('H', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"H");
					}
					else {
						wcscpy(g_LDownKeyName, L"h");
					}
					repeatvk = 'H';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					//keybd_event('J', 0, 0, 0);
					//keybd_event('J', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"J");
					}
					else {
						wcscpy(g_LDownKeyName, L"j");
					}
					repeatvk = 'J';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					//keybd_event('K', 0, 0, 0);
					//keybd_event('K', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"K");
					}
					else {
						wcscpy(g_LDownKeyName, L"k");
					}
					repeatvk = 'K';
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					//keybd_event('L', 0, 0, 0);
					//keybd_event('L', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"L");
					}
					else {
						wcscpy(g_LDownKeyName, L"l");
					}
					repeatvk = 'L';
				}else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					//keybd_event(VK_OEM_1, 0, 0, 0);
					//keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L":");
					}
					else {
						wcscpy(g_LDownKeyName, L";");
					}
					repeatvk = VK_OEM_1;
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta&& mx <= (width*xdpi) / 16 * 12 + delta) {
					//keybd_event(VK_OEM_7, 0, 0, 0);
					//keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"\"");
					}
					else {
						wcscpy(g_LDownKeyName, L"'");
					}
					repeatvk = VK_OEM_7;
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta && mx <= (width*xdpi) / 16 * 13 + delta+ (width*xdpi) / 16*1.25) {
					//keybd_event(VK_RETURN, 0, 0, 0);
					//keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RET");
					}
					else {
						wcscpy(g_LDownKeyName, L"ret");
					}
					repeatvk = VK_RETURN;
				}else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//keybd_event(VK_NEXT, 0, 0, 0);
					//keybd_event(VK_NEXT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"PDO");
					}
					else {
						wcscpy(g_LDownKeyName, L"pdo");
					}
					repeatvk = VK_NEXT;
				}
			}
			else if (my >= height*ydpi / 6 * 4 && my <= height*ydpi / 6 * 5) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//if (lshiftlongdown) {
					//	keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	lshiftlongdown = false;
					//}else if (lshiftdown == false) {
					//	keybd_event(VK_LSHIFT, 0, 0, 0);
					//	//keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	lshiftdown = true;
					//}
					//else {
					//	keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	lshiftdown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					wcscpy(g_LDownKeyName, L"LSH");
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					//keybd_event('Z', 0, 0, 0);
					//keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"Z");
					}
					else {
						wcscpy(g_LDownKeyName, L"z");
					}
					repeatvk = 'Z';
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//keybd_event('X', 0, 0, 0);
					//keybd_event('X', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"X");
					}
					else {
						wcscpy(g_LDownKeyName, L"x");
					}
					repeatvk = 'X';
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//keybd_event('C', 0, 0, 0);
					//keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"C");
					}
					else {
						wcscpy(g_LDownKeyName, L"c");
					}
					repeatvk = 'C';
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					//keybd_event('V', 0, 0, 0);
					//keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"V");
					}
					else {
						wcscpy(g_LDownKeyName, L"v");
					}
					repeatvk = 'V';
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					//keybd_event('B', 0, 0, 0);
					//keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"B");
					}
					else {
						wcscpy(g_LDownKeyName, L"b");
					}
					repeatvk = 'B';
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					//keybd_event('N', 0, 0, 0);
					//keybd_event('N', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"N");
					}
					else {
						wcscpy(g_LDownKeyName, L"n");
					}
					repeatvk = 'N';
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					//keybd_event('M', 0, 0, 0);
					//keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"M");
					}
					else {
						wcscpy(g_LDownKeyName, L"m");
					}
					repeatvk = 'M';
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					//keybd_event(VK_OEM_COMMA, 0, 0, 0);
					//keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"<");
					}
					else {
						wcscpy(g_LDownKeyName, L",");
					}
					repeatvk = VK_OEM_COMMA;
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					//keybd_event(VK_OEM_PERIOD, 0, 0, 0);
					//keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L">");
					}
					else {
						wcscpy(g_LDownKeyName, L".");
					}
					repeatvk = VK_OEM_PERIOD;
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					//keybd_event(VK_OEM_2, 0, 0, 0);
					//keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"?");
					}
					else {
						wcscpy(g_LDownKeyName, L"/");
					}
					repeatvk = VK_OEM_2;
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 13+ delta) {
					//if (rshiftlongdown) {
					//	keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	rshiftlongdown = false;
					//}
					//else if(rshiftdown==false){
					//	keybd_event(VK_RSHIFT, 0, 0, 0);
					//	rshiftdown = true;
					//}
					//else {
					//	keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	rshiftdown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					wcscpy(g_LDownKeyName, L"RSH");
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta&& mx <= (width*xdpi) / 16 * 14 + delta) {
					////keybd_event(VK_RSHIFT, 0, 0, 0);
					//keybd_event(VK_UP, 0, 0, 0);
					//keybd_event(VK_UP, 0, KEYEVENTF_KEYUP, 0);
					////keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"UP");
					}
					else {
						wcscpy(g_LDownKeyName, L"up");
					}
					repeatvk = VK_UP;
				}
				else if (mx >= (width*xdpi) / 16 * 14 + delta && mx <= (width*xdpi) / 16 * 15 + delta) {
					//keybd_event(VK_END, 0, 0, 0);
					//keybd_event(VK_END, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"END");
					}
					else {
						wcscpy(g_LDownKeyName, L"end");
					}
					repeatvk = VK_END;
				}
			}
			else if (my >= height*ydpi / 6 * 5 && my <= height*ydpi / 6 * 6) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 9) {
					delta = (width*xdpi) / 16*4;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//if (lctrllongdown) {
					//	keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	lctrllongdown = false;
					//}
					//else if(lctrldown==false){
					//	keybd_event(VK_LCONTROL, 0, 0, 0);
					//	//keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	lctrldown = true;
					//}
					//else {
					//	keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	lctrldown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LCT");
					}
					else {
						wcscpy(g_LDownKeyName, L"lct");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					////auto kl = GetKeyboardLayout(0);
					////if (ImmIsIME(kl)) {
					////	ImmSimulateHotKey(GetFocus(), IME_THOTKEY_IME_NONIME_TOGGLE);
					////}
					////else {
					////	ImmSimulateHotKey(GetFocus(), IME_THOTKEY_IME_NONIME_TOGGLE);
					////}
					//for (int i = 0; i < wcslen(zyh);i+=1 ) {
					//	if (zyh[i] == 'S') {
					//		keybd_event(VK_LSHIFT, 0, 0, 0);
					//	}else if (zyh[i] == 'C') {
					//		keybd_event(VK_LCONTROL, 0, 0, 0);
					//	}
					//	else if (zyh[i] == 'A') {
					//		keybd_event(VK_LCONTROL, 0, 0, 0);
					//	}
					//	else if (zyh[i] == '+') {
					//		i += 1;
					//		if (i+2<=wcslen(zyh)&&wcsncmp(zyh + i, L"SPACE", 2) == 0) {
					//			keybd_event(VK_SPACE, 0, 0, 0);
					//			keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
					//		}
					//		else if(i+1<=wcslen(zyh)){
					//			keybd_event(zyh[i], 0, 0, 0);
					//			keybd_event(zyh[i], 0, KEYEVENTF_KEYUP, 0);
					//		}
					//		break;
					//	}
					//}
					//for (int i = 0; i < wcslen(zyh); i += 1) {
					//	if (zyh[i] == 'S') {
					//		keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
					//	}
					//	else if (zyh[i] == 'C') {
					//		keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	}
					//	else if (zyh[i] == 'A') {
					//		keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	}
					//	else if (zyh[i] == '+') {
					//		break;
					//	}
					//}
					wcscpy(g_LDownKeyName, L"C/E");
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//if (lwinlongdown) {
					//	keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
					//	lwinlongdown = false;
					//}
					//else if(lwindown==false){
					//	keybd_event(VK_LWIN, 0, 0, 0);
					//	//keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
					//	lwindown = true;
					//}
					//else {
					//	keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
					//	lwindown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LWI");
					}
					else {
						wcscpy(g_LDownKeyName, L"lwi");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//if (laltlongdown) {
					//	keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
					//	laltlongdown = false;
					//}
					//else if(laltdown==false){
					//	keybd_event(VK_LMENU, 0, 0, 0);
					//	//keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
					//	laltdown = true;
					//}
					//else {
					//	keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
					//	laltdown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LAL");
					}
					else {
						wcscpy(g_LDownKeyName, L"lal");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 9) {
					//keybd_event(VK_SPACE, 0, 0, 0);
					//keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"SPA");
					}
					else {
						wcscpy(g_LDownKeyName, L"spa");
					}
					repeatvk = VK_SPACE;
				}
				else if (mx >= (width*xdpi) / 16 * 9  && mx <= (width*xdpi) / 16 * 10) {
					//if (raltlongdown) {
					//	keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
					//	raltlongdown = false;
					//}
					//else if(raltdown==false){
					//	keybd_event(VK_RMENU, 0, 0, 0);
					//	//keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
					//	raltdown = true;
					//}
					//else {
					//	keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
					//	raltdown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RAL");
					}
					else {
						wcscpy(g_LDownKeyName, L"ral");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					//keybd_event(VK_APPS, 0, 0, 0);
					//keybd_event(VK_APPS, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"APP");
					}
					else {
						wcscpy(g_LDownKeyName, L"app");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 *12) {
					//if (rctrllongdown) {
					//	keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	rctrllongdown = false;
					//}
					//else if(rctrldown==false){
					//	keybd_event(VK_RCONTROL, 0, 0, 0);
					//	//keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	rctrldown = true;
					//}
					//else {
					//	keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
					//	rctrldown = false;
					//}
					//InvalidateRect(hWnd, &rect, TRUE);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RCT");
					}
					else {
						wcscpy(g_LDownKeyName, L"rct");
					}
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					wcscpy(g_LDownKeyName, L"★");
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 14) {
					////keybd_event(VK_RSHIFT, 0, 0, 0);
					//keybd_event(VK_LEFT, 0, 0, 0);
					//keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
					////keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"LEF");
					}
					else {
						wcscpy(g_LDownKeyName, L"lef");
					}
					repeatvk = VK_LEFT;
				}
				else if (mx >= (width*xdpi) / 16 * 14 && mx <= (width*xdpi) / 16 * 15) {
					////keybd_event(VK_RSHIFT, 0, 0, 0);
					//keybd_event(VK_DOWN, 0, 0, 0);
					//keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0);
					////keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"DOW");
					}
					else {
						wcscpy(g_LDownKeyName, L"dow");
					}
					repeatvk = VK_DOWN;
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					////keybd_event(VK_RSHIFT, 0, 0, 0);
					//keybd_event(VK_RIGHT, 0, 0, 0);
					//keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
					////keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					if (GetKeyState(VK_SHIFT) < 0) {
						wcscpy(g_LDownKeyName, L"RIG");
					}
					else {
						wcscpy(g_LDownKeyName, L"rig");
					}
					repeatvk = VK_RIGHT;
				}
			}

			//if (slshiftdown != lshiftdown || slctrldown != lctrldown || slwindown != lwindown || slaltdown != laltdown
			//	|| srshiftdown != rshiftdown || srctrldown != rctrldown || sraltdown != raltdown
			//	) {

			//}
			//else {
			//	if (slshiftdown == true) {
			//		if (lshiftdown) {
			//			keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
			//			lshiftdown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (slctrldown == true) {
			//		if (lctrldown) {
			//			keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
			//			lctrldown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (slwindown == true) {
			//		if (lwindown) {
			//			keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
			//			lwindown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (slaltdown == true) {
			//		if (laltdown) {
			//			keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
			//			laltdown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (srshiftdown == true) {
			//		if (rshiftdown) {
			//			keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
			//			rshiftdown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (srctrldown == true) {
			//		if (rctrldown) {
			//			keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
			//			rctrldown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//	if (sraltdown == true) {
			//		if (raltdown) {
			//			keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
			//			raltdown = false;
			//			InvalidateRect(hWnd, &rect, TRUE);
			//		}
			//	}
			//}

			if (repeatvk != -1) {
				SetTimer(hWnd, 1987, 1000, TimeProc);
				dp("getvk %i\n", repeatvk);
			}

			if(showkeytip==true){

				//if (g_LDownKeyName[0] != 0) {
					//auto hdc = GetDC(hWnd);
					//RECT rt;
					//if (!(wcsicmp(g_LDownKeyName, L"esc") == 0 || wcsicmp(g_LDownKeyName, L"f1") == 0 || wcsicmp(g_LDownKeyName, L"f2") == 0 || wcsicmp(g_LDownKeyName, L"f3") == 0
					//	|| wcsicmp(g_LDownKeyName, L"`") == 0 || wcsicmp(g_LDownKeyName, L"~") == 0 || wcsicmp(g_LDownKeyName, L"1") == 0 || wcsicmp(g_LDownKeyName, L"!") == 0 || wcsicmp(g_LDownKeyName, L"2") == 0 || wcsicmp(g_LDownKeyName, L"@") == 0 || wcsicmp(g_LDownKeyName, L"3") == 0 || wcsicmp(g_LDownKeyName, L"#") == 0
					//	|| wcsicmp(g_LDownKeyName, L"tab") == 0 || wcsicmp(g_LDownKeyName, L"q") == 0 || wcsicmp(g_LDownKeyName, L"w") == 0 || wcsicmp(g_LDownKeyName, L"e") == 0
					//	|| wcsicmp(g_LDownKeyName, L"cap") == 0 || wcsicmp(g_LDownKeyName, L"a") == 0 || wcsicmp(g_LDownKeyName, L"s") == 0 || wcsicmp(g_LDownKeyName, L"d") == 0
					//	)) {
					//	rt.left = 10;
					//	rt.right = height*ydpi / 2;
					//	rt.top = 10;
					//	rt.bottom = height*ydpi / 2;
					//}
					//else {
					//	rt.left = width*xdpi - height*ydpi / 2 - 10;
					//	rt.right = width*xdpi - 10;
					//	rt.top = 10;
					//	rt.bottom = height*ydpi / 2;
					//}
					////修改颜色
					//if (GetKeyState(VK_CONTROL) < 0) {
					//	SetTextColor(hdc, RGB(0, 0, 255)); //设置字体颜色
					//}
					//else if (GetKeyState(VK_MENU) < 0) {
					//	SetTextColor(hdc, RGB(0, 255, 255)); //设置字体颜色
					//}
					//else {
					//	SetTextColor(hdc, RGB(0, 255, 0)); //设置字体颜色
					//}
					//SetBkColor(hdc, RGB(255, 255, 255)); //设置背景色
					//SetBkMode(hdc, OPAQUE); //非透明模式


					//HFONT hFont;
					//LOGFONT lFont;
					//memset(&lFont, 0, sizeof(LOGFONT));
					////计算字体高度与磅值 
					//int nNumerator = ::GetDeviceCaps(hdc, LOGPIXELSY);
					//lFont.lfHeight = height*ydpi / 4;
					//lFont.lfWeight = height*ydpi / 2;
					////创建字体 
					//hFont = ::CreateFontIndirect(&lFont);
					////选择字体 
					//HGDIOBJ hOldFont = ::SelectObject(hdc, hFont);
					//DrawText(hdc, g_LDownKeyName, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
					////释放GDI资源 
					//::SelectObject(hdc, hOldFont);
					//BOOL bRes = ::DeleteObject(hFont);
					//ReleaseDC(hWnd, hdc);

					
					//SendMessage(hWnd, WM_MOUSEMOVE, wParam, lParam);
					//POINT cpt2;
					//GetCursorPos(&cpt2);
					//cpt2.x += 1;
					//SetCursorPos(cpt2.x, cpt2.y);
					InvalidateRect(hWnd, &rect, TRUE);


					//POINT pt2;
					//pt2.x = rect.left;
					//pt2.y = rect.top;
					//ClientToScreen(hWnd, &pt2);
					//SetWindowPos(hWnd, NULL, pt2.x + 1, pt2.y + 1, 0, 0, SWP_NOSIZE);
					//SetWindowPos(hWnd, NULL, pt2.x, pt2.y, 0, 0, SWP_NOSIZE);




				//}
			}
		}
	}
		break;
	case WM_LBUTTONUP:
	{
		int mouse_x = (int)LOWORD(lParam);//取低位
		int mouse_y = (int)HIWORD(lParam);//取高位
		RECT rect;
		GetClientRect(hWnd, &rect);

		repeatvk = -1;
		KillTimer(hWnd, 1987); 
		KillTimer(hWnd, 1986);
		
		if (mouseupMoveWin) {
			POINT point;
			::GetCursorPos(&point);
			SetWindowPos(hWnd, NULL, point.x - msx, point.y - msy, 0, 0, SWP_NOSIZE);


			HKEY pk;
			RegCreateKey(HKEY_CURRENT_USER, L"Software\\CanDefKeyboard", &pk);
			RegCloseKey(pk);
			RegOpenKey(HKEY_CURRENT_USER, L"Software\\CanDefKeyboard", &pk);

			POINT pt;
			pt.x = 0;
			pt.y = 0;
			ClientToScreen(hWnd, &pt);
			wchar_t cc[96];
			swprintf_s(cc, L"%i,%i", pt.x, pt.y);
			auto rrr = RegSetValueEx(pk, L"lastpos", 0, REG_SZ, (BYTE*)cc, sizeof(wchar_t)*wcslen(cc));
			RegCloseKey(pk);
			KillProcessidByName(L"TabTip.exe");
			lastposx = pt.x;
			lastposy = pt.y;
		}
		else {
			//g_LDownKeyName[0] = 0;
			auto mx = mouse_x;
			auto my = mouse_y;

			bool slshiftdown = lshiftdown, slctrldown = lctrldown, slwindown = lwindown, slaltdown = laltdown;
			bool srshiftdown = rshiftdown, srctrldown = rctrldown, sraltdown = raltdown;

			if (my >= 0 && my <= height*ydpi / 6) {
				if (mx >= (width*xdpi) / 17 * 0 && mx <= (width*xdpi) / 17 * 1) {
					keybd_event(VK_ESCAPE, 0, 0, 0);
					keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 1 && mx <= (width*xdpi) / 17 * 2) {
					keybd_event(VK_F1, 0, 0, 0);
					keybd_event(VK_F1, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 2 && mx <= (width*xdpi) / 17 * 3) {
					keybd_event(VK_F2, 0, 0, 0);
					keybd_event(VK_F2, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 3 && mx <= (width*xdpi) / 17 * 4) {
					keybd_event(VK_F3, 0, 0, 0);
					keybd_event(VK_F3, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 4 && mx <= (width*xdpi) / 17 * 5) {
					keybd_event(VK_F4, 0, 0, 0);
					keybd_event(VK_F4, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 5 && mx <= (width*xdpi) / 17 * 6) {
					keybd_event(VK_F5, 0, 0, 0);
					keybd_event(VK_F5, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 6 && mx <= (width*xdpi) / 17 * 7) {
					keybd_event(VK_F6, 0, 0, 0);
					keybd_event(VK_F6, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 7 && mx <= (width*xdpi) / 17 * 8) {
					keybd_event(VK_F7, 0, 0, 0);
					keybd_event(VK_F7, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 8 && mx <= (width*xdpi) / 17 * 9) {
					keybd_event(VK_F8, 0, 0, 0);
					keybd_event(VK_F8, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 9 && mx <= (width*xdpi) / 17 * 10) {
					keybd_event(VK_F9, 0, 0, 0);
					keybd_event(VK_F9, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 10 && mx <= (width*xdpi) / 17 * 11) {
					keybd_event(VK_F10, 0, 0, 0);
					keybd_event(VK_F10, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 11 && mx <= (width*xdpi) / 17 * 12) {
					keybd_event(VK_F11, 0, 0, 0);
					keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 12 && mx <= (width*xdpi) / 17 * 13) {
					keybd_event(VK_F12, 0, 0, 0);
					keybd_event(VK_F12, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 13 && mx <= (width*xdpi) / 17 * 14) {
					keybd_event(VK_SNAPSHOT, 0, 0, 0);
					keybd_event(VK_SNAPSHOT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 14 && mx <= (width*xdpi) / 17 * 15) {
					keybd_event(VK_PAUSE, 0, 0, 0);
					keybd_event(VK_PAUSE, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 15 && mx <= (width*xdpi) / 17 * 16) {
					keybd_event(VK_INSERT, 0, 0, 0);
					keybd_event(VK_INSERT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 16 && mx <= (width*xdpi) / 17 * 17) {
					keybd_event(VK_DELETE, 0, 0, 0);
					keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else if (my >= height*ydpi / 6 && my <= height*ydpi / 6 * 2) {
				if (mx >= (width*xdpi) / 16 * 0 && mx <= (width*xdpi) / 16 * 1) {
					keybd_event(VK_OEM_3, 0, 0, 0);
					keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 1 && mx <= (width*xdpi) / 16 * 2) {
					keybd_event('1', 0, 0, 0);
					keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 2 && mx <= (width*xdpi) / 16 * 3) {
					keybd_event('2', 0, 0, 0);
					keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 3 && mx <= (width*xdpi) / 16 * 4) {
					keybd_event('3', 0, 0, 0);
					keybd_event('3', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 5) {
					keybd_event('4', 0, 0, 0);
					keybd_event('4', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 5 && mx <= (width*xdpi) / 16 * 6) {
					keybd_event('5', 0, 0, 0);
					keybd_event('5', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 6 && mx <= (width*xdpi) / 16 * 7) {
					keybd_event('6', 0, 0, 0);
					keybd_event('6', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 7 && mx <= (width*xdpi) / 16 * 8) {
					keybd_event('7', 0, 0, 0);
					keybd_event('7', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 8 && mx <= (width*xdpi) / 16 * 9) {
					keybd_event('8', 0, 0, 0);
					keybd_event('8', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 && mx <= (width*xdpi) / 16 * 10) {
					keybd_event('9', 0, 0, 0);
					keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					keybd_event('0', 0, 0, 0);
					keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					keybd_event(VK_OEM_MINUS, 0, 0, 0);
					keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					keybd_event(VK_OEM_PLUS, 0, 0, 0);
					keybd_event(VK_OEM_PLUS, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 15) {
					dp("lwindown %d\n", lwindown);
					if (lshiftdown) {
						if (dblsize == 1) {
							dblsize = 2;
							width *= 2;
							height *= 2;
							MoveWindow(hWnd, lastposx, lastposy, width*xdpi, height*ydpi, TRUE);
						}
						else {
							dblsize = 1;
							width /= 2;
							height /= 2;
							MoveWindow(hWnd, lastposx, lastposy, width*xdpi, height*ydpi, TRUE);
						}
						InvalidateRect(hWnd, &rect, TRUE);
					}
					else if (lctrldown) {
						ShowWindow(hWnd, SW_HIDE);
					}else if (lwindown) {
						showkeytip = !showkeytip;
						dp("showkeytip %d\n", showkeytip);

					}
					else if (laltdown) {
						autokilltabtip = !autokilltabtip;
						if (autokilltabtip) {
							MessageBoxW(hWnd, L"已经开", L"自动关闭TabTip.exe设置", 0);
						}
					}
					else {
						keybd_event(VK_BACK, 0, 0, 0);
						keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					keybd_event(VK_HOME, 0, 0, 0);
					keybd_event(VK_HOME, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else if (my >= height*ydpi / 6 * 2 && my <= height*ydpi / 6 * 3) {
				float delta = 0;
				dp("tab %i %f\n", mx, (width*xdpi) / 16 * 15);
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16 / 2;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					keybd_event(VK_TAB, 0, 0, 0);
					keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					keybd_event('Q', 0, 0, 0);
					keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					keybd_event('W', 0, 0, 0);
					keybd_event('W', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					keybd_event('E', 0, 0, 0);
					keybd_event('E', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					keybd_event('R', 0, 0, 0);
					keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					keybd_event('T', 0, 0, 0);
					keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					keybd_event('Y', 0, 0, 0);
					keybd_event('Y', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					keybd_event('U', 0, 0, 0);
					keybd_event('U', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					keybd_event('I', 0, 0, 0);
					keybd_event('I', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					keybd_event('O', 0, 0, 0);
					keybd_event('O', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					keybd_event('P', 0, 0, 0);
					keybd_event('P', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 12 + delta) {
					keybd_event(VK_OEM_4, 0, 0, 0);
					keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta&& mx <= (width*xdpi) / 16 * 13 + delta) {
					keybd_event(VK_OEM_6, 0, 0, 0);
					keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta && mx <= (width*xdpi) / 16 * 14 + delta + (width*xdpi) / 16 / 2) {
					keybd_event(VK_OEM_5, 0, 0, 0);
					keybd_event(VK_OEM_5, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					keybd_event(VK_PRIOR, 0, 0, 0);
					keybd_event(VK_PRIOR, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else if (my >= height*ydpi / 6 * 3 && my <= height*ydpi / 6 * 4) {
				float delta = 0; dp("pgdn %i %f\n", mx, (width*xdpi) / 16 * 12);
				if (mx > (width*xdpi) / 16 * 1 + delta) {
					delta = (width*xdpi) / 16 * 0.75;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					keybd_event(VK_CAPITAL, 0, 0, 0);
					keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
					//bCaps = !bCaps;其他按了就有问题;
					InvalidateRect(hWnd, &rect, TRUE);
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					keybd_event('A', 0, 0, 0);
					keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					keybd_event('S', 0, 0, 0);
					keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					keybd_event('D', 0, 0, 0);
					keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					keybd_event('F', 0, 0, 0);
					keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					keybd_event('G', 0, 0, 0);
					keybd_event('G', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					keybd_event('H', 0, 0, 0);
					keybd_event('H', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					keybd_event('J', 0, 0, 0);
					keybd_event('J', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					keybd_event('K', 0, 0, 0);
					keybd_event('K', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					keybd_event('L', 0, 0, 0);
					keybd_event('L', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					keybd_event(VK_OEM_1, 0, 0, 0);
					keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta&& mx <= (width*xdpi) / 16 * 12 + delta) {
					keybd_event(VK_OEM_7, 0, 0, 0);
					keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta && mx <= (width*xdpi) / 16 * 13 + delta + (width*xdpi) / 16 * 1.25) {
					keybd_event(VK_RETURN, 0, 0, 0);
					keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					keybd_event(VK_NEXT, 0, 0, 0);
					keybd_event(VK_NEXT, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else if (my >= height*ydpi / 6 * 4 && my <= height*ydpi / 6 * 5) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					if (lshiftlongdown) {
						keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
						lshiftlongdown = false;
					}
					else if (lshiftdown == false) {
						keybd_event(VK_LSHIFT, 0, 0, 0);
						//keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
						lshiftdown = true;
					}
					else {
						keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
						lshiftdown = false;
					}
					InvalidateRect(hWnd, &rect, TRUE);
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					keybd_event('Z', 0, 0, 0);
					keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					keybd_event('X', 0, 0, 0);
					keybd_event('X', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					keybd_event('C', 0, 0, 0);
					keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					keybd_event('V', 0, 0, 0);
					keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					keybd_event('B', 0, 0, 0);
					keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					keybd_event('N', 0, 0, 0);
					keybd_event('N', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					keybd_event('M', 0, 0, 0);
					keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					keybd_event(VK_OEM_COMMA, 0, 0, 0);
					keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					keybd_event(VK_OEM_PERIOD, 0, 0, 0);
					keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					keybd_event(VK_OEM_2, 0, 0, 0);
					keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 13 + delta) {
					if (rshiftlongdown) {
						keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
						rshiftlongdown = false;
					}
					else if (rshiftdown == false) {
						keybd_event(VK_RSHIFT, 0, 0, 0);
						//keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
						rshiftdown = true;
					}
					else {
						keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
						rshiftdown = false;
					}
					InvalidateRect(hWnd, &rect, TRUE);
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta&& mx <= (width*xdpi) / 16 * 14 + delta) {
					if (laltdown) {
						auto whin = new SheellInfo;
						whin->hWnd = hWnd;
						whin->x = rect.left + 20;
						whin->y = rect.top - 20;
						middleupscrollleft = true;
						middleupscrollright = false;
						middledownscrollleft = false;
						middledownscrollright = false;
						CreateThread(0, 1024 * 1024, wheelupscrollthrd, whin, 0, 0);
					}
					else if (raltdown) {
						auto whin = new SheellInfo;
						whin->hWnd = hWnd;
						whin->x = rect.right - 20;
						whin->y = rect.top - 20;
						middleupscrollleft = false;
						middleupscrollright = true;
						middledownscrollleft = false;
						middledownscrollright = false;
						CreateThread(0, 1024 * 1024, wheelupscrollthrd, whin, 0, 0);
					}
					else if (middleupscrollleft
						|| middleupscrollright
						|| middledownscrollleft
						|| middledownscrollright
						) {
						middleupscrollleft = false;
						middleupscrollright = false;
						middledownscrollleft = false;
						middledownscrollright = false;
					}
					else {
						//keybd_event(VK_RSHIFT, 0, 0, 0);
						keybd_event(VK_UP, 0, 0, 0);
						keybd_event(VK_UP, 0, KEYEVENTF_KEYUP, 0);
						//keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 14 + delta && mx <= (width*xdpi) / 16 * 15 + delta) {
					keybd_event(VK_END, 0, 0, 0);
					keybd_event(VK_END, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else if (my >= height*ydpi / 6 * 5 && my <= height*ydpi / 6 * 6) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 9) {
					delta = (width*xdpi) / 16 * 4;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					if (lctrllongdown) {
						keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
						lctrllongdown = false;
					}
					else if (lctrldown == false) {
						keybd_event(VK_LCONTROL, 0, 0, 0);
						//keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
						lctrldown = true;
					}
					else {
						keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
						lctrldown = false;
					}
					InvalidateRect(hWnd, &rect, TRUE);
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					/*auto kl = GetKeyboardLayout(0);
					if (ImmIsIME(kl)) {
					ImmSimulateHotKey(GetFocus(), IME_THOTKEY_IME_NONIME_TOGGLE);
					}
					else {
					ImmSimulateHotKey(GetFocus(), IME_THOTKEY_IME_NONIME_TOGGLE);
					}*/
					for (int i = 0; i < wcslen(zyh); i += 1) {
						if (zyh[i] == 'S') {
							keybd_event(VK_LSHIFT, 0, 0, 0);
						}
						else if (zyh[i] == 'C') {
							keybd_event(VK_LCONTROL, 0, 0, 0);
						}
						else if (zyh[i] == 'A') {
							keybd_event(VK_LCONTROL, 0, 0, 0);
						}
						else if (zyh[i] == '+') {
							i += 1;
							if (i + 2 <= wcslen(zyh) && wcsncmp(zyh + i, L"SPACE", 2) == 0) {
								keybd_event(VK_SPACE, 0, 0, 0);
								keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
							}
							else if (i + 1 <= wcslen(zyh)) {
								keybd_event(zyh[i], 0, 0, 0);
								keybd_event(zyh[i], 0, KEYEVENTF_KEYUP, 0);
							}
							break;
						}
					}
					for (int i = 0; i < wcslen(zyh); i += 1) {
						if (zyh[i] == 'S') {
							keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
						}
						else if (zyh[i] == 'C') {
							keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
						}
						else if (zyh[i] == 'A') {
							keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
						}
						else if (zyh[i] == '+') {
							break;
						}
					}
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					if (lwinlongdown) {
						keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
						lwinlongdown = false;
					}
					else if (lwindown == false) {
						keybd_event(VK_LWIN, 0, 0, 0);
						//keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
						lwindown = true;
					}
					else {
						keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
						lwindown = false;
					}
					InvalidateRect(hWnd, &rect, TRUE);
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					if (laltlongdown) {
						keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
						laltlongdown = false;
					}
					else if (laltdown == false) {
						keybd_event(VK_LMENU, 0, 0, 0);
						//keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
						laltdown = true;
					}
					else {
						keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
						laltdown = false;
					}
					InvalidateRect(hWnd, &rect, TRUE);
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 9) {
					keybd_event(VK_SPACE, 0, 0, 0);
					keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 && mx <= (width*xdpi) / 16 * 10) {
					if (raltlongdown) {
						keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
						raltlongdown = false;
					}
					else if (raltdown == false) {
						keybd_event(VK_RMENU, 0, 0, 0);
						//keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
						raltdown = true;
					}
					else {
						keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
						raltdown = false;
					}
					InvalidateRect(hWnd, &rect, TRUE);
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					keybd_event(VK_APPS, 0, 0, 0);
					keybd_event(VK_APPS, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					if (rctrllongdown) {
						keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
						rctrllongdown = false;
					}
					else if (rctrldown == false) {
						keybd_event(VK_RCONTROL, 0, 0, 0);
						//keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
						rctrldown = true;
					}
					else {
						keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
						rctrldown = false;
					}
					InvalidateRect(hWnd, &rect, TRUE);
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {

				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 14) {
					//keybd_event(VK_RSHIFT, 0, 0, 0);
					keybd_event(VK_LEFT, 0, 0, 0);
					keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
					//keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 14 && mx <= (width*xdpi) / 16 * 15) {
					if (laltdown) {
						auto whin = new SheellInfo;
						whin->hWnd = hWnd;
						whin->x = rect.left + 20;
						whin->y = rect.top - 20;
						middleupscrollleft = false;
						middleupscrollright = false;
						middledownscrollleft = true;
						middledownscrollright = false;
						CreateThread(0, 1024 * 1024, wheeldownscrollthrd, whin, 0, 0);
					}else if (raltdown) {
						auto whin = new SheellInfo;
						whin->hWnd = hWnd;
						whin->x = rect.right - 20;
						whin->y = rect.top - 20;
						middleupscrollleft = false;
						middleupscrollright = false;
						middledownscrollleft = false;
						middledownscrollright = true;
						CreateThread(0, 1024 * 1024, wheeldownscrollthrd, whin, 0, 0);
					}
					else if (middleupscrollleft
						|| middleupscrollright
						|| middledownscrollleft
						|| middledownscrollright
						) {
						middleupscrollleft = false;
						middleupscrollright = false;
						middledownscrollleft = false;
						middledownscrollright = false;
					}
					else {
						//keybd_event(VK_RSHIFT, 0, 0, 0);
						keybd_event(VK_DOWN, 0, 0, 0);
						keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0);
						//keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//keybd_event(VK_RSHIFT, 0, 0, 0);
					keybd_event(VK_RIGHT, 0, 0, 0);
					keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
					//keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
			}

			if (slshiftdown != lshiftdown || slctrldown != lctrldown || slwindown != lwindown || slaltdown != laltdown
				|| srshiftdown != rshiftdown || srctrldown != rctrldown || sraltdown != raltdown
				) {

			}
			else {
				if (slshiftdown == true) {
					if (lshiftdown) {
						keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
						lshiftdown = false;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				if (slctrldown == true) {
					if (lctrldown) {
						keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
						lctrldown = false;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				if (slwindown == true) {
					if (lwindown) {
						keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
						lwindown = false;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				if (slaltdown == true) {
					if (laltdown) {
						keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
						laltdown = false;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				if (srshiftdown == true) {
					if (rshiftdown) {
						keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
						rshiftdown = false;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				if (srctrldown == true) {
					if (rctrldown) {
						keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
						rctrldown = false;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				if (sraltdown == true) {
					if (raltdown) {
						keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
						raltdown = false;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				//slctrldown = lctrldown, slwindown = lwindown, slaltdown = laltdown;
				//bool srshiftdown = rshiftdown, srctrldown = rctrldown, sraltdown = raltdown;
				//g_LDownKeyName[0] = 0;
				//InvalidateRect(hWnd, &rect, TRUE);
			}
			//InvalidateRect(hWnd, &rect, TRUE);
		}
		mouseupMoveWin = false;
		break;
	}
	case WM_RBUTTONDOWN:
		break;
	case WM_RBUTTONUP:
	{
		int mouse_x = (int)LOWORD(lParam);//取低位
		int mouse_y = (int)HIWORD(lParam);//取高位

										  //DPIInit();
		RECT rect;
		GetClientRect(hWnd, &rect);
		dp("%i,%i, %i,%i,%i,%i\n", mouse_x, mouse_y, rect.right, rect.bottom, rect.left, rect.top);
		if (mouse_x > width*xdpi / 16 * 12 && mouse_x<width*xdpi / 16 * 13 && mouse_y > rect.bottom - height*ydpi / 6) {
			
		}
		else {
			auto mx = mouse_x;
			auto my = mouse_y;
			if (my >= 0 && my <= height*ydpi / 6) {
				if (mx >= (width*xdpi) / 17 * 0 && mx <= (width*xdpi) / 17 * 1) {
					//VK_ESCAPE
				}
				else if (mx >= (width*xdpi) / 17 * 1 && mx <= (width*xdpi) / 17 * 2) {
					//VK_F1
					keybd_event(VK_LCONTROL, 0, 0, 0);
					keybd_event('S', 0, 0, 0);
					keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 2 && mx <= (width*xdpi) / 17 * 3) {
					//VK_F2
					keybd_event(VK_LCONTROL, 0, 0, 0);
					keybd_event('C', 0, 0, 0);
					keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 3 && mx <= (width*xdpi) / 17 * 4) {
					//VK_F3
					keybd_event(VK_LCONTROL, 0, 0, 0);
					keybd_event('V', 0, 0, 0);
					keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 17 * 4 && mx <= (width*xdpi) / 17 * 5) {
					//VK_F4
					if (GetKeyState(VK_LCONTROL) < 0) {
						if (f4v != NULL)free(f4v);
						f4v = GetClipboardText();
					}
					else {
						auto tmp = GetClipboardText();
						if (f4v != NULL) {
							SetClipboardText(f4v, wcslen(f4v));
							keybd_event(VK_LCONTROL, 0, 0, 0);
							keybd_event('V', 0, 0, 0);
							keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
							Sleep(100);
							if(tmp!=NULL)SetClipboardText(tmp, wcslen(tmp));
						}
					}
				}
				else if (mx >= (width*xdpi) / 17 * 5 && mx <= (width*xdpi) / 17 * 6) {
					//VK_F5
					if (GetKeyState(VK_LCONTROL) < 0) {
						if (f5v != NULL)free(f5v);
						f5v = GetClipboardText();
					}
					else {
						auto tmp = GetClipboardText();
						if (f5v != NULL) {
							SetClipboardText(f5v, wcslen(f5v));
							keybd_event(VK_LCONTROL, 0, 0, 0);
							keybd_event('V', 0, 0, 0);
							keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
							Sleep(100);
							if(tmp!=NULL)SetClipboardText(tmp, wcslen(tmp));
						}
					}
				}
				else if (mx >= (width*xdpi) / 17 * 6 && mx <= (width*xdpi) / 17 * 7) {
					//VK_F6
					if (GetKeyState(VK_LCONTROL) < 0) {
						if (f6v != NULL)free(f6v);
						f6v = GetClipboardText();
					}
					else {
						auto tmp = GetClipboardText();
						if (f6v != NULL) {
							SetClipboardText(f6v, wcslen(f6v));
							keybd_event(VK_LCONTROL, 0, 0, 0);
							keybd_event('V', 0, 0, 0);
							keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
							keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
							Sleep(100);
							if(tmp!=NULL)SetClipboardText(tmp, wcslen(tmp));
						}
					}
				}
				else if (mx >= (width*xdpi) / 17 * 7 && mx <= (width*xdpi) / 17 * 8) {
					//VK_F7
				}
				else if (mx >= (width*xdpi) / 17 * 8 && mx <= (width*xdpi) / 17 * 9) {
					//VK_F8
				}
				else if (mx >= (width*xdpi) / 17 * 9 && mx <= (width*xdpi) / 17 * 10) {
					//VK_F9
				}
				else if (mx >= (width*xdpi) / 17 * 10 && mx <= (width*xdpi) / 17 * 11) {
					//VK_F10
				}
				else if (mx >= (width*xdpi) / 17 * 11 && mx <= (width*xdpi) / 17 * 12) {
					//VK_F11
				}
				else if (mx >= (width*xdpi) / 17 * 12 && mx <= (width*xdpi) / 17 * 13) {
					//VK_F12
				}
				else if (mx >= (width*xdpi) / 17 * 13 && mx <= (width*xdpi) / 17 * 14) {
					//VK_SNAPSHOT
				}
				else if (mx >= (width*xdpi) / 17 * 14 && mx <= (width*xdpi) / 17 * 15) {
					//VK_PAUSE
				}
				else if (mx >= (width*xdpi) / 17 * 15 && mx <= (width*xdpi) / 17 * 16) {
					//VK_INSERT
				}
				else if (mx >= (width*xdpi) / 17 * 16 && mx <= (width*xdpi) / 17 * 17) {
					//VK_DELETE
				}
			}
			else if (my >= height*ydpi / 6 && my <= height*ydpi / 6 * 2) {
				if (mx >= (width*xdpi) / 16 * 0 && mx <= (width*xdpi) / 16 * 1) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_3, 0, 0, 0);
					keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 1 && mx <= (width*xdpi) / 16 * 2) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('1', 0, 0, 0);
					keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 2 && mx <= (width*xdpi) / 16 * 3) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('2', 0, 0, 0);
					keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 3 && mx <= (width*xdpi) / 16 * 4) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('3', 0, 0, 0);
					keybd_event('3', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 5) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('4', 0, 0, 0);
					keybd_event('4', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 5 && mx <= (width*xdpi) / 16 * 6) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('5', 0, 0, 0);
					keybd_event('5', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 6 && mx <= (width*xdpi) / 16 * 7) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('6', 0, 0, 0);
					keybd_event('6', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 7 && mx <= (width*xdpi) / 16 * 8) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('7', 0, 0, 0);
					keybd_event('7', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 8 && mx <= (width*xdpi) / 16 * 9) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('8', 0, 0, 0);
					keybd_event('8', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 && mx <= (width*xdpi) / 16 * 10) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('9', 0, 0, 0);
					keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('0', 0, 0, 0);
					keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_MINUS, 0, 0, 0);
					keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_PLUS, 0, 0, 0);
					keybd_event(VK_OEM_PLUS, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 15) {
					//VK_BACK
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//VK_HOME
					keybd_event(VK_LCONTROL, 0, 0, 0);
					keybd_event('X', 0, 0, 0);
					keybd_event('X', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else if (my >= height*ydpi / 6 * 2 && my <= height*ydpi / 6 * 3) {
				float delta = 0;
				dp("tab %i %f\n", mx, (width*xdpi) / 16 * 15);
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16 / 2;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//VK_TAB
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('Q', 0, 0, 0);
					keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('W', 0, 0, 0);
					keybd_event('W', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('E', 0, 0, 0);
					keybd_event('E', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('R', 0, 0, 0);
					keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('T', 0, 0, 0);
					keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('Y', 0, 0, 0);
					keybd_event('Y', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('U', 0, 0, 0);
					keybd_event('U', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('I', 0, 0, 0);
					keybd_event('I', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('O', 0, 0, 0);
					keybd_event('O', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('P', 0, 0, 0);
					keybd_event('P', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 12 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_4, 0, 0, 0);
					keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta&& mx <= (width*xdpi) / 16 * 13 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_6, 0, 0, 0);
					keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta && mx <= (width*xdpi) / 16 * 14 + delta + (width*xdpi) / 16 / 2) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_5, 0, 0, 0);
					keybd_event(VK_OEM_5, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//VK_PRIOR
					keybd_event(VK_LCONTROL, 0, 0, 0);
					keybd_event('C', 0, 0, 0);
					keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else if (my >= height*ydpi / 6 * 3 && my <= height*ydpi / 6 * 4) {
				float delta = 0; dp("pgdn %i %f\n", mx, (width*xdpi) / 16 * 12);
				if (mx > (width*xdpi) / 16 * 1 + delta) {
					delta = (width*xdpi) / 16 * 0.75;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//VK_CAPITAL
					keybd_event(VK_NUMLOCK, 0, 0, 0);
					keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('A', 0, 0, 0);
					keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('S', 0, 0, 0);
					keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('D', 0, 0, 0);
					keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('F', 0, 0, 0);
					keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('G', 0, 0, 0);
					keybd_event('G', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('H', 0, 0, 0);
					keybd_event('H', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('J', 0, 0, 0);
					keybd_event('J', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('K', 0, 0, 0);
					keybd_event('K', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('L', 0, 0, 0);
					keybd_event('L', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_1, 0, 0, 0);
					keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta&& mx <= (width*xdpi) / 16 * 12 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_7, 0, 0, 0);
					keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 12 + delta && mx <= (width*xdpi) / 16 * 13 + delta + (width*xdpi) / 16 * 1.25) {
					//VK_RETURN
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//VK_NEXT
					keybd_event(VK_LCONTROL, 0, 0, 0);
					keybd_event('V', 0, 0, 0);
					keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else if (my >= height*ydpi / 6 * 4 && my <= height*ydpi / 6 * 5) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 1) {
					delta = (width*xdpi) / 16;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//VK_LSHIFT
					if (lshiftdown == false) {
						keybd_event(VK_LSHIFT, 0, 0, 0);
						lshiftlongdown = true;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('Z', 0, 0, 0);
					keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('X', 0, 0, 0);
					keybd_event('X', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('C', 0, 0, 0);
					keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 4 + delta&& mx <= (width*xdpi) / 16 * 5 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('V', 0, 0, 0);
					keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 5 + delta && mx <= (width*xdpi) / 16 * 6 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('B', 0, 0, 0);
					keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 6 + delta&& mx <= (width*xdpi) / 16 * 7 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('N', 0, 0, 0);
					keybd_event('N', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 7 + delta && mx <= (width*xdpi) / 16 * 8 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event('M', 0, 0, 0);
					keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 8 + delta && mx <= (width*xdpi) / 16 * 9 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_COMMA, 0, 0, 0);
					keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_PERIOD, 0, 0, 0);
					keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 10 + delta && mx <= (width*xdpi) / 16 * 11 + delta) {
					keybd_event(VK_LSHIFT, 0, 0, 0);
					keybd_event(VK_OEM_2, 0, 0, 0);
					keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 11 + delta && mx <= (width*xdpi) / 16 * 13 + delta) {
					//VK_RSHIFT
					if (rshiftdown == false) {
						keybd_event(VK_RSHIFT, 0, 0, 0);
						rshiftlongdown = true;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 13 + delta&& mx <= (width*xdpi) / 16 * 14 + delta) {
					//VK_UP
					//ShowWindow(hWnd, SW_HIDE);
					//Sleep(100);
					//mouse_event(MOUSEEVENTF_MOVE, -10, -10, 0, 0);
					//CreateThread(0, 1024 * 1024, wheeluthrd, hWnd, 0, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 14 + delta && mx <= (width*xdpi) / 16 * 15 + delta) {
					//VK_END
				}
			}
			else if (my >= height*ydpi / 6 * 5 && my <= height*ydpi / 6 * 6) {
				float delta = 0;
				if (mx > (width*xdpi) / 16 * 9) {
					delta = (width*xdpi) / 16 * 4;
				}
				if (mx >= (width*xdpi) / 16 * 0 + delta && mx <= (width*xdpi) / 16 * 1 + delta) {
					//VK_LCONTROL
					if (lctrldown == false) {
						keybd_event(VK_LCONTROL, 0, 0, 0);
						lctrllongdown = true;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 1 + delta && mx <= (width*xdpi) / 16 * 2 + delta) {
					//中英
				}
				else if (mx >= (width*xdpi) / 16 * 2 + delta && mx <= (width*xdpi) / 16 * 3 + delta) {
					//VK_LWIN
					if (lwindown == false) {
						keybd_event(VK_LWIN, 0, 0, 0);
						lwinlongdown = true;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 3 + delta&& mx <= (width*xdpi) / 16 * 4 + delta) {
					//VK_LMENU
					if (laltdown == false) {
						keybd_event(VK_LMENU, 0, 0, 0);
						laltlongdown = true;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 4 && mx <= (width*xdpi) / 16 * 9) {
					//VK_SPACE
				}
				else if (mx >= (width*xdpi) / 16 * 9 + delta && mx <= (width*xdpi) / 16 * 10) {
					//VK_RMENU
					if (raltdown == false) {
						keybd_event(VK_RMENU, 0, 0, 0);
						raltlongdown = true;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 10 && mx <= (width*xdpi) / 16 * 11) {
					//VK_APPS
				}
				else if (mx >= (width*xdpi) / 16 * 11 && mx <= (width*xdpi) / 16 * 12) {
					//VK_RCONTROL
					if (rctrldown == false) {
						keybd_event(VK_RCONTROL, 0, 0, 0);
						rctrllongdown = true;
						InvalidateRect(hWnd, &rect, TRUE);
					}
				}
				else if (mx >= (width*xdpi) / 16 * 12 && mx <= (width*xdpi) / 16 * 13) {
					//五角星
				}
				else if (mx >= (width*xdpi) / 16 * 13 && mx <= (width*xdpi) / 16 * 14) {
					//VK_LEFT
					keybd_event(VK_LCONTROL, 0, 0, 0);
					keybd_event('Z', 0, 0, 0);
					keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 14 && mx <= (width*xdpi) / 16 * 15) {
					//VK_DOWN
					//ShowWindow(hWnd, SW_HIDE);
					//Sleep(100);
					//mouse_event(MOUSEEVENTF_MOVE, -10,-10, 0, 0);
					//CreateThread(0, 1024 * 1024, wheeldthrd, hWnd, 0, 0);
				}
				else if (mx >= (width*xdpi) / 16 * 15 && mx <= (width*xdpi) / 16 * 16) {
					//VK_RIGHT
					keybd_event(VK_LCONTROL, 0, 0, 0);
					keybd_event('Y', 0, 0, 0);
					keybd_event('Y', 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
				}
			}
		}
	}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
			
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
			Rectangle(hdc, 0, 0, width*xdpi, height*ydpi);

			for (int i = 0; i < 17; i += 1) {
				MoveToEx(hdc, int(((width*xdpi) / 17)*i), 0, 0);
				LineTo(hdc, int(((width*xdpi) / 17)*i), (height*ydpi) / 6);
				//dp("%i,%f,%f,%f\n", int(((width*xdpi) / 17)*i),width,xdpi,width*xdpi);
				//Rectangle(hdc, int(((width*xdpi)/17)*i), 0, (width*xdpi)/17, (height*ydpi)/6);
				RECT rt;
				wchar_t *name;
				if (i == 0) {
					name = L"ESC";
				}else if (i == 1) {
					name = L"F1";
				}
				else if (i == 2) {
					name = L"F2";
				}
				else if (i == 3) {
					name = L"F3";
				}
				else if (i == 4) {
					name = L"F4";
				}
				else if (i == 5) {
					name = L"F5";
				}
				else if (i == 6) {
					name = L"F6";
				}
				else if (i == 7) {
					name = L"F7";
				}
				else if (i == 8) {
					name = L"F8";
				}
				else if (i == 9) {
					name = L"F9";
				}
				else if (i == 10) {
					name = L"F10";
				}
				else if (i == 11) {
					name = L"F11";
				}
				else if (i == 12) {
					name = L"F12";
				}
				else if (i == 13) {
					name = L"Pri";
				}
				else if (i == 14) {
					name = L"Pau";
				}
				else if (i == 15) {
					name = L"Ins";
				}
				else if (i == 16) {
					name = L"Del";
				}
				rt.left = ((width*xdpi) / 17)*i;
				rt.right = rt.left + (width*xdpi) / 17;
				rt.top = 0;
				rt.bottom = rt.top+(height*ydpi) / 6;
				DrawText(hdc, name, -1, &rt, DT_SINGLELINE | DT_CENTER| DT_VCENTER);
			}
			MoveToEx(hdc, 0, (height*ydpi) / 6, 0);
			LineTo(hdc, width*xdpi, (height*ydpi) / 6);
			
			//line 2
			for (int i = 0; i < 16; i += 1) {
				if (i == 14)continue;
				MoveToEx(hdc, int(((width*xdpi) / 16)*i), (height*ydpi) / 6, 0);
				LineTo(hdc, int(((width*xdpi) / 16)*i), (height*ydpi) / 6*2);
				RECT rt;
				wchar_t *name;
				if (i == 0) {
					name = L"`";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"~";
					}
				}
				else if (i == 1) {
					name = L"1";
					if ((lshiftdown || rshiftdown)&&!(lctrldown||rctrldown||lwindown||laltdown||raltdown)) {
						name = L"!";
					}
				}
				else if (i == 2) {
					name = L"2";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"@";
					}
				}
				else if (i == 3) {
					name = L"3";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"#";
					}
				}
				else if (i == 4) {
					name = L"4";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"$";
					}
				}
				else if (i == 5) {
					name = L"5";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"%";
					}
				}
				else if (i == 6) {
					name = L"6";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"^";
					}
				}
				else if (i == 7) {
					name = L"7";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"&&";
					}
				}
				else if (i == 8) {
					name = L"8";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"*";
					}
				}
				else if (i == 9) {
					name = L"9";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"(";
					}
				}
				else if (i == 10) {
					name = L"0";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L")";
					}
				}
				else if (i == 11) {
					name = L"-";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"_";
					}
				}
				else if (i == 12) {
					name = L"=";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"+";
					}
				}
				else if (i == 13) {
					name = L"←";
				}
				else if (i == 14) {
					
				}
				else if (i == 15) {
					name = L"Hom";
				}
				rt.left = ((width*xdpi) / 16)*i;
				rt.right = rt.left + (width*xdpi) / 16;
				if (i == 13) {
					rt.right = rt.left + (width*xdpi) / 16*2;
				}
				rt.top = (height*ydpi) / 6*1;
				rt.bottom = rt.top + (height*ydpi) / 6;
				DrawText(hdc, name, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
			MoveToEx(hdc, 0, (height*ydpi) / 6*2, 0);
			LineTo(hdc, width*xdpi, (height*ydpi) / 6*2);

			//line 3
			float delta = 0;
			for (int i = 0; i < 16; i += 1) {
				if (i == 15) {
					continue;
				}
				if (i == 1) {
					delta = (width*xdpi) / 16/2;
				}
				else if (i == 14) {
					delta = (width*xdpi) / 16;
				}
				MoveToEx(hdc, int(((width*xdpi) / 16)*i)+ delta, (height*ydpi) / 6*2, 0);
				LineTo(hdc, int(((width*xdpi) / 16)*i)+ delta, (height*ydpi) / 6 * 3);
				RECT rt;
				wchar_t *name;
				if (i == 0) {
					name = L"Tab";
				}
				else if (i == 1) {
					name = L"q";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"Q";
					}
				}
				else if (i == 2) {
					name = L"w";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"W";
					}
				}
				else if (i == 3) {
					name = L"e";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"E";
					}
				}
				else if (i == 4) {
					name = L"r";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"R";
					}
				}
				else if (i == 5) {
					name = L"t";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"T";
					}
				}
				else if (i == 6) {
					name = L"y";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"Y";
					}
				}
				else if (i == 7) {
					name = L"u";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"U";
					}
				}
				else if (i == 8) {
					name = L"i";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"I";
					}
				}
				else if (i == 9) {
					name = L"o";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"O";
					}
				}
				else if (i == 10) {
					name = L"p";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"P";
					}
				}
				else if (i == 11) {
					name = L"[";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"{";
					}
				}
				else if (i == 12) {
					name = L"]";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"}";
					}
				}
				else if (i == 13) {
					name = L"\\";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"|";
					}
				}else if (i == 14) {
					name = L"PgUp";
				}
				rt.left = ((width*xdpi) / 16)*i+delta;
				rt.right = rt.left + (width*xdpi) / 16;
				if (i == 0) {
					rt.right = rt.left + (width*xdpi) / 16 * 1.5;
				}
				if (i == 13) {
					rt.right = rt.left + (width*xdpi) / 16 * 1.5;
				}
				rt.top = (height*ydpi) / 6 * 2;
				rt.bottom = rt.top + (height*ydpi) / 6;
				DrawText(hdc, name, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
			MoveToEx(hdc, 0, (height*ydpi) / 6 * 3, 0);
			LineTo(hdc, width*xdpi, (height*ydpi) / 6 * 3);


			//line 4
			delta = 0;
			for (int i = 0; i < 16; i += 1) {
				int txtcolor = 0;
				if (i == 14||i == 15) {
					continue;
				}
				if (i == 1) {
					delta = (width*xdpi) / 16 *0.75;
				}
				else if (i == 13) {
					delta = (width*xdpi) / 16*2;
				}
				MoveToEx(hdc, int(((width*xdpi) / 16)*i) + delta, (height*ydpi) / 6 * 3, 0);
				LineTo(hdc, int(((width*xdpi) / 16)*i) + delta, (height*ydpi) / 6 * 4);
				RECT rt;
				wchar_t *name;
				if (i == 0) {
					name = L"Caps";
					bCaps = GetKeyState(VK_CAPITAL) == 1 ? TRUE : FALSE;
					if (bCaps) {
						txtcolor = 255;
					}
				}
				else if (i == 1) {
					name = L"a";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"A";
					}
				}
				else if (i == 2) {
					name = L"s";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"S";
					}
				}
				else if (i == 3) {
					name = L"d";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"D";
					}
				}
				else if (i == 4) {
					name = L"f";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"F";
					}
				}
				else if (i == 5) {
					name = L"g";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"G";
					}
				}
				else if (i == 6) {
					name = L"h";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"H";
					}
				}
				else if (i == 7) {
					name = L"j";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"J";
					}
				}
				else if (i == 8) {
					name = L"k";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"K";
					}
				}
				else if (i == 9) {
					name = L"l";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"L";
					}
				}
				else if (i == 10) {
					name = L";";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L":";
					}
				}
				else if (i == 11) {
					name = L"'";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"\"";
					}
				}
				else if (i == 12) {
					name = L"Return";
				}
				else if (i == 13) {
					name = L"PgDn";
				}
				rt.left = ((width*xdpi) / 16)*i + delta;
				rt.right = rt.left + (width*xdpi) / 16;
				if (i == 0) {
					rt.right = rt.left + (width*xdpi) / 16 * 1.75;
				}
				if (i == 12) {
					rt.right = rt.left + (width*xdpi) / 16 * 2.25;
				}
				rt.top = (height*ydpi) / 6 * 3;
				rt.bottom = rt.top + (height*ydpi) / 6;

				//修改颜色
				SetTextColor(hdc, RGB(txtcolor, 0, 0)); //设置字体颜色
				//SetBkColor(hdc, RGB(0, 255, 255)); //设置背景色
				//SetBkMode(hdc, OPAQUE); //非透明模式
				//SetBkMode(hdc, TRANSPARENT); //透明模式
				//1.创建字体
				//hfont hfont = createfont(0, //设置宽度30
				//	0, //设置0时，按汉字系统默认的宽度给一个值
				//	0, 0,
				//	0, //字体粗细900是默认，也不是一个很粗的值
				//	0,//斜体
				//	0,//下划线
				//	0,//中间划线
				//	gb2312_charset, //国标
				//	0, 0, 0, 0,
				//	l"黑体");
				////2.设置字体
				//hgdiobj holdgdiobj = selectobject(hdc, (hgdiobj)hfont);
				//3.画字体
				DrawText(hdc, name, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				//4.把旧的字体还回系统
				//SelectObject(hdc, hOldGdiobj);
			}
			MoveToEx(hdc, 0, (height*ydpi) / 6 * 4, 0);
			LineTo(hdc, width*xdpi, (height*ydpi) / 6 * 4);

			
			//line 5
			delta = 0;
			for (int i = 0; i < 16; i += 1) {
				int txtcolor = 0, blue = 0;
				if (i == 14 || i == 15) {
					continue;
				}
				if (i == 1) {
					delta = (width*xdpi) / 16;
				}
				else if (i == 12) {
					delta = (width*xdpi) / 16 * 2;
				}
				MoveToEx(hdc, int(((width*xdpi) / 16)*i) + delta, (height*ydpi) / 6 * 4, 0);
				LineTo(hdc, int(((width*xdpi) / 16)*i) + delta, (height*ydpi) / 6 * 5);
				RECT rt;
				wchar_t *name;
				if (i == 0) {
					name = L"Shift";
					if (lshiftdown) {
						txtcolor = 255;
					}
					else if (lshiftlongdown) {
						blue = 255;
					}
				}
				else if (i == 1) {
					name = L"z";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"Z";
					}
				}
				else if (i == 2) {
					name = L"x";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"X";
					}
				}
				else if (i == 3) {
					name = L"c";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"C";
					}
				}
				else if (i == 4) {
					name = L"v";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"V";
					}
				}
				else if (i == 5) {
					name = L"b";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"B";
					}
				}
				else if (i == 6) {
					name = L"n";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"N";
					}
				}
				else if (i == 7) {
					name = L"m";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"M";
					}
				}
				else if (i == 8) {
					name = L",";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"<";
					}
				}
				else if (i == 9) {
					name = L".";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L">";
					}
				}
				else if (i == 10) {
					name = L"/";
					if ((lshiftdown || rshiftdown) && !(lctrldown || rctrldown || lwindown || laltdown || raltdown)) {
						name = L"?";
					}
				}
				else if (i == 11) {
					name = L"Shift";
					if (rshiftdown) {
						txtcolor = 255;
					}
					else if (rshiftlongdown) {
						blue = 255;
					}
				}
				else if (i == 12) {
					name = L"↑";
				}
				else if (i == 13) {
					name = L"End";
				}
				rt.left = ((width*xdpi) / 16)*i + delta;
				rt.right = rt.left + (width*xdpi) / 16;
				if (i == 0) {
					rt.right = rt.left + (width*xdpi) / 16 * 1.75;
				}
				if (i == 11) {
					rt.right = rt.left + (width*xdpi) / 16 * 2.25;
				}
				rt.top = (height*ydpi) / 6 * 4;
				rt.bottom = rt.top + (height*ydpi) / 6;

				//修改颜色
				SetTextColor(hdc, RGB(txtcolor, 0, blue)); //设置字体颜色
				//SetBkColor(hdc, RGB(0, 255, 255)); //设置背景色
				//SetBkMode(hdc, OPAQUE); //非透明模式
				//SetBkMode(hdc, TRANSPARENT); //透明模式
				//1.创建字体
				//hfont hfont = createfont(0, //设置宽度30
				//	0, //设置0时，按汉字系统默认的宽度给一个值
				//	0, 0,
				//	0, //字体粗细900是默认，也不是一个很粗的值
				//	0,//斜体
				//	0,//下划线
				//	0,//中间划线
				//	gb2312_charset, //国标
				//	0, 0, 0, 0,
				//	l"黑体");
				////2.设置字体
				//hgdiobj holdgdiobj = selectobject(hdc, (hgdiobj)hfont);
				//3.画字体
				DrawText(hdc, name, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				//4.把旧的字体还回系统
				//SelectObject(hdc, hOldGdiobj);
			}
			MoveToEx(hdc, 0, (height*ydpi) / 6 * 5, 0);
			LineTo(hdc, width*xdpi, (height*ydpi) / 6 * 5);


			//line 6
			delta = 0;
			for (int i = 0; i < 16; i += 1) {
				int txtcolor = 0,blue=0;
				if (i >=12 && i<= 15) {
					continue;
				}
				if (i == 5) {
					delta = (width*xdpi) / 16*4;
				}
				MoveToEx(hdc, int(((width*xdpi) / 16)*i) + delta, (height*ydpi) / 6 * 5, 0);
				LineTo(hdc, int(((width*xdpi) / 16)*i) + delta, (height*ydpi) / 6 * 6);
				RECT rt;
				wchar_t *name;
				if (i == 0) {
					name = L"Ctrl";
					if (lctrldown) {
						txtcolor = 255;
					}
					else if (lctrllongdown) {
						blue = 255;
					}
				}
				else if (i == 1) {
					//auto foregroundwin = GetForegroundWindow();
					//auto thrdid=GetWindowThreadProcessId(foregroundwin, 0);
					//auto layout = GetKeyboardLayout(thrdid);
					//FILE *ff = fopen("aaa.txt", "a+");
					//fseek(ff, 0, SEEK_END);
					//char ibuf[30] = { 0 };
					//itoa(int(layout), ibuf, 10);
					//fwrite(ibuf, 1, strlen(ibuf),ff);
					//fwrite("\r\n", 1, 2, ff);
					//fclose(ff);
					//if (int(layout) == 0x409) {
					//	name = L"英";
					//}else if (int(layout) == 0x804) {
					//	name = L"中";
					//}
					//else 
					{
						name = L"中/英";
					}
				}
				else if (i == 2) {
					name = L"Win";
					if (lwindown) {
						txtcolor = 255;
					}
					else if (lwinlongdown) {
						blue = 255;
					}
				}
				else if (i == 3) {
					name = L"Alt";
					if (laltdown) {
						txtcolor = 255;
					}
					else if (laltlongdown) {
						blue = 255;
					}
				}
				else if (i == 4) {
					name = L"Space";
				}
				else if (i == 5) {
					name = L"Alt";
					if (raltdown) {
						txtcolor = 255;
					}
					else if (raltlongdown) {
						blue = 255;
					}
				}
				else if (i == 6) {
					name = L"App";
				}
				else if (i == 7) {
					name = L"Ctrl";
					if (rctrldown) {
						txtcolor = 255;
					}
					else if (rctrllongdown) {
						blue = 255;
					}
				}
				else if (i == 8) {
					name = L"★";
				}
				else if (i == 9) {
					name = L"←";
				}
				else if (i == 10) {
					name = L"↓";
				}
				else if (i == 11) {
					name = L"→";
				}
				rt.left = ((width*xdpi) / 16)*i + delta;
				rt.right = rt.left + (width*xdpi) / 16;
				if (i == 4) {
					rt.right = rt.left + (width*xdpi) / 16 * 5;
				}
				rt.top = (height*ydpi) / 6 * 5;
				rt.bottom = rt.top + (height*ydpi) / 6;

				//修改颜色
				SetTextColor(hdc, RGB(txtcolor, 0, blue)); //设置字体颜色
				//SetBkColor(hdc, RGB(0, 255, 255)); //设置背景色
				//SetBkMode(hdc, OPAQUE); //非透明模式
				//SetBkMode(hdc, TRANSPARENT); //透明模式
				//1.创建字体
				//HFONT hFont = CreateFont(0, //设置宽度30
				//	0, //设置0时，按汉字系统默认的宽度给一个值
				//	0, 0,
				//	0, //字体粗细900是默认，也不是一个很粗的值
				//	0,//斜体
				//	0,//下划线
				//	0,//中间划线
				//	GB2312_CHARSET, //国标
				//	0, 0, 0, 0,
				//	L"黑体");
				////2.设置字体
				//HGDIOBJ hOldGdiobj = SelectObject(hdc, (HGDIOBJ)hFont);
				//3.画字体
				DrawText(hdc, name, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				//4.把旧的字体还回系统
				//SelectObject(hdc, hOldGdiobj);

			}
			MoveToEx(hdc, 0, (height*ydpi) / 6 * 6, 0);
			LineTo(hdc, width*xdpi, (height*ydpi) / 6 * 6);

			dp("paint showkeytip %d\n", showkeytip == true);
			if (showkeytip == true) {
				if (g_LDownKeyName[0] != 0) {
					RECT rt;
					if (!(wcsicmp(g_LDownKeyName, L"esc") == 0 || wcsicmp(g_LDownKeyName, L"f1") == 0 || wcsicmp(g_LDownKeyName, L"f2") == 0 || wcsicmp(g_LDownKeyName, L"f3") == 0
						|| wcsicmp(g_LDownKeyName, L"`") == 0 || wcsicmp(g_LDownKeyName, L"~") == 0 || wcsicmp(g_LDownKeyName, L"1") == 0 || wcsicmp(g_LDownKeyName, L"!") == 0 || wcsicmp(g_LDownKeyName, L"2") == 0 || wcsicmp(g_LDownKeyName, L"@") == 0 || wcsicmp(g_LDownKeyName, L"3") == 0 || wcsicmp(g_LDownKeyName, L"#") == 0
						|| wcsicmp(g_LDownKeyName, L"tab") == 0 || wcsicmp(g_LDownKeyName, L"q") == 0 || wcsicmp(g_LDownKeyName, L"w") == 0 || wcsicmp(g_LDownKeyName, L"e") == 0
						|| wcsicmp(g_LDownKeyName, L"cap") == 0 || wcsicmp(g_LDownKeyName, L"a") == 0 || wcsicmp(g_LDownKeyName, L"s") == 0 || wcsicmp(g_LDownKeyName, L"d") == 0
						)) {
						rt.left = 10;
						rt.right = height*ydpi / 2;
						rt.top = 10;
						rt.bottom = height*ydpi / 2;
					}
					else {
						rt.left = width*xdpi - height*ydpi / 2 - 10;
						rt.right = width*xdpi - 10;
						rt.top = 10;
						rt.bottom = height*ydpi / 2;
					}
					//修改颜色
					if (GetKeyState(VK_CONTROL) < 0) {
						SetTextColor(hdc, RGB(0, 0, 255)); //设置字体颜色
					}
					else if (GetKeyState(VK_MENU) < 0) {
						SetTextColor(hdc, RGB(0, 255, 255)); //设置字体颜色
					}
					else {
						SetTextColor(hdc, RGB(255, 0, 0)); //设置字体颜色
					}
					SetBkColor(hdc, RGB(255, 255, 255)); //设置背景色
					SetBkMode(hdc, OPAQUE); //非透明模式


					HFONT hFont;
					LOGFONT lFont;
					memset(&lFont, 0, sizeof(LOGFONT));
					//计算字体高度与磅值 
					int nNumerator = ::GetDeviceCaps(hdc, LOGPIXELSY);
					lFont.lfHeight = height*ydpi / 4;
					lFont.lfWeight = height*ydpi / 2;
					//创建字体 
					hFont = ::CreateFontIndirect(&lFont);
					//选择字体 
					HGDIOBJ hOldFont = ::SelectObject(hdc, hFont);
					DrawText(hdc, g_LDownKeyName, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
					//释放GDI资源 
					::SelectObject(hdc, hOldFont);
					BOOL bRes = ::DeleteObject(hFont);
				}
			}


            EndPaint(hWnd, &ps);
        }
        break;
	case WM_HOTKEY:
		dp("hotkeyid:%d\n",wParam);
		switch(wParam){
		//case KBHOTKEYID:
		//	if (IsWindowVisible(hWnd)) {
		//		ShowWindow(hWnd, SW_HIDE);
		//	}
		//	else {
		//		bCaps = GetKeyState(VK_CAPITAL) == 1 ? TRUE : FALSE;
		//		ShowWindow(hWnd, SW_SHOW);
		//		KillProcessidByName(L"TabTip.exe");
		//	}
		//	break;
		case HK_ALTSHIFT_K_KILLTIP:
			bCaps = GetKeyState(VK_CAPITAL) == 1 ? TRUE : FALSE;
			ShowWindow(hWnd, SW_SHOW);
			KillProcessidByName(L"TabTip.exe");
			break;
		case HK_AUP_ID:
			mouse_event(MOUSEEVENTF_MOVE, 0, -GStep, 0, 0);
			break;
		case HK_ADN_ID:
			mouse_event(MOUSEEVENTF_MOVE, 0, GStep, 0, 0);
			break;
		case HK_ALT_ID:
			mouse_event(MOUSEEVENTF_MOVE, -GStep, 0, 0, 0);
			break;
		case HK_ART_ID:
			mouse_event(MOUSEEVENTF_MOVE, GStep, 0, 0, 0);
			break;
		case HK_SAH_ID:
			if (IsWindowVisible(hWnd) == true) {
				ShowWindow(hWnd, SW_HIDE);
			}
			else {
				ShowWindow(hWnd, SW_SHOW);
			}
			break;
		case HK_SAU_ID:
			mouse_event(MOUSEEVENTF_MOVE, 0, -GStep*2, 0, 0);
			break;
		case HK_SAD_ID:
			mouse_event(MOUSEEVENTF_MOVE, 0, GStep*2, 0, 0);
			break;
		case HK_SAL_ID:
			mouse_event(MOUSEEVENTF_MOVE, -GStep*2, 0, 0, 0);
			break;
		case HK_SAR_ID:
			mouse_event(MOUSEEVENTF_MOVE, GStep*2, 0, 0, 0);
			break;
		case HK_SAPLUS_LU_ID:
			mouse_event(MOUSEEVENTF_MOVE, -GStep * 2, -GStep * 2, 0, 0);
			break;
		case HK_SARB_LD_ID:
			mouse_event(MOUSEEVENTF_MOVE, -GStep * 2, GStep * 2, 0, 0);
			break;
		case HK_SAB_RU_ID:
			mouse_event(MOUSEEVENTF_MOVE, GStep * 2, -GStep * 2, 0, 0);
			break;
		case HK_SABS_RD_ID:
			mouse_event(MOUSEEVENTF_MOVE, GStep * 2, GStep * 2, 0, 0);
			break;
		//case HK_ALTCOMMA_MB2P_ID:
		//	mouse_event(MOUSEEVENTF_MOVE,  -2, 0, 0, 0);
		//	break;
		//case HK_ALTPRERIOD_MB2P_ID:
		//	mouse_event(MOUSEEVENTF_MOVE, 2, 0, 0, 0);
		//	break;
		//case HK_ALTLB_MB2P_ID:
		//	mouse_event(MOUSEEVENTF_MOVE,  0, -2, 0, 0);
		//	break;
		//case HK_ALTCONON_MB2P_ID:
		//	mouse_event(MOUSEEVENTF_MOVE, 0, 2, 0, 0);
		//	break;

		//case HK_CTRLSHIFT_U_ID:
		//	mouse_event(MOUSEEVENTF_MOVE, 0, -2, 0, 0);
		//	break;
		//case HK_CTRLSHIFT_D_ID:
		//	mouse_event(MOUSEEVENTF_MOVE, 0, 2, 0, 0);
		//	break;
		//case HK_CTRLSHIFT_L_ID:
		//	mouse_event(MOUSEEVENTF_MOVE, -2, 0, 0, 0);
		//	break;
		//case HK_CTRLSHIFT_R_ID:
		//	mouse_event(MOUSEEVENTF_MOVE, 2, 0, 0, 0);
		//	break;


		case HK_AINS_ID:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case HK_ADEL_ID:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case HK_SAHM_ID:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		case HK_SAED_ID:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		case HK_SAPU_ID:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case HK_SAPD_ID:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;




		case HK_WINS_LDBLCLK_ID:
			CreateThread(0, 1024 * 1024, winaltins_ldblclick, 0, 0, 0);
			break;
		case HK_WDEL_LCLK_ID:
			CreateThread(0, 1024 * 1024, winaltdel_lclick, 0, 0, 0);
			break;
		case HK_WPGUP_RDBLCLK_ID:
			CreateThread(0, 1024 * 1024, winaltpgup_rdblclick, 0, 0, 0);
			break;
		case HK_WPGUP_RCLK_ID:
			CreateThread(0, 1024 * 1024, winaltpgdn_rclick, 0, 0, 0);
			break;



		//case HK_WL_DRAGL_ID:
		//	CreateThread(0, 1024 * 1024, winleftdragthrd, 0, 0, 0);
		//	break;
		//case HK_WR_DRAGR_ID:
		//	CreateThread(0, 1024 * 1024, winrightdragthrd, 0, 0, 0);
		//	break;
		case HK_WU_DRAGU_ID:
			CreateThread(0, 1024 * 1024, winupdragthrd, 0, 0, 0);
			break;
		case HK_WD_DRAGD_ID:
			CreateThread(0, 1024 * 1024, windowndragthrd, 0, 0, 0);
			break;




		case HK_CAUP_ID:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			//Sleep(10);
			mouse_event(MOUSEEVENTF_MOVE, 0, -GStep, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//CreateThread(0, 1024 * 1024, ctrlaltupthrd, 0, 0, 0);
			break;
		case HK_CADN_ID: {
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			//Sleep(10);
			mouse_event(MOUSEEVENTF_MOVE, 0, GStep, 0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//CreateThread(0, 1024 * 1024, ctrlaltdownthrd, 0, 0, 0);
		}break;
		case HK_CALT_ID:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			//Sleep(10);
			mouse_event(MOUSEEVENTF_MOVE, -GStep,0,  0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//CreateThread(0, 1024 * 1024, ctrlaltleftthrd, 0, 0, 0);
			break;
		case HK_CART_ID:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			//Sleep(10);
			mouse_event(MOUSEEVENTF_MOVE, GStep,0,  0, 0);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//CreateThread(0, 1024 * 1024, ctrlaltrightthrd, 0, 0, 0);
			break;
		case HK_ALT_1:
			GStep = 2;
			break;
		case HK_ALT_2:
			GStep = GStepBase;
			break;
		case HK_ALT_3:
			GStep = 2* GStepBase;
			break;
		case HK_ALT_4:
			GStep = 4 * GStepBase;
			break;
		case HK_SALU_ID:
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case HK_SAMU_ID:
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		case HK_SARU_ID:
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case HK_SALD_ID:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			break;
		case HK_SAMD_ID:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		case HK_SARD_ID:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			break;
		case HK_ALTSHIFT_PLUS_SU:
		case HK_ALTBACK_SCROLLUP: {
			POINT pt;
			::GetCursorPos(&pt);
			auto whin = new SheellInfo;
			whin->hWnd = hWnd;
			whin->x = pt.x;
			whin->y = pt.y - GStep;
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, GStep, 0);
			delete  whin;
			//CreateThread(0, 1024 * 1024, altscrollupthrd, whin, 0, 0);
		}break;
		case HK_ALTSHIFT_BRACKET_SD:
		case HK_ALTOR_SCROLLDN: {
			POINT pt;
			::GetCursorPos(&pt);
			auto whin = new SheellInfo;
			whin->hWnd = hWnd;
			whin->x = pt.x ;
			whin->y = pt.y + GStep;
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, -GStep, 0);// -WHEEL_DELTA, 0);
			delete  whin;
			//CreateThread(0, 1024 * 1024, altscrolldownthrd, whin, 0, 0);
		}break;
		
		case HK_CTRLSHIFT_BACK_FASTSU: {
			POINT pt;
			::GetCursorPos(&pt);
			auto whin = new SheellInfo;
			whin->hWnd = hWnd;
			whin->x = pt.x;
			whin->y = pt.y - GStep;
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, GStep, 0);
			Sleep(30);
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, GStep, 0);
			Sleep(30); 
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, GStep, 0);
			Sleep(30); 
			delete  whin;
			//CreateThread(0, 1024 * 1024, altscrollupthrd, whin, 0, 0);
		}break;
		case HK_CTRLSHIFT_OR_FASTSD: {
			POINT pt;
			::GetCursorPos(&pt);
			auto whin = new SheellInfo;
			whin->hWnd = hWnd;
			whin->x = pt.x;
			whin->y = pt.y + GStep;
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, -GStep, 0);// -WHEEL_DELTA, 0);
			Sleep(30); 
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, -GStep, 0);// -WHEEL_DELTA, 0);
			Sleep(30); 
			mouse_event(MOUSEEVENTF_WHEEL, whin->x, whin->y, -GStep, 0);// -WHEEL_DELTA, 0);
			Sleep(30); 
			delete  whin;
			//CreateThread(0, 1024 * 1024, altscrolldownthrd, whin, 0, 0);
		}break;


		
		case HK_CAHM_WHEELUP: {
			POINT pt;
			::GetCursorPos(&pt);
			auto whin = new SheellInfo;
			whin->hWnd = hWnd;
			whin->x = pt.x;
			whin->y = pt.y - GStep;
			CreateThread(0, 1024 * 1024, altscrollupthrd, whin, 0, 0);
		}break;
		case HK_CAED_WHEELDOWN:{
			POINT pt;
			::GetCursorPos(&pt);
			auto whin = new SheellInfo;
			whin->hWnd = hWnd;
			whin->x = pt.x;
			whin->y = pt.y + GStep;
			CreateThread(0, 1024 * 1024, altscrolldownthrd, whin, 0, 0);
		}break;
		case HK_SAQ_ID:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case HK_SAGITHUB_ID:
			if (checkgithubthrdh == 0) {
				Shell_NotifyIcon(NIM_DELETE, &nid);
				checkgithubthrdh = CreateThread(0, 1024 * 1024, checkgithubthrd, hWnd, 0, 0);
			}
			else {
				TerminateThread(checkgithubthrdh, 0);
				Shell_NotifyIcon(NIM_DELETE, &nid);
				nid.cbSize = sizeof(nid);
				nid.hWnd = hWnd;
				nid.uID = 86450;
				nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
				nid.uCallbackMessage = WM_USER;
				nid.hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CANDEFKEYBOARD));
				lstrcpy(nid.szTip, L"CanDefKeyboard");
				Shell_NotifyIcon(NIM_ADD, &nid);

			}
		case HK_ALTDOT_TOLEFTSLASH: {
				SYSTEMTIME st;
				GetLocalTime(&st);
				wchar_t buffer[80] = { 0 };
				GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, L"yyyyMMddT", buffer, 40);
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, L"HHmmss", buffer+wcslen(buffer), 40);
				dpw(buffer);
				typethrd(buffer);
				//SetClipboardText(buffer, wcslen(buffer));
				//CreateThread(0, 1024 * 1024, pastethrd, hWnd, 0, 0);
			}break;
		case HK_ALTRIGHTSLASH_TORIGHTSLASH: {
			while (::OpenClipboard(NULL) == FALSE) {
				Sleep(1000);
			}
			auto hGlobal = GetClipboardData(CF_UNICODETEXT);
			dp("%d", hGlobal);
			if (hGlobal != 0) // is equal "NULL" condition
			{
				auto pGlobal = (wchar_t*)GlobalLock(hGlobal);
				if (pGlobal == 0) {
					CloseClipboard();
				}
				HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, (wcslen(pGlobal)*4 + 1) * sizeof(wchar_t));
				if (hMem == 0) {
				}
				wchar_t *bufferbuffer = (wchar_t *)::GlobalLock(hMem);
				if (bufferbuffer == 0) {
				}
				for (int i = 0; i < wcslen(pGlobal); i++) {
					if (pGlobal[i] == '\\') {
						pGlobal[i] = '/';
					}
				}
				wcscpy_s(bufferbuffer, (wcslen(pGlobal) + 1) * sizeof(wchar_t), pGlobal);
				dpw(bufferbuffer);
				::GlobalUnlock(hMem);
				if (GlobalUnlock(hGlobal) == FALSE) {
				}
				if (::SetClipboardData(CF_UNICODETEXT, hMem) == FALSE) {
				}
			}
			if (CloseClipboard() == FALSE) {
			}
			}break;
		case HK_ALTCOMMA_TOMINGWPATH:{
			while (::OpenClipboard(NULL) == FALSE) {
				Sleep(1000);
			}
			auto hGlobal = GetClipboardData(CF_UNICODETEXT);
			dp("%d", hGlobal);
			if (hGlobal != 0) // is equal "NULL" condition
			{
				auto pGlobal = (wchar_t*)GlobalLock(hGlobal);
				if (pGlobal == 0) {
					CloseClipboard();
				}
				HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, (wcslen(pGlobal)*4 + 1) * sizeof(wchar_t));
				if (hMem == 0) {
				}
				wchar_t *bufferbuffer = (wchar_t *)::GlobalLock(hMem);
				if (bufferbuffer == 0) {
				}
				auto bleftsla = false;
				auto brightsla = false;
				for (int i = 0; i < wcslen(pGlobal); i++) {
					for (; i - 1 >= 0 && ((pGlobal[i - 1] == '\\' || pGlobal[i - 1] == '/') && (pGlobal[i] == '\\' || pGlobal[i] == '/'));) {
						for (int i1 = i; i1 < wcslen(pGlobal); i1++) {
							pGlobal[i1] = pGlobal[i1 + 1];
						}
					} 
					if (pGlobal[i] == '\\' && bleftsla ==false) {
						pGlobal[i] = '/';
						brightsla = true;
					}else if (pGlobal[i] == '/' && brightsla==false) {
						pGlobal[i] = '\\';
						bleftsla = true;
					}
				}
				//if (bleftsla) {
				//	if (pGlobal[0] == '/' && (pGlobal[1] >= 'A' && pGlobal[1] <= 'Z' || pGlobal[1] >= 'a' && pGlobal[1] <= 'z') && pGlobal[2] == '/') {
				//		for (int i = 0; i < wcslen(pGlobal); i++) {
				//			if (pGlobal[i] == '/') {
				//				pGlobal[i] = '\\';
				//			}
				//		}
				//		pGlobal[0] = pGlobal[1];
				//		pGlobal[2] = ':';
				//	}
				//}
				//else {
				//	if ((pGlobal[0] >= 'A' && pGlobal[0] <= 'Z' || pGlobal[0] >= 'a' && pGlobal[0] <= 'z') && pGlobal[1] == ':') {
				//		pGlobal[1] = pGlobal[0];
				//		pGlobal[0] = '/';
				//	}
				//}
				wcscpy_s(bufferbuffer, (wcslen(pGlobal) + 1) * sizeof(wchar_t), pGlobal);
				dpw(bufferbuffer);
				::GlobalUnlock(hMem);
				if (GlobalUnlock(hGlobal) == FALSE) {
				}
				if (::SetClipboardData(CF_UNICODETEXT, hMem) == FALSE) {
				}
			}
			if (CloseClipboard() == FALSE) {
			}
			}break;
		case HK_ALTQUOT: {
			while (::OpenClipboard(NULL) == FALSE) {
				Sleep(1000);
			}
			auto hGlobal = GetClipboardData(CF_UNICODETEXT);
			dp("水水水水水水水 %d\n", hGlobal);
			if (hGlobal != 0) // is equal "NULL" condition
			{
				auto pGlobal = (wchar_t*)GlobalLock(hGlobal);
				if (pGlobal == 0) {
					CloseClipboard();
				}
				HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, (wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t));
				if (hMem == 0) {
				}
				wchar_t *bufferbuffer = (wchar_t *)::GlobalLock(hMem);
				if (bufferbuffer == 0) {
				}
				auto bleftsla = false;
				auto brightsla = false;
				dp("testtag %d\n", hGlobal);
				if(!(pGlobal[0] == '"' && ((wchar_t*)pGlobal)[wcslen((wchar_t*)pGlobal) - 1]=='"')){
					dp("testtag %d\n", wcslen((wchar_t*)pGlobal));
					auto hMem1 = (wchar_t*)malloc((wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t)*4);
					auto n1=wstr_replace(  (wchar_t*)hMem1, pGlobal, L"\\", L"\\\\");
					//((wchar_t*)hMem1)[n1] = 0;
					dp("testtag %d\n", wcslen((wchar_t*)hMem1));
					auto hMem2 = (wchar_t*)malloc((wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t) * 4);
					auto n2=wstr_replace((wchar_t*)hMem2, (wchar_t*)hMem1, L"\r", L"\\r");
					//((wchar_t*)hMem2)[n2] = 0;
					dp("testtag %d\n", wcslen((wchar_t*)hMem2));
					auto hMem3 = (wchar_t*)malloc((wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t) * 4);
					auto n3=wstr_replace((wchar_t*)hMem3, (wchar_t*)hMem2, L"\n", L"\\n");
					//((wchar_t*)hMem3)[n3] = 0;
					dp("testtag %d\n", wcslen((wchar_t*)hMem3));
					auto hMem4 = (wchar_t*)malloc((wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t) * 4);
					auto n4=wstr_replace((wchar_t*)hMem4, (wchar_t*)hMem3, L"\"", L"\\\"");
					//((wchar_t*)hMem4)[n4] = 0;
					dp("testtag %d\n", wcslen((wchar_t*)hMem4));
					bufferbuffer[0] = '"';
					wcscpy(bufferbuffer+1, (wchar_t*)hMem4);
					auto kk = wcslen(bufferbuffer);
					bufferbuffer[kk] = '"';
					bufferbuffer[kk+1] = 0;
					free(hMem1);
					free(hMem2);
					free(hMem3);
					free(hMem4);
				}
				else {
					wprintf(L"666");
					wprintf((wchar_t*)pGlobal);
					auto hMem1 = (wchar_t*)malloc((wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t) * 4);
					((wchar_t*)pGlobal)[wcslen((wchar_t*)pGlobal) - 1] = 0;
					auto n1 = wstr_replace((wchar_t*)hMem1, pGlobal+1, L"\\\"", L"\"");
					//((wchar_t*)hMem1)[n1] = 0;
					auto hMem2 = (wchar_t*)malloc((wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t) * 4);
					auto n2 = wstr_replace((wchar_t*)hMem2, (wchar_t*)hMem1, L"\\r", L"\r");
					//((wchar_t*)hMem2)[n2] = 0;
					auto hMem3 = (wchar_t*)malloc((wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t) * 4);
					auto n3 = wstr_replace((wchar_t*)hMem3, (wchar_t*)hMem2, L"\\n", L"\n");
					//((wchar_t*)hMem3)[n3] = 0;
					auto hMem4 = (wchar_t*)malloc((wcslen(pGlobal) * 4 + 1) * sizeof(wchar_t) * 4);
					auto n4 = wstr_replace((wchar_t*)hMem4, (wchar_t*)hMem3, L"\\\\", L"\\");
					//((wchar_t*)hMem4)[n4] = 0;
					wcscpy(bufferbuffer, (wchar_t*)hMem4);
					free(hMem1);
					free(hMem2);
					free(hMem3);
					free(hMem4);
				}
				dpw(bufferbuffer);
				::GlobalUnlock(hMem);
				if (GlobalUnlock(hGlobal) == FALSE) {
				}
				if (::SetClipboardData(CF_UNICODETEXT, hMem) == FALSE) {
				}
			}
			if (CloseClipboard() == FALSE) {
			}
		}break;
		case HK_ALT0:
			if(Alt0!=0)
				CreateThread(0, 1024 * 1024, cppastethrd, Alt0, 0, 0);
			break;
		//case HK_ALT1:
		//	if (Alt1 != 0)
		//	cppastethrd(Alt1);
		//case HK_ALT2:
		//	if (Alt2 != 0)
		//	cppastethrd(Alt2);
		//case HK_ALT3:
		//	if (Alt3 != 0)
		//	cppastethrd(Alt3);
		//case HK_ALT4:
		//	if (Alt4 != 0)
		//	cppastethrd(Alt4);
		case HK_ALT5:
			if (Alt5 != 0)
				CreateThread(0, 1024 * 1024, cppastethrd, Alt5, 0, 0);
			break;
		case HK_ALT6:
			if (Alt6 != 0)
				CreateThread(0, 1024 * 1024, cppastethrd, Alt6, 0, 0);
			break;
		case HK_ALT7:
			if (Alt7 != 0)
				CreateThread(0, 1024 * 1024, cppastethrd, Alt7, 0, 0);
			break;
		case HK_ALT8:
			if (Alt8 != 0)
				CreateThread(0, 1024 * 1024, cppastethrd, Alt8, 0, 0);
			break;
		case HK_ALT9:
			if (Alt9 != 0)
				CreateThread(0, 1024 * 1024, cppastethrd, Alt9, 0, 0);
			break;
		case HK_WINC_SHOWHIDEKB_ID:
			if (IsWindowVisible(hWnd) == true) {
				ShowWindow(hWnd, SW_HIDE);
			}
			else {
				ShowWindow(hWnd, SW_SHOW);
			}
			break;

		}
		break;
    case WM_DESTROY:
		UnregisterHotKey(hWnd, KBHOTKEYID);
		Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:

		SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), L"Help\r\nRight Key:F1\r\nCtrl+S  F2\r\nCtrl+C F3\r\nCtrl+V F4\r\nclipboard1(CTRL+RF4 set\r\nRF4 Paste)\r\nF5 clipboard2\r\nF6 clipboard3 Del\r\nCtrl+X Home\r\nCtrl+C  PgUp\r\nCtrl+V  Left_Array\r\nCtrl+Z  Right_Array\r\nctrl+Y Caps Num_Lock\r\nshift+back size change\r\nctrl+back hide\r\nalt+back 自动关闭(auto close)Tabtip.exe\r\nwin+back 提升开关(tip switch)\r\n左角模拟上滚(auto scrollup)lalt+aup\r\n左角模拟下滚(auto scrolldown)lalt+adown\r\n右角模拟上滚(scroll up)ralt+aup\r\n右角模拟下滚(scroll down)ralt+adown\r\nlwin+back提示开关\r\n虚拟鼠标功能(virtual mouse):\r\nAlt+方向键 移动鼠标(move mouse);\r\nCtrl+Alt+方向键 鼠标拖动(mouse drag);\r\nAlt+Del 左键单击(mouse left click),\r\nEnd 中键单击,\r\nPageDown 右键键单击(mouse middle click);\r\nAlt+Ins 左键双击(mouse left double click),\r\nHome 中键双击(mouse middle double click),\r\nPageUp 右键双击(mouse right double click);\r\nShift+Alt+Del 左键按下(left press),\r\nEnd 中键按下(middle press),\r\nPageDown 右键按下(right press);\r\nShift+Alt+Ins 左键松开(left release),\r\nHome 中键松开(middle release),\r\nPageUp 右键松开(right release);\r\nAlt+Z 一格大小10点(1 grid 10 pixel),\r\nX 一格大小20点(1 grid 20 pixel),\r\nC 一格大小40点(1 grid 40 pixel);\r\nAlt+Back滚轮上滚(Scroll up),\r\nSlash滚轮下滚(scroll down),\r\nCtrl+Alt+Home滚轮上滚(Scroll up),\r\nEnd滚轮下滚(scroll down);\r\n隐藏(hide)Shift+Alt+H;退出(quit)Shift+Alt+Q;win+ins延迟左键双击(left delay dbl click),\r\ndel延迟左键单击(left delay click),\r\npageup延迟右键双击(right delay dbl click),\r\npagedown延迟右键单击(right delay click);\r\nwin+`开启关闭虚拟键盘(show hide virtual keyboard);\r\nalt+shift+方向键2倍数(2 times speed);\r\nalt+'<','>'慢速左右移动(low speed move)\r\n;':','/'慢速上下移动(low speed move);\r\n alt+'+',']','\',BACK, 左上,左下,右下,右上2倍数移动;(bevel move)\r\nALT+句号 替换剪切板文本/到\\, / 替换\\为/\nALT0-9= conf文件自定义粘贴{cd}替换为当前剪切板内容\nalt+'\"' 替换剪切板内容为编程字符串;");
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
