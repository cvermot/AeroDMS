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
#ifndef AERODMS_H
#define AERODMS_H

#include <QMainWindow>
#include <QtWidgets>
#include <QPdfDocument>
#include <QPdfView>
#include <QPrinter>

#include "ManageDb.h"
#include "PdfRenderer.h"
#include "DialogueGestionPilote.h"
#include "DialogueAjouterCotisation.h"
#include "DialogueAjouterSortie.h"
#include "DialogueGestionAeronefs.h"
#include "DialogueProgressionGenerationPdf.h"
#include "DialogueProgressionImpression.h"
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

    void chargerUneFacture(QString p_fichier);

    void peuplerListesPilotes();
    void peuplerListeSorties();
    void peuplerListeBaladesEtSorties();
    void peuplerListeDeroulanteAnnee();
    void peuplerTableVolsDetectes(const AeroDmsTypes::ListeDonneesFacture p_factures);

    bool lePiloteEstAJourDeCotisation();

    void initialiserOngletGraphiques();
    void initialiserOngletSubventionsDemandees();

    void initialiserTableauVolsDetectes(QGridLayout* p_infosVol);

    void imprimer(QPrinter& p_printer);
    bool selectionnerImprimante(QPrinter& p_printer);
    void ouvrirFenetreProgressionImpression(const int p_nombreDeFichiersAImprimer);

    float calculerCoutHoraire();
    int calculerValeurGraphAGenererPdf();

    QString rechercherDerniereDemande();

    bool uneMaJEstDisponible(const QString p_chemin,
                             const QString p_fichier);
    void mettreAJourApplication(const QString p_chemin);
    void terminerMiseAJourApplication();
    void passerLeLogicielEnLectureSeule();
    void demanderFermetureSplashscreen();
    void ouvrirSplashscreen();

    bool eventFilter(QObject* object, QEvent* event);

    ManageDb* db;
    PdfRenderer* pdf;
    AeroDmsTypes::Signature signature = AeroDmsTypes::Signature_SANS;
    AeroDmsTypes::TypeGenerationPdf typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_TOUTES;
    AeroDmsTypes::ListeDonneesFacture factures;
    int idFactureDetectee = -1;
    bool scanAutomatiqueDesFacturesEstActif = true;
    bool logicielEnModeLectureSeule = false;

    //Fenêtres
    DialogueGestionPilote* dialogueGestionPilote;
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
    DialogueProgressionImpression* progressionImpression = nullptr;

    //Bouttons
    QAction* bouttonGenerePdf;
    QAction* bouttonGenerePdfRecapHdv;
    QAction* miseAJourAction;
    QAction* boutonModeDebug;
    QAction* bouttonAjouterUnVol;
    QAction* bouttonAjouterCotisation;
    QAction* bouttonAjouterPilote;
    QAction* bouttonAjouterSortie;

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
    QAction* boutonFusionnerLesPdf;
    QAction* boutonNePasFusionnerLesPdf;
    QAction* boutonActivationScanAutoFactures;
    QAction* boutonParametresDuLogiciel;
    QAction* boutonOptionRecapAnnuelRecettes;
    QAction* boutonOptionRecapAnnuelBaladesSorties;
    QAction* boutonGraphRecapAnnuelHeuresAnnuelles;
    QAction* boutonGraphRecapAnnuelHeuresParPilote;
    QAction* boutonGraphRecapAnnuelHeuresParTypeDeVol;
    QAction* boutonGraphRecapAnnuelHeuresParActivite;
    QAction* boutonGraphRecapAnnuelStatutsDesPilotes;
    QAction* boutonGraphRecapAnnuelAeronefs;
    QAction* boutonGraphRecapAnnuelSelectionnerTousLesGraphs;
    QAction* boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap;
    QAction* boutonGraphResolutionFullHd;
    QAction* boutonGraphResolutionQhd;
    QAction* boutonGraphResolution4k;

    //Menu outils
    QAction* mailingPilotesAyantCotiseCetteAnnee;
    QAction* mailingPilotesActifs;
    QAction* mailingPilotesActifsAyantCotiseCetteAnnee;
    QAction* mailingPilotesActifsBrevetes;
    QAction* mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement;
    QAction* mailingPilotesDerniereDemandeSubvention;
    
    QAction* scanAutoOpenFlyer;
    QAction* scanAutoAerogest;
    QAction* scanAutoAca;
    QAction* scanAutoDaca;
    QAction* scanAutoSepavia;
    QAction* scanAutoUaca;
    QAction* scanAutoGenerique1Passe;
    QAction* scanAutoGenerique;
    QAction* scanAutoCsv;

    QAction* boutonEditerLePiloteSelectionne;

    //Données internes
    QString piloteAEditer = "";
    float montantSubventionDejaAlloue = 0.0;
    int anneeAEditer = 0;
    int volAEditer = -1;
    bool volPartiellementEditable = false;
    int factureIdEnBdd = 0;
    QString fichierAImprimer = "";
    QString dossierSortieGeneration = "";
    QString cheminDeLaFactureCourante = "";

    //Parametres
    AeroDmsTypes::ParametresMetier parametresMetiers;
    AeroDmsTypes::ParametresSysteme parametresSysteme;

    //Statistiques
    QWidget* widgetGraphiques;
    StatistiqueWidget* m_activeWidget = nullptr;
    QWidget* m_contentArea = nullptr;
    QHBoxLayout* graphiques;

    QSplashScreen* splash;

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
    void changerInfosVolSurSelectionTypeVol();
    void chargerBaladesSorties();
    void ajouterUnPilote();
    void ajouterUnPiloteEnBdd();
    void ajouterUneSortie();
    void ajouterUneSortieEnBdd();
    void ajouterUneCotisation();
    void ajouterUneCotisationEnBdd();
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
    void mettreAJourNbPagesFichierCourant(const int p_nombreDePagesAImprimer);
    void aPropos();
    void ouvrirAide();
    void menuContextuelPilotes(const QPoint& pos);
    void editerPilote();
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
    void selectionnerTousLesGraphsPourRecapAnnuel();
    void verifierSignatureNumerisee();
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
    void switchOnglet();
    void imprimerApresGenerationPdf();
    void imprimerLaDerniereDemande();
    void imprimerLaDerniereDemandeAgrafage();
    void ouvrirDialogueParametresApplication();
    void enregistrerParametresApplication( AeroDmsTypes::ParametresMetier p_parametresMetiers,
                                           AeroDmsTypes::ParametresSysteme p_parametresSysteme);
    void detruireFenetreProgressionImpression();
    void detruireFenetreProgressionGenerationPdf();

};
#endif // AERODMS_H
