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
#include "PdfPrinterWorker.h"

void PdfPrinterWorker::run()
{
    QPdfDocument doc;
    int attenteChargementFichier = 0;
    doc.load(m_filePath);

    while (doc.status() != QPdfDocument::Status::Ready
        || attenteChargementFichier > 500)
    {
        attenteChargementFichier++;
        QThread::usleep(10);
    }

    QPainter painter;
    if (!painter.begin(m_printer))
    {
        emit error(tr("Impossible de démarrer l'impression."));
        return;
    }

    const int totalPages = doc.pageCount();
    for (int i = 0; i < totalPages; ++i)
    {
        QSizeF pageSize = doc.pagePointSize(i);
        QImage image = doc.render(i,
            QSize( pageSize.width() * m_printer->resolution() / 72,
                   pageSize.height() * m_printer->resolution() / 72));

        //Si la page du PDF est en paysage, on retourne l'image pour la place en portrait
        //pour l'impression
        if (pageSize.width() > pageSize.height())
        {
            QTransform transformation;
            transformation.rotate(270);
            image = image.transformed(transformation);
            painter.drawImage(0, 0, image);
        }
        else
        {
            painter.drawImage(0, 0, image);
        }

        if (!image.isNull())
        {
            painter.drawImage(0, 0, image);
            if (i + 1 < totalPages)
            {
                m_printer->newPage();

                // Ajout d'une page blanche si forçage recto est activé
                if (m_forceRecto)
                {
                    m_printer->newPage();
                }
            }
        }
        emit progress(i + 1, totalPages); // Mise à jour de la progression
    }

    painter.end();
    emit finished();
}