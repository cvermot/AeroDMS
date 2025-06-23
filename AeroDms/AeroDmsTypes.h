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

#ifndef AERODMSTYPES_H
#define AERODMSTYPES_H

#include <QtWidgets>
#include <QPrinter>

class AeroDmsTypes
{
public:

    enum PiloteTableElement {
        PiloteTableElement_NOM = 0x0,
        PiloteTableElement_PRENOM = 0x1,
        PiloteTableElement_AEROCLUB = 0x2,
        PiloteTableElement_TERRAIN = 0x3,
        PiloteTableElement_ANNEE = 0x4,
        PiloteTableElement_HEURES_ENTRAINEMENT_SUBVENTIONNEES = 0x5,
        PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE = 0x6,
        PiloteTableElement_HEURES_BALADES_SUBVENTIONNEES = 0x7,
        PiloteTableElement_MONTANT_BALADES_SUBVENTIONNE = 0x8,
        PiloteTableElement_HEURES_SORTIES_SUBVENTIONNEES = 0x9,
        PiloteTableElement_MONTANT_SORTIES_SUBVENTIONNE = 0xA,
        PiloteTableElement_HEURES_TOTALES_SUBVENTIONNEES = 0xB,
        PiloteTableElement_MONTANT_TOTAL_SUBVENTIONNE = 0xC,
        PiloteTableElement_PILOTE_ID = 0xD,
        PiloteTableElement_NB_COLONNES = 0xE,
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

    enum SubventionDemandeeTableElement {
        SubventionDemandeeTableElement_DATE = 0x0,
        SubventionDemandeeTableElement_PILOTE = 0x1,
        SubventionDemandeeTableElement_BENEFICIAIRE = 0x2,
        SubventionDemandeeTableElement_TYPE_DEMANDE = 0x3,
        SubventionDemandeeTableElement_MONTANT = 0x4,
        SubventionDemandeeTableElement_MONTANT_VOL = 0x5,
        SubventionDemandeeTableElement_MODE_DE_REGLEMENT = 0x6,
        SubventionDemandeeTableElement_NOTE = 0x7,
        SubventionDemandeeTableElement_ID_DEMANDE = 0x8,
        SubventionDemandeeTableElementTableElement_NB_COLONNES = 0x9,
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

    enum ResultatCreationBdd {
        ResultatCreationBdd_SUCCES = 0x0,
        ResultatCreationBdd_ELEMENT_EXISTE = 0x1,
        ResultatCreationBdd_AUTRE = 0x2
    };

    enum PdfTypeDeDemande {
        PdfTypeDeDemande_RECAP_ANNUEL,
        PdfTypeDeDemande_HEURE_DE_VOL,
        PdfTypeDeDemande_COTISATION,
        PdfTypeDeDemande_PAIEMENT_SORTIE_OU_BALADE,
        PdfTypeDeDemande_FACTURE
    };

    enum Unites {
        Unites_HEURES,
        Unites_EUROS
    };

    enum EtapeChargementBdd {
        EtapeChargementBdd_DEMANDE_SHA256 = 0,
        EtapeChargementBdd_DEMANDE_SHA256_CONNEXION,
        EtapeChargementBdd_DEMANDE_SHA256_RECU,
        EtapeChargementBdd_DEMANDE_TELECHARGEMENT_BDD_CONNEXION,
        EtapeChargementBdd_DEMANDE_TELECHARGEMENT_BDD_RECU,
        EtapeChargementBdd_DEMANDE_TELECHARGEMENT_PRISE_EN_COMPTE_BDD_TELECHARGEE,
        EtapeChargementBdd_PRISE_VERROU,
        EtapeChargementBdd_DEMANDE_ENVOI_BDD,
        EtapeChargementBdd_DEMANDE_ENVOI_BDD_ENVOI,
        EtapeChargementBdd_DEMANDE_ENVOI_BDD_FIN,
        EtapeChargementBdd_TERMINE,
    };
    static const QString recupererChaineEtapeChargementBdd(EtapeChargementBdd p_etape);

