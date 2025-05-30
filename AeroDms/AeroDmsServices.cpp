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

#include "AeroDmsServices.h"

#include <podofo/podofo.h>

#ifdef Q_OS_WIN
#pragma comment(lib, "crypt32.lib")
#include <windows.h>
#include <wincrypt.h>
#endif

const QString AeroDmsServices::convertirMinutesEnHeuresMinutes(const int p_minutes)
{
    const int heures = p_minutes / 60;
    const int minutes = p_minutes % 60;
    QString minutesString = QString::number(minutes);
    if (minutesString.size() == 1)
    {
        minutesString = QString("0").append(minutesString);
    }
    QString heuresMinutes = QString::number(heures).append("h").append(minutesString);
    return heuresMinutes;
}

const QTime AeroDmsServices::convertirMinutesEnQTime(const int p_minutes)
{
    const int heures = p_minutes / 60;
    const int minutes = p_minutes % 60;
    return QTime(heures, minutes, 0);
}

const QTime AeroDmsServices::convertirHeuresDecimalesEnQTime(const double p_heureDecimale)
{
    QTime heureHhmm;
    int heure = floor(p_heureDecimale);
    int minutes = 60 * (p_heureDecimale - floor(p_heureDecimale));
    heureHhmm.setHMS(heure, minutes, 0);

    return heureHhmm;
}

const QIcon AeroDmsServices::recupererIcone(const QString& p_icone)
{  
    if (p_icone == "ULM")
    {
        return recupererIcone(AeroDmsTypes::Icone_ULM);
    }
    else if (p_icone == "Avion")
    {
        return recupererIcone(AeroDmsTypes::Icone_AVION);
    }
    else if (p_icone == "Planeur")
    {
        return recupererIcone(AeroDmsTypes::Icone_PLANEUR);
    }
    else if (p_icone == "Hélicoptère")
    {
        return recupererIcone(AeroDmsTypes::Icone_HELICOPTERE);
    }
    else if (p_icone == "Avion électrique")
    {
        return recupererIcone(AeroDmsTypes::Icone_AVION_ELECTRIQUE);
    }
    else if (p_icone == "Balade")
    {
        return recupererIcone(AeroDmsTypes::Icone_BALADE);
    }
    else if (p_icone == "Sortie")
    {
        return recupererIcone(AeroDmsTypes::Icone_SORTIE);
    }
    else if (p_icone == "Entrainement")
    {
        return recupererIcone(AeroDmsTypes::Icone_ENTRAINEMENT);
    }
    else if (p_icone == "Oui")
    {
        return recupererIcone(AeroDmsTypes::Icone_OUI);
    }
    else if (p_icone == "Non")
    {
        return recupererIcone(AeroDmsTypes::Icone_NON);
    }  
    else
    {
        return recupererIcone(AeroDmsTypes::Icone_INCONNUE);
    }
}

const QIcon AeroDmsServices::recupererIcone(const QChar p_caractere)
{
    return recupererIcone(AeroDmsTypes::Icone(p_caractere.toLatin1()));
}

