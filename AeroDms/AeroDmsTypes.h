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
#include <QPrinter>

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

    enum SubventionDemandeeTableElement {
        SubventionDemandeeTableElement_DATE = 0x0,
        SubventionDemandeeTableElement_PILOTE = 0x1,
        SubventionDemandeeTableElement_BENEFICIAIRE = 0x2,
        SubventionDemandeeTableElement_TYPE_DEMANDE = 0x3,
        SubventionDemandeeTableElement_MONTANT = 0x4,
        SubventionDemandeeTableElement_MONTANT_VOL = 0x5,
        SubventionDemandeeTableElement_MODE_DE_REGLEMENT = 0x6,
        SubventionDemandeeTableElement_ID_DEMANDE = 0x7,
        SubventionDemandeeTableElementTableElement_NB_COLONNES = 0x8,
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

    struct DemandeEnCoursDeTraitement {
        PdfTypeDeDemande typeDeDemande = PdfTypeDeDemande_HEURE_DE_VOL;
        QString idPilote = "";
        QString typeDeVol = "";
        QString nomBeneficiaire = "";
        float montant = 0.0;
        ModeDeReglement modeDeReglement = ModeDeReglement_CHEQUE;
        int annee = -1;
        QString nomTresorier = "";
        QString nomFichier = "";
        QStringList listeFactures = QStringList();
        AeroDmsTypes::TypeGenerationPdf typeDeGenerationDemandee = AeroDmsTypes::TypeGenerationPdf_TOUTES;
        AeroDmsTypes::Signature typeDeSignatureDemandee = AeroDmsTypes::Signature_SANS;
        bool mergerTousLesPdf = false;
        bool recapHdVAvecRecettes = false;
        bool recapHdVAvecBaladesEtSorties = false;
        bool virementEstAutorise = false;
        int recapHdvGraphAGenerer = 0;
        //int idFacture;
    };

    struct Pilote {
        QString idPilote;
        QString nom;
        QString prenom;
        QString aeroclub;
        int idAeroclub;
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

    struct Club {
        int idAeroclub;
        QString aeroclub;
        QString raisonSociale;
        QString iban;
        QString bic;
    };
    static const Club K_INIT_CLUB;
    typedef QList<Club> ListeAeroclubs;

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
        QString idPilote;
        int annee;
        QString nom;
        QString prenom;
        QString mail;
        float montantSubventionEntrainement;
        HeureDeVolRemboursement entrainement;
        HeureDeVolRemboursement balade;
        HeureDeVolRemboursement sortie;
        HeureDeVolRemboursement totaux;

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
        int facture;
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

    struct DemandeRemboursementSoumise {
        int id;
        QDate dateDemande;
        QString nomBeneficiaire;
        QString nomPilote;
        QString piloteId;
        QString typeDeVol;
        AeroDmsTypes::ModeDeReglement modeDeReglement;
        float montant;
        float coutTotalVolAssocies;
        int anneeVol;     
    };
    typedef QList<DemandeRemboursementSoumise> ListeDemandesRemboursementSoumises;

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

        float subventionVolAvion;
        float subventionVolAvionElectrique;
        float subventionVolUlm;
        float subventionVolPlaneur;
        float subventionVolHelicoptere;

        float coutVolAvion;
        float coutVolAvionElectrique;
        float coutVolUlm;
        float coutVolPlaneur;
        float coutVolHelicoptere;
    };
    static const StatsHeuresDeVolParActivite K_INIT_STATS_HEURES_DE_VOL_PAR_ACTIVITES;
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
        QString objetMailDispoCheques;
        QString texteMailDispoCheques;
        QString objetMailSubventionRestante;
        QString texteMailSubventionRestante;
        QString objetMailAutresMailings;
    };

    struct ParametresImpression {
        QString imprimante;
        QPrinter::ColorMode modeCouleurImpression;
        int resolutionImpression;
        bool forcageImpressionRecto;
    };

    struct ParametresSysteme {
        QString cheminStockageBdd;
        QString cheminStockageFacturesTraitees;
        QString cheminStockageFacturesATraiter;
        QString cheminSortieFichiersGeneres;
        QString nomBdd;
        QString loginSiteDaca;
        QString motDePasseSiteDaca;
        int periodiciteVerificationPresenceFactures;
        int margesHautBas;
        int margesGaucheDroite;
        ParametresImpression parametresImpression;
        bool autoriserReglementParVirement;
        bool utiliserRessourcesHtmlInternes;
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

    struct CleStringValeur
    {
        QString cle = "";
        QString texte = "";
        QString idPilote = "";
    };
    typedef QList<CleStringValeur> ListeCleStringValeur;
    struct DonneesFacturesDaca
    {
        ListeCleStringValeur listePilotes;
        ListeCleStringValeur listePilotesNonConnus;
        QList<QDate> listeMoisAnnees;
    };

    struct IdentifiantFacture
    {
        QDate moisAnnee;
        QString pilote;
        QString idPilote;
        QString nomPrenomPilote;
    };

    struct GroupeBaladesEtSortiesAssociees
    {
        QVector<AeroDmsTypes::DetailsBaladesEtSorties> volsUniques;
        QVector<AeroDmsTypes::DetailsBaladesEtSorties> recettesUniques;
    };

    struct BaladesEtSortiesParId
    {
        int idSortie = 0;
        int nombreDeLignes = 0;
        QVector<GroupeBaladesEtSortiesAssociees> baladesEtSortiesAssociees = QVector<GroupeBaladesEtSortiesAssociees>();
    };
    typedef QVector<BaladesEtSortiesParId> ListeBaladesEtSortiesParIdSortie;

    struct ResolutionEtParametresStatistiques
    {
        QSize tailleMiniImage;
        int tailleDePolice;
    };
    static const ResolutionEtParametresStatistiques K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES;

    struct TailleFichier
    {
        quint64 compresse = 0;
        quint64 nonCompresse = 0;
    };
    struct TailleFichiers
    {
        TailleFichier total;
        TailleFichier svg;
        TailleFichier png;
        TailleFichier autres;
        TailleFichier html;
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
        Icone_RESSOURCE,
        Icone_RESSOURCE_EXTERNE,
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

    static const int K_DPI_PAR_DEFAUT = 72;
};

Q_DECLARE_METATYPE(AeroDmsTypes::IdentifiantFacture)

#endif // AERODMSTYPES_H
