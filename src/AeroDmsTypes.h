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

#ifndef AERODMSTYPES_H
#define AERODMSTYPES_H

#include <QtWidgets>
#include "AeroDmsServices.h"

class AeroDmsTypes
{
public:

    enum PiloteTableElement {
        PiloteTableElement_NOM = 0x0,
        PiloteTableElement_PRENOM = 0x1,
        PiloteTableElement_ANNEE = 0x2,
        PiloteTableElement_HEURES_ENTRAINEMENT_SUBVENTIONNEES = 0x3,
        PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE = 0x4,
        PiloteTableElement_HEURES_BALADES_SUBVENTIONNEES = 0x5,
        PiloteTableElement_MONTANT_BALADES_SUBVENTIONNE = 0x6,
        PiloteTableElement_HEURES_SORTIES_SUBVENTIONNEES = 0x7,
        PiloteTableElement_MONTANT_SORTIES_SUBVENTIONNE = 0x8,
        PiloteTableElement_HEURES_TOTALES_SUBVENTIONNEES = 0x9,
        PiloteTableElement_MONTANT_TOTAL_SUBVENTIONNE = 0xA,
        PiloteTableElement_PILOTE_ID = 0xB,
        PiloteTableElement_NB_COLONNES = 0xC,
    };

    enum VolsDetectesTableElement {
        VolsDetectesTableElement_DATE = 0x0,
        VolsDetectesTableElement_DUREE = 0x1,
        VolsDetectesTableElement_MONTANT = 0x2,
        VolsDetectesTableElement_IMMAT = 0x3,
        VolsDetectesTableElement_TYPE = 0x4,
        VolsDetectesTableElement_NB_COLONNES = 0x5,
    };

    enum VolTableElement {
        VolTableElement_PILOTE = 0x0,
        VolTableElement_DATE = 0x1,
        VolTableElement_TYPE_DE_VOL = 0x2,
        VolTableElement_DUREE = 0x3,
        VolTableElement_COUT = 0x4,
        VolTableElement_SUBVENTION = 0x5,
        VolTableElement_SOUMIS_CE = 0x6,
        VolTableElement_REMARQUE = 0x7,
        VolTableElement_IMMAT = 0x8,
        VolTableElement_ACTIVITE = 0x9,
        VolTableElement_VOL_ID = 0xA,
        VolTableElement_DUREE_EN_MINUTES = 0xB,
        VolTableElement_NB_COLONNES = 0xC,
    };

    enum FactureTableElement {
        FactureTableElement_INTITULE = 0x0,
        FactureTableElement_MONTANT = 0x1,
        FactureTableElement_PAYEUR = 0x2,
        FactureTableElement_NOM_SORTIE = 0x3,
        FactureTableElement_TYPE_SORTIE = 0x4,
        FactureTableElement_DATE = 0x5,
        FactureTableElement_ANNEE = 0x6,
        FactureTableElement_NOM_FACTURE = 0x7,
        FactureTableElement_SOUMIS_CE = 0x8,
        FactureTableElement_FACTURE_ID = 0x9,
        FactureTableElement_NB_COLONNES = 0xA,
    };

    enum RecetteTableElement {
        RecetteTableElement_DATE = 0x0,
        RecetteTableElement_TYPE_DE_RECETTE = 0x1,
        RecetteTableElement_INTITULE = 0x2,
        RecetteTableElement_MONTANT = 0x3,
        RecetteTableElement_SOUMIS_CE = 0x4,
        RecetteTableElement_ID = 0x5,
        RecetteTableElement_NB_COLONNES = 0x6,
    };

    enum AeronefTableElement {
        AeronefTableElement_IMMAT = 0x0,
        AeronefTableElement_TYPE = 0x1,
        AeronefTableElement_NB_COLONNES = 0x2,
    };

    enum ResultatCreationPilote {
        ResultatCreationPilote_SUCCES = 0x0,
        ResultatCreationPilote_PILOTE_EXISTE = 0x1,
        ResultatCreationPilote_AUTRE = 0x2
    };

    enum PdfTypeDeDemande {
        PdfTypeDeDemande_RECAP_ANNUEL,
        PdfTypeDeDemande_HEURE_DE_VOL,
        PdfTypeDeDemande_COTISATION,
        PdfTypeDeDemande_PAIEMENT_SORTIE_OU_BALADE,
        PdfTypeDeDemande_FACTURE
    };

    enum Statistiques {
        Statistiques_HEURES_ANNUELLES,
        Statistiques_HEURES_PAR_PILOTE,
        Statistiques_HEURES_PAR_TYPE_DE_VOL,
        Statistiques_HEURES_PAR_ACTIVITE,
        Statistiques_STATUTS_PILOTES,
        Statistiques_AERONEFS
    };

