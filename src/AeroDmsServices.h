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

#ifndef AERODMSSERVICES_H
#define AERODMSSERVICES_H

#include <QtWidgets>

class AeroDmsServices
{
public:
    enum Icone
    {
        Icone_0 = '0',
        Icone_1 = '1',
        Icone_2 = '2',
        Icone_3 = '3',
        Icone_4 = '4',
        Icone_5 = '5',
        Icone_6 = '6',
        Icone_7 = '7',
        Icone_8 = '8',
        Icone_9 = '9',

        Icone_A = 'A',
        Icone_B = 'B',
        Icone_C = 'C',
        Icone_D = 'D',
        Icone_E = 'E',
        Icone_F = 'F',
        Icone_G = 'G',
        Icone_H = 'H',
        Icone_I = 'I',
        Icone_J = 'J',
        Icone_K = 'K',
        Icone_L = 'L',
        Icone_M = 'M',
        Icone_N = 'N',
        Icone_O = 'O',
        Icone_P = 'P',
        Icone_Q = 'Q',
        Icone_R = 'R',
        Icone_S = 'S',
        Icone_T = 'T',
        Icone_U = 'U',
        Icone_V = 'V',
        Icone_W = 'W',
        Icone_X = 'X',
        Icone_Y = 'Y',
        Icone_Z = 'Z',

        Icone_A_MINUSCULE = 'a',
        Icone_B_MINUSCULE = 'b',
        Icone_C_MINUSCULE = 'c',
        Icone_D_MINUSCULE = 'd',
        Icone_E_MINUSCULE = 'e',
        Icone_F_MINUSCULE = 'f',
        Icone_G_MINUSCULE = 'g',
        Icone_H_MINUSCULE = 'h',
        Icone_I_MINUSCULE = 'i',
        Icone_J_MINUSCULE = 'j',
        Icone_K_MINUSCULE = 'k',
        Icone_L_MINUSCULE = 'l',
        Icone_M_MINUSCULE = 'm',
        Icone_N_MINUSCULE = 'n',
        Icone_O_MINUSCULE = 'o',
        Icone_P_MINUSCULE = 'p',
        Icone_Q_MINUSCULE = 'q',
        Icone_R_MINUSCULE = 'r',
        Icone_S_MINUSCULE = 's',
        Icone_T_MINUSCULE = 't',
        Icone_U_MINUSCULE = 'u',
        Icone_V_MINUSCULE = 'v',
        Icone_W_MINUSCULE = 'w',
        Icone_X_MINUSCULE = 'x',
        Icone_Y_MINUSCULE = 'y',
        Icone_Z_MINUSCULE = 'z',

        Icone_ULM,
        Icone_AVION,
        Icone_AVION_ELECTRIQUE,
        Icone_PLANEUR,
        Icone_HELICOPTERE,

        Icone_ENTRAINEMENT,
        Icone_SORTIE,
        Icone_BALADE,
        Icone_TOTAL,

        Icone_PILOTE,
        Icone_COTISATION,
        Icone_RECETTE,
        Icone_DEPENSE,
        Icone_FACTURE,
        Icone_FINANCIER,

        Icone_GENERE_DEMANDE_SUBVENTIONS,
        Icone_GENERE_RECAP_HDV,
        Icone_AJOUTER_VOL,
        Icone_MAILING,
        Icone_IMPRIMER,
        Icone_OUVRIR_DOSSIER,
        Icone_FICHIER,
        Icone_SCAN_AUTO_FACTURE,
        Icone_SCAN_AUTO_FACTURE_GENERIQUE,
        Icone_SCAN_AUTO_VOL,
        Icone_DEBUG,

        Icone_RATIO,

        Icone_STATS_BARRES_EMPILEES,
        Icone_STATS_CAMEMBERT,
        Icone_STATS_DONUT,
        Icone_STATS,

        Icone_OUI,
        Icone_NON,
        Icone_TOUS,
        Icone_TOUT_COCHER,
        Icone_MOINS_1,

        Icone_ICONE_APPLICATION,

        Icone_INCONNUE
    };

    static const QString convertirMinutesEnHeuresMinutes(const int p_minutes);
    static const QTime convertirMinutesEnQTime(const int p_minutes);
    static const QTime convertirHeuresDecimalesEnQTime(const double p_heureDecimale);
    static QIcon recupererIcone(const QString& p_icone);
    static QIcon recupererIcone(const QChar p_caractere);
    static QIcon recupererIcone(const Icone p_icone);
    static void ajouterIconesComboBox(QComboBox &p_activite);
    static void normaliser(QString &p_texte);
};

#endif // AERODMSSERVICES_H
