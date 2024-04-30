#pragma once

#include <iostream>

#include "Application.h"

#ifdef WL_PLATFORM_WINDOWS

//extern LightBox::Application* LightBox::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

namespace LightBox {

	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning)
		{
			LightBox::Application* app = LightBox::CreateApplication(argc, argv);

			try {
				app->Run();
				delete app;
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << std::endl;
				return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;

			g_ApplicationRunning = false;
		}
		return 0;
	}
}

#ifdef WL_DIST

#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return LightBox::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return LightBox::Main(argc, argv);
}

#endif // WL_DIST

#endif // WL_PLATFORM_WINDOWS
