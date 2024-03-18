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

#include "PdfExtractor.h"

PdfExtractor::PdfExtractor()
{
    
}

AeroDmsTypes::ListeDonneesFacture PdfExtractor::recupererLesDonneesDuPdf(const QString p_fichier)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    PoDoFo::PdfMemDocument doc;
    doc.Load(p_fichier.toStdString());
    auto& pages = doc.GetPages();
    for (unsigned i = 0; i < pages.GetCount(); i++)
    {
        auto& page = pages.GetPageAt(i);

        std::vector<PoDoFo::PdfTextEntry> entries;
        page.ExtractTextTo(entries);

        //for (auto& entry : entries)
        //{
        //    qDebug() << entry.X << entry.Y << entry.Text.data();
        //}

        AeroDmsTypes::Aeroclub aeroclub = AeroDmsTypes::Aeroclub_INCONNU;

        int index = 0 ;
        while ( index < entries.size())
        {
            //qDebug() << entries.at(index).Text.data();
            if (QString(entries.at(index).Text.data()).contains("SIRET : 78194723900013"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_DACA;
                qDebug() << "Aéroclub trouvé : DACA";
            }
            else if (QString(entries.at(index).Text.data()).contains("CAPAM"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_CAPAM;
                qDebug() << "Aéroclub trouvé : CAPAM";
            }
            index++;
        }

        switch (aeroclub)
        {
            case AeroDmsTypes::Aeroclub_CAPAM:
            {
                liste.append(extraireDonneesCapam(entries));
            }
            break;
            case AeroDmsTypes::Aeroclub_DACA:
            {
                liste.append(extraireDonneesDaca(entries));
            }
            break;
        }
    }

    qDebug() << "taille" << liste.size();

    return liste;
}

AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesCapam(std::vector<PoDoFo::PdfTextEntry> p_entries)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    int index = 0;
    while ( index < p_entries.size())
    {
        QString data = QString(p_entries.at(index).Text.data()).replace("\xc2\xa0", " ");
        if (data.contains("Vol"))
        {
            AeroDmsTypes::DonneesFacture donneesFactures;
            QStringList str = data.replace("Vol du ", "").split(" ");
            const QStringList date = str.at(0).split("/");
            donneesFactures.dateDuVol = QDate( date.at(2).toInt(),
                                               date.at(1).toInt(),
                                               date.at(0).toInt());
           
            index = index + 3;
            const QStringList duree = QString(p_entries.at(index).Text.data()).split(":");
            donneesFactures.dureeDuVol = QTime( duree.at(0).toInt(), 
                                                duree.at(1).toInt(), 
                                                0);
            index = index + 2;
            donneesFactures.coutDuVol = QString(p_entries.at(index).Text.data()).replace("€", "").replace(",", ".").toFloat();

            qDebug() << donneesFactures.dateDuVol << donneesFactures.dureeDuVol << donneesFactures.coutDuVol;

            liste.append(donneesFactures);
        }
        index++;
    }

    return liste;
}

AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesDaca(std::vector<PoDoFo::PdfTextEntry> p_entries)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    int index = 0;
    while (index < p_entries.size())
    {
        if (QString(p_entries.at(index).Text.data()).contains("Vol de"))
        {
            AeroDmsTypes::DonneesFacture donneesFactures;

            const QStringList str = QString(p_entries.at(index).Text.data()).split(" ");
            
            const QStringList date = str.at(4).split("/");
            donneesFactures.dateDuVol = QDate(date.at(2).toInt(),
                date.at(1).toInt(),
                date.at(0).toInt());
            
            const QStringList duree = str.at(2).split(":");
            donneesFactures.dureeDuVol = QTime(duree.at(0).toInt(),
                duree.at(1).toInt(),
                0);
            index = index + 2;
            //Cas du vol avec FI... ça décalle d'un index
            if (!QString(p_entries.at(index).Text.data()).contains("€"))
            {
                index++;
            }
            donneesFactures.coutDuVol = QString(p_entries.at(index).Text.data()).replace("€", "").replace(",", ".").toFloat();

            qDebug() << donneesFactures.dateDuVol << donneesFactures.dureeDuVol << donneesFactures.coutDuVol;

            liste.append(donneesFactures);
        }
        index++;
    }

    return liste;
}