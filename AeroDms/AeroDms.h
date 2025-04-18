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
#ifndef AERODMS_H
#define AERODMS_H

#include <QMainWindow>
#include <QtWidgets>
#include <QPdfDocument>
#include <QPdfView>
#include <QPrinter>

#include "ManageDb.h"
#include "PdfRenderer.h"
#include "PdfDownloader.h"
#include "GestionnaireDonneesEnLigne.h"
#include "DialogueGestionPilote.h"
#include "DialogueGestionAeroclub.h"
#include "DialogueAjouterCotisation.h"
#include "DialogueAjouterSortie.h"
#include "DialogueGestionAeronefs.h"
#include "DialogueProgressionGenerationPdf.h"
#include "StatistiqueWidget.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
QT_END_NAMESPACE

class AeroDms : public QMainWindow
{
    Q_OBJECT

public:
    AeroDms(QWidget* parent = nullptr);
    ~AeroDms();

    QString fileName;

private:
    void initialiserBaseApplication();
    void lireParametresEtInitialiserBdd();
    void initialiserOngletPilotes();
    void initialiserOngletVols();
    void initialiserOngletFactures();
    void initialiserOngletRecettes();
    void initialiserOngletAjoutDepenses();
    void initialiserOngletAjoutRecettes();
    void initialiserBarreDOutils();
    void initialiserBarreDeFiltres();
    void initialiserBoitesDeDialogues();
    void initialiserMenuFichier();
    void initialiserMenuOptions();
    void initialiserMenuOutils();
    void initialiserMenuAide();
    void initialiserRaccourcisClavierSansActionIhm();
    void gererBddDistante();

    void chargerUneFactureAvecScan(const QString p_fichier, 
        const bool p_laFactureAChargerEstTelechargeeDInternet = false);
    void chargerUneFacture(const QString p_fichier, 
        const bool p_laFactureAChargerEstTelechargeeDInternet = false);
    void mettreAJourBoutonsFichierSuivantPrecedent();

    void peuplerMenuMailDemandesSubvention();
    void peuplerMenuMailPilotesDUnAerodrome();
    void peuplerListesPilotes();
    void peuplerListeSorties();
    void peuplerListeBaladesEtSorties();
    void peuplerListeDeroulanteAnnee();
    void peuplerTableVolsDetectes(const AeroDmsTypes::ListeDonneesFacture p_factures);

    void gererBoutonEditionPilote();

    const bool lePiloteEstAJourDeCotisation();

    void initialiserOngletGraphiques();
    void initialiserOngletSubventionsDemandees();

    void initialiserTableauVolsDetectes(QGridLayout* p_infosVol);

    void afficherEtatImpression(const AeroDmsTypes::EtatImpression p_etatImpression);
    const QString texteSubventionRestante();

    const double calculerCoutHoraire();
    const int calculerValeurGraphAGenererPdf();
    
    const QString rechercherDerniereDemande();

    const bool uneMaJEstDisponible(const QString p_chemin,
        const QString p_fichier);
    void mettreAJourApplication(const QString p_chemin);
    void terminerMiseAJourApplication();
    void passerLeLogicielEnLectureSeule(const bool p_lectureSeuleEstDemandee,
        const bool p_figerLesListes);
    void preparerStatusBar();
    void demanderFermetureSplashscreen();
    void ouvrirSplashscreen();

    void verifierSignatureNumerisee();
    void verifierDispoIdentifiantsDaca();
    void ajouterPilotesDansMenuFacturesDaca(QMenu* p_menu, 
        const AeroDmsTypes::ListeCleStringValeur p_listePilote, 
        const QDate p_mois);
    const int recupererAnneeAGenerer();

    const QString elaborerCheminRessourcesHtml();

    bool eventFilter(QObject* object, 
        QEvent* event);
    void closeEvent(QCloseEvent* event);

    //Classes gestionnaires et actionnaires
    ManageDb* db = nullptr;
    PdfRenderer* pdf = nullptr;
    PdfDownloader* pdfdl = nullptr;
    GestionnaireDonneesEnLigne* gestionnaireDonneesEnLigne = nullptr;