    enum Statistiques {                              //     RESO|STATS
        Statistiques_HEURES_ANNUELLES       = 0x1,   //     XXXX|0000 0000 0001
        Statistiques_HEURES_PAR_PILOTE      = 0x2,   //     XXXX|0000 0000 0010
        Statistiques_HEURES_PAR_TYPE_DE_VOL = 0x4,   //     XXXX|0000 0000 0100
        Statistiques_HEURES_PAR_ACTIVITE    = 0x8,   //     XXXX|0000 0000 1000
        Statistiques_STATUTS_PILOTES        = 0x10,  //     XXXX|0000 0001 0000
        Statistiques_AERONEFS               = 0x20,  //     XXXX|0000 0010 0000
        Statistiques_EUROS_PAR_PILOTE       = 0x40,  //     XXXX|0000 0100 0000
        Statistiques_EUROS_PAR_TYPE_DE_VOL  = 0x80,  //     XXXX|0000 1000 0000
        Statistiques_EUROS_PAR_ACTIVITE     = 0x100  //     XXXX|0001 0000 0000
    };

                                     //Encodage vers PdrRenderer : 4 bits pour la résolution puis 12 bits pour la stats
    enum Resolution {                //                     RESO|STATS
        Resolution_Full_HD           = 0x4000, //1920x1080  0100|XXXX XXXX XXXX
        Resolution_QHD               = 0x8000, //2560x1440  1000|XXXX XXXX XXXX
        Resolution_4K                = 0xC000, //3840x2160  1100|XXXX XXXX XXXX
        Resolution_RATIO_16_9        = 0x2000, //           0010|XXXX XXXX XXXX
        Resolution_MASQUE_RESOLUTION = 0xC000, //           1100|0000 0000 0000
        Resolution_MASQUE_RATIO      = 0x2000  //           0010|0000 0000 0000
    };

    enum OptionsDonneesStatistiques {
        OptionsDonneesStatistiques_TOUS_LES_VOLS                   = 0x0,
        OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT   = 0x1,
        OptionsDonneesStatistiques_EXCLURE_PLANEUR                 = 0x2,
        OptionsDonneesStatistiques_EXCLURE_ULM                     = 0x4,
        OptionsDonneesStatistiques_EXCLURE_HELICOPTERE             = 0x8,
        OptionsDonneesStatistiques_EXCLURE_AVION                   = 0x10,
    };

    enum ElementSoumis {
        ElementSoumis_TOUS_LES_ELEMENTS,
        ElementSoumis_ELEMENTS_SOUMIS,
        ElementSoumis_ELEMENTS_NON_SOUMIS
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
        Aeroclub_UACA,
        Aeroclub_ATVV,
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

    enum MailPilotes
    {
        MailPilotes_AYANT_COTISE,
        MailPilotes_ACTIF_AYANT_COTISE,
        MailPilotes_ACTIFS,
        MailPilotes_ACTIFS_ET_BREVETES,
        MailPilotes_BREVETES,
        MailPilotes_SUBVENTION_NON_CONSOMMEE
    };

    enum EtatGeneration
    {
        EtatGeneration_OK,
        EtatGeneration_FICHIER_ABSENT,
        EtatGeneration_CREATION_DOSSIER_IMPOSSIBLE
    };

    enum EtatImpression
    {
        EtatImpression_TERMINEE,
        EtatImpression_ANNULEE_PAR_UTILISATEUR
    };

    enum EtatRecuperationDonneesFactures
    {
        EtatRecuperationDonneesFactures_AUCUN,
        EtatRecuperationDonneesFactures_CONNEXION_EN_COURS,
        EtatRecuperationDonneesFactures_CONNECTE,
        EtatRecuperationDonnnesFactures_ECHEC_CONNEXION,
        EtatRecuperationDonnnesFactures_ECHEC_CONNEXION_SSL_TLS_INDISPONIBLE,
        EtatRecuperationDonneesFactures_RECUPERATION_DONNEES_EN_COURS,
        EtatRecuperationDonneesFactures_DONNEES_RECUPEREES,
        EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_DONNEES,
        EtatRecuperationDonneesFactures_RECUPERATION_FACTURE_EN_COURS,
        EtatRecuperationDonneesFactures_FACTURE_RECUPEREE,
        EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_FACTURE
    };

