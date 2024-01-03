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

#include "ManageDb.h"
#include "PdfRenderer.h"
#include "DialogueGestionPilote.h"
#include "DialogueAjouterCotisation.h"
#include "DialogueAjouterSortie.h"

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
    void peuplerTablePilotes();

    float calculerCoutHoraire();

    ManageDb* db;
    PdfRenderer* pdf;

    //Fenêtres
    DialogueGestionPilote* dialogueGestionPilote;
    DialogueAjouterCotisation* dialogueAjouterCotisation;
    DialogueAjouterSortie* dialogueAjouterSortie;

    int factureIdEnBdd;

    QPdfDocument* pdfDocument;

    QTabWidget* mainTabWidget;

    QTableWidget* vuePilotes;

    //Onglet "Ajouter dépense"
    QComboBox* typeDeVol;
    QComboBox* choixPilote;
    QDateTimeEdit* dateDuVol;
    QTimeEdit* dureeDuVol;
    QDoubleSpinBox* prixDuVol;
    QComboBox* choixBalade;
    QLineEdit* remarqueVol;
    QPushButton* validerLeVol;

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

    QString cheminStockageFacturesTraitees = "C:/Users/cleme/OneDrive/Documents/AeroDMS/FacturesTraitees/";
    QString cheminStockageFacturesATraiter = "C:/Users/cleme/OneDrive/Documents/AeroDMS/FacturesATraiter/";
    QString cheminDeLaFactureCourante = "";

public slots:
    void selectionnerUneFacture();
    void enregistrerUnVol();
    void enregistrerUneFacture();
    void enregistrerUneRecette();
    void genererPdf();
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


};
#endif // AERODMS_H
