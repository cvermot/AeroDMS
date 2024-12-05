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

#include "AeroDmsServices.h"

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
        return recupererIcone(Icone_ULM);
    }
    else if (p_icone == "Avion")
    {
        return recupererIcone(Icone_AVION);
    }
    else if (p_icone == "Planeur")
    {
        return recupererIcone(Icone_PLANEUR);
    }
    else if (p_icone == "Hélicoptère")
    {
        return recupererIcone(Icone_HELICOPTERE);
    }
    else if (p_icone == "Avion électrique")
    {
        return recupererIcone(Icone_AVION_ELECTRIQUE);
    }
    else if (p_icone == "Balade")
    {
        return recupererIcone(Icone_BALADE);
    }
    else if (p_icone == "Sortie")
    {
        return recupererIcone(Icone_SORTIE);
    }
    else if (p_icone == "Entrainement")
    {
        return recupererIcone(Icone_ENTRAINEMENT);
    }
    else if (p_icone == "Oui")
    {
        return recupererIcone(Icone_OUI);
    }
    else if (p_icone == "Non")
    {
        return recupererIcone(Icone_NON);
    }  
    else
    {
        return recupererIcone(Icone_INCONNUE);
    }
}

const QIcon AeroDmsServices::recupererIcone(const QChar p_caractere)
{
    return recupererIcone(AeroDmsServices::Icone(p_caractere.toLatin1()));
}