    enum ModeDeReglement
    {
        ModeDeReglement_CHEQUE,
        ModeDeReglement_VIREMENT,
    };

    enum Status
    {
        Status_AUCUNE_ACTION_EFFECTUEE,
        Status_MIS_A_JOUR,
        Status_CREATION
    };

    enum DonnesMailingType
    {
        DonnesMailingType_DEMANDE_DE_SUBVENTION,
        DonnesMailingType_SUBVENTION_VERSEE_PAR_VIREMENT,
        DonnesMailingType_PILOTES_ACTIFS_D_UN_AERODROME,
        DonnesMailingType_PILOTES_ACTIFS_BREVETES_VOL_MOTEUR_D_UN_AERODROME
    };

    enum ModeFonctionnementLogiciel
    {
        ModeFonctionnementLogiciel_INTERNE_UNIQUEMENT = 0,
        ModeFonctionnementLogiciel_EXERNE_AUTORISE_MODE_INTERNE = 1,
        ModeFonctionnementLogiciel_EXERNE_AUTORISE_MODE_EXTERNE = 2
    };

    static const QString K_INIT_QSTRING;
    static const int K_INIT_INT_INVALIDE;
    static const int K_INIT_INT;

    struct DonneesMailing {
        DonnesMailingType typeMailing = DonnesMailingType_DEMANDE_DE_SUBVENTION;
        QString donneeComplementaire = K_INIT_QSTRING;
    };

    struct DemandeEnCoursDeTraitement {
        PdfTypeDeDemande typeDeDemande = PdfTypeDeDemande_HEURE_DE_VOL;
        QString idPilote = K_INIT_QSTRING;
        QString typeDeVol = K_INIT_QSTRING;
        QString nomBeneficiaire = K_INIT_QSTRING;
        double montant = 0.0;
        ModeDeReglement modeDeReglement = ModeDeReglement_CHEQUE;
        int annee = K_INIT_INT_INVALIDE;
        int anneeATraiter = K_INIT_INT_INVALIDE;
        QString nomTresorier = K_INIT_QSTRING;
        QString nomFichier = K_INIT_QSTRING;
        QStringList listeFactures = QStringList();
        AeroDmsTypes::TypeGenerationPdf typeDeGenerationDemandee = AeroDmsTypes::TypeGenerationPdf_TOUTES;
        AeroDmsTypes::Signature typeDeSignatureDemandee = AeroDmsTypes::Signature_SANS;
        bool mergerTousLesPdf = false;
        bool recapHdVAvecRecettes = false;
        bool recapHdVAvecBaladesEtSorties = false;
        bool virementEstAutorise = false;
        int recapHdvGraphAGenerer = K_INIT_INT;
        //int idFacture;
    };

    struct Pilote {
        QString idPilote = K_INIT_QSTRING;
        QString nom = K_INIT_QSTRING;
        QString prenom = K_INIT_QSTRING;
        QString aeroclub = K_INIT_QSTRING;
        int idAeroclub = K_INIT_INT_INVALIDE;
        QString activitePrincipale = K_INIT_QSTRING;
        bool estAyantDroit = false;
        QString mail = K_INIT_QSTRING;
        QString telephone = K_INIT_QSTRING;
        QString remarque = K_INIT_QSTRING;
        bool estActif = false;
        bool estBrevete = false;
    };
    static const Pilote K_INIT_PILOTE;
    typedef QList<Pilote> ListePilotes;

	struct Aerodrome {
		QString indicatifOaci = K_INIT_QSTRING;
		QString nom = K_INIT_QSTRING;
	};
    typedef QList<Aerodrome> ListeAerodromes;

    struct Club {
        int idAeroclub = K_INIT_INT_INVALIDE;
        QString aeroclub = K_INIT_QSTRING;
        QString aerodrome = K_INIT_QSTRING;
        QString raisonSociale = K_INIT_QSTRING;
        QString iban = K_INIT_QSTRING;
        QString bic = K_INIT_QSTRING;
    };
    static const Club K_INIT_CLUB;
    typedef QList<Club> ListeAeroclubs;

