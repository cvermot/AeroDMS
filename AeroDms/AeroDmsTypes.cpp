/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2023-2026 Clément VERMOT-DESROCHES (clement@vermot.net)

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

const AeroDmsTypes::QuantiteDontCompense AeroDmsTypes::K_INIT_QUANTITE_DONT_COMPENSE =
{
    0.0,
    0.0,
    K_INIT_QSTRING
};
const AeroDmsTypes::QuantiteDontCompenseInt AeroDmsTypes::K_INIT_QUANTITE_DONT_COMPENSE_INT =
{
    K_INIT_INT,
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

AeroDmsTypes::TotalConsoEmissionsCo2::TotalConsoEmissionsCo2()
{
    litresEssence = K_INIT_QUANTITE_DONT_COMPENSE;
    litreKerosene = K_INIT_QUANTITE_DONT_COMPENSE; 
    kwhElectricite = K_INIT_QUANTITE_DONT_COMPENSE; 
    co2Direct = K_INIT_QUANTITE_DONT_COMPENSE;
    co2Indirect = 0.0;
    dureeDesVolsEnMinute = K_INIT_QUANTITE_DONT_COMPENSE_INT;
    nombreDeVols = K_INIT_QUANTITE_DONT_COMPENSE_INT;
    typeDecompte = TypeDecompte_INDEFINI;
    unite = UniteTypeConsommation_INDEFINI;

    litresEssence.unite = " L";
    litresEssence.typeDEnergie = "Essence";
    litreKerosene.unite = " L";
    litreKerosene.typeDEnergie = "Kérosène";
    kwhElectricite.unite = " kWh";
    kwhElectricite.typeDEnergie = "Électricité";
    co2Direct.unite = " kg";
}

const QString AeroDmsTypes::QuantiteDontCompense::consoAvecUnites(const bool p_total) const
{
    QString conso = "";

    if (quantite != 0.0)
    {
        if (p_total)
        {
            conso = typeDEnergie + " :<br />";
        }

        conso = conso + QString::number(quantite, 'f', 0) + unite;
        if (dontCompense != 0.0)
        {
            conso = conso + "<br />(" + QString::number(dontCompense, 'f', 0) + unite + ")";
        }
    }

    return conso;
}

const QString AeroDmsTypes::TotalConsoEmissionsCo2::nbVols() const
{
    QString nbVols = "";


    nbVols = QString::number(nombreDeVols.quantite, 'f', 0);
    if (nombreDeVols.dontCompense != 0.0)
    {
        nbVols = nbVols + "<br />(" + QString::number(nombreDeVols.dontCompense, 'f', 0) + ")";
    }

    return nbVols;
}

const QString AeroDmsTypes::TotalConsoEmissionsCo2::consoAvecUnites(const bool p_total) const
{
    QString conso = "";

    QString consoEssence = litresEssence.consoAvecUnites(p_total);
    QString consoKerosene = litreKerosene.consoAvecUnites(p_total);
    QString consoElectricite = kwhElectricite.consoAvecUnites(p_total);

    if (consoEssence != "")
    {
        conso = conso + consoEssence;
    } 

    if (consoKerosene != "")
    {
        if (conso != "")
        {
            conso = conso + "<br />";
        }
        conso = conso + consoKerosene;
    }
    
    if (consoElectricite != "")
    {
        if (conso != "")
        {
            conso = conso + "<br />";
        }
        conso = conso + consoElectricite;
    }

    return conso;
}

const QString AeroDmsTypes::TotalConsoEmissionsCo2::emissionsAvecUnite(TypeEmissionsDemande p_demande) const
{
    QString emissions = "";

    switch (p_demande)
    {
        case TypeEmissionsDemande_DIRECTES:
        {
            emissions = co2Direct.consoAvecUnites(false);
        }
        break;
        case TypeEmissionsDemande_INDIRECTES:
        {
            emissions = QString::number(co2Indirect, 'f', 0) + co2Direct.unite;
        }
        break;
        case TypeEmissionsDemande_TOTALES:
        {
            emissions = QString::number(co2Direct.quantite + co2Indirect, 'f', 0) + co2Direct.unite;
            if (co2Direct.dontCompense != 0.0)
            {
                emissions = emissions + "<br />(" + QString::number(co2Direct.dontCompense, 'f', 0) + co2Direct.unite + ")";
            }
        }
        break;
    }
    
    return emissions;
}

void AeroDmsTypes::TotauxConsoEmissionsCo2::calculerEmissions(const StatsEmissionsCo2& p_statsDuType,
    const StatsEmissionsCo2& p_dontCompense,
    const ParametresEmissionsCo2& p_parametresEmissionsCo2)
{
    courant = TotalConsoEmissionsCo2();
    courant.typeDecompte = p_statsDuType.typeDecompte;
    courant.unite = p_statsDuType.uniteTypeConsommation;

    //la conso d'energie est faite par défaut sur la base d'une conso horaire, on l'ecrase si elle est en fait unitaire
    double conso = p_statsDuType.consommationHoraireDuType * p_statsDuType.nombreMinutesVol/60.0;
    double consoCompense = p_statsDuType.consommationHoraireDuType * p_dontCompense.nombreMinutesVol / 60.0;
    if (p_statsDuType.typeDecompte == AeroDmsTypes::TypeDecompte_UNITAIRE)
    {
        conso = p_statsDuType.consommationHoraireDuType * p_statsDuType.nombreDeVols;
        consoCompense = p_statsDuType.consommationHoraireDuType * p_dontCompense.nombreDeVols;
    }

    switch (p_statsDuType.uniteTypeConsommation)
    {
        case AeroDmsTypes::UniteTypeConsommation_LITRES_ESSENCE:
        {
            courant.litresEssence.quantite = conso;
            courant.co2Direct.quantite = conso * p_parametresEmissionsCo2.kgCo2ParLitreEssence;
            total.litresEssence.quantite = total.litresEssence.quantite + conso;
            totalGeneral.litresEssence.quantite = totalGeneral.litresEssence.quantite + conso;

            courant.litresEssence.dontCompense = consoCompense;
            courant.co2Direct.dontCompense = consoCompense * p_parametresEmissionsCo2.kgCo2ParLitreEssence;
            total.litresEssence.dontCompense = total.litresEssence.dontCompense + consoCompense;
            totalGeneral.litresEssence.dontCompense = totalGeneral.litresEssence.dontCompense + consoCompense;
        }
        break;
        case AeroDmsTypes::UniteTypeConsommation_LITRES_GASOIL_KEROSENE:
        {
            courant.litreKerosene.quantite = conso;
            courant.co2Direct.quantite = conso * p_parametresEmissionsCo2.kgCo2ParLitreGasoil;
            total.litreKerosene.quantite = total.litreKerosene.quantite + conso;
            totalGeneral.litreKerosene.quantite = totalGeneral.litreKerosene.quantite + conso;

            courant.litreKerosene.dontCompense = consoCompense;
            courant.co2Direct.dontCompense = consoCompense * p_parametresEmissionsCo2.kgCo2ParLitreGasoil;
            total.litreKerosene.dontCompense = total.litreKerosene.dontCompense + consoCompense;
            totalGeneral.litreKerosene.dontCompense = totalGeneral.litreKerosene.dontCompense + consoCompense;
        }
        break;
        case AeroDmsTypes::UniteTypeConsommation_KILOWATTHEURES:
        {
            courant.kwhElectricite.quantite = conso;
            courant.co2Direct.quantite = conso * p_parametresEmissionsCo2.kgCo2ParKwh;
            total.kwhElectricite.quantite = total.kwhElectricite.quantite + conso;
            totalGeneral.kwhElectricite.quantite = totalGeneral.kwhElectricite.quantite + conso;

            courant.kwhElectricite.dontCompense = consoCompense;
            courant.co2Direct.dontCompense = consoCompense * p_parametresEmissionsCo2.kgCo2ParKwh;
            total.kwhElectricite.dontCompense = total.kwhElectricite.dontCompense + consoCompense;
            totalGeneral.kwhElectricite.dontCompense = totalGeneral.kwhElectricite.dontCompense + consoCompense;
        }
        break;
        default:
            break;
    }

    //On sommes les émissions directes dans les 2 autres structures :
    total.co2Direct.quantite = total.co2Direct.quantite + courant.co2Direct.quantite;
    totalGeneral.co2Direct.quantite = totalGeneral.co2Direct.quantite + courant.co2Direct.quantite;
    total.co2Direct.dontCompense = total.co2Direct.dontCompense + courant.co2Direct.dontCompense;
    totalGeneral.co2Direct.dontCompense = totalGeneral.co2Direct.dontCompense + courant.co2Direct.dontCompense;

    //les emissions indirectes sont toujours calculées sur une base horaire
    courant.co2Indirect = p_parametresEmissionsCo2.kgCo2IndirectsParHdv * p_statsDuType.nombreMinutesVol / 60.0;
    //On somme dans les 2 autres structures :
    total.co2Indirect = total.co2Indirect + courant.co2Indirect;
    totalGeneral.co2Indirect = totalGeneral.co2Indirect + courant.co2Indirect;

    courant.dureeDesVolsEnMinute.quantite = p_statsDuType.nombreMinutesVol;
    courant.dureeDesVolsEnMinute.dontCompense = p_dontCompense.nombreMinutesVol;
    total.dureeDesVolsEnMinute.quantite = total.dureeDesVolsEnMinute.quantite + courant.dureeDesVolsEnMinute.quantite;
    total.dureeDesVolsEnMinute.dontCompense = total.dureeDesVolsEnMinute.dontCompense + courant.dureeDesVolsEnMinute.dontCompense;
    totalGeneral.dureeDesVolsEnMinute.quantite = totalGeneral.dureeDesVolsEnMinute.quantite + courant.dureeDesVolsEnMinute.quantite;
    totalGeneral.dureeDesVolsEnMinute.dontCompense = totalGeneral.dureeDesVolsEnMinute.dontCompense + courant.dureeDesVolsEnMinute.dontCompense;

    courant.nombreDeVols.quantite = p_statsDuType.nombreDeVols;
    courant.nombreDeVols.dontCompense = p_dontCompense.nombreDeVols;
    total.nombreDeVols.quantite = total.nombreDeVols.quantite + courant.nombreDeVols.quantite;
    total.nombreDeVols.dontCompense = total.nombreDeVols.dontCompense + courant.nombreDeVols.dontCompense;
    totalGeneral.nombreDeVols.quantite = totalGeneral.nombreDeVols.quantite + courant.nombreDeVols.quantite;
    totalGeneral.nombreDeVols.dontCompense = totalGeneral.nombreDeVols.dontCompense + courant.nombreDeVols.dontCompense;
}

void AeroDmsTypes::TotauxConsoEmissionsCo2::rincerTotal()
{
    total = TotalConsoEmissionsCo2();
}

const AeroDmsTypes::TotalConsoEmissionsCo2& AeroDmsTypes::TotauxConsoEmissionsCo2::getCourant()
{
    return courant;
}
const AeroDmsTypes::TotalConsoEmissionsCo2& AeroDmsTypes::TotauxConsoEmissionsCo2::getTotal()
{
    return total;
}
const AeroDmsTypes::TotalConsoEmissionsCo2 & AeroDmsTypes::TotauxConsoEmissionsCo2::getTotalGeneral()
{
    return totalGeneral;
}
