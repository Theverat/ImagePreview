#ifndef TRASHEDFILE_H
#define TRASHEDFILE_H

#include <QUrl>

class TrashedFile
{
public:
    TrashedFile(QUrl originalUrl, QUrl url);
    QUrl getOriginalUrl() const;
    QUrl getUrl() const;

private:
    QUrl originalUrl;
    QUrl url;
};

#endif // TRASHEDFILE_H