    struct HeureDeVolRemboursement
    {
        double montantRembourse = 0.0;
        double coutTotal = 0.0;
        QString heuresDeVol = K_INIT_QSTRING;
        int tempsDeVolEnMinutes = 0.0;

        HeureDeVolRemboursement& operator+(const HeureDeVolRemboursement& a)
        {
            montantRembourse = montantRembourse + a.montantRembourse;
            coutTotal = coutTotal + a.coutTotal;
            tempsDeVolEnMinutes = tempsDeVolEnMinutes + a.tempsDeVolEnMinutes;
            const int heures = tempsDeVolEnMinutes / 60;
            const int minutes = tempsDeVolEnMinutes % 60;
            QString minutesString = QString::number(minutes);
            if (minutesString.size() == 1)
            {
                minutesString = QString("0").append(minutesString);
            }
            heuresDeVol = QString::number(heures).append("h").append(minutesString);
            return *this;
        }
    };
    static const HeureDeVolRemboursement K_INIT_HEURE_DE_VOL_REMBOURSEMENT;

    struct SubventionsParPilote {
        QString idPilote = K_INIT_QSTRING;
        int annee = K_INIT_INT_INVALIDE;
        QString nom = K_INIT_QSTRING;
        QString prenom = K_INIT_QSTRING;
        QString mail = K_INIT_QSTRING;
        double montantSubventionEntrainement = 0.0;
        HeureDeVolRemboursement entrainement = K_INIT_HEURE_DE_VOL_REMBOURSEMENT;
        HeureDeVolRemboursement balade = K_INIT_HEURE_DE_VOL_REMBOURSEMENT;
        HeureDeVolRemboursement sortie = K_INIT_HEURE_DE_VOL_REMBOURSEMENT;
        HeureDeVolRemboursement totaux = K_INIT_HEURE_DE_VOL_REMBOURSEMENT;

        SubventionsParPilote& operator+(const SubventionsParPilote& a)
        {
            montantSubventionEntrainement = montantSubventionEntrainement + a.montantSubventionEntrainement;
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
        QString idPilote = K_INIT_QSTRING;
        QDate date = QDate();
        QString nomPilote = K_INIT_QSTRING;
        QString prenomPilote = K_INIT_QSTRING;
        QString typeDeVol = K_INIT_QSTRING;
        QString duree = K_INIT_QSTRING;
        QString remarque = K_INIT_QSTRING;
        QString immat = K_INIT_QSTRING;
        QString activite = K_INIT_QSTRING;
        QString estSoumisCe = K_INIT_QSTRING;
        bool estSoumis = false;
        bool soumissionEstDelayee = false;
        double coutVol = 0.0;
        double montantRembourse = 0.0;
        int volId = K_INIT_INT_INVALIDE;
        int dureeEnMinutes = K_INIT_INT;
        int baladeId = K_INIT_INT_INVALIDE;
        int facture = K_INIT_INT_INVALIDE;
    };
    static const Vol K_INIT_VOL;
    typedef QList<Vol> ListeVols;

    struct DemandeRemboursement {
        QString typeDeVol = K_INIT_QSTRING;
        QString piloteId = K_INIT_QSTRING;
        double montantARembourser = 0.0;
        int annee = K_INIT_INT_INVALIDE;
        QString nomFichierFacture = K_INIT_QSTRING;
    };
    typedef QList<DemandeRemboursement> ListeDemandeRemboursement;

    struct Recette {
        QString intitule = K_INIT_QSTRING;
        QString typeDeSortie = K_INIT_QSTRING;
        double montant = 0.0;
        int annee = K_INIT_INT_INVALIDE;
    };
    typedef QList<Recette> ListeRecette;

    struct RecetteDetail {
        QString intitule = K_INIT_QSTRING;
        QString typeDeRecette = K_INIT_QSTRING;
        double montant = 0.0;
        QDate date = QDate();
        int annee = K_INIT_INT_INVALIDE;
        bool estSoumisCe = false;
        int id = K_INIT_INT_INVALIDE;
        QString remarque = K_INIT_QSTRING;
    };
    typedef QList<RecetteDetail> ListeRecetteDetail;