    AeroDmsTypes::Signature signature = AeroDmsTypes::Signature_SANS;
    AeroDmsTypes::TypeGenerationPdf typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_TOUTES;
    AeroDmsTypes::ListeDonneesFacture factures;

    //Fenêtres
    DialogueGestionPilote* dialogueGestionPilote = nullptr;
    DialogueGestionAeroclub* dialogueGestionAeroclub = nullptr;
    DialogueAjouterCotisation* dialogueAjouterCotisation = nullptr;
    DialogueAjouterSortie* dialogueAjouterSortie = nullptr;
    DialogueGestionAeronefs* dialogueGestionAeronefs = nullptr;

    QPdfDocument* pdfDocument = nullptr;
    QPdfView* pdfView = nullptr;

    QTabWidget* mainTabWidget = nullptr;

    QTableWidget* vuePilotes = nullptr;
    QTableWidget* vueVols = nullptr;
    QTableWidget* vueFactures = nullptr;
    QTableWidget* vueRecettes = nullptr;
    QTableWidget* vueSubventions = nullptr;
    QTableWidget* vueVolsDetectes = nullptr;

    //Onglet "Ajouter dépense"
    QWidget* widgetAjoutVol = nullptr;
    QComboBox* typeDeVol = nullptr;
    QComboBox* choixPilote = nullptr;
    QLineEdit* aeroclubPiloteSelectionne = nullptr;
    QComboBox* activite = nullptr;
    QDateTimeEdit* dateDuVol = nullptr;
    QTimeEdit* dureeDuVol = nullptr;
    QDoubleSpinBox* prixDuVol = nullptr;
    QComboBox* choixBalade = nullptr;
    QLineEdit* remarqueVol = nullptr;
    QLineEdit* immat = nullptr;
    QPushButton* validerLeVol = nullptr;
    QPushButton* validerLesVols = nullptr;
    QPushButton* supprimerLeVolSelectionne = nullptr;

    QComboBox* choixPayeur = nullptr;
    QDateEdit* dateDeFacture = nullptr;
    QDoubleSpinBox* montantFacture = nullptr;
    QLineEdit* remarqueFacture = nullptr;
    QComboBox* choixBaladeFacture = nullptr;
    QPushButton* validerLaFacture = nullptr;

    //Onglet "Ajouter recette"
    QWidget* widgetAjoutRecette = nullptr;
    QListWidget* listeBaladesEtSorties = nullptr;
    QComboBox* typeDeRecette = nullptr;
    QLineEdit* nomEmetteurChequeRecette = nullptr;
    QLineEdit* banqueNumeroChequeRecette = nullptr;
    QDoubleSpinBox* montantRecette = nullptr;
    QPushButton* validerLaRecette = nullptr;

    //Listes déroulantes de la barre principale
    QAction* selecteurFiltreInformationsSupplementaires = nullptr;
    QComboBox* listeDeroulanteAnnee = nullptr;
    QAction* actionListeDeroulanteAnnee = nullptr;
    QComboBox* listeDeroulantePilote = nullptr;
    QAction* actionListeDeroulantePilote = nullptr;
    QComboBox* listeDeroulanteType = nullptr;
    QAction* actionListeDeroulanteType = nullptr;
    QComboBox* listeDeroulanteElementsSoumis = nullptr;
    QAction* actionListeDeroulanteElementsSoumis = nullptr;
    QComboBox* listeDeroulanteStatistique = nullptr;
    QAction* actionListeDeroulanteStatistique = nullptr;

    //Barre de progression
    DialogueProgressionGenerationPdf* progressionGenerationPdf = nullptr;
    QProgressBar* barreDeProgressionStatusBar = nullptr;

    //Boutons
    QAction* boutonGenerePdf = nullptr;
    QAction* boutonGenerePdfRecapHdv = nullptr;
    QAction* miseAJourAction = nullptr;
    QAction* boutonModeDebug = nullptr;
    QAction* boutonAjouterUnVol = nullptr;
    QAction* boutonAjouterUnAeroclub = nullptr;
    QAction* boutonAjouterCotisation = nullptr;
    QAction* boutonAjouterPilote = nullptr;
    QAction* boutonAjouterSortie = nullptr;
    QAction* fichierPrecedent = nullptr;
    QAction* fichierSuivant = nullptr;

