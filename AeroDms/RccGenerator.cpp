// Copyright (C) 2018 The Qt Company Ltd.
// Copyright (C) 2018 Intel Corporation.
// Copyright (C) 2024 Christoph Cullmann <christoph@cullmann.io>
// Copyright (C) 2025 Clément Vermot-Desroches
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "RccGenerator.h"

#include <QByteArray>
#include <qcryptographichash.h>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <qlocale.h>
#include <QStack>
#include <QMessageBox>
#include <QApplication>
#include <QObject>

// Note: A copy of this file is used in Qt Widgets Designer (qttools/src/designer/src/lib/shared/rcc.cpp)

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

void RCCResourceLibrary::write(const char* str, int len)
{
    int n = m_out.size();
    m_out.resize(n + len);
    memcpy(m_out.data() + n, str, len);
}

void RCCResourceLibrary::writeByteArray(const QByteArray& other)
{
    m_out.append(other);
}

///////////////////////////////////////////////////////////
//
// RCCFileInfo
//
///////////////////////////////////////////////////////////

class RCCFileInfo
{
public:
    enum Flags
    {
        // must match qresource.cpp
        NoFlags = 0x00,
        Directory = 0x02
    };


    RCCFileInfo() = default;
    RCCFileInfo(const QString& name, const QFileInfo& fileInfo, uint flags);

    ~RCCFileInfo();
    RCCFileInfo(const RCCFileInfo&) = delete;
    RCCFileInfo& operator=(const RCCFileInfo&) = delete;
    RCCFileInfo(RCCFileInfo&&) = default;
    RCCFileInfo& operator=(RCCFileInfo&& other) = delete;

    struct DeduplicationKey {
        QByteArray hash;

        bool operator==(const DeduplicationKey& other) const
        {
            return hash == other.hash;
        }
    };

    typedef QMultiHash<DeduplicationKey, RCCFileInfo*> DeduplicationMultiHash;

public:
    qint64 writeDataBlob(RCCResourceLibrary& lib,
        qint64 offset,
        DeduplicationMultiHash& dedupByContent,
        QString* errorMessage);
    qint64 writeDataName(RCCResourceLibrary&, qint64 offset);
    void writeDataInfo(RCCResourceLibrary& lib);

    int m_flags = NoFlags;
    QLocale::Language m_language = QLocale::AnyLanguage;
    QLocale::Territory m_territory = QLocale::AnyTerritory;
    QString m_name;
    QFileInfo m_fileInfo;
    RCCFileInfo* m_parent = nullptr;
    QMultiHash<QString, RCCFileInfo*> m_children;

    qint64 m_nameOffset = 0;
    qint64 m_dataOffset = 0;
    qint64 m_childOffset = 0;
};

static size_t qHash(const RCCFileInfo::DeduplicationKey& key, size_t seed) noexcept
{
    return qHashMulti(seed, key.hash);
}

RCCFileInfo::RCCFileInfo(const QString& name, const QFileInfo& fileInfo, uint flags)
    : m_flags(flags),
    m_name(name),
    m_fileInfo(fileInfo)
{
}

RCCFileInfo::~RCCFileInfo()
{
    qDeleteAll(m_children);
}

void RCCFileInfo::writeDataInfo(RCCResourceLibrary& lib)
{
    //pointer data
    if (m_flags & RCCFileInfo::Directory) {
        // name offset
        lib.writeNumber4(m_nameOffset);

        // flags
        lib.writeNumber2(m_flags);

        // child count
        lib.writeNumber4(m_children.size());

        // first child offset
        lib.writeNumber4(m_childOffset);
    }
    else {
        // name offset
        lib.writeNumber4(m_nameOffset);

        // flags
        lib.writeNumber2(m_flags);

        // locale
        lib.writeNumber2(QLocale::AnyTerritory);
        lib.writeNumber2(QLocale::AnyLanguage);

        //data offset
        lib.writeNumber4(m_dataOffset);
    }

	//On ne gère que les version de format supérieur ou égale à 2
    // last modified time stamp
    const QDateTime lastModified = m_fileInfo.lastModified(QTimeZone::UTC);
    quint64 lastmod = quint64(lastModified.isValid() ? lastModified.toMSecsSinceEpoch() : 0);
    static const quint64 sourceDate = 1000 * qgetenv("QT_RCC_SOURCE_DATE_OVERRIDE").toULongLong();
    if (sourceDate != 0)
        lastmod = sourceDate;
    static const quint64 sourceDate2 = 1000 * qgetenv("SOURCE_DATE_EPOCH").toULongLong();
    if (sourceDate2 != 0)
        lastmod = sourceDate2;
    lib.writeNumber8(lastmod);
}