    struct DemandeRemboursementSoumise {
        int id = K_INIT_INT_INVALIDE;
        QDate dateDemande = QDate();
        QString nomBeneficiaire = K_INIT_QSTRING;
        QString nomPilote = K_INIT_QSTRING;
        QString piloteId = K_INIT_QSTRING;
        QString typeDeVol = K_INIT_QSTRING;
        AeroDmsTypes::ModeDeReglement modeDeReglement = AeroDmsTypes::ModeDeReglement_CHEQUE;
        QString note = K_INIT_QSTRING;
        QString mail = K_INIT_QSTRING;
        double montant = 0.0;
        double coutTotalVolAssocies = 0.0;
        int anneeVol = K_INIT_INT_INVALIDE;     
    };
    typedef QList<DemandeRemboursementSoumise> ListeDemandesRemboursementSoumises;

    struct SubventionAAnoter {
        int id = K_INIT_INT;
        QString texteActuel = K_INIT_QSTRING;
    };

    struct Sortie {
        int id = K_INIT_INT_INVALIDE;
        QString nom = K_INIT_QSTRING;
        QDate date = QDate();
    };
    typedef QList<Sortie> ListeSortie;

    struct Aeronef {
        QString immatriculation = K_INIT_QSTRING;
        QString type = K_INIT_QSTRING;
    };
    typedef QList<Aeronef> ListeAeronefs;

    struct CotisationAnnuelle {
        QString idPilote = K_INIT_QSTRING;
        int annee = K_INIT_INT_INVALIDE;
        double montantSubvention = 0.0;
        double montant = 0.0;
        bool estEnEdition = false;
        QString remarque = K_INIT_QSTRING;
    };
    
    struct DemandeRemboursementFacture {
        int id = K_INIT_INT_INVALIDE;
        QString intitule = K_INIT_QSTRING;
        double montant = 0.0;
        QString payeur = K_INIT_QSTRING;
        QString nomSortie = K_INIT_QSTRING;
        QString typeDeSortie = K_INIT_QSTRING;
        QDate date = QDate();
        int annee = K_INIT_INT_INVALIDE;
        bool soumisCe = false;
        QString nomFacture = K_INIT_QSTRING;
    };
    typedef QList<DemandeRemboursementFacture> ListeDemandeRemboursementFacture;

    struct StatsHeuresDeVol {
        QString mois = K_INIT_QSTRING;
        int minutesEntrainement = K_INIT_INT;
        int minutesSortie = K_INIT_INT;
        int minutesBalade = K_INIT_INT;
    };
    typedef QList< StatsHeuresDeVol> ListeStatsHeuresDeVol;

    struct VolSortieOuBalade {
        QString nomVol = K_INIT_QSTRING;
        double montantSubventionAttendu = K_INIT_INT;
        bool volAAuMoinsUnPaiement = K_INIT_INT;
    };
    typedef QList<VolSortieOuBalade> ListeVolSortieOuBalade;

    struct StatsHeuresDeVolParActivite {
        QString piloteId = K_INIT_QSTRING;
        QString nomPrenomPilote = K_INIT_QSTRING;

        int minutesVolAvion = K_INIT_INT;
        int minutesVolAvionElectrique = K_INIT_INT;
        int minutesVolUlm = K_INIT_INT;
        int minutesVolPlaneur = K_INIT_INT;
        int minutesVolHelicoptere = K_INIT_INT;

        double subventionVolAvion = 0.0;
        double subventionVolAvionElectrique = 0.0;
        double subventionVolUlm = 0.0;
        double subventionVolPlaneur = 0.0;
        double subventionVolHelicoptere = 0.0;

        double coutVolAvion = 0.0;
        double coutVolAvionElectrique = 0.0;
        double coutVolUlm = 0.0;
        double coutVolPlaneur = 0.0;
        double coutVolHelicoptere = 0.0;
    };
    static const StatsHeuresDeVolParActivite K_INIT_STATS_HEURES_DE_VOL_PAR_ACTIVITES;
    typedef QList< StatsHeuresDeVolParActivite> ListeStatsHeuresDeVolParActivite;

