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
#include "AeroDmsTypes.h"

const QString AeroDmsTypes::K_INIT_QSTRING = "";
const int AeroDmsTypes::K_INIT_INT_INVALIDE = -1;
const int AeroDmsTypes::K_INIT_INT = 0;

const AeroDmsTypes::TotauxRecettes AeroDmsTypes::K_INIT_TOTAUX_RECETTE = { 0.0, 
                                                                           0.0, 
                                                                           0.0 };

const AeroDmsTypes::HeureDeVolRemboursement AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT = { 0.0, 
                                                                                                0.0, 
                                                                                                "0h00",
                                                                                                0 };

const AeroDmsTypes::SubventionsParPilote AeroDmsTypes::K_INIT_SUBVENTION_PAR_PILOTE = { K_INIT_QSTRING, K_INIT_INT, K_INIT_QSTRING, K_INIT_QSTRING, K_INIT_QSTRING, 0.0,
																						AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT ,
																						AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT ,
																						AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT ,
																						AeroDmsTypes::K_INIT_HEURE_DE_VOL_REMBOURSEMENT };
const AeroDmsTypes::Vol AeroDmsTypes::K_INIT_VOL = { K_INIT_QSTRING,//QString idPilote;
                                                     QDate() ,//QDate date;
                                                     K_INIT_QSTRING,//QString nomPilote;
                                                     K_INIT_QSTRING,//QString prenomPilote;
                                                     "0h00",//QString typeDeVol;
                                                     K_INIT_QSTRING,//QString duree;
                                                     K_INIT_QSTRING,//QString remarque;
                                                     K_INIT_QSTRING,//QString immat;
                                                     K_INIT_QSTRING,//QString activite;
                                                     K_INIT_QSTRING,//QString estSoumisCe;
                                                     false,//bool estSoumis
                                                     false,//bool soumissionEstDelayee
                                                     0.0,//double coutVol;
                                                     0.0,//double montantRembourse;
                                                     K_INIT_INT,//int volId;
                                                     K_INIT_INT,//int dureeEnMinutes;
                                                     K_INIT_INT_INVALIDE,//int baladeId;
                                                     K_INIT_INT_INVALIDE//int facture
};

const AeroDmsTypes::DonneesFacture AeroDmsTypes::K_INIT_DONNEES_FACTURE = { QDate(),
                                                                            QTime(),
                                                                            0.0,
                                                                            QString(),
                                                                            K_INIT_INT
};

const AeroDmsTypes::Pilote AeroDmsTypes::K_INIT_PILOTE = { K_INIT_QSTRING, //QString idPilote;
                                                           K_INIT_QSTRING, //QString nom;
                                                           K_INIT_QSTRING, //QString prenom;
                                                           K_INIT_QSTRING, //QString aeroclub;
                                                           K_INIT_INT, // int idAeroclub;
                                                           K_INIT_QSTRING, //QString activitePrincipale;
                                                           false, //bool estAyantDroit;
                                                           K_INIT_QSTRING, //QString mail;
                                                           K_INIT_QSTRING, //QString telephone;
                                                           K_INIT_QSTRING, //QString remarque;
                                                           false, //bool estActif;
                                                           false //bool estPiloteBrevete;
};

const AeroDmsTypes::Club AeroDmsTypes::K_INIT_CLUB = { K_INIT_INT, //int idAeroclub;
                                                       K_INIT_QSTRING, //QString aeroclub;
                                                       "LF", //QString aerodrome;
                                                       K_INIT_QSTRING, //QString raisonSociale;
                                                       K_INIT_QSTRING, //QString iban;
                                                       K_INIT_QSTRING, //QString bic;
};

const AeroDmsTypes::StatsPilotes AeroDmsTypes::K_INIT_DONNEES_STATS_PILOTES =
{
    K_INIT_INT,
    K_INIT_INT,
    K_INIT_INT,
    K_INIT_INT
};

const AeroDmsTypes::StatsAeronef AeroDmsTypes::K_INIT_STAT_AERONEF = 
{
    K_INIT_QSTRING,
    K_INIT_QSTRING,
    K_INIT_INT
};

const AeroDmsTypes::DetailsBaladesEtSorties AeroDmsTypes::K_INIT_DETAILS_BALADES_ET_SORTIES =
{
    K_INIT_INT_INVALIDE,
    K_INIT_INT_INVALIDE,
    K_INIT_INT_INVALIDE,
    QDate(),
    K_INIT_INT,
    0.0,
    0.0,
    K_INIT_QSTRING,
    K_INIT_QSTRING,
    0.0,
    K_INIT_QSTRING
};

const AeroDmsTypes::ResolutionEtParametresStatistiques AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES =
{
    QSize(800,600),
    10
};

const AeroDmsTypes::StatsHeuresDeVolParActivite AeroDmsTypes::K_INIT_STATS_HEURES_DE_VOL_PAR_ACTIVITES =
{
    K_INIT_QSTRING ,//QString piloteId;
    K_INIT_QSTRING,//QString nomPrenomPilote;

    K_INIT_INT,//int minutesVolAvion;
    K_INIT_INT,//int minutesVolAvionElectrique;
    K_INIT_INT,//int minutesVolUlm;
    K_INIT_INT,//int minutesVolPlaneur;
    K_INIT_INT,//int minutesVolHelicoptere;

    0.0,//double subventionVolAvion;
    0.0,//double subventionVolAvionElectrique;
    0.0,//double subventionVolUlm;
    0.0,//double subventionVolPlaneur;
    0.0,//double subventionVolHelicoptere;

    0.0,//double coutVolAvion;
    0.0,//double coutVolAvionElectrique;
    0.0,//double coutVolUlm;
    0.0,//double coutVolPlaneur;
    0.0,//double coutVolHelicoptere;
};

const QString AeroDmsTypes::recupererChaineEtapeChargementBdd(EtapeChargementBdd p_etape)
{
    switch (p_etape)
    {
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_SHA256:
    {
        return "Demande SHA256";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_SHA256_CONNEXION:
    {
        return "Demande SHA256 connexion";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_SHA256_RECU:
    {
        return "Demande SHA256 reçu";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_TELECHARGEMENT_BDD_CONNEXION:
    {
        return "Demande téléchargement BDD connexion";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_TELECHARGEMENT_BDD_RECU:
    {
        return "Demande téléchargement BDD reçu";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_TELECHARGEMENT_PRISE_EN_COMPTE_BDD_TELECHARGEE:
    {
        return "Demande prise en compte BDD téléchargée";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_PRISE_VERROU:
    {
        return "Prise verrou";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_ENVOI_BDD:
    {
        return "Demande envoi BDD";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_ENVOI_BDD_ENVOI:
    {
        return "Demande envoi BDD envoi";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_DEMANDE_ENVOI_BDD_FIN:
    {
        return "Demande envoi BDD fin";
    }
    break;
    case AeroDmsTypes::EtapeChargementBdd_TERMINE:
    {
        return "Terminé";
    }
    break;
    default:
    {
        return "Valeur indéterminée";
    }
    break;
    }
}