    enum VolSoumis {
        VolSoumis_TOUS_LES_VOLS,
        VolSoumis_VOL_SOUMIS,
        VolSoumis_VOL_NON_SOUMIS
    };

    enum Signature {
        Signature_SANS,
        Signature_MANUSCRITE_IMAGE,
        Signature_NUMERIQUE_LEX_COMMUNITY
    };

    enum TypeGenerationPdf {
        TypeGenerationPdf_TOUTES,
        TypeGenerationPdf_RECETTES_SEULEMENT,
        TypeGenerationPdf_DEPENSES_SEULEMENT
    };

    enum Aeroclub {
        Aeroclub_INCONNU,
        Aeroclub_Generique_OpenFlyer,
        Aeroclub_Generique_Aerogest,
        Aeroclub_DACA,
        Aeroclub_CAPAM,
        Aeroclub_ACB,
        Aeroclub_ACBA,
        Aeroclub_SEPAVIA,
        Aeroclub_ACAndernos,
        Aeroclub_GENERIQUE,
        Aeroclub_GENERIQUE_1_PASSE
    };

    enum Onglet {
        Onglet_PILOTES,
        Onglet_VOLS,
        Onglet_FACTURES,
        Onglet_RECETTES,
        Onglet_AJOUT_DEPENSES,
        Onglet_AJOUT_RECETTES,
        Onglet_STATISTIQUES
    };

    struct DemandeEnCoursDeTraitement {
        PdfTypeDeDemande typeDeDemande;
        QString idPilote;
        QString typeDeVol;
        QString nomBeneficiaire;
        float montant;
        int annee;
        QString nomTresorier;
        QString nomFichier;
        QStringList listeFactures;
        AeroDmsTypes::TypeGenerationPdf typeDeGenerationDemandee;
        AeroDmsTypes::Signature typeDeSignatureDemandee;
        bool mergerTousLesPdf;
        bool recapHdVAvecRecettes;
        bool recapHdVAvecBaladesEtSorties;
        //int idFacture;
    };

    struct Pilote {
        QString idPilote;
        QString nom;
        QString prenom;
        QString aeroclub;
        QString activitePrincipale;
        bool estAyantDroit;
        QString mail;
        QString telephone;
        QString remarque;
        bool estActif;
        bool estBrevete;
    };
    static const Pilote K_INIT_PILOTE;
    typedef QList<Pilote> ListePilotes;

    struct HeureDeVolRemboursement
    {
        float montantRembourse;
        float coutTotal;
        QString heuresDeVol;
        int tempsDeVolEnMinutes;

        HeureDeVolRemboursement& operator+(const HeureDeVolRemboursement& a)
        {
            montantRembourse = montantRembourse + a.montantRembourse;
            coutTotal = coutTotal + a.coutTotal;
            tempsDeVolEnMinutes = tempsDeVolEnMinutes + a.tempsDeVolEnMinutes;
            heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(tempsDeVolEnMinutes);
            return *this;
        }
    };
    static const HeureDeVolRemboursement K_INIT_HEURE_DE_VOL_REMBOURSEMENT;

    struct SubventionsParPilote {
        QString idPilote;
        int annee;
        QString nom;
        QString prenom;
        QString aeroclub;
        HeureDeVolRemboursement entrainement;
        HeureDeVolRemboursement balade;
        HeureDeVolRemboursement sortie;
        HeureDeVolRemboursement totaux;

        SubventionsParPilote& operator+(const SubventionsParPilote& a)
        {
            entrainement = entrainement + a.entrainement;
            balade = balade + a.balade;
            sortie = sortie + a.sortie;
            totaux = totaux + a.totaux;
            return *this;
        }
    };
    static const SubventionsParPilote K_INIT_SUBVENTION_PAR_PILOTE;
    typedef QList<SubventionsParPilote> ListeSubventionsParPilotes;

    struct Vol {
        QString idPilote;
        QDate date;
        QString nomPilote;
        QString prenomPilote;
        QString typeDeVol;
        QString duree;
        QString remarque;
        QString immat;
        QString activite;
        QString estSoumisCe;
        bool estSoumis;
        float coutVol;
        float montantRembourse;
        int volId;
        int dureeEnMinutes;
        int baladeId;
    };
    static const Vol K_INIT_VOL;
    typedef QList<Vol> ListeVols;

    struct DemandeRemboursement {
        QString typeDeVol;
        QString piloteId;
        float montantARembourser;
        int annee;
        QString nomFichierFacture;
    };
    typedef QList<DemandeRemboursement> ListeDemandeRemboursement;