const QIcon AeroDmsServices::recupererIcone(const Icone p_icone)
{
    switch (p_icone)
    {
    case AeroDmsServices::Icone_ULM:
    {
        return QIcon(":/AeroDms/ressources/feather.svg");
    }
    break;
    case AeroDmsServices::Icone_AVION:
    {
        return QIcon(":/AeroDms/ressources/airplane.svg");
    }
    break;
    case AeroDmsServices::Icone_PLANEUR:
    {
        return QIcon(":/AeroDms/ressources/airplane-landing.svg");
    }
    break;
    case AeroDmsServices::Icone_HELICOPTERE:
    {
        return QIcon(":/AeroDms/ressources/helicopter.svg");
    }
    break;
    case AeroDmsServices::Icone_AVION_ELECTRIQUE:
    {
        return QIcon(":/AeroDms/ressources/lightning-bolt.svg");
    }
    break;
    case AeroDmsServices::Icone_BALADE:
    {
        return QIcon(":/AeroDms/ressources/account-group.svg");
    }
    break;
    case AeroDmsServices::Icone_SORTIE:
    {
        return QIcon(":/AeroDms/ressources/bag-checked.svg");
    }
    break;
    case AeroDmsServices::Icone_ENTRAINEMENT:
    {
        return QIcon(":/AeroDms/ressources/airport.svg");
    }
    break;
    case AeroDmsServices::Icone_TOTAL:
    {
        return QIcon(":/AeroDms/ressources/sigma.svg");
    }
    break;
    case AeroDmsServices::Icone_OUI:
    {
        return QIcon(":/AeroDms/ressources/check.svg");
    }
    break;
    case AeroDmsServices::Icone_NON:
    {
        return QIcon(":/AeroDms/ressources/close.svg");
    }
    break;
    case AeroDmsServices::Icone_TOUT_COCHER:
    {
        return QIcon(":/AeroDms/ressources/check-all.svg");
    }
    break;
    case AeroDmsServices::Icone_COTISATION:
    {
        return QIcon(":/AeroDms/ressources/ticket.svg");
    }
    break;
    case AeroDmsServices::Icone_RECETTE:
    {
        return QIcon(":/AeroDms/ressources/file-document-plus.svg");
    }
    break;
    case AeroDmsServices::Icone_DEPENSE:
    {
        return QIcon(":/AeroDms/ressources/file-document-minus.svg");
    }
    break;
    case AeroDmsServices::Icone_FACTURE:
    {
        return QIcon(":/AeroDms/ressources/file-document.svg");
    }
    break;
    case AeroDmsServices::Icone_FINANCIER:
    {
        return QIcon(":/AeroDms/ressources/cash-multiple.svg");
    }
    break;
    case AeroDmsServices::Icone_TOUS:
    {
        return QIcon(":/AeroDms/ressources/all-inclusive.svg");
    }
    break;
    case AeroDmsServices::Icone_PILOTE:
    {
        return QIcon(":/AeroDms/ressources/account-tie-hat.svg");
    }
    break;
    case AeroDmsServices::Icone_GENERE_DEMANDE_SUBVENTIONS:
    {
        return QIcon(":/AeroDms/ressources/file-cog.svg");
    }
    break;
    case AeroDmsServices::Icone_GENERE_RECAP_HDV:
    {
        return QIcon(":/AeroDms/ressources/account-file-text.svg");
    }
    break;
    case AeroDmsServices::Icone_AJOUTER_VOL:
    {
        return QIcon(":/AeroDms/ressources/airplane-plus.svg");
    }
    break;
    case AeroDmsServices::Icone_MAILING:
    {
        return QIcon(":/AeroDms/ressources/email-multiple.svg");
    }
    break;
    case AeroDmsServices::Icone_IMPRIMER:
    {
        return QIcon(":/AeroDms/ressources/printer.svg");
    }
    break;
    case AeroDmsServices::Icone_OUVRIR_DOSSIER:
    {
        return QIcon(":/AeroDms/ressources/folder-open.svg");
    }
    break;
    case AeroDmsServices::Icone_FICHIER:
    {
        return QIcon(":/AeroDms/ressources/file.svg");
    }
    break;
    case AeroDmsServices::Icone_FICHIER_SUIVANT:
    {
        return QIcon(":/AeroDms/ressources/skip-next.svg");
    }
    break;
    case AeroDmsServices::Icone_FICHIER_PRECEDENT:
    {
        return QIcon(":/AeroDms/ressources/skip-previous.svg");
    }
    break;
    case AeroDmsServices::Icone_SCAN_AUTO_FACTURE:
    {
        return QIcon(":/AeroDms/ressources/file-search.svg");
    }
    break;
    case AeroDmsServices::Icone_SCAN_AUTO_FACTURE_GENERIQUE:
    {
        return QIcon(":/AeroDms/ressources/text-box-search.svg");
    }
    break;
    case AeroDmsServices::Icone_SCAN_AUTO_VOL:
    {
        return QIcon(":/AeroDms/ressources/airplane-search.svg");
    }
    break;
    case AeroDmsServices::Icone_TELECHARGER_CLOUD:
    {
        return QIcon(":/AeroDms/ressources/cloud-download.svg");
    }
    break;
    case AeroDmsServices::Icone_TELECHARGER_DOSSIER:
    {
        return QIcon(":/AeroDms/ressources/folder-download.svg");
    }
    break;
    case AeroDmsServices::Icone_TELECHARGER_FICHIER:
    {
        return QIcon(":/AeroDms/ressources/file-download.svg");
    }
    break;
    case AeroDmsServices::Icone_FICHIER_TELECHARGE:
    {
        return QIcon(":/AeroDms/ressources/file-check.svg");
    }
    break;
    case AeroDmsServices::Icone_CHIFFREMENT:
    {
        return QIcon(":/AeroDms/ressources/lock.svg");
    }
    break;
    case AeroDmsServices::Icone_DEBUG:
    {
        return QIcon(":/AeroDms/ressources/bug.svg");
    }
    break;

    case AeroDmsServices::Icone_RATIO:
    {
        return QIcon(":/AeroDms/ressources/aspect-ratio.svg");
    }
    break;

    case AeroDmsServices::Icone_STATS_BARRES_EMPILEES:
    {
        return QIcon(":/AeroDms/ressources/chart-bar-stacked.svg");
    }
    break;
    case AeroDmsServices::Icone_STATS_CAMEMBERT:
    {
        return QIcon(":/AeroDms/ressources/chart-pie.svg");
    }
    break;
    case AeroDmsServices::Icone_STATS_DONUT:
    {
        return QIcon(":/AeroDms/ressources/chart-donut-variant.svg");
    }
    break;
    case AeroDmsServices::Icone_STATS:
    {
        return QIcon(":/AeroDms/ressources/chart-areaspline.svg");
    }
    break;

    case AeroDmsServices::Icone_A:
    case AeroDmsServices::Icone_A_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-a.svg");
    }
    break;
    case AeroDmsServices::Icone_B:
    case AeroDmsServices::Icone_B_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-b.svg");
    }
    break;
    case AeroDmsServices::Icone_C:
    case AeroDmsServices::Icone_C_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-c.svg");
    }
    break;
    case AeroDmsServices::Icone_D:
    case AeroDmsServices::Icone_D_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-d.svg");
    }
    break;
    case AeroDmsServices::Icone_E:
    case AeroDmsServices::Icone_E_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-e.svg");
    }
    break;
    case AeroDmsServices::Icone_F:
    case AeroDmsServices::Icone_F_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-f.svg");
    }
    break;
    case AeroDmsServices::Icone_G:
    case AeroDmsServices::Icone_G_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-g.svg");
    }
    break;
    case AeroDmsServices::Icone_H:
    case AeroDmsServices::Icone_H_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-h.svg");
    }
    break;
    case AeroDmsServices::Icone_I:
    case AeroDmsServices::Icone_I_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-i.svg");
    }
    break;
    case AeroDmsServices::Icone_J:
    case AeroDmsServices::Icone_J_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-j.svg");
    }
    break;
    case AeroDmsServices::Icone_K:
    case AeroDmsServices::Icone_K_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-k.svg");
    }
    break;
    case AeroDmsServices::Icone_L:
    case AeroDmsServices::Icone_L_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-l.svg");
    }
    break;
    case AeroDmsServices::Icone_M:
    case AeroDmsServices::Icone_M_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-m.svg");
    }
    break;
    case AeroDmsServices::Icone_N:
    case AeroDmsServices::Icone_N_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-n.svg");
    }
    break;
    case AeroDmsServices::Icone_O:
    case AeroDmsServices::Icone_O_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-o.svg");
    }
    break;
    case AeroDmsServices::Icone_P:
    case AeroDmsServices::Icone_P_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-p.svg");
    }
    break;
    case AeroDmsServices::Icone_Q:
    case AeroDmsServices::Icone_Q_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-q.svg");
    }
    break;
    case AeroDmsServices::Icone_R:
    case AeroDmsServices::Icone_R_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-r.svg");
    }
    break;
    case AeroDmsServices::Icone_S:
    case AeroDmsServices::Icone_S_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-s.svg");
    }
    break;
    case AeroDmsServices::Icone_T:
    case AeroDmsServices::Icone_T_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-t.svg");
    }
    break;
    case AeroDmsServices::Icone_U:
    case AeroDmsServices::Icone_U_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-u.svg");
    }
    break;
    case AeroDmsServices::Icone_V:
    case AeroDmsServices::Icone_V_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-v.svg");
    }
    break;
    case AeroDmsServices::Icone_W:
    case AeroDmsServices::Icone_W_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-w.svg");
    }
    break;
    case AeroDmsServices::Icone_X:
    case AeroDmsServices::Icone_X_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-x.svg");
    }
    break;
    case AeroDmsServices::Icone_Y:
    case AeroDmsServices::Icone_Y_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-y.svg");
    }
    break;
    case AeroDmsServices::Icone_Z:
    case AeroDmsServices::Icone_Z_MINUSCULE:
    {
        return QIcon(":/AeroDms/ressources/alpha-z.svg");
    }
    break;
    case AeroDmsServices::Icone_MOINS_1:
    {
        return QIcon(":/AeroDms/ressources/numeric-negative-1.svg");
    }
    break;
    case AeroDmsServices::Icone_0:
    {
        return QIcon(":/AeroDms/ressources/numeric-0.svg");
    }
    break;
    case AeroDmsServices::Icone_1:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-1.svg");
    }
    break;
    case AeroDmsServices::Icone_2:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-2.svg");
    }
    break;
    case AeroDmsServices::Icone_3:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-3.svg");
    }
    break;
    case AeroDmsServices::Icone_4:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-4.svg");
    }
    break;
    case AeroDmsServices::Icone_5:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-5.svg");
    }
    break;
    case AeroDmsServices::Icone_6:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-6.svg");
    }
    break;
    case AeroDmsServices::Icone_7:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-7.svg");
    }
    break;
    case AeroDmsServices::Icone_8:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-8.svg");
    }
    break;
    case AeroDmsServices::Icone_9:
    {
        return QIcon(":/AeroDms/ressources/numeric-positive-9.svg");
    }
    break;

    case AeroDmsServices::Icone_ICONE_APPLICATION:
    {
        return QIcon(":/AeroDms/ressources/shield-airplane.svg");
    }
    break;
    case Icone_INCONNUE:
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

