#include "trashhandler.h"

#include <iostream>

TrashHandler::TrashHandler()
{
}

bool TrashHandler::moveToTrash(QUrl url) {
    if(!trashDir.isValid()) {
        return false;
    }

    QUrl trashUrl = QUrl::fromLocalFile(trashDir.path() + "/" + url.fileName());
    TrashedFile trashedFile(url, trashUrl);
    trash.append(trashedFile);

    QFile file(url.toLocalFile());
    if(file.exists()) {
        file.rename(trashUrl.toLocalFile());
    }

    return true;
}

QStringList TrashHandler::getFileNames() {
    QStringList fileNames;

    for(int i = 0; i < trash.size(); i++) {
        fileNames.append(trash.at(i).getUrl().fileName());
    }

    return fileNames;
}

bool TrashHandler::restore(int index) {
    if(index < 0 || index >= trash.size())
        return false;

    TrashedFile trashedFile = trash.at(index);

    QFile file(trashedFile.getUrl().toLocalFile());
    if(file.exists()) {
        file.rename(trashedFile.getOriginalUrl().toLocalFile());
    }

    trash.removeAt(index);
    return true;
}

bool TrashHandler::isEmpty() {
    return trash.size() == 0;
}

QUrl TrashHandler::getTrashUrl() {
    return QUrl::fromLocalFile(trashDir.path());
}