qint64 RCCFileInfo::writeDataBlob(RCCResourceLibrary& lib,
    qint64 offset,
    DeduplicationMultiHash& dedupByContent,
    QString* errorMessage)
{
    //capture the offset
    m_dataOffset = offset;
    QByteArray data;

        // find the data to be written
        const QString absoluteFilePath = m_fileInfo.absoluteFilePath();
        QFile file(absoluteFilePath);
        if (!file.open(QFile::ReadOnly)) {
            QMessageBox::critical(nullptr,
                QApplication::applicationName() + " - " + QObject::tr("Gestion fichier image signature"),
                QObject::tr("Le fichier signature") +
                absoluteFilePath +
                QObject::tr("est impossible à ouvrir. Erreur :\n")
                + file.errorString());
            return 0;
        }
        data = file.readAll();

        // de-duplicate the same file content, we can re-use already written data
        // we only do that if we have the same compression settings
        const QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
        const DeduplicationKey key{ hash };
        const QList<RCCFileInfo*> potentialCandidates = dedupByContent.values(key);
        for (const RCCFileInfo* candidate : potentialCandidates) {
            // check real content, we can have collisions
            QFile candidateFile(candidate->m_fileInfo.absoluteFilePath());
            if (!candidateFile.open(QFile::ReadOnly)) {
                QMessageBox::critical(nullptr,
                    QApplication::applicationName() + " - " + QObject::tr("Gestion fichier image signature"),
                    QObject::tr("Le fichier signature") +
                    candidate->m_fileInfo.absoluteFilePath() +
                    QObject::tr("est impossible à ouvrir. Erreur :\n")
                    + candidateFile.errorString());
                return 0;
            }
            if (data != candidateFile.readAll()) {
                continue;
            }
            // just remember the offset & flags with final compression state
            // of the already written data and be done
            m_dataOffset = candidate->m_dataOffset;
            m_flags = candidate->m_flags;
            return offset;
        }
        dedupByContent.insert(key, this);

    // write the length
    lib.writeNumber4(data.size());
    offset += 4;

    // write the payload
    const char* p = data.constData();
    lib.writeByteArray(data);

    offset += data.size();

    return offset;
}

qint64 RCCFileInfo::writeDataName(RCCResourceLibrary& lib, qint64 offset)
{
    // capture the offset
    m_nameOffset = offset;

    // write the length
    lib.writeNumber2(m_name.size());
    offset += 2;

    // write the hash
    lib.writeNumber4(qt_hash(m_name));
    offset += 4;

    // write the m_name
    const QChar* unicode = m_name.unicode();
    for (int i = 0; i < m_name.size(); ++i) {
        lib.writeNumber2(unicode[i].unicode());
    }
    offset += m_name.size() * 2;

    // done
    return offset;
}

///////////////////////////////////////////////////////////
//
// RCCResourceLibrary
//
///////////////////////////////////////////////////////////
RCCResourceLibrary::RCCResourceLibrary()
    : m_root(nullptr),
    m_treeOffset(0),
    m_namesOffset(0),
    m_dataOffset(0),
    m_overallFlags(0),
    m_formatVersion(3)
{
    m_out.reserve(30 * 1000 * 1000);
}

RCCResourceLibrary::~RCCResourceLibrary()
{
    delete m_root;
}

