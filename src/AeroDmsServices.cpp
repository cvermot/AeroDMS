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

QIcon AeroDmsServices::recupererIcone(const QString& p_icone)
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

QIcon AeroDmsServices::recupererIcone(const QChar p_caractere)
{
    return recupererIcone(AeroDmsServices::Icone(p_caractere.toLatin1()));
}

QIcon AeroDmsServices::recupererIcone(const Icone p_icone)
{
    switch (p_icone)
    {
    case AeroDmsServices::Icone_ULM:
    {
        return QIcon("./ressources/feather.svg");
    }
    break;
    case AeroDmsServices::Icone_AVION:
    {
        return QIcon("./ressources/airplane.svg");
    }
    break;
    case AeroDmsServices::Icone_PLANEUR:
    {
        return QIcon("./ressources/airplane-landing.svg");
    }
    break;
    case AeroDmsServices::Icone_HELICOPTERE:
    {
        return QIcon("./ressources/helicopter.svg");
    }
    break;
    case AeroDmsServices::Icone_AVION_ELECTRIQUE:
    {
        return QIcon("./ressources/lightning-bolt.svg");
    }
    break;
    case AeroDmsServices::Icone_BALADE:
    {
        return QIcon("./ressources/account-group.svg");
    }
    break;
    case AeroDmsServices::Icone_SORTIE:
    {
        return QIcon("./ressources/bag-checked.svg");
    }
    break;
    case AeroDmsServices::Icone_ENTRAINEMENT:
    {
        return QIcon("./ressources/airport.svg");
    }
    break;
    case AeroDmsServices::Icone_TOTAL:
    {
        return QIcon("./ressources/sigma.svg");
    }
    break;
    case AeroDmsServices::Icone_OUI:
    {
        return QIcon("./ressources/check.svg");
    }
    break;
    case AeroDmsServices::Icone_NON:
    {
        return QIcon("./ressources/close.svg");
    }
    break;
    case AeroDmsServices::Icone_TOUT_COCHER:
    {
        return QIcon("./ressources/check-all.svg");
    }
    break;
    case AeroDmsServices::Icone_COTISATION:
    {
        return QIcon("./ressources/ticket.svg");
    }
    break;
    case AeroDmsServices::Icone_RECETTE:
    {
        return QIcon("./ressources/file-document-plus.svg");
    }
    break;
    case AeroDmsServices::Icone_DEPENSE:
    {
        return QIcon("./ressources/file-document-minus.svg");
    }
    break;
    case AeroDmsServices::Icone_FACTURE:
    {
        return QIcon("./ressources/file-document.svg");
    }
    break;
    case AeroDmsServices::Icone_FINANCIER:
    {
        return QIcon("./ressources/cash-multiple.svg");
    }
    break;
    case AeroDmsServices::Icone_TOUS:
    {
        return QIcon("./ressources/all-inclusive.svg");
    }
    break;
    case AeroDmsServices::Icone_PILOTE:
    {
        return QIcon("./ressources/account-tie-hat.svg");
    }
    break;
    case AeroDmsServices::Icone_GENERE_DEMANDE_SUBVENTIONS:
    {
        return QIcon("./ressources/file-cog.svg");
    }
    break;
    case AeroDmsServices::Icone_GENERE_RECAP_HDV:
    {
        return QIcon("./ressources/account-file-text.svg");
    }
    break;
    case AeroDmsServices::Icone_AJOUTER_VOL:
    {
        return QIcon("./ressources/airplane-plus.svg");
    }
    break;
    case AeroDmsServices::Icone_MAILING:
    {
        return QIcon("./ressources/email-multiple.svg");
    }
    break;
    case AeroDmsServices::Icone_IMPRIMER:
    {
        return QIcon("./ressources/printer.svg");
    }
    break;
    case AeroDmsServices::Icone_OUVRIR_DOSSIER:
    {
        return QIcon("./ressources/folder-open.svg");
    }
    break;
    case AeroDmsServices::Icone_FICHIER:
    {
        return QIcon("./ressources/file.svg");
    }
    break;
    case AeroDmsServices::Icone_SCAN_AUTO_FACTURE:
    {
        return QIcon("./ressources/file-search.svg");
    }
    break;
    case AeroDmsServices::Icone_SCAN_AUTO_FACTURE_GENERIQUE:
    {
        return QIcon("./ressources/text-box-search.svg");
    }
    break;
    case AeroDmsServices::Icone_SCAN_AUTO_VOL:
    {
        return QIcon("./ressources/airplane-search.svg");
    }
    break;
    case AeroDmsServices::Icone_DEBUG:
    {
        return QIcon("./ressources/bug.svg");
    }
    break;

    case AeroDmsServices::Icone_STATS_BARRES_EMPILEES:
    {
        return QIcon("./ressources/chart-bar-stacked.svg");
    }
    break;
    case AeroDmsServices::Icone_STATS_CAMEMBERT:
    {
        return QIcon("./ressources/chart-pie.svg");
    }
    break;
    case AeroDmsServices::Icone_STATS_DONUT:
    {
        return QIcon("./ressources/chart-donut-variant.svg");
    }
    break;
    case AeroDmsServices::Icone_STATS:
    {
        return QIcon("./ressources/chart-areaspline.svg");
    }
    break;

    case AeroDmsServices::Icone_A:
    case AeroDmsServices::Icone_A_MINUSCULE:
    {
        return QIcon("./ressources/alpha-a.svg");
    }
    break;
    case AeroDmsServices::Icone_B:
    case AeroDmsServices::Icone_B_MINUSCULE:
    {
        return QIcon("./ressources/alpha-b.svg");
    }
    break;
    case AeroDmsServices::Icone_C:
    case AeroDmsServices::Icone_C_MINUSCULE:
    {
        return QIcon("./ressources/alpha-c.svg");
    }
    break;
    case AeroDmsServices::Icone_D:
    case AeroDmsServices::Icone_D_MINUSCULE:
    {
        return QIcon("./ressources/alpha-d.svg");
    }
    break;
    case AeroDmsServices::Icone_E:
    case AeroDmsServices::Icone_E_MINUSCULE:
    {
        return QIcon("./ressources/alpha-e.svg");
    }
    break;
    case AeroDmsServices::Icone_F:
    case AeroDmsServices::Icone_F_MINUSCULE:
    {
        return QIcon("./ressources/alpha-f.svg");
    }
    break;
    case AeroDmsServices::Icone_G:
    case AeroDmsServices::Icone_G_MINUSCULE:
    {
        return QIcon("./ressources/alpha-g.svg");
    }
    break;
    case AeroDmsServices::Icone_H:
    case AeroDmsServices::Icone_H_MINUSCULE:
    {
        return QIcon("./ressources/alpha-h.svg");
    }
    break;
    case AeroDmsServices::Icone_I:
    case AeroDmsServices::Icone_I_MINUSCULE:
    {
        return QIcon("./ressources/alpha-i.svg");
    }
    break;
    case AeroDmsServices::Icone_J:
    case AeroDmsServices::Icone_J_MINUSCULE:
    {
        return QIcon("./ressources/alpha-j.svg");
    }
    break;
    case AeroDmsServices::Icone_K:
    case AeroDmsServices::Icone_K_MINUSCULE:
    {
        return QIcon("./ressources/alpha-k.svg");
    }
    break;
    case AeroDmsServices::Icone_L:
    case AeroDmsServices::Icone_L_MINUSCULE:
    {
        return QIcon("./ressources/alpha-l.svg");
    }
    break;
    case AeroDmsServices::Icone_M:
    case AeroDmsServices::Icone_M_MINUSCULE:
    {
        return QIcon("./ressources/alpha-m.svg");
    }
    break;
    case AeroDmsServices::Icone_N:
    case AeroDmsServices::Icone_N_MINUSCULE:
    {
        return QIcon("./ressources/alpha-n.svg");
    }
    break;
    case AeroDmsServices::Icone_O:
    case AeroDmsServices::Icone_O_MINUSCULE:
    {
        return QIcon("./ressources/alpha-o.svg");
    }
    break;
    case AeroDmsServices::Icone_P:
    case AeroDmsServices::Icone_P_MINUSCULE:
    {
        return QIcon("./ressources/alpha-p.svg");
    }
    break;
    case AeroDmsServices::Icone_Q:
    case AeroDmsServices::Icone_Q_MINUSCULE:
    {
        return QIcon("./ressources/alpha-q.svg");
    }
    break;
    case AeroDmsServices::Icone_R:
    case AeroDmsServices::Icone_R_MINUSCULE:
    {
        return QIcon("./ressources/alpha-r.svg");
    }
    break;
    case AeroDmsServices::Icone_S:
    case AeroDmsServices::Icone_S_MINUSCULE:
    {
        return QIcon("./ressources/alpha-s.svg");
    }
    break;
    case AeroDmsServices::Icone_T:
    case AeroDmsServices::Icone_T_MINUSCULE:
    {
        return QIcon("./ressources/alpha-t.svg");
    }
    break;
    case AeroDmsServices::Icone_U:
    case AeroDmsServices::Icone_U_MINUSCULE:
    {
        return QIcon("./ressources/alpha-u.svg");
    }
    break;
    case AeroDmsServices::Icone_V:
    case AeroDmsServices::Icone_V_MINUSCULE:
    {
        return QIcon("./ressources/alpha-v.svg");
    }
    break;
    case AeroDmsServices::Icone_W:
    case AeroDmsServices::Icone_W_MINUSCULE:
    {
        return QIcon("./ressources/alpha-w.svg");
    }
    break;
    case AeroDmsServices::Icone_X:
    case AeroDmsServices::Icone_X_MINUSCULE:
    {
        return QIcon("./ressources/alpha-x.svg");
    }
    break;
    case AeroDmsServices::Icone_Y:
    case AeroDmsServices::Icone_Y_MINUSCULE:
    {
        return QIcon("./ressources/alpha-y.svg");
    }
    break;
    case AeroDmsServices::Icone_Z:
    case AeroDmsServices::Icone_Z_MINUSCULE:
    {
        return QIcon("./ressources/alpha-z.svg");
    }
    break;
    case AeroDmsServices::Icone_MOINS_1:
    {
        return QIcon("./ressources/numeric-negative-1.svg");
    }
    break;
    case AeroDmsServices::Icone_0:
    {
        return QIcon("./ressources/numeric-0.svg");
    }
    break;
    case AeroDmsServices::Icone_1:
    {
        return QIcon("./ressources/numeric-positive-1.svg");
    }
    break;
    case AeroDmsServices::Icone_2:
    {
        return QIcon("./ressources/numeric-positive-2.svg");
    }
    break;
    case AeroDmsServices::Icone_3:
    {
        return QIcon("./ressources/numeric-positive-3.svg");
    }
    break;
    case AeroDmsServices::Icone_4:
    {
        return QIcon("./ressources/numeric-positive-4.svg");
    }
    break;
    case AeroDmsServices::Icone_5:
    {
        return QIcon("./ressources/numeric-positive-5.svg");
    }
    break;
    case AeroDmsServices::Icone_6:
    {
        return QIcon("./ressources/numeric-positive-6.svg");
    }
    break;
    case AeroDmsServices::Icone_7:
    {
        return QIcon("./ressources/numeric-positive-7.svg");
    }
    break;
    case AeroDmsServices::Icone_8:
    {
        return QIcon("./ressources/numeric-positive-8.svg");
    }
    break;
    case AeroDmsServices::Icone_9:
    {
        return QIcon("./ressources/numeric-positive-9.svg");
    }
    break;

    case AeroDmsServices::Icone_ICONE_APPLICATION:
    {
        return QIcon("./ressources/shield-airplane.svg");
    }
    break;
    case Icone_INCONNUE:
    default:
    {
        return QIcon("./ressources/help.svg");
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
