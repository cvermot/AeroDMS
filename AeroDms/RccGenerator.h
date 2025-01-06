// Copyright (C) 2018 The Qt Company Ltd.
// Copyright (C) 2018 Intel Corporation.
// Copyright (C) 2025 Clément Vermot-Desroches
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

// Note: A copy of this file is used in Qt Widgets Designer (qttools/src/designer/src/lib/shared/rcc_p.h)

#ifndef RCC_H
#define RCC_H

#include <QStringList>
#include <QHash>
#include <QString>

QT_BEGIN_NAMESPACE

class RCCFileInfo;
class QIODevice;

class RCCResourceLibrary
{
    RCCResourceLibrary(const RCCResourceLibrary&);

public:
    RCCResourceLibrary();
    ~RCCResourceLibrary();

    bool output(QString p_fichierSortie);

    bool readFiles(QString p_fichier);

    QStringList dataFiles() const;

    // Return a map of resource identifier (':/newPrefix/images/p1.png') to file.
    typedef QHash<QString, QString> ResourceDataFileMap;

private:
    friend class RCCFileInfo;
    void reset();
    bool addFile(const QString& alias, RCCFileInfo file);
    bool writeHeader();
    bool writeDataBlobs();
    bool writeDataNames();
    bool writeDataStructure();
    bool writeInitializer();
    void write2HexDigits(quint8 number);
    void writeNumber2(quint16 number);
    void writeNumber4(quint32 number);
    void writeNumber8(quint64 number);
    void writeChar(char c) { m_out.append(c); }
    void writeByteArray(const QByteArray&);
    void write(const char*, int len);
    void writeString(const char* s) { write(s, static_cast<int>(strlen(s))); }

    RCCFileInfo* m_root;
    int m_treeOffset;
    int m_namesOffset;
    int m_dataOffset;
    quint32 m_overallFlags;
    QByteArray m_out;
    quint8 m_formatVersion;
};

QT_END_NAMESPACE

#endif // RCC_H