/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2023-2025 Clément VERMOT-DESROCHES (clement@vermot.net)

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
#ifndef ZIPEXTRACTORWORKER_H
#define ZIPEXTRACTORWORKER_H

#include <QThread>
#include <QPdfDocument>
#include <QPrinter>
#include <QImage>
#include <QPainter>

class ZipExtractorWorker : public QThread
{
    Q_OBJECT

public:
    ZipExtractorWorker(const QString& p_fichierZip,
        const QString& p_dossierDeSortie,
        QObject* parent = nullptr)
        : QThread(parent), cheminFichierZip(p_fichierZip), cheminDossierDeSortie(p_dossierDeSortie) {
    }

signals:
    void progress(uint64_t p_nbOctetsDecompresses,
        uint64_t p_nbOctetsTotaux);
    void finished();
    void error(const QString& message);

protected:
    void run() override;

private:
    const QString cheminFichierZip;
    const QString cheminDossierDeSortie;

    uint64_t nbOctetsTotaux = 0;

    bool traiterProgressionDecompression(uint64_t p_nbOctetsDecompresses);
    void recupererTailleATraiter(uint64_t p_nbTotalOctets);
};

#endif // ZIPEXTRACTORWORKER_H
