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
#include "AeroDmsTypes.h"

const AeroDmsTypes::TotauxRecettes AeroDmsTypes::K_INIT_TOTAUX_RECETTE = { 0.0, 0.0, 0.0 };

const AeroDmsTypes::HeureDeVolRemboursement AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT = { 0.0, 
                                                                                                0.0, 
                                                                                                "0h00",
                                                                                                0 };

const AeroDmsTypes::SubventionsParPilote AeroDmsTypes::K_INIT_SUBVENTION_PAR_PILOTE = { "", 0, "", "", "", 0.0,
																						AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT ,
																						AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT ,
																						AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT ,
																						AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT };
const AeroDmsTypes::Vol AeroDmsTypes::K_INIT_VOL = { "",//QString idPilote;
                                                     QDate() ,//QDate date;
                                                     "",//QString nomPilote;
                                                     "",//QString prenomPilote;
                                                     "0h00",//QString typeDeVol;
                                                     "",//QString duree;
                                                     "",//QString remarque;
                                                     "",//QString immat;
                                                     "",//QString activite;
                                                     "",//QString estSoumisCe;
                                                     false,//bool estSoumis
                                                     0.0,//float coutVol;
                                                     0.0,//float montantRembourse;
                                                     0,//int volId;
                                                     0,//int dureeEnMinutes;
                                                     -1,//int baladeId;
                                                     -1//int facture
};

const AeroDmsTypes::DonneesFacture AeroDmsTypes::K_INIT_DONNEES_FACTURE = { QDate(),
                                                                            QTime(),
                                                                            0.0,
                                                                            QString(),
                                                                            0
};

const AeroDmsTypes::Pilote AeroDmsTypes::K_INIT_PILOTE = { "", //QString idPilote;
                                                           "", //QString nom;
                                                           "", //QString prenom;
                                                           "", //QString aeroclub;
                                                           0, // int idAeroclub;
                                                           "", //QString activitePrincipale;
                                                           false, //bool estAyantDroit;
                                                           "", //QString mail;
                                                           "", //QString telephone;
                                                           "", //QString remarque;
                                                           false, //bool estActif;
                                                           false //bool estPiloteBrevete;
};

const AeroDmsTypes::Club AeroDmsTypes::K_INIT_CLUB = { 0, //int idAeroclub;
                                                       "", //QString aeroclub;
                                                       "", //QString raisonSociale;
                                                       "", //QString iban;
                                                       "", //QString bic;
};

const AeroDmsTypes::StatsPilotes AeroDmsTypes::K_INIT_DONNEES_STATS_PILOTES =
{
    0,
    0,
    0,
    0
};

const AeroDmsTypes::StatsAeronef AeroDmsTypes::K_INIT_STAT_AERONEF = 
{
    "",
    "",
    0
};

const AeroDmsTypes::DetailsBaladesEtSorties AeroDmsTypes::K_INIT_DETAILS_BALADES_ET_SORTIES =
{
    -1,
    -1,
    -1,
    QDate(),
    0,
    0.0,
    0.0,
    "",
    "",
    0.0,
    ""
};

const AeroDmsTypes::ResolutionEtParametresStatistiques AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES =
{
    QSize(800,600),
    10
};

const AeroDmsTypes::StatsHeuresDeVolParActivite AeroDmsTypes::K_INIT_STATS_HEURES_DE_VOL_PAR_ACTIVITES =
{
    "" ,//QString piloteId;
    "",//QString nomPrenomPilote;

    0,//int minutesVolAvion;
    0,//int minutesVolAvionElectrique;
    0,//int minutesVolUlm;
    0,//int minutesVolPlaneur;
    0,//int minutesVolHelicoptere;

    0.0,//float subventionVolAvion;
    0.0,//float subventionVolAvionElectrique;
    0.0,//float subventionVolUlm;
    0.0,//float subventionVolPlaneur;
    0.0,//float subventionVolHelicoptere;

    0.0,//float coutVolAvion;
    0.0,//float coutVolAvionElectrique;
    0.0,//float coutVolUlm;
    0.0,//float coutVolPlaneur;
    0.0,//float coutVolHelicoptere;
};

