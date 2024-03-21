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

#include "ManageDb.h"
#include "PdfRenderer.h"
#include "DialogueGestionPilote.h"
#include "DialogueAjouterCotisation.h"
#include "DialogueAjouterSortie.h"
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
    void chargerUneFacture(QString p_fichier);

    void peuplerListesPilotes();
    void peuplerListeSorties();
    void peuplerListeBaladesEtSorties();
    void peuplerListeDeroulanteAnnee();
    void peuplerTableVolsDetectes(const AeroDmsTypes::ListeDonneesFacture p_factures);

    bool lePiloteEstAJourDeCotisation();

    void initialiserOngletGraphiques();

    void initialiserTableauVolsDetectes(QGridLayout* p_infosVol);

    float calculerCoutHoraire();

    bool eventFilter(QObject* object, QEvent* event);

    ManageDb* db;
    PdfRenderer* pdf;
    AeroDmsTypes::Signature signature;
    AeroDmsTypes::TypeGenerationPdf typeGenerationPdf;
    AeroDmsTypes::ListeDonneesFacture factures;
    int idFactureDetectee;
    bool scanAutomatiqueDesFacturesEstActif = true;

    //Fenêtres
    DialogueGestionPilote* dialogueGestionPilote;
    DialogueAjouterCotisation* dialogueAjouterCotisation;
    DialogueAjouterSortie* dialogueAjouterSortie;

    QPdfDocument* pdfDocument;
    QPdfView* pdfView;

    QTabWidget* mainTabWidget;

    QTableWidget* vuePilotes;
    QTableWidget* vueVols;
    QTableWidget* vueFactures;
    QTableWidget* vueRecettes;
    QTableWidget* vueVolsDetectes;

    //Onglet "Ajouter dépense"
    QComboBox* typeDeVol;
    QComboBox* choixPilote;
    QLineEdit* aeroclubPiloteSelectionne;
    QComboBox* activite;
    QDateTimeEdit* dateDuVol;
    QTimeEdit* dureeDuVol;
    QDoubleSpinBox* prixDuVol;
    QComboBox* choixBalade;
    QLineEdit* remarqueVol;
    QPushButton* validerLeVol;
    QPushButton* validerLesVols;

    QComboBox* choixPayeur;
    QDateEdit* dateDeFacture;
    QDoubleSpinBox* montantFacture;
    QLineEdit* remarqueFacture;
    QComboBox* choixBaladeFacture;
    QPushButton* validerLaFacture;

    //Onglet "Ajouter recette"
    QListWidget* listeBaladesEtSorties;
    QComboBox* typeDeRecette;
    QLineEdit* intituleRecette;
    QDoubleSpinBox* montantRecette;
    QPushButton* validerLaRecette;

    QComboBox* listeDeroulanteAnnee;
    QComboBox* listeDeroulantePilote;
    QComboBox* listeDeroulanteStatistique;

    QString cheminStockageFacturesTraitees;
    QString cheminStockageFacturesATraiter;
    QString cheminSortieFichiersGeneres;
    QString cheminDeLaFactureCourante;

    //Barre de progression
    QProgressDialog *progressionGenerationPdf;
    QPushButton *boutonProgressionGenerationPdf;
    AeroDmsTypes::ParametresMetier parametresMetiers;

    //Bouttons
    QAction* bouttonGenerePdfRecapHdv;
    QAction* boutonModeDebug;

    //Menu options
    QAction* boutonAucuneSignature;
    QAction* boutonSignatureManuelle;
    QAction* boutonSignatureNumerique;
    QAction* boutonDemandesAGenererToutes;
    QAction* boutonDemandesAGenererRecettes;
    QAction* boutonDemandesAGenererDepenses;
    QAction* boutonFusionnerLesPdf;
    QAction* boutonNePasFusionnerLesPdf;
    QAction* mailingPilotesAyantCotiseCetteAnnee;
    QAction* mailingPilotesActifsAyantCotiseCetteAnnee;
    QAction* mailingPilotesDerniereDemandeSubvention;
    QAction* boutonActivationScanAutoFactures;

    //Données internes
    QString piloteAEditer = "";
    int volAEditer = -1;
    bool volPartiellementEditable = false;
    int factureIdEnBdd = 0;

    //Statistiques
    StatistiqueWidget* m_activeWidget = nullptr;
    QWidget* m_contentArea = nullptr;
    QHBoxLayout* graphiques;

signals:
    void toucheEchapEstAppuyee();

public slots:
    void selectionnerUneFacture();
    void enregistrerUnVol();
    void enregistrerLesVols();
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
    void peuplerTablePilotes();
    void peuplerTableVols();
    void peuplerTableFactures();
    void peuplerTableRecettes();
    void ouvrirFenetreProgressionGenerationPdf(const int p_nombreDeFacturesATraiter);
    void mettreAJourFenetreProgressionGenerationPdf(const int p_nombreDeFacturesTraitees);
    void mettreAJourBarreStatusFinGenerationPdf(const QString p_cheminDossier);
    void aPropos();
    void ouvrirAide();
    void menuContextuelPilotes(const QPoint& pos);
    void editerPilote();
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
    void verifierSignatureNumerisee();
    void envoyerMail();
    void mettreAJourInfosSurSelectionPilote();
    void volsSelectionnes();
    void chargerUnVolDetecte(int row, int column);
    void deselectionnerVolDetecte();

};
#endif // AERODMS_H