const QIcon AeroDmsServices::recupererIcone(const AeroDmsTypes::Icone p_icone)
{
    switch (p_icone)
    {
        case AeroDmsTypes::Icone_ULM:
        {
            return QIcon(":/AeroDms/ressources/feather.svg");
        }
        break;
        case AeroDmsTypes::Icone_AVION:
        {
            return QIcon(":/AeroDms/ressources/airplane.svg");
        }
        break;
        case AeroDmsTypes::Icone_PLANEUR:
        {
            return QIcon(":/AeroDms/ressources/airplane-landing.svg");
        }
        break;
        case AeroDmsTypes::Icone_HELICOPTERE:
        {
            return QIcon(":/AeroDms/ressources/helicopter.svg");
        }
        break;
        case AeroDmsTypes::Icone_AVION_ELECTRIQUE:
        {
            return QIcon(":/AeroDms/ressources/lightning-bolt.svg");
        }
        break;
        case AeroDmsTypes::Icone_BALADE:
        {
            return QIcon(":/AeroDms/ressources/account-group.svg");
        }
        break;
        case AeroDmsTypes::Icone_SORTIE:
        {
            return QIcon(":/AeroDms/ressources/bag-checked.svg");
        }
        break;
        case AeroDmsTypes::Icone_ENTRAINEMENT:
        {
            return QIcon(":/AeroDms/ressources/airport.svg");
        }
        break;
        case AeroDmsTypes::Icone_TOTAL:
        {
            return QIcon(":/AeroDms/ressources/sigma.svg");
        }
        break;
        case AeroDmsTypes::Icone_OUI:
        {
            return QIcon(":/AeroDms/ressources/check.svg");
        }
        break;
        case AeroDmsTypes::Icone_NON:
        {
            return QIcon(":/AeroDms/ressources/close.svg");
        }
        break;
        case AeroDmsTypes::Icone_TOUT_COCHER:
        {
            return QIcon(":/AeroDms/ressources/check-all.svg");
        }
        break;
        case AeroDmsTypes::Icone_COTISATION:
        {
            return QIcon(":/AeroDms/ressources/ticket.svg");
        }
        break;
        case AeroDmsTypes::Icone_RECETTE:
        {
            return QIcon(":/AeroDms/ressources/file-document-plus.svg");
        }
        break;
        case AeroDmsTypes::Icone_DEPENSE:
        {
            return QIcon(":/AeroDms/ressources/file-document-minus.svg");
        }
        break;
        case AeroDmsTypes::Icone_FACTURE:
        {
            return QIcon(":/AeroDms/ressources/file-document.svg");
        }
        break;
        case AeroDmsTypes::Icone_FINANCIER:
        {
            return QIcon(":/AeroDms/ressources/cash-multiple.svg");
        }
        break;
        case AeroDmsTypes::Icone_TOUS:
        {
            return QIcon(":/AeroDms/ressources/all-inclusive.svg");
        }
        break;
        case AeroDmsTypes::Icone_PILOTE:
        {
            return QIcon(":/AeroDms/ressources/account-tie-hat.svg");
        }
        break;
        case AeroDmsTypes::Icone_GENERE_DEMANDE_SUBVENTIONS:
        {
            return QIcon(":/AeroDms/ressources/file-cog.svg");
        }
        break;
        case AeroDmsTypes::Icone_GENERE_RECAP_HDV:
        {
            return QIcon(":/AeroDms/ressources/account-file-text.svg");
        }
        break;
        case AeroDmsTypes::Icone_AJOUTER_VOL:
        {
            return QIcon(":/AeroDms/ressources/airplane-plus.svg");
        }
        break;
        case AeroDmsTypes::Icone_AJOUTER_AEROCLUB:
        {
            return QIcon(":/AeroDms/ressources/account-multiple-plus.svg");
        }
        break;
        case AeroDmsTypes::Icone_MAILING:
        {
            return QIcon(":/AeroDms/ressources/email-multiple.svg");
        }
        break;
        case AeroDmsTypes::Icone_IMPRIMER:
        {
            return QIcon(":/AeroDms/ressources/printer.svg");
        }
        break;
        case AeroDmsTypes::Icone_OUVRIR_DOSSIER:
        {
            return QIcon(":/AeroDms/ressources/folder-open.svg");
        }
        break;
        case AeroDmsTypes::Icone_FICHIER:
        {
            return QIcon(":/AeroDms/ressources/file.svg");
        }
        break;
        case AeroDmsTypes::Icone_FICHIER_SUIVANT:
        {
            return QIcon(":/AeroDms/ressources/skip-next.svg");
        }
        break;
        case AeroDmsTypes::Icone_FICHIER_PRECEDENT:
        {
            return QIcon(":/AeroDms/ressources/skip-previous.svg");
        }
        break;
        case AeroDmsTypes::Icone_SCAN_AUTO_FACTURE:
        {
            return QIcon(":/AeroDms/ressources/file-search.svg");
        }
        break;
        case AeroDmsTypes::Icone_SCAN_AUTO_FACTURE_GENERIQUE:
        {
            return QIcon(":/AeroDms/ressources/text-box-search.svg");
        }
        break;
        case AeroDmsTypes::Icone_SCAN_AUTO_VOL:
        {
            return QIcon(":/AeroDms/ressources/airplane-search.svg");
        }
        break;
        case AeroDmsTypes::Icone_TELECHARGER_CLOUD:
        {
            return QIcon(":/AeroDms/ressources/cloud-download.svg");
        }
        break;
        case AeroDmsTypes::Icone_TELECHARGER_DOSSIER:
        {
            return QIcon(":/AeroDms/ressources/folder-download.svg");
        }
        break;
        case AeroDmsTypes::Icone_TELECHARGER_FICHIER:
        {
            return QIcon(":/AeroDms/ressources/file-download.svg");
        }
        break;
        case AeroDmsTypes::Icone_FICHIER_TELECHARGE:
        {
            return QIcon(":/AeroDms/ressources/file-check.svg");
        }
        break;
        case AeroDmsTypes::Icone_CHIFFREMENT:
        {
            return QIcon(":/AeroDms/ressources/lock.svg");
        }
        break;
        case AeroDmsTypes::Icone_AJOUTER_NOTE:
        {
            return QIcon(":/AeroDms/ressources/note-edit.svg");
        }
        break;
        case AeroDmsTypes::Icone_RESSOURCE:
        {
            return QIcon(":/AeroDms/ressources/package-variant.svg");
        }
        break;
        case AeroDmsTypes::Icone_RESSOURCE_EXTERNE:
        {
            return QIcon(":/AeroDms/ressources/code-block-tags.svg");
        }
        break;
        case AeroDmsTypes::Icone_DEBUG:
        {
            return QIcon(":/AeroDms/ressources/bug.svg");
        }
        break;
        case AeroDmsTypes::Icone_CHOIX_ANNEE:
        {
            return QIcon(":/AeroDms/ressources/calendar-filter.svg");
        }
        break;
        case AeroDmsTypes::Icone_TOUTES_LES_ANNEES:
        {
            return QIcon(":/AeroDms/ressources/calendar-multiple.svg");
        }
        break;
        case AeroDmsTypes::Icone_UNE_ANNEE:
        {
            return QIcon(":/AeroDms/ressources/calendar.svg");
        }
        break;
        case AeroDmsTypes::Icone_INFOS_COMPLEMENTAIRES:
        {
            return QIcon(":/AeroDms/ressources/information-slab-circle.svg");
        }
        break;

        case AeroDmsTypes::Icone_RATIO:
        {
            return QIcon(":/AeroDms/ressources/aspect-ratio.svg");
        }
        break;

        case AeroDmsTypes::Icone_STATS_BARRES_EMPILEES:
        {
            return QIcon(":/AeroDms/ressources/chart-bar-stacked.svg");
        }
        break;
        case AeroDmsTypes::Icone_STATS_CAMEMBERT:
        {
            return QIcon(":/AeroDms/ressources/chart-pie.svg");
        }
        break;
        case AeroDmsTypes::Icone_STATS_DONUT:
        {
            return QIcon(":/AeroDms/ressources/chart-donut-variant.svg");
        }
        break;
        case AeroDmsTypes::Icone_STATS:
        {
            return QIcon(":/AeroDms/ressources/chart-areaspline.svg");
        }
        break;

        case AeroDmsTypes::Icone_A:
        case AeroDmsTypes::Icone_A_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-a.svg");
        }
        break;
        case AeroDmsTypes::Icone_B:
        case AeroDmsTypes::Icone_B_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-b.svg");
        }
        break;
        case AeroDmsTypes::Icone_C:
        case AeroDmsTypes::Icone_C_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-c.svg");
        }
        break;
        case AeroDmsTypes::Icone_D:
        case AeroDmsTypes::Icone_D_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-d.svg");
        }
        break;
        case AeroDmsTypes::Icone_E:
        case AeroDmsTypes::Icone_E_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-e.svg");
        }
        break;
        case AeroDmsTypes::Icone_F:
        case AeroDmsTypes::Icone_F_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-f.svg");
        }
        break;
        case AeroDmsTypes::Icone_G:
        case AeroDmsTypes::Icone_G_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-g.svg");
        }
        break;
        case AeroDmsTypes::Icone_H:
        case AeroDmsTypes::Icone_H_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-h.svg");
        }
        break;
        case AeroDmsTypes::Icone_I:
        case AeroDmsTypes::Icone_I_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-i.svg");
        }
        break;
        case AeroDmsTypes::Icone_J:
        case AeroDmsTypes::Icone_J_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-j.svg");
        }
        break;
        case AeroDmsTypes::Icone_K:
        case AeroDmsTypes::Icone_K_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-k.svg");
        }
        break;
        case AeroDmsTypes::Icone_L:
        case AeroDmsTypes::Icone_L_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-l.svg");
        }
        break;
        case AeroDmsTypes::Icone_M:
        case AeroDmsTypes::Icone_M_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-m.svg");
        }
        break;
        case AeroDmsTypes::Icone_N:
        case AeroDmsTypes::Icone_N_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-n.svg");
        }
        break;
        case AeroDmsTypes::Icone_O:
        case AeroDmsTypes::Icone_O_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-o.svg");
        }
        break;
        case AeroDmsTypes::Icone_P:
        case AeroDmsTypes::Icone_P_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-p.svg");
        }
        break;
        case AeroDmsTypes::Icone_Q:
        case AeroDmsTypes::Icone_Q_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-q.svg");
        }
        break;
        case AeroDmsTypes::Icone_R:
        case AeroDmsTypes::Icone_R_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-r.svg");
        }
        break;
        case AeroDmsTypes::Icone_S:
        case AeroDmsTypes::Icone_S_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-s.svg");
        }
        break;
        case AeroDmsTypes::Icone_T:
        case AeroDmsTypes::Icone_T_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-t.svg");
        }
        break;
        case AeroDmsTypes::Icone_U:
        case AeroDmsTypes::Icone_U_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-u.svg");
        }
        break;
        case AeroDmsTypes::Icone_V:
        case AeroDmsTypes::Icone_V_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-v.svg");
        }
        break;
        case AeroDmsTypes::Icone_W:
        case AeroDmsTypes::Icone_W_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-w.svg");
        }
        break;
        case AeroDmsTypes::Icone_X:
        case AeroDmsTypes::Icone_X_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-x.svg");
        }
        break;
        case AeroDmsTypes::Icone_Y:
        case AeroDmsTypes::Icone_Y_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-y.svg");
        }
        break;
        case AeroDmsTypes::Icone_Z:
        case AeroDmsTypes::Icone_Z_MINUSCULE:
        {
            return QIcon(":/AeroDms/ressources/alpha-z.svg");
        }
        break;
        case AeroDmsTypes::Icone_MOINS_1:
        {
            return QIcon(":/AeroDms/ressources/numeric-negative-1.svg");
        }
        break;
        case AeroDmsTypes::Icone_0:
        {
            return QIcon(":/AeroDms/ressources/numeric-0.svg");
        }
        break;
        case AeroDmsTypes::Icone_1:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-1.svg");
        }
        break;
        case AeroDmsTypes::Icone_2:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-2.svg");
        }
        break;
        case AeroDmsTypes::Icone_3:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-3.svg");
        }
        break;
        case AeroDmsTypes::Icone_4:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-4.svg");
        }
        break;
        case AeroDmsTypes::Icone_5:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-5.svg");
        }
        break;
        case AeroDmsTypes::Icone_6:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-6.svg");
        }
        break;
        case AeroDmsTypes::Icone_7:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-7.svg");
        }
        break;
        case AeroDmsTypes::Icone_8:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-8.svg");
        }
        break;
        case AeroDmsTypes::Icone_9:
        {
            return QIcon(":/AeroDms/ressources/numeric-positive-9.svg");
        }
        break;

        case AeroDmsTypes::Icone_ICONE_APPLICATION:
        {
            return QIcon(":/AeroDms/ressources/shield-airplane.svg");
        }
        break;
        case AeroDmsTypes::Icone_INCONNUE:
        default:
        {
            return QIcon(":/AeroDms/ressources/help.svg");
        }
    }
}