    struct ParametresMetier {
        double montantSubventionEntrainement = 0.0;
        double montantCotisationPilote = 0.0;
        double proportionRemboursementEntrainement = 0.0;
        double proportionRemboursementBalade = 0.0;
        double plafondHoraireRemboursementEntrainement = 0.0;
        double proportionParticipationBalade = 0.0;
        QString nomTresorier = K_INIT_QSTRING;
        int delaisDeGardeBdd = K_INIT_INT;
        QString objetMailDispoCheques = K_INIT_QSTRING;
        QString texteMailDispoCheques = K_INIT_QSTRING;
        QString objetMailSubventionRestante = K_INIT_QSTRING;
        QString texteMailSubventionRestante = K_INIT_QSTRING;
        QString objetMailVirementSubvention = K_INIT_QSTRING;
        QString texteMailVirementSubvention = K_INIT_QSTRING;
        QString objetMailAutresMailings = K_INIT_QSTRING;
    };

    struct ParametresImpression {
        QString imprimante = K_INIT_QSTRING;
        QPrinter::ColorMode modeCouleurImpression = QPrinter::GrayScale;
        int resolutionImpression = K_INIT_INT;
        bool forcageImpressionRecto = false;
    };

    struct ParametresSysteme {
        QString cheminStockageBdd = K_INIT_QSTRING;
        QString cheminStockageFacturesTraitees = K_INIT_QSTRING;
        QString cheminStockageFacturesATraiter = K_INIT_QSTRING;
        QString cheminSortieFichiersGeneres = K_INIT_QSTRING;
        QString nomBdd = K_INIT_QSTRING;
        QString loginSiteDaca = K_INIT_QSTRING;
        QString motDePasseSiteDaca = K_INIT_QSTRING;
        int periodiciteVerificationPresenceFactures = K_INIT_INT;
        int margesHautBas = K_INIT_INT;
        int margesGaucheDroite = K_INIT_INT;
        ParametresImpression parametresImpression;
        bool autoriserReglementParVirement = false;
        ModeFonctionnementLogiciel modeFonctionnementLogiciel = ModeFonctionnementLogiciel_INTERNE_UNIQUEMENT;
        QString adresseServeurModeExterne = "";
        QString loginServeurModeExterne = "";
        QString motDePasseServeurModeExterne = "";
        bool utiliserRessourcesHtmlInternes = false;
    };

    struct TotauxRecettes {
        double cotisations = 0.0;
        double balades = 0.0;
        double sorties = 0.0;
    };
    static const TotauxRecettes K_INIT_TOTAUX_RECETTE;

    struct DonneesFacture
    {
        QDate dateDuVol = QDate();
        QTime dureeDuVol = QTime();
        double coutDuVol = 0.0;
        QString immat = K_INIT_QSTRING;
        int pageDansLeFichierPdf = K_INIT_INT;
    };
    static const DonneesFacture K_INIT_DONNEES_FACTURE;
    typedef QList<DonneesFacture> ListeDonneesFacture;

    struct MailEtVirements
    {
        QString mail = "";
        QString pilote = "";
        QList<double> listeMontantsVirements;
        QList<int> listeIdVirements;
    };
    typedef QList<MailEtVirements> ListeMailsEtVirements;

    struct StatsPilotes
    {
        int nbBrevete = K_INIT_INT;
        int nbNonBrevete = K_INIT_INT;
        int nbOuvranDroit = K_INIT_INT;
        int nbAyantDroit = K_INIT_INT;
    };
    static const StatsPilotes K_INIT_DONNEES_STATS_PILOTES;

    struct StatsAeronef
    {
        QString immat = K_INIT_QSTRING;
        QString type = K_INIT_QSTRING;
        int nombreMinutesVol = K_INIT_INT;
    };
    static const StatsAeronef K_INIT_STAT_AERONEF;
    typedef QList<StatsAeronef> StatsAeronefs;

