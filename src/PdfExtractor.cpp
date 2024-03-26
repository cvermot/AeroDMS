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

AeroDmsTypes::ListeDonneesFacture PdfExtractor::recupererLesDonneesDuPdf( const QString p_fichier,
                                                                          const AeroDmsTypes::Aeroclub p_aeroclub)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    PoDoFo::PdfMemDocument doc;
    doc.Load(p_fichier.toStdString());
    auto& pages = doc.GetPages();
    for (unsigned noPage = 0; noPage < pages.GetCount(); noPage++)
    {
        auto& page = pages.GetPageAt(noPage);

        std::vector<PoDoFo::PdfTextEntry> entries;
        page.ExtractTextTo(entries);

        AeroDmsTypes::Aeroclub aeroclub = p_aeroclub;

        int index = 0 ;
        while ( index < entries.size() && aeroclub == AeroDmsTypes::Aeroclub_INCONNU)
        {
            //qDebug() << entries.at(index).Text.data();
            if (QString(entries.at(index).Text.data()).contains("SIRET : 78194723900013"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_DACA;
                //qDebug() << "Aéroclub trouvé : DACA";
            }
            else if (QString(entries.at(index).Text.data()).contains("CAPAM"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_CAPAM;
                //qDebug() << "Aéroclub trouvé : CAPAM";
            }
            else if (QString(entries.at(index).Text.data()).contains("Aéro-club de Bordeaux"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_ACB;
                //qDebug() << "Aéroclub trouvé : ACB";
            }
            else if (QString(entries.at(index).Text.data()).contains("Aéro Club d'Andernos"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_ACAndernos;
                //qDebug() << "Aéroclub trouvé : Aéroclub d'Andernos";
            }

            index++;
        }

        switch (aeroclub)
        {
            case AeroDmsTypes::Aeroclub_CAPAM:
            case AeroDmsTypes::Aeroclub_ACB:
            case AeroDmsTypes::Aeroclub_Generique_OpenFlyer:
            {
                liste.append(extraireDonneesOpenFlyer(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_DACA:
            {
                liste.append(extraireDonneesDaca(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_ACAndernos:
            {
                liste.append(extraireDonneesACAndernos(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_GENERIQUE:
            {
                liste.append(extraireDonneesGenerique(entries, noPage));
            }
            break;
            default:
                break;
        }
    }

    return liste;
}

AeroDmsTypes::DonneesFacture PdfExtractor::extraireDonneesOpenFlyer( std::vector<PoDoFo::PdfTextEntry> p_entries,
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
                donneesFacture.coutDuVol = recupererMontantAca(data);
            }        
            else
            {
                index++;
                donneesFacture.coutDuVol = recupererMontantAca(QString(p_entries.at(index).Text.data()));
            }
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
        //qDebug() << QString(p_entries.at(index).Text.data());
        if (QString(p_entries.at(index).Text.data()).contains("Vol de"))
        {
            AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
            donneesFactures.pageDansLeFichierPdf = p_noPage;

            const QStringList str = QString(p_entries.at(index).Text.data()).replace("\xc2\xa0", " ").split(" ");
            //qDebug() << str;
            
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

AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesGenerique( std::vector<PoDoFo::PdfTextEntry> p_entries, 
                                                                          const unsigned p_noPage)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    QRegularExpression date("(?<date>\\d\\d)/(?<month>\\d\\d)/(?<year>\\d\\d\\d\\d)");
    QRegularExpressionMatch match;
    QRegularExpression heure("(?<heure>\\d):(?<minutes>\\d\\d)");
    QRegularExpression euro("(?<montant>\\d+\\.\\d+)€");

    int index = 0;
    AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;

    while (index < p_entries.size())
    {
        QString str = QString(p_entries.at(index).Text.data())
            .replace("\xc2\xa0", " ")
            .replace(" ", "")
            .replace(",", ".")
            .replace('\u0013', "0")
            .replace('\u0014', "1")
            .replace('\u0015', "2")
            .replace('\u0016', "3")
            .replace('\u0017', "4")
            .replace('\u0018', "5")
            .replace('\u0019', "6")
            .replace('\u0020', "7")
            .replace('\u0021', "8")
            .replace('\u0022', "9");

        if (str.contains(date))
        {
            if (donneesFactures.coutDuVol != 0.0 && donneesFactures.dureeDuVol != QTime())
            {
                liste.append(donneesFactures);
                donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
            }
            donneesFactures.pageDansLeFichierPdf = p_noPage;
            donneesFactures.dateDuVol = extraireDateRegex(str);
        }
        //Si on a une date, on cherche les autres champs
        if (donneesFactures.dateDuVol != QDate())
        {
            if (str.contains(heure))
            {
                donneesFactures.dureeDuVol = extraireDureeRegex(str);
            }
            if (str.contains(euro))
            {
                QRegularExpressionMatch match = euro.match(str);
                donneesFactures.coutDuVol = match.captured("montant").toFloat();
            }
        }
        
        index++;
    }

    if (donneesFactures.dateDuVol != QDate()
        && donneesFactures.coutDuVol != 0.0
        && donneesFactures.dureeDuVol != QTime())
    {
        liste.append(donneesFactures);
    }

    return liste;
}

const QDate PdfExtractor::extraireDateRegex(const QString p_str)
{
    QRegularExpression dateRe("(?<jour>\\d\\d)/(?<mois>\\d\\d)/(?<annee>\\d\\d\\d\\d)");
    QRegularExpressionMatch match = dateRe.match(p_str);

    const QDate date = QDate(match.captured("annee").toInt(),
        match.captured("mois").toInt(),
        match.captured("jour").toInt());

    return date;
}

const QTime PdfExtractor::extraireDureeRegex(const QString p_str)
{
    QRegularExpression heureRe("(?<heure>\\d):(?<minutes>\\d\\d)");
    QRegularExpressionMatch match = heureRe.match(p_str);
   
    const QTime duree = QTime(match.captured("heure").toInt(),
        match.captured("minutes").toInt(),
                               0);
    return duree;
}

const QDate PdfExtractor::extraireDate(const QString p_date)
{
    const QStringList dateStr = p_date.split("/");
    const QDate date = QDate(dateStr.at(2).toInt(),
        dateStr.at(1).toInt(),
        dateStr.at(0).toInt());

    return date;
}

const QTime PdfExtractor::extraireDuree(const QString p_duree)
{
    const QStringList dureeStr = p_duree.split(":");
    const QTime duree = QTime(dureeStr.at(0).toInt(),
        dureeStr.at(1).toInt(),
        0);
    return duree;
}

const float PdfExtractor::recupererMontantAca(QString p_chaine)
{
    //On ne sais pas trop pourquoi mais parfois (pas toujours...) les montant sont
    // encodés en partie avec des \uXXXX
    // => "Total TTC 10\u0017,\u0013\u0013€" pour 104,00€ par exemple

    //Donc on fait un peu de ménage et on retourne le float...
    return p_chaine.replace("Total TTC", "")
        .replace(" ", "")
        .replace("€", "")
        .replace(",", ".")
        .replace('\u0013', "0")
        .replace('\u0014', "1")
        .replace('\u0015', "2")
        .replace('\u0016', "3")
        .replace('\u0017', "4")
        .replace('\u0018', "5")
        .replace('\u0019', "6")
        .replace('\u0020', "7")
        .replace('\u0021', "8")
        .replace('\u0022', "9")
        .toFloat();
}