void AeroDmsServices::ajouterIconesComboBox(QComboBox& p_activite)
{
    for (int i = 0; i < p_activite.count(); i++)
    {
        p_activite.setItemIcon(i, recupererIcone(p_activite.itemText(i)));
    }
}

void AeroDmsServices::normaliser(QString & p_texte)
{
    p_texte.replace(QRegularExpression("[ ']"), "");

    p_texte.replace(QRegularExpression("[éèëê]"), "e");
    p_texte.replace(QRegularExpression("[ÉÈËÊ]"), "E");

    p_texte.replace(QRegularExpression("[aâä]"), "a");
    p_texte.replace(QRegularExpression("[AÂÄ]"), "A");

    p_texte.replace(QRegularExpression("[îï]"), "i");
    p_texte.replace(QRegularExpression("[ÎÏ]"), "I");

    p_texte.replace(QRegularExpression("[ôö]"), "o");
    p_texte.replace(QRegularExpression("[ÔÖ]"), "O");

    p_texte.replace(QRegularExpression("[ùûü]"), "u");
    p_texte.replace(QRegularExpression("[ÙÛÜ]"), "U");

    p_texte.replace(QRegularExpression("[ÿ]"), "y");
    p_texte.replace(QRegularExpression("[Ÿ]"), "Y");

    p_texte.replace(QRegularExpression("[ç]"), "c");
    p_texte.replace(QRegularExpression("[Ç]"), "C");

    p_texte.replace(QRegularExpression("[æ]"), "ae");
    p_texte.replace(QRegularExpression("[Æ]"), "AE");
    p_texte.replace(QRegularExpression("[œ]"), "oe");
    p_texte.replace(QRegularExpression("[Œ]"), "OE");
}

