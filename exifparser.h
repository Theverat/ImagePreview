#ifndef EXIFPARSER_H
#define EXIFPARSER_H

#include <QUrl>
#include <QByteArray>
#include <vector>

// http://www.waimea.de/downloads/exif/EXIF-Datenformat.pdf

class ExifParser
{
public:
    ExifParser(QUrl imageUrl);
    bool isValidExifData();
    unsigned short getOrientation();

    //intel = little endian, motorola = big endian
    enum FormatType {
        INTEL,
        MOTOROLA
    };

private:
    QUrl imageUrl;
    FormatType format;
    bool isValid;
    unsigned short exifLength;
    //positions of relevant data
    int exifStartPos;
    unsigned short exifLengthPos;
    unsigned short exifCodePos;
    unsigned short tiffHeaderPos;
    unsigned short tagAmountPos;
    std::vector<unsigned short> tagPositions;
    //comparison codes
    QByteArray markerJpegStart;
    QByteArray markerExifStart;
    QByteArray exifCode;
    QByteArray intelFormatCode;
    QByteArray motorolaFormatCode;
    //data from tags
    unsigned short orientation;

    //private methods
    bool compareBytes(QByteArray &source, QByteArray &comparison, int startIndex);
    QByteArray decodeFormat(QByteArray &bytes);
    QString readTag(unsigned short tagPos, QByteArray &buffer);
    unsigned short readUnsignedShort(QByteArray bytes);
    QString readQString(QByteArray bytes);
    unsigned long readUnsignedLong(QByteArray bytes);
};

#endif // EXIFPARSER_H
