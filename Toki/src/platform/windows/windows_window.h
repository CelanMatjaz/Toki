#pragma once

#include "core/window.h"

namespace Toki {

    class TokiWindowsWindow : public Window {
    public:
        TokiWindowsWindow(const WindowConfig& windowConfig, void* engine);
        virtual ~TokiWindowsWindow();

        virtual void pollEvents() override;
        virtual bool shouldClose() override { return shouldWindowClose; };
        virtual void* getHandle() override { return handle; };
        virtual void showWindow() override;

        HWND handle;

    private:
        static inline void* engine = nullptr;
        static inline const wchar_t* WINDOW_CLASS_NAME = L"Toki window";
        static inline bool isClassRegistered = false;
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        bool shouldWindowClose = false;
        DWORD windowStyle;
    };


}