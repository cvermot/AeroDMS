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

    const float calculerCoutHoraire();
    const int calculerValeurGraphAGenererPdf();
    
    const QString rechercherDerniereDemande();

    const bool uneMaJEstDisponible(const QString p_chemin,
        const QString p_fichier);
    void mettreAJourApplication(const QString p_chemin);
    void terminerMiseAJourApplication();
    void passerLeLogicielEnLectureSeule();
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
    ManageDb* db;
    PdfRenderer* pdf;
    PdfDownloader* pdfdl;

    AeroDmsTypes::Signature signature = AeroDmsTypes::Signature_SANS;
    AeroDmsTypes::TypeGenerationPdf typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_TOUTES;
    AeroDmsTypes::ListeDonneesFacture factures;

    //Fenêtres
    DialogueGestionPilote* dialogueGestionPilote;
    DialogueGestionAeroclub* dialogueGestionAeroclub;
    DialogueAjouterCotisation* dialogueAjouterCotisation;
    DialogueAjouterSortie* dialogueAjouterSortie;
    DialogueGestionAeronefs* dialogueGestionAeronefs;

    QPdfDocument* pdfDocument;
    QPdfView* pdfView;

    QTabWidget* mainTabWidget;

    QTableWidget* vuePilotes;
    QTableWidget* vueVols;
    QTableWidget* vueFactures;
    QTableWidget* vueRecettes;
    QTableWidget* vueSubventions;
    QTableWidget* vueVolsDetectes;

    //Onglet "Ajouter dépense"
    QWidget* widgetAjoutVol;
    QComboBox* typeDeVol;
    QComboBox* choixPilote;
    QLineEdit* aeroclubPiloteSelectionne;
    QComboBox* activite;
    QDateTimeEdit* dateDuVol;
    QTimeEdit* dureeDuVol;
    QDoubleSpinBox* prixDuVol;
    QComboBox* choixBalade;
    QLineEdit* remarqueVol;
    QLineEdit* immat;
    QPushButton* validerLeVol;
    QPushButton* validerLesVols;
    QPushButton* supprimerLeVolSelectionne;

    QComboBox* choixPayeur;
    QDateEdit* dateDeFacture;
    QDoubleSpinBox* montantFacture;
    QLineEdit* remarqueFacture;
    QComboBox* choixBaladeFacture;
    QPushButton* validerLaFacture;

    //Onglet "Ajouter recette"
    QWidget* widgetAjoutRecette;
    QListWidget* listeBaladesEtSorties;
    QComboBox* typeDeRecette;
    QLineEdit* nomEmetteurChequeRecette;
    QLineEdit* banqueNumeroChequeRecette;
    QDoubleSpinBox* montantRecette;
    QPushButton* validerLaRecette;

    //Listes déroulantes de la barre principale
    QComboBox* listeDeroulanteAnnee;
    QAction* actionListeDeroulanteAnnee;
    QComboBox* listeDeroulantePilote;
    QAction* actionListeDeroulantePilote;
    QComboBox* listeDeroulanteType;
    QAction* actionListeDeroulanteType;
    QComboBox* listeDeroulanteElementsSoumis;
    QAction* actionListeDeroulanteElementsSoumis;
    QComboBox* listeDeroulanteStatistique;
    QAction* actionListeDeroulanteStatistique;

    //Barre de progression
    DialogueProgressionGenerationPdf* progressionGenerationPdf = nullptr;
    QProgressBar* barreDeProgressionStatusBar;

    //Boutons
    QAction* boutonGenerePdf;
    QAction* boutonGenerePdfRecapHdv;
    QAction* miseAJourAction;
    QAction* boutonModeDebug;
    QAction* boutonAjouterUnVol;
    QAction* boutonAjouterUnAeroclub;
    QAction* boutonAjouterCotisation;
    QAction* boutonAjouterPilote;
    QAction* boutonAjouterSortie;
    QAction* fichierPrecedent;
    QAction* fichierSuivant;

    //QAction pour raccourcis claviers sans item d'IHM
    QShortcut* ongletSuivantRaccourciClavier;
    QShortcut* ongletPrecedentRaccourciClavier;

    //Menu Fichier
    QMenu* menuOuvrirAutreDemande;

    //Menu options
    QAction* boutonAucuneSignature;
    QAction* boutonSignatureManuelle;
    QAction* boutonSignatureNumerique;
    QAction* boutonDemandesAGenererToutes;
    QAction* boutonDemandesAGenererRecettes;
    QAction* boutonDemandesAGenererDepenses;
    QMenu* menuDemandesAGenererAnnees;
    QAction* boutonFusionnerLesPdf;
    QAction* boutonNePasFusionnerLesPdf;
    QAction* boutonActivationScanAutoFactures;
    QAction* boutonParametresDuLogiciel;
    QAction* boutonOptionRecapAnnuelRecettes;
    QAction* boutonOptionRecapAnnuelBaladesSorties;
    QAction* boutonGraphRecapAnnuelHeuresAnnuelles;
    QAction* boutonGraphRecapAnnuelHeuresParPilote;
    QAction* boutonGraphRecapAnnuelEurosParPilote;
    QAction* boutonGraphRecapAnnuelHeuresParTypeDeVol;
    QAction* boutonGraphRecapAnnuelEurosParTypeDeVol;
    QAction* boutonGraphRecapAnnuelHeuresParActivite;
    QAction* boutonGraphRecapAnnuelEurosParActivite;
    QAction* boutonGraphRecapAnnuelStatutsDesPilotes;
    QAction* boutonGraphRecapAnnuelAeronefs;
    QAction* boutonGraphRecapAnnuelSelectionnerTousLesGraphs;
    QAction* boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap;
    QAction* boutonGraphResolutionFullHd;
    QAction* boutonGraphResolutionQhd;
    QAction* boutonGraphResolution4k;
    QAction* boutonGraphRatioIso216;
    QAction* boutonGraphRatio16x9;
    QAction* boutonGraphiquesVolsSubventionnesUniquement;
    QAction* boutonGraphiquesExclurePlaneur;
    QAction* boutonGraphiquesExclureUlm;
    QAction* boutonGraphiquesExclureAvion;
    QAction* boutonGraphiquesExclureHelico;

    QMenu* facturesDaca;
    QAction* boutonChargerFacturesDaca;
    QMenu* menuOption;
    QMenu* menuOptionsRecapAnnuel;
    QMenu* graphiquesDuRecapAnnuel;
    QMenu* resolutionGraphiques;

    //Menu outils
    QAction* mailingPilotesAyantCotiseCetteAnnee;
    QAction* mailingPilotesActifs;
    QAction* mailingPilotesActifsAyantCotiseCetteAnnee;
    QAction* mailingPilotesActifsBrevetes;
    QAction* mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement;
    QAction* mailingPilotesDerniereDemandeSubvention;
    QMenu* menuMailDemandesSubvention;
    QMenu* menuMailPilotesDUnAerodrome;
    QMenu* menuMailPilotesDUnAerodromeActif;
    QMenu* menuMailPilotesDUnAerodromeActifBreveteMoteur;
    
    QAction* scanAutoOpenFlyer;
    QAction* scanAutoAerogest;
    QAction* scanAutoAca;
    QAction* scanAutoDaca;
    QAction* scanAutoSepavia;
    QAction* scanAutoUaca;
    QAction* scanAutoAtvv;
    QAction* scanAutoGenerique1Passe;
    QAction* scanAutoGenerique;
    QAction* scanAutoCsv;

    QAction* boutonEditerLePiloteSelectionne;
    QAction* boutonEditerUnAeroclub;
    QAction* boutonGestionAeronefs;
    QAction* boutonMettreAJourAerodromes;

    //Données internes
    //Gestion des actions métier
    QString piloteAEditer = "";
    float montantSubventionDejaAlloue = 0.0;
    int anneeAEditer = 0;
    int volAEditer = -1;
    bool volPartiellementEditable = false;
    int factureIdEnBdd = 0;
    QString fichierAImprimer = "";
    QString dossierSortieGeneration = "";
    QString cheminDeLaFactureCourante = "";
    int idFactureDetectee = -1;
    bool scanAutomatiqueDesFacturesEstActif = true;

    //Etats internes application
    bool logicielEnModeLectureSeule = false;
    bool miseAJourApplicationEstEnCours = false;

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
    QHBoxLayout* graphiques;

    QSplashScreen* splash;

    //Textes de certains éléments d'IHM
    const QString texteTitreQMenuFacturesDaca = tr("Récupération des factures DACA");
    const QString texteBoutonActiverModeDebogage = tr("Activer le mode &débogage");