    //QAction pour raccourcis claviers sans item d'IHM
    QShortcut* ongletSuivantRaccourciClavier = nullptr;
    QShortcut* ongletPrecedentRaccourciClavier = nullptr;

    //Menu Fichier
    QMenu* menuOuvrirAutreDemande = nullptr;

    //Menu options
    QAction* boutonAucuneSignature = nullptr;
    QAction* boutonSignatureManuelle = nullptr;
    QAction* boutonSignatureNumerique = nullptr;
    QAction* boutonDemandesAGenererToutes = nullptr;
    QAction* boutonDemandesAGenererRecettes = nullptr;
    QAction* boutonDemandesAGenererDepenses = nullptr;
    QMenu* menuDemandesAGenererAnnees = nullptr;
    QAction* boutonFusionnerLesPdf = nullptr;
    QAction* boutonNePasFusionnerLesPdf = nullptr;
    QAction* boutonActivationScanAutoFactures = nullptr;
    QAction* boutonParametresDuLogiciel = nullptr;
    QAction* boutonOptionRecapAnnuelRecettes = nullptr;
    QAction* boutonOptionRecapAnnuelBaladesSorties = nullptr;
    QAction* boutonGraphRecapAnnuelHeuresAnnuelles = nullptr;
    QAction* boutonGraphRecapAnnuelHeuresParPilote = nullptr;
    QAction* boutonGraphRecapAnnuelEurosParPilote = nullptr;
    QAction* boutonGraphRecapAnnuelHeuresParTypeDeVol = nullptr;
    QAction* boutonGraphRecapAnnuelEurosParTypeDeVol = nullptr;
    QAction* boutonGraphRecapAnnuelHeuresParActivite = nullptr;
    QAction* boutonGraphRecapAnnuelEurosParActivite = nullptr;
    QAction* boutonGraphRecapAnnuelStatutsDesPilotes = nullptr;
    QAction* boutonGraphRecapAnnuelAeronefs = nullptr;
    QAction* boutonGraphRecapAnnuelSelectionnerTousLesGraphs = nullptr;
    QAction* boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap = nullptr;
    QAction* boutonGraphResolutionFullHd = nullptr;
    QAction* boutonGraphResolutionQhd = nullptr;
    QAction* boutonGraphResolution4k = nullptr;
    QAction* boutonGraphRatioIso216 = nullptr;
    QAction* boutonGraphRatio16x9 = nullptr;
    QAction* boutonGraphiquesVolsSubventionnesUniquement = nullptr;
    QAction* boutonGraphiquesExclurePlaneur = nullptr;
    QAction* boutonGraphiquesExclureUlm = nullptr;
    QAction* boutonGraphiquesExclureAvion = nullptr;
    QAction* boutonGraphiquesExclureHelico = nullptr;

    QMenu* facturesDaca = nullptr;
    QAction* boutonChargerFacturesDaca = nullptr;
    QMenu* menuOption = nullptr;
    QMenu* menuOptionsRecapAnnuel = nullptr;
    QMenu* graphiquesDuRecapAnnuel = nullptr;
    QMenu* resolutionGraphiques = nullptr;

    //Menu outils
    QAction* mailingPilotesAyantCotiseCetteAnnee = nullptr;
    QAction* mailingPilotesActifs = nullptr;
    QAction* mailingPilotesActifsAyantCotiseCetteAnnee = nullptr;
    QAction* mailingPilotesActifsBrevetes = nullptr;
    QAction* mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement = nullptr;
    QAction* mailingPilotesDerniereDemandeSubvention = nullptr;
    QMenu* menuMailDemandesSubvention = nullptr;
    QMenu* menuMailPilotesSubventionVerseeParVirement = nullptr;
    QMenu* menuMailPilotesDUnAerodrome = nullptr;
    QMenu* menuMailPilotesDUnAerodromeActif = nullptr;
    QMenu* menuMailPilotesDUnAerodromeActifBreveteMoteur = nullptr;
    
