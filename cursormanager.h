#ifndef CURSORMANAGER_H
#define CURSORMANAGER_H

class CursorManager
{
public:
    CursorManager();
    static void hideCursor();
    static void showCursor();
    static bool toggleCursorVisibility();
    static bool isCursorVisible();
    static void restoreCursorVisibility();

private:
    static bool isVisible;
    static bool wasVisible;
};

#endif // CURSORMANAGER_H
