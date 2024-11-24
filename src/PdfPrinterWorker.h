/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2023-2024 Clément VERMOT-DESROCHES (clement@vermot.net)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
/******************************************************************************/
#ifndef PDFPRINTERWORKER_H
#define PDFPRINTERWORKER_H

#include <QThread>
#include <QPdfDocument>
#include <QPrinter>
#include <QImage>
#include <QPainter>

class PdfPrinterWorker : public QThread
{
    Q_OBJECT

public:
    PdfPrinterWorker(const QString& filePath, QPrinter *printer, bool forceRecto, QObject* parent = nullptr)
        : QThread(parent), m_filePath(filePath), m_printer(printer), m_forceRecto(forceRecto) {
       
    }

signals:
    void progress(int currentPage, int totalPages);
    void finished();
    void error(const QString& message);

protected:
    void run() override;

private:
    QString m_filePath;
    QPrinter *m_printer;
    bool m_forceRecto;
};

#endif // PDFPRINTERWORKER_H