QString AeroDmsServices::capitaliserTexte(const QString& p_input)
{
    QLocale locale; // Utilise la locale par défaut
    QString result;
    bool capitalizeNext = true;

    for (int i = 0; i < p_input.length(); ++i) 
    {
        if (p_input[i].isSpace()
            || p_input[i] == "-") 
        {
            capitalizeNext = true;
            result.append(p_input[i]);
        }
        else if (capitalizeNext) 
        {
            result.append(locale.toUpper(p_input[i]));
            capitalizeNext = false;
        }
        else 
        {
            result.append(locale.toLower(p_input[i]));
        }
    }

    return result;
}

QString AeroDmsServices::chiffrerDonnees(QString& p_string) 
{
#ifdef Q_OS_WIN
    QByteArray data = p_string.toUtf8(); // Conversion en QByteArray
    DATA_BLOB inputBlob;
    DATA_BLOB outputBlob;
    inputBlob.pbData = reinterpret_cast<BYTE*>(data.data());
    inputBlob.cbData = data.size();

    if (CryptProtectData(&inputBlob, nullptr, nullptr, nullptr, nullptr, 0, &outputBlob)) {
        QByteArray donneesChiffrees(reinterpret_cast<char*>(outputBlob.pbData), outputBlob.cbData);
        LocalFree(outputBlob.pbData); // Libérer la mémoire allouée
        return QString::fromUtf8(donneesChiffrees.toBase64()); // Encodage en base64 pour le stockage
    }
#else
    #warning Attention : chiffrement des mots de passe non défini.Implémenter la méthode nécessaire dans AeroDmsServices::chiffrerDonnees
#endif

    // En cas d'échec
    return "";
}

