#include "exifparser.h"

#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <iostream>

ExifParser::ExifParser(QUrl imageUrl) {
    this->imageUrl = imageUrl;
    format = MOTOROLA; //standard is big endian
    orientation = 1; //default: top left

    markerJpegStart = QByteArray::fromHex("FFD8");
    markerExifStart = QByteArray::fromHex("FFE1");
    exifCode = QByteArray::fromHex("457869660000");
    intelFormatCode = QByteArray::fromHex("4949");
    motorolaFormatCode = QByteArray::fromHex("4D4D");

    //for now, only jpeg/jpg images are supported
    QFileInfo fileInfo(imageUrl.toLocalFile());
    if(fileInfo.suffix().toLower() != "jpg" && fileInfo.suffix().toLower() != "jpeg") {
        // // std::cout << "suffix not \"jpg\" or \"jpeg\". No EXIF data available." << std::endl;
        isValid = false;
        return;
    }

    QFile file(imageUrl.toLocalFile());

    if(!file.open(QIODevice::ReadOnly)) {
        std::cerr << "could not open file" << std::endl;
        isValid = false;
        return;
    }

    QDataStream input(&file);
    QByteArray buffer;
    int length = 65536; //theoretical max size of jpeg header (bytes)

    char temp[length];
    input.readRawData(temp, length);
    buffer.append(temp, length);

    //test if jpeg image is intact (2 bytes)
    if(compareBytes(buffer, markerJpegStart, 0)) {
        // std::cout << "intact jpeg image" << std::endl;
    }
    else {
        // std::cout << "not a jpeg image or corrupt image!" << std::endl;
        isValid = false;
        return;
    }

    //find exif begin marker FF-E1 (2 bytes)
    exifStartPos = buffer.indexOf(markerExifStart);
    // std::cout << "EXIF marker found at: " << exifStartPos << std::endl;

    //get length of exif data (2 bytes)
    exifLengthPos = exifStartPos + 2;
    exifLength = readUnsignedShort(buffer.mid(exifLengthPos, 2));
    // std::cout << "EXIF data is " << exifLength << " bytes long" << std::endl;

    //check if it contains the code "Exif" (6 bytes)
    exifCodePos = exifLengthPos + 2;

    if(compareBytes(buffer, exifCode, exifCodePos)) {
        // std::cout << "contains EXIF data" << std::endl;
    }
    else {
        // std::cout << "does not contain EXIF data" << std::endl;
        isValid = false;
        return;
    }

    //now we are in the TIFF header (8 bytes)
    tiffHeaderPos = exifCodePos + 6;

    //check if intel or motorola format is used (2 bytes in TIFF header)
    if(compareBytes(buffer, intelFormatCode, tiffHeaderPos)) {
        // std::cout << "intel format" << std::endl;
        format = INTEL;
    }
    else if(compareBytes(buffer, motorolaFormatCode, tiffHeaderPos)) {
        // std::cout << "motorola format" << std::endl;
        format = MOTOROLA;
    }

    //last 6 bytes of TIFF header contain always the same data, skip them

    //now we are in the first image file directory (IDF)
    //get amount of EXIF tags (2 bytes)
    tagAmountPos = tiffHeaderPos + 8;
    unsigned short tagAmount = readUnsignedShort(buffer.mid(tagAmountPos, 2));
    // std::cout << "tag amount: " << tagAmount << std::endl;

    //tags are always 12 bytes long, get their positions
    tagPositions = std::vector<unsigned short>(tagAmount);
    for(int i = 0; i < tagAmount; i++) {
        tagPositions.at(i) = tagAmountPos + 2 + (i * 12);
    }

    //read tags
    for(int i = 0; i < tagAmount; i++) {
        // std::cout << "    tag " << i + 1 << ": " << readTag(tagPositions.at(i), buffer).toStdString() << std::endl;
    }

    isValid = true;
}

bool ExifParser::isValidExifData() {
    return isValid;
}

unsigned short ExifParser::getOrientation() {
    return orientation;
}

bool ExifParser::compareBytes(QByteArray &source, QByteArray &comparison, int startIndex) {
    for(int i = 0; i < comparison.size(); i++) {
        if(source.at(startIndex + i) != comparison.at(i))
            return false;
    }

    return true;
}

QByteArray ExifParser::decodeFormat(QByteArray &bytes) {
    if(format == INTEL) {
        //intel format, switch high and low bytes
        QByteArray result;

        if(bytes.size() % 2 != 0 && bytes.size() != 1) {
            std::cerr << "decodeFormat: Error" << std::endl;
            return result;
        }

        if(bytes.size() < 2)
            return bytes;

        result.resize(bytes.size());

        int j = bytes.size() - 1;
        for(int i = 0; i < bytes.size(); i++) {
            result[i] = bytes.at(j);
            j--;
        }

        return result;
    }
    else {
        //motorola format, bytes are in correct order
        return bytes;
    }
}

QString ExifParser::readTag(unsigned short tagPos, QByteArray &buffer) {
    QString result;
    QByteArray tag = buffer.mid(tagPos, 12);

    //one tag is 12 bytes long
    //2 bytes: type of tag
    unsigned short vendorType = 0x10F;
    unsigned short cameraType = 0x110;
    unsigned short orientationType = 0x112;
    //... other types

    unsigned short tagType = readUnsignedShort(tag.mid(0, 2));
    if(tagType == vendorType) {
        result.append("Vendor: ");
    }
    else if(tagType == orientationType) {
        result.append("Orientation: ");
    }

    //4 bytes: length of data (as 32 bit number)
    unsigned long dataLength = readUnsignedLong(tag.mid(4, 4));

    //4 bytes: data in the tag or offset to the data if more than 4 bytes
    if(tagType == orientationType) {
        orientation = readUnsignedShort(tag.mid(8, 2));
        result.append(QString::number(orientation));
    }
    else if(tagType == vendorType) {
        if(dataLength > 4) {
            //read offset (from tiff header begin)
            unsigned long offset = readUnsignedLong(tag.mid(8, 4));
            result.append("Offset: " + QString::number(offset));

            //use offset to read data
            QString vendorString = readQString(buffer.mid(12 + offset, dataLength));
            result.append(" VendorString: " + vendorString);
        }
    }

    return result;
}

unsigned short ExifParser::readUnsignedShort(QByteArray bytes) {
    if(bytes.size() > 2) {
        std::cerr << "readUnsignedShort: Error: argument contains more than 2 bytes!" << std::endl;
        return 0;
    }

    //convert from intel format if neccessary
    QByteArray decoded = decodeFormat(bytes);
    //convert high byte and move 8 bits to the left, then add low byte
    unsigned short result = ((unsigned short)decoded.at(0) << 8)
            | decoded.at(1);

    return result;
}

QString ExifParser::readQString(QByteArray bytes) {
    QString result;

    char *data = bytes.data();
    while (*data) {
        result.append(*data);
        ++data;
    }

    return result;
}

unsigned long ExifParser::readUnsignedLong(QByteArray bytes) {
    if(bytes.size() > 4) {
        std::cerr << "readUnsignedLong: Error: argument contains more than 4 bytes!" << std::endl;
        return 0;
    }

    //convert from intel format if neccessary
    QByteArray decoded = decodeFormat(bytes);

    unsigned long result = ((unsigned long)decoded.at(0) << 24)
            | ((unsigned long)decoded.at(1) << 16)
            | ((unsigned long)decoded.at(2) << 8)
            | (unsigned long)decoded.at(3);

    return result;
}