    struct Recette {
        QString intitule;
        QString typeDeSortie;
        float montant;
        int annee;
    };
    typedef QList<Recette> ListeRecette;

    struct RecetteDetail {
        QString intitule;
        QString typeDeRecette;
        float montant;
        QDate date;
        int annee;
        bool estSoumisCe;
        int id;
    };
    typedef QList<RecetteDetail> ListeRecetteDetail;

    struct Sortie {
        int id;
        QString nom;
        QDate date;
    };
    typedef QList<Sortie> ListeSortie;

    struct Aeronef {
        QString immatriculation;
        QString type;
    };
    typedef QList<Aeronef> ListeAeronefs;

    struct CotisationAnnuelle {
        QString idPilote;
        int annee;
        float montantSubvention;
        float montant;
        bool estEnEdition;
    };
    
    struct DemandeRemboursementFacture {
        int id;
        QString intitule;
        float montant;
        QString payeur;
        QString nomSortie;
        QString typeDeSortie;
        QDate date;
        int annee;
        bool soumisCe;
        QString nomFacture;
    };
    typedef QList<DemandeRemboursementFacture> ListeDemandeRemboursementFacture;

    struct StatsHeuresDeVol {
        QString mois;
        int minutesEntrainement;
        int minutesSortie;
        int minutesBalade;
    };
    typedef QList< StatsHeuresDeVol> ListeStatsHeuresDeVol;

    struct VolSortieOuBalade {
        QString nomVol = "";
        float montantSubventionAttendu = 0;
        bool volAAuMoinsUnPaiement = 0;
    };
    typedef QList<VolSortieOuBalade> ListeVolSortieOuBalade;

    struct StatsHeuresDeVolParActivite {
        QString piloteId;
        QString nomPrenomPilote;
        int minutesVolAvion;
        int minutesVolAvionElectrique;
        int minutesVolUlm;
        int minutesVolPlaneur;
        int minutesVolHelicoptere;
    };
    typedef QList< StatsHeuresDeVolParActivite> ListeStatsHeuresDeVolParActivite;

    struct ParametresMetier {
        float montantSubventionEntrainement;
        float montantCotisationPilote;
        float proportionRemboursementEntrainement;
        float proportionRemboursementBalade;
        float plafondHoraireRemboursementEntrainement;
        float proportionParticipationBalade;
        QString nomTresorier;
        int delaisDeGardeBdd;
        QString texteMailDispoCheques;
        bool ouvertureAutomatiqueApresGeneration;
    };

    struct TotauxRecettes {
        double cotisations;
        double balades;
        double sorties;
    };
    static const TotauxRecettes K_INIT_TOTAUX_RECETTE;

    struct DonneesFacture
    {
        QDate dateDuVol;
        QTime dureeDuVol;
        float coutDuVol;
        QString immat;
        int pageDansLeFichierPdf;
    };
    static const DonneesFacture K_INIT_DONNEES_FACTURE;
    typedef QList<DonneesFacture> ListeDonneesFacture;

    struct StatsPilotes
    {
        int nbBrevete;
        int nbNonBrevete;
        int nbOuvranDroit;
        int nbAyantDroit;
    };
    static const StatsPilotes K_INIT_DONNEES_STATS_PILOTES;

    struct StatsAeronef
    {
        QString immat;
        QString type;
        int nombreMinutesVol;
    };
    static const StatsAeronef K_INIT_STAT_AERONEF;
    typedef QList<StatsAeronef> StatsAeronefs;

    struct DetailsBaladesEtSorties
    {
        int volId;
        int idSortie;
        int idRecette;
        QDate dateVol;
        int dureeVol;
        float coutVol;
        float montantRembouse;
        QString nomPassagers;
        QString intituleRecette;
        float montantRecette;
        QString nomSortie;
    };
    static const DetailsBaladesEtSorties K_INIT_DETAILS_BALADES_ET_SORTIES;
    typedef QList<DetailsBaladesEtSorties> ListeDetailsBaladesEtSorties;

    struct GroupeBaladesEtSortiesAssociees
    {
        QVector<AeroDmsTypes::DetailsBaladesEtSorties> volsUniques;
        QVector<AeroDmsTypes::DetailsBaladesEtSorties> recettesUniques;
    };

    struct BaladesEtSortiesParId
    {
        int idSortie;
        int nombreDeLignes;
        QVector<GroupeBaladesEtSortiesAssociees> baladesEtSortiesAssociees;
    };
    typedef QVector<BaladesEtSortiesParId> ListeBaladesEtSortiesParIdSortie;
};

#endif // AERODMSTYPES_H