QString AeroDmsServices::dechiffrerDonnees(QString& encryptedText)
{
#ifdef Q_OS_WIN
    QByteArray donneesChiffrees = QByteArray::fromBase64(encryptedText.toUtf8());
    DATA_BLOB inputBlob;
    DATA_BLOB outputBlob;
    inputBlob.pbData = reinterpret_cast<BYTE*>(donneesChiffrees.data());
    inputBlob.cbData = donneesChiffrees.size();

    if (CryptUnprotectData(&inputBlob, nullptr, nullptr, nullptr, nullptr, 0, &outputBlob)) {
        QByteArray donneesDechiffrees(reinterpret_cast<char*>(outputBlob.pbData), outputBlob.cbData);
        LocalFree(outputBlob.pbData); // Libérer la mémoire allouée
        return QString::fromUtf8(donneesDechiffrees); // Conversion en QString
    }
#else
#warning Attention : chiffrement des mots de passe non défini. Implémenter la méthode nécessaire dans AeroDmsServices::dechiffrerDonnees
#endif

    // En cas d'échec
    return "";
}

QString AeroDmsServices::recupererCheminFichierImageSignature()
{
    const QString cheminEtNomBaseSignature = QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + "/signature.";

    if (QFile(cheminEtNomBaseSignature + "svg").exists())
    {
        return cheminEtNomBaseSignature + "svg";
    }
    else if (QFile(cheminEtNomBaseSignature + "png").exists())
    {
        return cheminEtNomBaseSignature + "png";
    }
    else if (QFile(cheminEtNomBaseSignature + "jpg").exists())
    {
        return cheminEtNomBaseSignature + "jpg";
    }
    else if (QFile(cheminEtNomBaseSignature + "jpeg").exists())
    {
        return cheminEtNomBaseSignature + "jpeg";
    }
    return "";
}

