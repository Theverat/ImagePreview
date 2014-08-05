#include "cursormanager.h"
#include <QApplication>

bool CursorManager::isVisible = true;
bool CursorManager::wasVisible = true;

CursorManager::CursorManager() {
    isVisible = true;
}

void CursorManager::hideCursor() {
    QApplication::setOverrideCursor(Qt::BlankCursor);
    wasVisible = isVisible;
    isVisible = false;
}

void CursorManager::showCursor() {
    QApplication::restoreOverrideCursor();
    wasVisible = isVisible;
    isVisible = true;
}

bool CursorManager::toggleCursorVisibility() {
    if(isCursorVisible()) {
        hideCursor();
    }
    else {
        showCursor();
    }

    return isCursorVisible();
}

bool CursorManager::isCursorVisible() {
    return isVisible;
}

void CursorManager::restoreCursorVisibility() {
    if(wasVisible) {
        showCursor();
    }
    else {
        hideCursor();
    }
}
