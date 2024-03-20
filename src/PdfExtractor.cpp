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
    for (unsigned noPage = 0; noPage < pages.GetCount(); noPage++)
    {
        qDebug() << "debut page" << noPage;
        auto& page = pages.GetPageAt(noPage);

        std::vector<PoDoFo::PdfTextEntry> entries;
        page.ExtractTextTo(entries);
        qDebug() << "fin extract page" << noPage;

        //for (auto& entry : entries)
        //{
        //    qDebug() << entry.X << entry.Y << entry.Text.data();
        //}

        AeroDmsTypes::Aeroclub aeroclub = AeroDmsTypes::Aeroclub_INCONNU;

        int index = 0 ;
        while ( index < entries.size() && aeroclub == AeroDmsTypes::Aeroclub_INCONNU)
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
            else if (QString(entries.at(index).Text.data()).contains("Aéro-club de Bordeaux"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_ACB;
                qDebug() << "Aéroclub trouvé : ACB";
            }
            else if (QString(entries.at(index).Text.data()).contains("Aéro Club d'Andernos"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_ACAndernos;
                qDebug() << "Aéroclub trouvé : Aéroclub d'Andernos";
            }

            index++;
        }

        switch (aeroclub)
        {
            case AeroDmsTypes::Aeroclub_CAPAM:
            {
                liste.append(extraireDonneesCapam(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_DACA:
            {
                liste.append(extraireDonneesDaca(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_ACB:
            {
                liste.append(extraireDonneesCapam(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_ACAndernos:
            {
                liste.append(extraireDonneesACAndernos(entries, noPage));
            }
            break;
            default:
                break;
        }
    }

    //qDebug() << "taille" << liste.size();

    return liste;
}

AeroDmsTypes::DonneesFacture PdfExtractor::extraireDonneesCapam( std::vector<PoDoFo::PdfTextEntry> p_entries,
                                                                 const unsigned p_noPage)
{
    int index = 0;
    AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
    donneesFactures.pageDansLeFichierPdf = p_noPage;

    while ( index < p_entries.size())
    {
        QString data = QString(p_entries.at(index).Text.data()).replace("\xc2\xa0", " ");
        if (data.contains("Vol"))
        {
            QStringList str = data.replace("Vol du ", "").split(" ");
            donneesFactures.dateDuVol = extraireDate(str.at(0));
           
            index = index + 2;
            if (index < p_entries.size())
            {
                if (!QString(p_entries.at(index).Text.data()).contains(":") && index+1 < p_entries.size())
                    index++;
                donneesFactures.dureeDuVol = extraireDuree(QString(p_entries.at(index).Text.data()));
            }

            if (index+2 < p_entries.size())
            { 
                if (QString(p_entries.at(index + 2).Text.data()).contains("€"))
                {
                    donneesFactures.coutDuVol = donneesFactures.coutDuVol + QString(p_entries.at(index + 2).Text.data()).replace("€", "").replace(",", ".").toFloat();
                }
                else if (QString(p_entries.at(index + 1).Text.data()).contains("€"))
                {
                    QString montant = QString(p_entries.at(index + 1).Text.data()).split(" ").at(1);
                    donneesFactures.coutDuVol = donneesFactures.coutDuVol + montant.replace("€", "").replace(",", ".").toFloat();
                }
            }            
        }
        index++;
    }
    //qDebug() << donneesFactures.dateDuVol << donneesFactures.dureeDuVol << donneesFactures.coutDuVol;
    return donneesFactures;
}

AeroDmsTypes::DonneesFacture PdfExtractor::extraireDonneesACAndernos( std::vector<PoDoFo::PdfTextEntry> p_entries, 
                                                                      const unsigned p_noPage)
{
    int index = 0;
    AeroDmsTypes::DonneesFacture donneesFacture = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
    donneesFacture.pageDansLeFichierPdf = p_noPage;

    while (index < p_entries.size())
    {
        QString data = QString(p_entries.at(index).Text.data());
        if (data.contains("Vol n°"))
        {
            QStringList str = data.replace("Vol n°", "").split(" ");
            
            donneesFacture.dateDuVol = extraireDate(str.at(2));
            QString duree = str.at(4);
            donneesFacture.dureeDuVol = AeroDmsServices::convertirMinutesEnQTime(duree.replace("min", "").toInt());
        }
        else if (data.contains("Total TTC"))
        {
            //On ne sais pas pourquoi mais parfois la ligne total TTC ne contient pas le montant et il faut chercher en dessous...
            if (data.contains("€"))
            {
                donneesFacture.coutDuVol = data.replace("Total TTC", "").replace(" €", "").replace(",", ".").toFloat();
            }        
            else
            {
                index++;
                donneesFacture.coutDuVol = QString(p_entries.at(index).Text.data()).replace(" €", "").replace(",", ".").toFloat();
            }
            //TODO traiter le cas ou on a une chaine de type "Total TTC 10\u0017,\u0013\u0013€"... (comment faire pour que QString interprete les \u...)
        }
        index++;
    }
    //qDebug() << donneesFacture.dateDuVol << donneesFacture.dureeDuVol << donneesFacture.coutDuVol;

    return donneesFacture;
}

AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesDaca( std::vector<PoDoFo::PdfTextEntry> p_entries,
                                                                     const unsigned p_noPage)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    int index = 0;
    while (index < p_entries.size())
    {
        qDebug() << QString(p_entries.at(index).Text.data());
        if (QString(p_entries.at(index).Text.data()).contains("Vol de"))
        {
            AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
            donneesFactures.pageDansLeFichierPdf = p_noPage;

            const QStringList str = QString(p_entries.at(index).Text.data()).replace("\xc2\xa0", " ").split(" ");
            qDebug() << str;
            
            for (int i = 0; i < str.size(); i++)
            {
                if (str.at(i).contains("/"))
                {
                    donneesFactures.dateDuVol = extraireDate(str.at(i));
                }
                else if(str.at(i).contains(":"))
                {
                    donneesFactures.dureeDuVol = extraireDuree(str.at(i));
                }
                else if (str.at(i).contains("vol"))
                {
                    QString montant = str.at(i);
                    donneesFactures.coutDuVol = montant.replace("vol", "").replace(",", ".").toFloat();
                }

            }
            //donneesFactures.dateDuVol = extraireDate(str.at(4));
            //donneesFactures.dureeDuVol = extraireDuree(str.at(2));

            if (donneesFactures.coutDuVol == 0)
            {
                index = index + 2;
                //Cas du vol avec FI... ça décalle d'un index
                if (!QString(p_entries.at(index).Text.data()).contains("€"))
                {
                    index++;
                }
                donneesFactures.coutDuVol = QString(p_entries.at(index).Text.data()).replace("€", "").replace(",", ".").toFloat();
            }

            //qDebug() << donneesFactures.dateDuVol << donneesFactures.dureeDuVol << donneesFactures.coutDuVol;

            liste.append(donneesFactures);
        }
        index++;
    }

    return liste;
}

const QDate PdfExtractor::extraireDate(const QString p_date)
{
    const QStringList dateStr = p_date.split("/");
    const QDate date = QDate( dateStr.at(2).toInt(),
                              dateStr.at(1).toInt(),
                              dateStr.at(0).toInt());

    return date;
}

const QTime PdfExtractor::extraireDuree(const QString p_duree)
{
    const QStringList dureeStr = p_duree.split(":");
    const QTime duree = QTime( dureeStr.at(0).toInt(),
                                        dureeStr.at(1).toInt(),
                                        0);
    return duree;
}