void AeroDmsServices::calculerTailleQResources(AeroDmsTypes::TailleFichiers& p_tailleFichiers, const QString& prefix) 
{
    // Récupérer la liste des fichiers et répertoires dans le préfixe donné
    QDir resourceDir(":" + prefix);
    QFileInfoList entries = resourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& entry : entries)
    {
        QString resourcePath = prefix + entry.fileName();

        if (entry.isDir())
        {
            // Appeler récursivement les sous-répertoires
            calculerTailleQResources(p_tailleFichiers, resourcePath + "/");
        }
        else if (entry.isFile())
        {
            const QString extension = entry.fileName().split(".").last();
            // Créer un objet QResource pour accéder aux tailles
            QResource resource(resourcePath);

            const quint64 compresse = resource.size();
            const quint64 nonCompresse = resource.uncompressedSize();

            // Ajouter les tailles au total
            p_tailleFichiers.total.compresse += compresse;
            p_tailleFichiers.total.nonCompresse += nonCompresse;

            if (extension == "svg")
            {
                p_tailleFichiers.svg.compresse += compresse;
                p_tailleFichiers.svg.nonCompresse += nonCompresse;
            }
            else if (extension == "png"
                     || extension == "webp")
            {
                p_tailleFichiers.png.compresse += compresse;
                p_tailleFichiers.png.nonCompresse += nonCompresse;
            }
            else if (extension == "html"
                || extension == "htm")
            {
                p_tailleFichiers.html.compresse += compresse;
                p_tailleFichiers.html.nonCompresse += nonCompresse;
            }
            else
            {
                p_tailleFichiers.autres.compresse += compresse;
                p_tailleFichiers.autres.nonCompresse += nonCompresse;
            }
        }
    }
}

QFile AeroDmsServices::fichierDepuisQUrl(QUrl& p_url, QString p_nomFichier)
{
    QString filePath;
    if (p_url.scheme() == "qrc") 
    {
        filePath = ":" + p_url.path(); // Récupère ":/resources/example.txt"
    }
    else 
    {
        filePath = p_url.toLocalFile(); // Utilisé pour d'autres schémas (file:, etc.)
    }

    if (filePath.last(1) != "/")
    {
        filePath = filePath + "/";
    }
    filePath = filePath + p_nomFichier;

    return QFile(filePath);
}

QString AeroDmsServices::mergerPdf(const QString p_cheminRepertoireFichiersAMerger,
    const QString p_cheminFichierPdfDeSortie,
    const QStringList p_listeFichiersPdfAMerger,
    const QString p_nomTresorier,
    const QString p_titreDuFichuer,
    const QString p_sujetDuFichier)
{
    PoDoFo::PdfMemDocument document;

    for (int i = 0; i < p_listeFichiersPdfAMerger.size(); i++)
    {
        const QString nomFacture = p_cheminRepertoireFichiersAMerger
            + p_listeFichiersPdfAMerger.at(i);
        PoDoFo::PdfMemDocument facture;
        facture.Load(nomFacture.toStdString());
        document.GetPages().AppendDocumentPages(facture);
    }

    const QString appVersion = "AeroDMS v" + QApplication::applicationVersion();
    document.GetMetadata().SetCreator(PoDoFo::PdfString(appVersion.toStdString()));
    document.GetMetadata().SetAuthor(PoDoFo::PdfString(p_nomTresorier.toStdString()));
    document.GetMetadata().SetTitle(PoDoFo::PdfString(p_titreDuFichuer.toStdString()));
    document.GetMetadata().SetSubject(PoDoFo::PdfString(p_sujetDuFichier.toStdString()));
    document.Save(p_cheminFichierPdfDeSortie.toStdString());

    return p_cheminFichierPdfDeSortie;
}