    struct DetailsBaladesEtSorties
    {
        int volId = K_INIT_INT_INVALIDE;
        int idSortie = K_INIT_INT_INVALIDE;
        int idRecette = K_INIT_INT_INVALIDE;
        QDate dateVol;
        int dureeVol = K_INIT_INT;
        double coutVol = 0.0;
        double montantRembouse = 0.0;
        QString nomPassagers = K_INIT_QSTRING;
        QString intituleRecette = K_INIT_QSTRING;
        double montantRecette = 0.0;
        QString nomSortie = K_INIT_QSTRING;
    };
    static const DetailsBaladesEtSorties K_INIT_DETAILS_BALADES_ET_SORTIES;
    typedef QList<DetailsBaladesEtSorties> ListeDetailsBaladesEtSorties;

    struct CleStringValeur
    {
        QString cle = K_INIT_QSTRING;
        QString texte = K_INIT_QSTRING;
        QString idPilote = K_INIT_QSTRING;
    };
    typedef QList<CleStringValeur> ListeCleStringValeur;
    struct DonneesFacturesDaca
    {
        ListeCleStringValeur listePilotes = ListeCleStringValeur();
        ListeCleStringValeur listePilotesNonConnus = ListeCleStringValeur();
        QList<QDate> listeMoisAnnees = QList<QDate>();
    };

    struct IdentifiantFacture
    {
        QDate moisAnnee = QDate();
        QString pilote = K_INIT_QSTRING;
        QString idPilote = K_INIT_QSTRING;
        QString nomPrenomPilote = K_INIT_QSTRING;
    };

    struct GroupeBaladesEtSortiesAssociees
    {
        QVector<AeroDmsTypes::DetailsBaladesEtSorties> volsUniques = QVector<AeroDmsTypes::DetailsBaladesEtSorties>();
        QVector<AeroDmsTypes::DetailsBaladesEtSorties> recettesUniques = QVector<AeroDmsTypes::DetailsBaladesEtSorties>();
    };

    struct BaladesEtSortiesParId
    {
        int idSortie = K_INIT_INT;
        int nombreDeLignes = K_INIT_INT;
        QVector<GroupeBaladesEtSortiesAssociees> baladesEtSortiesAssociees = QVector<GroupeBaladesEtSortiesAssociees>();
    };
    typedef QVector<BaladesEtSortiesParId> ListeBaladesEtSortiesParIdSortie;

    struct ResolutionEtParametresStatistiques
    {
        QSize tailleMiniImage = QSize();
        int tailleDePolice = K_INIT_INT;
    };
    static const ResolutionEtParametresStatistiques K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES;

    struct TailleFichier
    {
        quint64 compresse = K_INIT_INT;
        quint64 nonCompresse = K_INIT_INT;
    };
    struct TailleFichiers
    {
        TailleFichier total = TailleFichier();
        TailleFichier svg = TailleFichier();
        TailleFichier png = TailleFichier();
        TailleFichier autres = TailleFichier();
        TailleFichier html = TailleFichier();
    };

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
        Icone_AJOUTER_AEROCLUB,
        Icone_MAILING,
        Icone_IMPRIMER,
        Icone_OUVRIR_DOSSIER,
        Icone_FICHIER,
        Icone_FICHIER_SUIVANT,
        Icone_FICHIER_PRECEDENT,
        Icone_SCAN_AUTO_FACTURE,
        Icone_SCAN_AUTO_FACTURE_GENERIQUE,
        Icone_SCAN_AUTO_VOL,
        Icone_TELECHARGER_CLOUD,
        Icone_TELECHARGER_DOSSIER,
        Icone_TELECHARGER_FICHIER,
        Icone_FICHIER_TELECHARGE,
        Icone_CHIFFREMENT,
        Icone_AJOUTER_NOTE,
        Icone_RESSOURCE,
        Icone_RESSOURCE_EXTERNE,
        Icone_DEBUG,
        Icone_CHOIX_ANNEE,
        Icone_TOUTES_LES_ANNEES,
        Icone_UNE_ANNEE,
        Icone_INFOS_COMPLEMENTAIRES,

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

    static const int K_DPI_PAR_DEFAUT = 72;
};

Q_DECLARE_METATYPE(AeroDmsTypes::IdentifiantFacture)
Q_DECLARE_METATYPE(AeroDmsTypes::DonneesMailing)

#endif // AERODMSTYPES_H