signals:
    void toucheEchapEstAppuyee();
    void debuterImpression(int p_nombreDePagesAImprimer);
    void mettreAJourNombreDePagesImprimees(int p_nombreDePagesTraitees);

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
    void peuplerTablePilotes();
    void peuplerTableVols();
    void peuplerTableFactures();
    void peuplerTableRecettes();
    void peuplerTableSubventionsDemandees();
    void ouvrirFenetreProgressionGenerationPdf(const int p_nombreDeFacturesATraiter);
    void mettreAJourFenetreProgressionGenerationPdf(const int p_nombreDeFacturesTraitees);
    void mettreAJourBarreStatusFinGenerationPdf(const QString p_cheminDossier, const QString p_cheminFichierPdfMerge);
    void mettreAJourEchecGenerationPdf();
    void aPropos();
    void ouvrirAide();
    void menuContextuelPilotes(const QPoint& pos);
    void editerPilote();
    void editerAeroclub();
    void editerCotisation();
    void editerVol();
    void supprimerVol();
    void menuContextuelVols(const QPoint& pos);
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

    //Téléchargement de factures
    void gererChargementDonneesSitesExternes(const AeroDmsTypes::EtatRecuperationDonneesFactures p_etatRecuperation);
    void demanderTelechargementFactureDaca();
    void demanderTelechargementPremiereFactureDaca();
    void demanderTelechagementFactureSuivanteOuPrecedente();
    void chargerListeFacturesDaca();

};
#endif // AERODMS_H