void AeroDmsServices::afficherErreur(const QNetworkReply* p_reponse)
{
    const QString titreFenetre = QApplication::applicationName() + " - " + QObject::tr("Erreur réseau");

    switch (p_reponse->error())
    {
    case QNetworkReply::ConnectionRefusedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The remote server refused the connection (the server is not accepting "
            "requests)\n\nErreur remontée : QNetworkReply::ConnectionRefusedError ("
            + QString::number(QNetworkReply::ConnectionRefusedError) + ")");
    }
    break;
    case QNetworkReply::RemoteHostClosedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The remote server closed the connection prematurely, before the entire "
            "reply was received and processed\n\nErreur remontée : "
            "QNetworkReply::RemoteHostClosedError ("
            + QString::number(QNetworkReply::RemoteHostClosedError) + ")");
    }
    break;
    case QNetworkReply::HostNotFoundError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The remote host name was not found (invalid hostname)\n\nErreur remontée "
            ": QNetworkReply::HostNotFoundError ("
            + QString::number(QNetworkReply::HostNotFoundError) + ")");
    }
    break;
    case QNetworkReply::TimeoutError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The connection to the remote server timed out\n\nErreur remontée : "
            "QNetworkReply::TimeoutError ("
            + QString::number(QNetworkReply::TimeoutError) + ")");
    }
    break;
    case QNetworkReply::OperationCanceledError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The operation was canceled via calls to abort() or close() before it "
            "was finished.\n\nErreur remontée : QNetworkReply::OperationCanceledError ("
            + QString::number(QNetworkReply::OperationCanceledError) + ")");
    }
    break;
    case QNetworkReply::SslHandshakeFailedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The SSL/TLS handshake failed and the encrypted channel could not be "
            "established. The sslErrors() signal should have been emitted.\n Erreur "
            "remontée : QNetworkReply::SslHandshakeFailedError ("
            + QString::number(QNetworkReply::SslHandshakeFailedError) + ")");
    }
    break;
    case QNetworkReply::TemporaryNetworkFailureError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The connection was broken due to disconnection from the network, "
            "however the system has initiated roaming to another access point. The "
            "request should be resubmitted and will be processed as soon as the "
            "connection is re-established.\n\nErreur remontée : "
            "QNetworkReply::TemporaryNetworkFailureError ("
            + QString::number(QNetworkReply::TemporaryNetworkFailureError)
            + ")");
    }
    break;
    case QNetworkReply::NetworkSessionFailedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The connection was broken due to disconnection from the network or "
            "failure to start the network.\n\nErreur remontée : "
            "QNetworkReply::NetworkSessionFailedError ("
            + QString::number(QNetworkReply::NetworkSessionFailedError) + ")");
    }
    break;
    case QNetworkReply::BackgroundRequestNotAllowedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The background request is not currently allowed due to platform "
            "policy.\n\nErreur remontée : "
            "QNetworkReply::BackgroundRequestNotAllowedError ("
            + QString::number(QNetworkReply::BackgroundRequestNotAllowedError)
            + ")");
    }
    break;
    case QNetworkReply::TooManyRedirectsError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "While following redirects, the maximum limit was reached. The limit is "
            "by default set to 50 or as set by "
            "QNetworkRequest::setMaxRedirectsAllowed(). (This value was introduced "
            "in 5.6.)\n\nErreur remontée : QNetworkReply::TooManyRedirectsError ("
            + QString::number(QNetworkReply::TooManyRedirectsError) + ")");
    }
    break;
    case QNetworkReply::InsecureRedirectError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "While following redirects, the network access API detected a redirect "
            "from a encrypted protocol (https) to an unencrypted one (http). (This "
            "value was introduced in 5.6.)\n\nErreur remontée : "
            "QNetworkReply::InsecureRedirectError ("
            + QString::number(QNetworkReply::InsecureRedirectError) + ")");
    }
    break;
    case QNetworkReply::ProxyConnectionRefusedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The connection to the proxy server was refused (the proxy server is not "
            "accepting requests)\n\nErreur remontée : "
            "QNetworkReply::ProxyConnectionRefusedError ("
            + QString::number(QNetworkReply::ProxyConnectionRefusedError) + ")");
    }
    break;
    case QNetworkReply::ProxyConnectionClosedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The proxy server closed the connection prematurely, before the entire "
            "reply was received and processed\n\nErreur remontée : "
            "QNetworkReply::ProxyConnectionClosedError ("
            + QString::number(QNetworkReply::ProxyConnectionClosedError) + ")");
    }
    break;
    case QNetworkReply::ProxyNotFoundError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The proxy host name was not found (invalid proxy hostname)\n Erreur "
            "remontée : QNetworkReply::ProxyNotFoundError ("
            + QString::number(QNetworkReply::ProxyNotFoundError) + ")");
    }
    break;
    case QNetworkReply::ProxyTimeoutError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The connection to the proxy timed out or the proxy did not reply in "
            "time to the request sent\n\nErreur remontée : "
            "QNetworkReply::ProxyTimeoutError ("
            + QString::number(QNetworkReply::ProxyTimeoutError) + ")");
    }
    break;
    case QNetworkReply::ProxyAuthenticationRequiredError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The proxy requires authentication in order to honour the request but "
            "did not accept any credentials offered (if any)\n\nErreur remontée : "
            "QNetworkReply::ProxyAuthenticationRequiredError ("
            + QString::number(QNetworkReply::ProxyAuthenticationRequiredError)
            + ")");
    }
    break;
    case QNetworkReply::ContentAccessDenied:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The access to the remote content was denied (similar to HTTP error "
            "403)\n\nErreur remontée : QNetworkReply::ContentAccessDenied ("
            + QString::number(QNetworkReply::ContentAccessDenied) + ")");
    }
    break;
    case QNetworkReply::ContentOperationNotPermittedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The operation requested on the remote content is not permitted\n Erreur "
            "remontée : QNetworkReply::ContentOperationNotPermittedError ("
            + QString::number(QNetworkReply::ContentOperationNotPermittedError)
            + ")");
    }
    break;
    case QNetworkReply::ContentNotFoundError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The remote content was not found at the server (similar to HTTP error "
            "404)\n\nErreur remontée : QNetworkReply::ContentNotFoundError ("
            + QString::number(QNetworkReply::ContentNotFoundError) + ")");
    }
    break;
    case QNetworkReply::AuthenticationRequiredError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The remote server requires authentication to serve the content but the "
            "credentials provided were not accepted (if any)\n\nErreur remontée : "
            "QNetworkReply::AuthenticationRequiredError ("
            + QString::number(QNetworkReply::AuthenticationRequiredError) + ")");
    }
    break;
    case QNetworkReply::ContentReSendError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The request needed to be sent again, but this failed for example "
            "because the upload data could not be read a second time.\n Erreur "
            "remontée : QNetworkReply::ContentReSendError ("
            + QString::number(QNetworkReply::ContentReSendError) + ")");
    }
    break;
    case QNetworkReply::ContentConflictError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The request could not be completed due to a conflict with the current "
            "state of the resource.\n\nErreur remontée : "
            "QNetworkReply::ContentConflictError ("
            + QString::number(QNetworkReply::ContentConflictError) + ")");
    }
    break;
    case QNetworkReply::ContentGoneError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The requested resource is no longer available at the server.\n Erreur "
            "remontée : QNetworkReply::ContentGoneError ("
            + QString::number(QNetworkReply::ContentGoneError) + ")");
    }
    break;
    case QNetworkReply::InternalServerError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The server encountered an unexpected condition which prevented it from "
            "fulfilling the request.\n\nErreur remontée : "
            "QNetworkReply::InternalServerError ("
            + QString::number(QNetworkReply::InternalServerError) + ")");
    }
    break;
    case QNetworkReply::OperationNotImplementedError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The server does not support the functionality required to fulfill the "
            "request.\n\nErreur remontée : "
            "QNetworkReply::OperationNotImplementedError ("
            + QString::number(QNetworkReply::OperationNotImplementedError)
            + ")");
    }
    break;
    case QNetworkReply::ServiceUnavailableError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The server is unable to handle the request at this time.\n Erreur "
            "remontée : QNetworkReply::ServiceUnavailableError ("
            + QString::number(QNetworkReply::ServiceUnavailableError) + ")");
    }
    break;
    case QNetworkReply::ProtocolUnknownError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The Network Access API cannot honor the request because the protocol is "
            "not known\n\nErreur remontée : QNetworkReply::ProtocolUnknownError ("
            + QString::number(QNetworkReply::ProtocolUnknownError) + ")");
    }
    break;
    case QNetworkReply::ProtocolInvalidOperationError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "The requested operation is invalid for this protocol\n\nErreur remontée "
            ": QNetworkReply::ProtocolInvalidOperationError ("
            + QString::number(QNetworkReply::ProtocolInvalidOperationError)
            + ")");
    }
    break;
    case QNetworkReply::UnknownNetworkError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "An unknown network-related error was detected\n\nErreur remontée : "
            "QNetworkReply::UnknownNetworkError ("
            + QString::number(QNetworkReply::UnknownNetworkError) + ")");
    }
    break;
    case QNetworkReply::UnknownProxyError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "An unknown proxy-related error was detected\n\nErreur remontée : "
            "QNetworkReply::UnknownProxyError ("
            + QString::number(QNetworkReply::UnknownProxyError) + ")");
    }
    break;
    case QNetworkReply::UnknownContentError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "An unknown error related to the remote content was detected\n Erreur "
            "remontée : QNetworkReply::UnknownContentError ("
            + QString::number(QNetworkReply::UnknownContentError) + ")");
    }
    break;
    case QNetworkReply::ProtocolFailure:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "A breakdown in protocol was detected (parsing error, invalid or "
            "unexpected responses, etc.)\n\nErreur remontée : "
            "QNetworkReply::ProtocolFailure ("
            + QString::number(QNetworkReply::ProtocolFailure) + ")");
    }
    break;
    case QNetworkReply::UnknownServerError:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "An unknown error related to the server response was detected\n Erreur "
            "remontée : QNetworkReply::UnknownServerError ("
            + QString::number(QNetworkReply::UnknownServerError) + ")");
    }
    break;
    default:
    {
        QMessageBox::critical(
            nullptr, titreFenetre,
            p_reponse->errorString() + "\n" +
            "Erreur indéterminée\n"
            "Code d'erreur remonté : "
            + QString::number(p_reponse->error()));
    }
    break;
    }
}
