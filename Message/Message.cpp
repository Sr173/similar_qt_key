// Message.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "windows.h"
#include "vector"

#define SELECT_INDEX 0x1000

const char shared_memory_name[] = "file_name_io";
const auto memory_size = 1000 * MAX_PATH + 4;

void set_file_name_list(const std::vector<std::string>& file_name)
{
	static void* shared_memory_ptr = nullptr;
	static char* data_start = nullptr;
	static int* data_size_ptr = nullptr;
	
	if (shared_memory_ptr == nullptr)
	{
		const auto memory_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, memory_size, shared_memory_name);
		shared_memory_ptr = MapViewOfFile(memory_handle, FILE_MAP_ALL_ACCESS, 0, 0, memory_size);
		if (shared_memory_ptr == nullptr)
		{
			std::cout << "打开共享内存失败" << std::endl;
			return;
		}
		data_start = static_cast<char*>(shared_memory_ptr) + 4;
		data_size_ptr = static_cast<int*>(shared_memory_ptr);
	}

	if (file_name.size() > 1000)
	{
		std::cout << "size too large" << std::endl;
		return;
	}
	
	data_size_ptr[0] = file_name.size();

	int count = 0;
	for (const auto& i : file_name)
	{
		if (i.length() >= MAX_PATH)
		{
			std::cout << "path too long" << std::endl;
			data_size_ptr[0]--;
			continue;
		}
		memcpy(data_start + (count++) * MAX_PATH, i.data(), i.length() + 1);
	}
	
}

int main()
{
	std::vector<std::string> file_path;
	file_path.push_back("C:\\Users\\admin\\Desktop\\47853117_p0.jpg");
	file_path.push_back("C:\\Users\\admin\\Desktop\\test.png");
	file_path.push_back("C:\\Users\\admin\\Desktop\\Z`T5@HZM32FBK7J6QUC]6M0.png");
	set_file_name_list(file_path);
	
	auto hwnd = FindWindow(L"Qt5QWindowOwnDCIcon", L"Topaz Gigapixel AI v4.0.2");
	SendMessageA(hwnd, WM_USER + 1, 10, 10);
	Sleep(500);
	SendMessageA(hwnd, SELECT_INDEX, 0, 0);
}
