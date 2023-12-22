#ifndef AERODMS_H
#define AERODMS_H

#include <QMainWindow>
#include <QtWidgets>
#include <QPdfDocument>

#include "ManageDb.h"
#include "PdfRenderer.h"
#include "DialogueGestionPilote.h"
#include "DialogueAjouterCotisation.h"

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

    float calculerCoutHoraire();

    ManageDb* db;
    PdfRenderer* pdf;

    //Fenêtres
    DialogueGestionPilote* dialogueGestionPilote;
    DialogueAjouterCotisation* dialogueAjouterCotisation;

    int factureIdEnBdd;

    QPdfDocument* pdfDocument;

    QTabWidget* mainTabWidget;

    //Onglet "Ajouter Vol"
    QComboBox* typeDeVol;
    QComboBox* choixPilote;
    QDateTimeEdit* dateDuVol;
    QTimeEdit* dureeDuVol;
    QDoubleSpinBox* prixDuVol;
    QComboBox* choixBalade;
    QLineEdit* remarqueVol;
    QPushButton* validerLeVol;

    //Onglet "Ajouter recette"
    QListWidget* listeBaladesEtSorties;
    QComboBox* typeDeRecette;
    QLineEdit* intituleRecette;
    QDoubleSpinBox* montantRecette;
    QPushButton* validerLaRecette;

    QComboBox* listeDeroulanteAnnee;
    QComboBox* listeDeroulantePilote;

    QString cheminStockageFacturesTraitees = "C:/Users/cleme/OneDrive/Documents/AeroDMS/FacturesTraitees/";
    QString cheminStockageFacturesATraiter = "C:/Users/cleme/OneDrive/Documents/AeroDMS/FacturesATraiter/";
    QString cheminDeLaFactureCourante = "";

public slots:
    void selectionnerUneFacture();
    void enregistrerUnVol();
    void enregistrerUneRecette();
    void genererPdf();
    void prevaliderDonnnesSaisies();
    void changerInfosVolSurSelectionTypeVol();
    void ajouterUnPilote();
    void ajouterUnPiloteEnBdd();
    void ajouterUneCotisation();
    void ajouterUneCotisationEnBdd();


};
#endif // AERODMS_H