    QAction* scanAutoOpenFlyer = nullptr;
    QAction* scanAutoAerogest = nullptr;
    QAction* scanAutoAca = nullptr;
    QAction* scanAutoDaca = nullptr;
    QAction* scanAutoSepavia = nullptr;
    QAction* scanAutoUaca = nullptr;
    QAction* scanAutoAtvv = nullptr;
    QAction* scanAutoGenerique1Passe = nullptr;
    QAction* scanAutoGenerique = nullptr;
    QAction* scanAutoCsv = nullptr;

    QAction* boutonEditerLePiloteSelectionne = nullptr;
    QAction* boutonEditerUnAeroclub = nullptr;
    QAction* boutonGestionAeronefs = nullptr;
    QAction* boutonMettreAJourAerodromes = nullptr;

    //Données internes
    //Gestion des actions métier
    QString piloteAEditer = "";
    double montantSubventionDejaAlloue = 0.0;
    int anneeAEditer = 0;
    int volAEditer = AeroDmsTypes::K_INIT_INT_INVALIDE;
    AeroDmsTypes::SubventionAAnoter subventionAAnoter;
    bool volPartiellementEditable = false;
    int factureIdEnBdd = 0;
    QString fichierAImprimer = "";
    QString dossierSortieGeneration = "";
    QString cheminDeLaFactureCourante = "";
    int idFactureDetectee = AeroDmsTypes::K_INIT_INT_INVALIDE;
    bool scanAutomatiqueDesFacturesEstActif = true;

    //Etats internes application
    bool logicielEnModeLectureSeule = false;
    bool miseAJourApplicationEstEnCours = false;
    bool verificationDeNouvelleFacturesAChargerEnLigneEstEffectue = false;
    enum EtapeFermeture
    {
        EtapeFermeture_NON_DEMANDE,
        EtapeFermeture_FERMETURE_BDD,
        EtapeFermeture_BDD_FERMEE
    };
    EtapeFermeture etapeFermetureEnCours = EtapeFermeture_NON_DEMANDE;

    //Gestion chargement factures DACA
    bool factureRecupereeEnLigneEstNonTraitee = false;
    QAction* actionFactureDacaEnCours = nullptr;

    //Parametres
    AeroDmsTypes::ParametresMetier parametresMetiers;
    AeroDmsTypes::ParametresSysteme parametresSysteme;

    //Statistiques
    QWidget* widgetGraphiques;
    StatistiqueWidget* m_activeWidget = nullptr;
    QWidget* m_contentArea = nullptr;
    QHBoxLayout* graphiques = nullptr;

    QSplashScreen* splash = nullptr;

