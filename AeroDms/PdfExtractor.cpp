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

#include "PdfExtractor.h"
#include "AeroDmsServices.h"

QRegularExpression dateRe("(?<jour>\\d\\d)/(?<mois>\\d\\d)/(?<annee>\\d\\d\\d\\d)");
QRegularExpression dateReTiretsUs("(?<jour>\\d{1,2})-(?<mois>\\d{1,2})-(?<annee>\\d{4})");
QRegularExpression heureRe("(?<heure>\\d):(?<minutes>\\d\\d)");
QRegularExpression heureHMinRe("(?<heure>\\d)h {0,1}(?<minutes>\\d\\d)min");
QRegularExpression euroRe("(?<montant>\\d+\\[.,]\\d+)€");
QRegularExpression immatRe("(?<immat>F-[A-Z]{4})");

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
        while ( index < entries.size() 
                && aeroclub == AeroDmsTypes::Aeroclub_INCONNU)
        {
            //qDebug() << entries.at(index).Text.data();
            if (QString(entries.at(index).Text.data()).contains("SIRET : 78194723900013")
                || QString(entries.at(index).Text.data()).contains("SIRET : 781 947 239 00021") )
            {
                aeroclub = AeroDmsTypes::Aeroclub_DACA;
                //qDebug() << "Aéroclub trouvé : DACA";
            }
            else if (QString(entries.at(index).Text.data()).contains("CAPAM"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_CAPAM;
                //qDebug() << "Aéroclub trouvé : CAPAM";
            }
            else if ( QString(entries.at(index).Text.data()).contains("Aéroclub Arcachon"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_ACBA;
                //qDebug() << "Aéroclub trouvé : ACBA";
            }
            else if ( QString(entries.at(index).Text.data()).contains("Aerogest"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_Generique_Aerogest;
                //qDebug() << "Aéroclub trouvé : Aerogest (générique)";
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
            else if (QString(entries.at(index).Text.data()).contains("SEPAVIA"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_SEPAVIA;
                //qDebug() << "Aéroclub trouvé : SEPAVIA";
            }
            else if (QString(entries.at(index).Text.data()).contains("Aérodrome de CANNES-MANDELIEU"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_UACA;
                //qDebug() << "Aéroclub trouvé : UACA";
            }
            else if (QString(entries.at(index).Text.data()).contains("ATVV"))
            {
                aeroclub = AeroDmsTypes::Aeroclub_ATVV;
                //qDebug() << "Aéroclub trouvé : ATVV";
            }
            index++;
        }

        //Si on a pas identifié de club, on tente le tout pour le tout avec la méthode générique 1 passe...
        if (aeroclub == AeroDmsTypes::Aeroclub_INCONNU)
        {
            aeroclub = AeroDmsTypes::Aeroclub_GENERIQUE_1_PASSE;
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
            case AeroDmsTypes::Aeroclub_ACBA:
            case AeroDmsTypes::Aeroclub_Generique_Aerogest:
            {
                liste.append(extraireDonneesAerogest(entries, noPage));
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
            case AeroDmsTypes::Aeroclub_SEPAVIA:
            {
                liste.append(extraireDonneesSepavia(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_UACA:
            {
                liste.append(extraireDonneesUaca(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_ATVV:
            {
                liste.append(extraireDonneesAtvv(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_GENERIQUE:
            {
                liste.append(extraireDonneesGenerique(entries, noPage));
            }
            break;
            case AeroDmsTypes::Aeroclub_GENERIQUE_1_PASSE:
            {
                liste.append(extraireDonneesGenerique1Passe(entries, noPage));
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
            donneesFactures.dateDuVol = extraireDateRegex(str.at(0));

            if (QString(p_entries.at(index + 1).Text.data()).contains(immatRe))
            {
                QRegularExpressionMatch match = immatRe.match(QString(p_entries.at(index + 1).Text.data()));
                donneesFactures.immat = match.captured("immat");
            }
           
            index = index + 2;
            if (index < p_entries.size())
            {
                if (!QString(p_entries.at(index).Text.data()).contains(":") && index+1 < p_entries.size())
                    index++;
                donneesFactures.dureeDuVol = extraireDureeRegex(QString(p_entries.at(index).Text.data()));
            }

            if (index+2 < p_entries.size())
            { 
                if (QString(p_entries.at(index + 2).Text.data()).contains("€"))
                {
                    donneesFactures.coutDuVol = donneesFactures.coutDuVol + QString(p_entries.at(index + 2).Text.data()).replace("€", "").replace(",", ".").toDouble();
                }
                else if (QString(p_entries.at(index + 1).Text.data()).contains("€"))
                {
                    QString montant = QString(p_entries.at(index + 1).Text.data()).split(" ").at(1);
                    donneesFactures.coutDuVol = donneesFactures.coutDuVol + montant.replace("€", "").replace(",", ".").toDouble();
                }
            }            
        }
        index++;
    }

    return donneesFactures;
}

AeroDmsTypes::DonneesFacture PdfExtractor::extraireDonneesAerogest( std::vector<PoDoFo::PdfTextEntry> p_entries,
                                                                         const unsigned p_noPage)
{
    int index = 0;
    AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
    donneesFactures.pageDansLeFichierPdf = p_noPage;

    while (index < p_entries.size())
    {
        QString data = QString(p_entries.at(index).Text.data());

        if (data.contains(immatRe))
        {
            QRegularExpressionMatch match = immatRe.match(data);
            donneesFactures.immat = match.captured("immat");
        }

        if (data.contains(dateRe))
        {
            donneesFactures.dateDuVol = extraireDateRegex(data);
        }

        if (data.contains("€") && data.contains("Tarif standard"))
        {
            donneesFactures.coutDuVol = data.remove("Tarif standard : ").remove("€").replace(",",".").toDouble();
        }

        if (data.contains(euroRe))
        {
            QRegularExpressionMatch match = euroRe.match(data);
            donneesFactures.coutDuVol = match.captured("montant").toDouble();
        }

        if (data.contains(" minutes"))
        {
            donneesFactures.dureeDuVol = AeroDmsServices::convertirMinutesEnQTime(data.remove(" minutes").toInt());
        }

        index++;
    }
    
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
            
            donneesFacture.dateDuVol = extraireDateRegex(str.at(2));
            QString duree = str.at(4);
            donneesFacture.dureeDuVol = AeroDmsServices::convertirMinutesEnQTime(duree.replace("min", "").toInt());

        if (data.contains(immatRe))
        {
            QRegularExpressionMatch match = immatRe.match(data);
            donneesFacture.immat = match.captured("immat");
        }
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
            
            for (int i = 0; i < str.size(); i++)
            {
                if (str.at(i).contains("/"))
                {
                    donneesFactures.dateDuVol = extraireDateRegex(str.at(i));
                }
                else if(str.at(i).contains(":"))
                {
                    donneesFactures.dureeDuVol = extraireDureeRegex(str.at(i));
                }
                else if (str.at(i).contains("vol"))
                {
                    QString montant = str.at(i);
                    donneesFactures.coutDuVol = montant.replace("vol", "").replace(",", ".").toDouble();
                }
                else if (str.at(i).contains(immatRe))
                {
                    QRegularExpressionMatch match = immatRe.match(str.at(i));
                    donneesFactures.immat = match.captured("immat");
                }
            }

            if (donneesFactures.coutDuVol == 0)
            {
                index = index + 2;
                //Cas du vol avec FI... ça décale d'un index
                if (!QString(p_entries.at(index).Text.data()).contains("€"))
                {
                    index++;
                }
                donneesFactures.coutDuVol = QString(p_entries.at(index).Text.data()).replace("€", "").replace(",", ".").toDouble();
            }

            liste.append(donneesFactures);
        }
        index++;
    }

    return liste;
}

AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesSepavia( std::vector<PoDoFo::PdfTextEntry> p_entries,
                                                                        const unsigned p_noPage)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    QRegularExpressionMatch match;

    int index = 0;
    AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;

    while (index < p_entries.size())
    {
        QString str = QString(p_entries.at(index).Text.data())
            .replace("\xc2\xa0", " ")
            .replace(",", ".");

        if (str.contains(dateRe) && str.contains("Vol du"))
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
            str = str.replace(" ", "");

            if (str.contains(immatRe))
            {
                QRegularExpressionMatch match = immatRe.match(str);
                donneesFactures.immat = match.captured("immat");
            }

            if (index + 3 < p_entries.size())
            {
                //Durée du vol, index +2
                index = index + 2;
                str = p_entries.at(index).Text.data();
                donneesFactures.dureeDuVol = AeroDmsServices::convertirHeuresDecimalesEnQTime(str.replace(",",".").toDouble());

                //Cout du vol, index +3 par rapport à la date
                index = index + 1;
                str = QString(p_entries.at(index).Text.data()).replace(",",".").replace(" ", "");
                if (str.contains("€")
                    && donneesFactures.coutDuVol == AeroDmsTypes::K_INIT_DONNEES_FACTURE.coutDuVol)
                {
                    //La condition évite d'écraser un éventuel montant déjà trouvé par un montant nul
                    //(certaines factures indiquent un crédit nul en dernière ligne)
                    if (str.replace("€","").toDouble() != 0)
                    {
                        donneesFactures.coutDuVol = str.replace("€", "").toDouble();
                    }
                }
            }
            
        }
        if (donneesFactures.dateDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.dateDuVol
            && donneesFactures.coutDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.coutDuVol
            && donneesFactures.dureeDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.dureeDuVol)
        {
            liste.append(donneesFactures);
            donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
        }

        index++;
    }

    return liste;
}

AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesUaca(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    QRegularExpressionMatch match;

    int index = 0;
    AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;

    while (index < p_entries.size())
    {
        QString str = QString(p_entries.at(index).Text.data());
        if (str.contains(dateReTiretsUs) && str.contains("Vol de"))
        {
            if (donneesFactures.coutDuVol != 0.0 && donneesFactures.dureeDuVol != QTime())
            {
                liste.append(donneesFactures);
                donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
            }
            donneesFactures.pageDansLeFichierPdf = p_noPage;
            donneesFactures.dateDuVol = extraireDateTiretsUsRegex(str);
            donneesFactures.dureeDuVol = extraireDureeRegex(str);
            donneesFactures.immat = immatRe.match(str).captured("immat");
        }
        //Si on a une date, on cherche le montant dans le champ suivant
        if (donneesFactures.dateDuVol != QDate())
        {
            if (index + 1 < p_entries.size())
            {
                index = index + 1;
                str = p_entries.at(index).Text.data();
                donneesFactures.coutDuVol = str.replace("€", "").replace(",", ".").toDouble();
            }
        }
       
        if (donneesFactures.dateDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.dateDuVol
            && donneesFactures.coutDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.coutDuVol
            && donneesFactures.dureeDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.dureeDuVol)
        {
            liste.append(donneesFactures);
            donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
        }

        index++;
    }

    return liste;
}

AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesAtvv(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    QRegularExpression immatReGlobal("(?<immat>(F-[A-Z]{4}|D-\\d{4}))");
    QRegularExpression dureeRe("\\((?<heure>\\d{1,2}):(?<minute>\\d{2})\\)");
    QRegularExpression montantRe("\\b(?<montant>(\\d+[.,]\\d{2}))\\b"); // Détecte les nombres au format XX,XX ou XX.XX

    QList<AeroDmsTypes::DonneesFacture> vols;
    QList<AeroDmsTypes::DonneesFacture> lancements;

    bool recherchePrixLancement = false;
    bool recherchePrixVol = false;

    for (const auto& entry : p_entries)
    {
        QString data = QString(entry.Text.data()).replace("\xc2\xa0", " ");

        if (data.contains("Cellule") && data.contains(immatReGlobal))
        {
            AeroDmsTypes::DonneesFacture vol;
            vol.pageDansLeFichierPdf = p_noPage;

            // Extraction des données de vol
            QRegularExpressionMatch match = immatReGlobal.match(data);
            vol.immat = match.captured("immat");

            vol.dateDuVol = extraireDateRegex(data);

            // Extraction de la durée de vol
            QRegularExpressionMatch dureeMatch = dureeRe.match(data);
            if (dureeMatch.hasMatch())
            {
                vol.dureeDuVol = QTime(dureeMatch.captured("heure").toInt(), dureeMatch.captured("minute").toInt());  
            }

            recherchePrixLancement = false;
            recherchePrixVol = false;

            //Parfois, on a le montant en bout de ligne... on vérifie si c'est le cas...
            QRegularExpressionMatch montantMatch = montantRe.match(data);
            if (montantMatch.hasMatch())
            {
                vol.coutDuVol = montantMatch.captured("montant").replace(",", ".").toDouble();;
            }
            else
            {
                recherchePrixVol = true;
            }
            
            vols.push_back(vol);
        }
        else if (data.contains("Treuillage") 
            || data.contains("Remorqué") 
            || data.contains("Tour de piste"))
        {
            // Extraction des données de lancement
            AeroDmsTypes::DonneesFacture lancement;

            QRegularExpressionMatch match = immatReGlobal.match(data);
            lancement.immat = match.captured("immat");

            lancement.dateDuVol = extraireDateRegex(data);

            recherchePrixLancement = false;
            recherchePrixVol = false;

            //Parfois, on a le montant en bout de ligne... on vérifie si c'est le cas...
            QRegularExpressionMatch montantMatch = montantRe.match(data);
            if (montantMatch.hasMatch())
            {
                lancement.coutDuVol = montantMatch.captured("montant").replace(",", ".").toDouble();;
            }
            else
            {
                recherchePrixLancement = true;
            }

            lancements.push_back(lancement);
        }
        else
        {
            // Extraction du coût
            QStringList parts = data.split(" ");
            for (QString& part : parts)
            {
                QRegularExpressionMatch montantMatch = montantRe.match(data);
                if (montantMatch.hasMatch())
                {
                    if (recherchePrixVol)
                    {
                        vols.last().coutDuVol = part.replace(",", ".").toDouble();
                    }
                    else if (recherchePrixLancement)
                    {
                        lancements.last().coutDuVol = part.replace(",", ".").toDouble();
                    }

                    recherchePrixVol = false;
                    recherchePrixLancement = false;
                }
            }
        }
    }

    // Association des lancements aux vols
    size_t indexLancement = 0;
    for (auto& vol : vols)
    {
        while (indexLancement < lancements.size())
        {
            const auto& lancement = lancements[indexLancement];
            if (lancement.dateDuVol == vol.dateDuVol)
            {
                vol.coutDuVol += lancement.coutDuVol;
                lancements.remove(indexLancement);
                break;
            }
            indexLancement++;
        }
        liste.push_back(vol);
    }

    return liste;
}


AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesGenerique( std::vector<PoDoFo::PdfTextEntry> p_entries, 
                                                                          const unsigned p_noPage)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    QRegularExpressionMatch match;

    int index = 0;
    AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;

    while (index < p_entries.size())
    {
        QString str = QString(p_entries.at(index).Text.data())
            .replace("\xc2\xa0", " ")
            .replace(",", ".");
      
        if (str.contains(dateRe))
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
            str = str.replace(" ", "");
            if (str.contains(heureRe)
                || str.contains(heureHMinRe))
            {
                donneesFactures.dureeDuVol = extraireDureeRegex(str);
            }
            if (str.contains(euroRe))
            {
                QRegularExpressionMatch match = euroRe.match(str);
                //La condition évite d'écraser un éventuel montant déjà trouvé par un montant nul
                //(certaines factures indiquent un crédit nul en dernière ligne)
                if (match.captured("montant").toDouble() != 0)
                {
                    donneesFactures.coutDuVol = match.captured("montant").toDouble();
                }
            }
            if (str.contains(immatRe))
            {
                QRegularExpressionMatch match = immatRe.match(str);
                donneesFactures.immat = match.captured("immat");
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

AeroDmsTypes::ListeDonneesFacture PdfExtractor::extraireDonneesGenerique1Passe( std::vector<PoDoFo::PdfTextEntry> p_entries,
                                                                                const unsigned p_noPage)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    QRegularExpressionMatch match;

    int index = 0;
    AeroDmsTypes::DonneesFacture donneesFactures = AeroDmsTypes::K_INIT_DONNEES_FACTURE;

    while (index < p_entries.size())
    {
        QString str = QString(p_entries.at(index).Text.data())
            .replace("\xc2\xa0", " ")
            .replace(",", ".");

        if (str.contains(dateRe))
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
            str = str.replace(" ", "");
            if (str.contains(heureRe)
                || str.contains(heureHMinRe)
                && donneesFactures.dureeDuVol == AeroDmsTypes::K_INIT_DONNEES_FACTURE.dureeDuVol)
            {
                donneesFactures.dureeDuVol = extraireDureeRegex(str);
            }
            if (str.contains(euroRe)
                && donneesFactures.coutDuVol == AeroDmsTypes::K_INIT_DONNEES_FACTURE.coutDuVol)
            {
                QRegularExpressionMatch match = euroRe.match(str);
                //La condition évite d'écraser un évenutel montant déjà trouvé par un montant nul
                //(certaines factures indiquent un credit nul en dernière ligne)
                if (match.captured("montant").toDouble() != 0)
                {
                    donneesFactures.coutDuVol = match.captured("montant").toDouble();
                }
            }
            if (str.contains(immatRe))
            {
                QRegularExpressionMatch match = immatRe.match(str);
                donneesFactures.immat = match.captured("immat");
            }
        }
        index++;
    }

    if (donneesFactures.dateDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.dateDuVol
        && donneesFactures.coutDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.coutDuVol
        && donneesFactures.dureeDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.dureeDuVol)
    {
        liste.append(donneesFactures);
    }

    return liste;
}

const QDate PdfExtractor::extraireDateRegex(const QString p_str)
{
    QRegularExpressionMatch match = dateRe.match(p_str);

    const QDate date = QDate(match.captured("annee").toInt(),
        match.captured("mois").toInt(),
        match.captured("jour").toInt());

    return date;
}

const QDate PdfExtractor::extraireDateTiretsUsRegex(const QString p_str)
{
    QRegularExpressionMatch match = dateReTiretsUs.match(p_str);

    const QDate date = QDate(match.captured("annee").toInt(),
        match.captured("mois").toInt(),
        match.captured("jour").toInt());

    return date;
}

const QTime PdfExtractor::extraireDureeRegex(const QString p_str)
{
    QRegularExpressionMatch match ;

    if (p_str.contains(heureRe))
    {
        match = heureRe.match(p_str);
    }
    else
    {
        match = heureHMinRe.match(p_str);
    }
   
    const QTime duree = QTime( match.captured("heure").toInt(),
                               match.captured("minutes").toInt(),
                               0);
    return duree;
}

const double PdfExtractor::recupererMontantAca(QString p_chaine)
{
    //On ne sais pas trop pourquoi mais parfois (pas toujours...) les montant sont
    // encodés en partie avec des \uXXXX
    // => "Total TTC 10\u0017,\u0013\u0013€" pour 104,00€ par exemple
        
    //Donc on fait un peu de ménage et on retourne le double...
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
        .toDouble();
}

AeroDmsTypes::ListeDonneesFacture PdfExtractor::recupererLesDonneesDuCsv(const QString p_fichier)
{
    AeroDmsTypes::ListeDonneesFacture liste;

    QFile fichierCsv(p_fichier);
    if (fichierCsv.open(QIODevice::ReadOnly))
    {
        QTextStream in(&fichierCsv);
        //in.setAutoDetectUnicode(true);
        in.setEncoding(QStringConverter::System);
        if (!in.readAll().contains("€"))
        {
            in.setEncoding(QStringConverter::Utf8);
        }
        in.seek(0);
        while (!in.atEnd())
        {
            AeroDmsTypes::DonneesFacture donnesFacture = AeroDmsTypes::K_INIT_DONNEES_FACTURE;
            QString line = in.readLine();
            QStringList csvItem = line.split(";");
            for (int i = 0; i < csvItem.size(); i++)
            {
                QString str = csvItem.at(i);
                str.replace(",", ".").replace(" ", "");
                if (str.contains(heureRe))
                {
                    donnesFacture.dureeDuVol = extraireDureeRegex(str);
                }
                else if (str.contains(dateRe))
                {
                    donnesFacture.dateDuVol = extraireDateRegex(str);
                }
                else if (str.contains("€"))
                {
                    donnesFacture.coutDuVol = str.replace("€", "").toDouble();
                }
                else if (str.contains(immatRe))
                {
                    QRegularExpressionMatch match = immatRe.match(str);
                    donnesFacture.immat = match.captured("immat");
                }
            }
            //On remonte toute donnes pour laquelle on a récupéré au moins un champ...
            if (donnesFacture.coutDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.coutDuVol
                || donnesFacture.immat != AeroDmsTypes::K_INIT_DONNEES_FACTURE.immat
                || donnesFacture.dateDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.dateDuVol
                || donnesFacture.dureeDuVol != AeroDmsTypes::K_INIT_DONNEES_FACTURE.dureeDuVol)
            {
                liste.append(donnesFacture);
            }
        }
        fichierCsv.close();
    }

    return liste;
}
