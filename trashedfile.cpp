#include "trashedfile.h"

TrashedFile::TrashedFile(QUrl originalUrl, QUrl url)
{
    this->originalUrl = originalUrl;
    this->url = url;
}

QUrl TrashedFile::getOriginalUrl() const {
    return originalUrl;
}

QUrl TrashedFile::getUrl() const {
    return url;
}