bool RCCResourceLibrary::addFile(const QString& alias, RCCFileInfo file)
{
    if (file.m_fileInfo.size() > 0xffffffff) {
        QMessageBox::critical(nullptr,
            QApplication::applicationName() + " - " + QObject::tr("Gestion fichier image signature"),
            QObject::tr("Le fichier signature") +
            file.m_fileInfo.absoluteFilePath() +
            QObject::tr("est trop gros."));
        return false;
    }
    if (!m_root) {
        m_root = new RCCFileInfo{};
        m_root->m_flags = RCCFileInfo::Directory;
    }

    RCCFileInfo* parent = m_root;
    const QStringList nodes = alias.split(u'/');
    const QString filename = nodes.at(nodes.size() - 1);
    RCCFileInfo* s = new RCCFileInfo(std::move(file));
    s->m_parent = parent;
    parent->m_children.insert(filename, s);
    return true;
}

void RCCResourceLibrary::reset()
{
    if (m_root) {
        delete m_root;
        m_root = nullptr;
    }
}


bool RCCResourceLibrary::readFiles(QString p_fichier)
{
    reset();
    //read in data
    
    QFileInfo file(p_fichier);
    addFile("/signature/"+file.fileName(),
        RCCFileInfo(file.fileName(),
            file,
            RCCFileInfo::NoFlags));

    return true;
}

QStringList RCCResourceLibrary::dataFiles() const
{
    QStringList ret;
    QStack<RCCFileInfo*> pending;

    if (!m_root)
        return ret;
    pending.push(m_root);
    while (!pending.isEmpty()) {
        RCCFileInfo* file = pending.pop();
        for (auto it = file->m_children.begin();
            it != file->m_children.end(); ++it) {
            RCCFileInfo* child = it.value();
            if (child->m_flags & RCCFileInfo::Directory)
                pending.push(child);
            else
                ret.append(child->m_fileInfo.filePath());
        }
    }
    return ret;
}

bool RCCResourceLibrary::output(QString p_fichierSortie)
{
    //write out
    if (!writeHeader()) {
        return false;
    }
    if (m_root) {
        if (!writeDataBlobs()) {
            return false;
        }
        if (!writeDataNames()) {
            return false;
        }
        if (!writeDataStructure()) {
            return false;
        }
    }
    if (!writeInitializer()) {
        return false;
    }

    QFile outDevice;
    outDevice.setFileName(p_fichierSortie);
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    outDevice.open(mode);
    

    outDevice.write(m_out.constData(), m_out.size());
    outDevice.close();
    return true;
}

static const char hexDigits[] = "0123456789abcdef";

inline void RCCResourceLibrary::write2HexDigits(quint8 number)
{
    writeChar(hexDigits[number >> 4]);
    writeChar(hexDigits[number & 0xf]);
}

void RCCResourceLibrary::writeNumber2(quint16 number)
{
        writeChar(number >> 8);
        writeChar(number);
}

void RCCResourceLibrary::writeNumber4(quint32 number)
{
        writeChar(number >> 24);
        writeChar(number >> 16);
        writeChar(number >> 8);
        writeChar(number);
}

void RCCResourceLibrary::writeNumber8(quint64 number)
{
        writeChar(number >> 56);
        writeChar(number >> 48);
        writeChar(number >> 40);
        writeChar(number >> 32);
        writeChar(number >> 24);
        writeChar(number >> 16);
        writeChar(number >> 8);
        writeChar(number);
}

bool RCCResourceLibrary::writeHeader()
{
        writeString("qres");
        writeNumber4(0);
        writeNumber4(0);
        writeNumber4(0);
        writeNumber4(0);
        if (m_formatVersion >= 3)
            writeNumber4(m_overallFlags);
    return true;
}

bool RCCResourceLibrary::writeDataBlobs()
{
    m_dataOffset = m_out.size();

    if (!m_root)
        return false;

    QStack<RCCFileInfo*> pending;
    pending.push(m_root);
    qint64 offset = 0;
    RCCFileInfo::DeduplicationMultiHash dedupByContent;
    QString errorMessage;
    while (!pending.isEmpty()) {
        RCCFileInfo* file = pending.pop();
        for (auto it = file->m_children.cbegin(); it != file->m_children.cend(); ++it) {
            RCCFileInfo* child = it.value();
            if (child->m_flags & RCCFileInfo::Directory)
                pending.push(child);
            else {
                offset = child->writeDataBlob(*this, offset,
                    dedupByContent, &errorMessage);
                if (offset == 0) {
                    QMessageBox::critical(nullptr,
                        QApplication::applicationName() + " - " + QObject::tr("Gestion fichier image signature"),
                        QObject::tr("Erreur sur l'écriture du fichier signature :\n") +
                        errorMessage);
                    return false;
                }
            }
        }
    }
    return true;
}

