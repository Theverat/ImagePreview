#ifndef TRASHHANDLER_H
#define TRASHHANDLER_H

#include <QList>
#include <QUrl>
#include <QTemporaryDir>
#include "trashedfile.h"

class TrashHandler
{
public:
    TrashHandler();
    bool moveToTrash(QUrl url);
    QStringList getFileNames();
    bool restore(int index);
    bool isEmpty();
    QUrl getTrashUrl();

private:
    QList<TrashedFile> trash;
    QTemporaryDir trashDir;
};

#endif // TRASHHANDLER_H