    //Textes de certains éléments d'IHM
    const QString texteTitreQMenuFacturesDaca = tr("Récupération des factures DACA");
    const QString texteBoutonActiverModeDebogage = tr("Activer le mode &débogage");

protected:
    void resizeEvent(QResizeEvent* event);

signals:
    void toucheEchapEstAppuyee();
    void debuterImpression(int p_nombreDePagesAImprimer);
    void mettreAJourNombreDePagesImprimees(int p_nombreDePagesTraitees);
    void demanderVerificationEtTelechargementBdd();
    void autoriserSortieLogiciel();

public slots:
    void fermerSplashscreen();
    void selectionnerUneFacture();
    void enregistrerUnVol();
    void enregistrerLesVols();
    void supprimerLeVolDeLaVueVolsDetectes();
    void enregistrerUneFacture();
    void enregistrerUneRecette();
    void genererPdf();
    void genererPdfRecapHdV();
    void prevaliderDonnnesSaisies();
    void prevaliderDonnneesSaisiesRecette();
    void traiterClicSurVolBaladesEtSorties();
    void changerInfosVolSurSelectionTypeVol();
    void chargerBaladesSorties();
    void ajouterUnPilote();
    void ajouterUnPiloteEnBdd();
    void ajouterUnAeroclubEnBdd();
    void ajouterUneSortie();
    void ajouterUneSortieEnBdd();
    void ajouterUneCotisation();
    void ajouterUneCotisationEnBdd();
    void ajouterUnAeroclub();
    void ouvrirGestionAeronefs();
    void peuplerListesEtTables();
    void peuplerTablePilotes();
    void peuplerTableVols();
    void peuplerTableFactures();
    void peuplerTableRecettes();
    void peuplerTableSubventionsDemandees();
    void ajusterTableSubventionsDemandeesAuContenu();
    void ouvrirFenetreProgressionGenerationPdf(const int p_nombreDeFacturesATraiter);
    void mettreAJourFenetreProgressionGenerationPdf(const int p_nombreDeFacturesTraitees);
    void mettreAJourBarreStatusFinGenerationPdf(const QString p_cheminDossier, const QString p_cheminFichierPdfMerge);
    void mettreAJourEchecGenerationPdf();
    void aPropos();
    void ouvrirAide();
    void menuContextuelPilotes(const QPoint& pos);
    void menuContextuelSubvention(const QPoint& pos);
    void menuContextuelVols(const QPoint& pos);
    void editerPilote();
    void editerAeroclub();
    void editerCotisation();
    void editerVol();
    void supprimerVol();
    void ajouterUneNoteSubvention();
    void switchModeDebug();
    void switchScanAutomatiqueDesFactures();
    void convertirHeureDecimalesVersHhMm();
    void peuplerStatistiques();
    void changerModeSignature();
    void changerDemandesAGenerer();
    void changerFusionPdf();
    void changerResolutionExportGraphiques();
    void changerRatioExportGraphiques();
    void maintenirMenuSelectionGraphsOuvert();
    void selectionnerTousLesGraphsPourRecapAnnuel();
    void envoyerMail();
    void mettreAJourInfosSurSelectionPilote();
    void volsSelectionnes();
    void chargerUnVolDetecte(int row, int column);
    void deselectionnerVolDetecte();
    void scannerUneFactureSelonMethodeChoisie();
    void recupererVolDepuisCsv();
    void ouvrirPdfDemandeSuvbvention();
    void ouvrirDossierDemandesSubventions();
    void ouvrirDossierFichierVenantDEtreGenere();
    void peuplerMenuAutreDemande();
    void ouvrirPdfGenere();
    void ouvrirUnFichierDeDemandeDeSubvention();
    void gererChangementOnglet();
    void verifierPresenceDeMiseAjour();
    void initialiserGestionnaireTelechargement();
    void switchOnglet();
    void switchAffichageInfosComplementaires();
    void imprimerLaDemande();
    void imprimerLaDemandeAgrafage();
    void imprimerLaDerniereDemande();
    void imprimerLaDerniereDemandeAgrafage();
    void ouvrirDialogueParametresApplication();
    void enregistrerParametresApplication( const AeroDmsTypes::ParametresMetier p_parametresMetiers,
        const AeroDmsTypes::ParametresSysteme p_parametresSysteme);
    void detruireFenetreProgressionGenerationPdf();
    void mettreAJourAerodromes();
    void afficherProgressionMiseAJourAerodromes(int nombreTotal,
        int nombreTraite,
        int nombreCree,
        int nombreMisAJour);
    void masquerBarreDeProgressionDeLaStatusBar();
    void gererSelectionAnneeAGenerer();

    //Gestion du chargement de la BDD en ligne
    void verifierVersionBddSuiteChargement();
    void afficherStatusDebutTelechargementBdd();
    void signalerBaseDeDonneesBloqueeParUnAutreUtilisateur(const QString p_nomVerrou,
        const QDateTime p_heureVerrouInitial,
        const QDateTime p_heureDerniereVerrou);
    void passerLeLogicielEnLectureSeule();
    void sortirLeLogicielDeLectureSeule();

    //Téléchargement de factures
    void gererChargementDonneesSitesExternes(const AeroDmsTypes::EtatRecuperationDonneesFactures p_etatRecuperation);
    void demanderTelechargementFactureDaca();
    void demanderTelechargementPremiereFactureDaca();
    void demanderTelechagementFactureSuivanteOuPrecedente();
    void chargerListeFacturesDaca();

};
#endif // AERODMS_H