bool RCCResourceLibrary::writeDataNames()
{ 
    m_namesOffset = m_out.size();

    QHash<QString, int> names;
    QStack<RCCFileInfo*> pending;

    if (!m_root)
        return false;

    pending.push(m_root);
    qint64 offset = 0;
    while (!pending.isEmpty()) {
        RCCFileInfo* file = pending.pop();
        for (auto it = file->m_children.cbegin(); it != file->m_children.cend(); ++it) {
            RCCFileInfo* child = it.value();
            if (child->m_flags & RCCFileInfo::Directory)
                pending.push(child);
            if (names.contains(child->m_name)) {
                child->m_nameOffset = names.value(child->m_name);
            }
            else {
                names.insert(child->m_name, offset);
                offset = child->writeDataName(*this, offset);
            }
        }
    }
 
    return true;
}

struct qt_rcc_compare_hash
{
    typedef bool result_type;
    result_type operator()(const RCCFileInfo* left, const RCCFileInfo* right) const
    {
        return qt_hash(left->m_name) < qt_hash(right->m_name);
    }
};

bool RCCResourceLibrary::writeDataStructure()
{  
    m_treeOffset = m_out.size();

    QStack<RCCFileInfo*> pending;

    if (!m_root)
        return false;

    //calculate the child offsets (flat)
    pending.push(m_root);
    int offset = 1;
    while (!pending.isEmpty()) {
        RCCFileInfo* file = pending.pop();
        file->m_childOffset = offset;

        //sort by hash value for binary lookup
        QList<RCCFileInfo*> m_children = file->m_children.values();
        std::sort(m_children.begin(), m_children.end(), qt_rcc_compare_hash());

        //write out the actual data now
        for (int i = 0; i < m_children.size(); ++i) {
            RCCFileInfo* child = m_children.at(i);
            ++offset;
            if (child->m_flags & RCCFileInfo::Directory)
                pending.push(child);
        }
    }

    //write out the structure (ie iterate again!)
    pending.push(m_root);
    m_root->writeDataInfo(*this);
    while (!pending.isEmpty()) {
        RCCFileInfo* file = pending.pop();

        //sort by hash value for binary lookup
        QList<RCCFileInfo*> m_children = file->m_children.values();
        std::sort(m_children.begin(), m_children.end(), qt_rcc_compare_hash());

        //write out the actual data now
        for (int i = 0; i < m_children.size(); ++i) {
            RCCFileInfo* child = m_children.at(i);
            child->writeDataInfo(*this);
            if (child->m_flags & RCCFileInfo::Directory)
                pending.push(child);
        }
    }
    return true;
}

bool RCCResourceLibrary::writeInitializer()
{ 
        int i = 4;
        char* p = m_out.data();
        p[i++] = 0;
        p[i++] = 0;
        p[i++] = 0;
        p[i++] = m_formatVersion;

        p[i++] = (m_treeOffset >> 24) & 0xff;
        p[i++] = (m_treeOffset >> 16) & 0xff;
        p[i++] = (m_treeOffset >> 8) & 0xff;
        p[i++] = (m_treeOffset >> 0) & 0xff;

        p[i++] = (m_dataOffset >> 24) & 0xff;
        p[i++] = (m_dataOffset >> 16) & 0xff;
        p[i++] = (m_dataOffset >> 8) & 0xff;
        p[i++] = (m_dataOffset >> 0) & 0xff;

        p[i++] = (m_namesOffset >> 24) & 0xff;
        p[i++] = (m_namesOffset >> 16) & 0xff;
        p[i++] = (m_namesOffset >> 8) & 0xff;
        p[i++] = (m_namesOffset >> 0) & 0xff;

        if (m_formatVersion >= 3) {
            p[i++] = (m_overallFlags >> 24) & 0xff;
            p[i++] = (m_overallFlags >> 16) & 0xff;
            p[i++] = (m_overallFlags >> 8) & 0xff;
            p[i++] = (m_overallFlags >> 0) & 0xff;
    }
    return true;
}

QT_END_NAMESPACE
