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
#include "AeroDms.h"
#include "AeroDmsTypes.h"

#include "StatistiqueHistogrammeEmpile.h"
#include "StatistiqueDiagrammeCirculaireWidget.h"

#include <QtWidgets>
#include <QToolBar>
#include <QPdfDocument>
#include <QPdfPageSelector>
#include <QPdfView>

AeroDms::AeroDms(QWidget* parent):QMainWindow(parent)
{
    QApplication::setApplicationName("AeroDms");
    QApplication::setApplicationVersion("1.8.1");
    QApplication::setWindowIcon(QIcon("./ressources/shield-airplane.svg"));
    mainTabWidget = new QTabWidget(this);
    setCentralWidget(mainTabWidget);

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,"AeroDMS", "AeroDMS");
    
    if (settings.value("baseDeDonnees/chemin", "") == "")
    {
        settings.beginGroup("baseDeDonnees");
        settings.setValue("chemin", QCoreApplication::applicationDirPath());
        settings.setValue("nom", "AeroDMS.sqlite");
        settings.endGroup();
    }

    if (settings.value("dossiers/facturesATraiter", "") == "")
    {
        settings.beginGroup("dossiers");
        settings.setValue("facturesATraiter", QCoreApplication::applicationDirPath());
        settings.setValue("facturesSaisies", QCoreApplication::applicationDirPath());
        settings.setValue("sortieFichiersGeneres", QCoreApplication::applicationDirPath());
        settings.endGroup();
    }

    if (settings.value("noms/nomTresorier", "") == "")
    {
        settings.beginGroup("noms");
        settings.setValue("nomTresorier", "Trésorier");
    }

    //Fichier de conf commun => le fichier AeroDMS.ini est mis au même endroit que la BDD SQLite
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, settings.value("baseDeDonnees/chemin", "").toString() + QString("/"));
    QSettings settingsMetier(QSettings::IniFormat, QSettings::SystemScope, "AeroDMS");
    if (settingsMetier.value("parametresMetier/montantSubventionEntrainement", "") == "")
    {
        settingsMetier.beginGroup("parametresMetier");
        settingsMetier.setValue("montantSubventionEntrainement", "750");
        settingsMetier.setValue("montantCotisationPilote", "15");
        settingsMetier.setValue("proportionRemboursementEntrainement", "0.5");
        settingsMetier.setValue("plafondHoraireRemboursementEntrainement", "150");
        settingsMetier.setValue("proportionRemboursementBalade", "0.875");
        settingsMetier.endGroup();
    }

    const QString database = settings.value("baseDeDonnees/chemin", "").toString() + 
        QString("/") + 
        settings.value("baseDeDonnees/nom", "").toString();
    const QString ressourcesHtml = settings.value("baseDeDonnees/chemin", "").toString() + 
        QString("/ressources/HTML") ;
    cheminStockageFacturesTraitees = settings.value("dossiers/facturesSaisies", "").toString();
    cheminStockageFacturesATraiter = settings.value("dossiers/facturesATraiter", "").toString();
    cheminSortieFichiersGeneres = settings.value("dossiers/sortieFichiersGeneres", "").toString();

    parametresMetiers.montantSubventionEntrainement = settingsMetier.value("parametresMetier/montantSubventionEntrainement", "").toFloat();
    parametresMetiers.montantCotisationPilote = settingsMetier.value("parametresMetier/montantCotisationPilote", "").toFloat();
    parametresMetiers.proportionRemboursementEntrainement = settingsMetier.value("parametresMetier/proportionRemboursementEntrainement", "").toFloat();
    parametresMetiers.plafondHoraireRemboursementEntrainement = settingsMetier.value("parametresMetier/plafondHoraireRemboursementEntrainement", "").toFloat();
    parametresMetiers.proportionRemboursementBalade = settingsMetier.value("parametresMetier/proportionRemboursementBalade", "").toFloat();
    parametresMetiers.nomTresorier = settings.value("noms/nomTresorier", "").toString();

    db = new ManageDb(database);
    pdf = new PdfRenderer( db, 
                           ressourcesHtml);

    //==========Onglet Pilotes
    vuePilotes = new QTableWidget(0, AeroDmsTypes::PiloteTableElement_NB_COLONNES, this);
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_NOM, new QTableWidgetItem("Nom"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_PRENOM, new QTableWidgetItem("Prénom"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_ANNEE, new QTableWidgetItem("Année"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_ENTRAINEMENT_SUBVENTIONNEES, new QTableWidgetItem("HdV Entrainement"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE, new QTableWidgetItem("Subvention Entrainement"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_BALADES_SUBVENTIONNEES, new QTableWidgetItem("HdV Balade"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_BALADES_SUBVENTIONNE, new QTableWidgetItem("Subvention Balade"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_SORTIES_SUBVENTIONNEES, new QTableWidgetItem("HdV Sortie"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_SORTIES_SUBVENTIONNE, new QTableWidgetItem("Subvention Sortie"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_TOTALES_SUBVENTIONNEES, new QTableWidgetItem("HdV Totales"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_TOTAL_SUBVENTIONNE, new QTableWidgetItem("Subvention Totale"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_PILOTE_ID, new QTableWidgetItem("Pilote Id (masqué)"));
    vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_PILOTE_ID, true);
    vuePilotes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vuePilotes->setSelectionBehavior(QAbstractItemView::SelectRows);
    vuePilotes->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vuePilotes, QIcon("./ressources/account-tie-hat.svg"), "Pilotes");
    connect(vuePilotes, &QTableWidget::customContextMenuRequested, this, &AeroDms::menuContextuelPilotes);

    //==========Onglet Vols
    vueVols = new QTableWidget(0, AeroDmsTypes::VolTableElement_NB_COLONNES, this);;
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_PILOTE, new QTableWidgetItem("Pilote"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_DATE, new QTableWidgetItem("Date"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_TYPE_DE_VOL, new QTableWidgetItem("Type"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_DUREE, new QTableWidgetItem("Durée"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_COUT, new QTableWidgetItem("Coût"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_SUBVENTION, new QTableWidgetItem("Subvention"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_SOUMIS_CE, new QTableWidgetItem("Soumis CE"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_REMARQUE, new QTableWidgetItem("Remarque"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_VOL_ID, new QTableWidgetItem("ID"));
    vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_VOL_ID, true);
    vueVols->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vueVols->setSelectionBehavior(QAbstractItemView::SelectRows);
    vueVols->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vueVols, QIcon("./ressources/airplane.svg"), "Vols");
    connect(vueVols, &QTableWidget::customContextMenuRequested, this, &AeroDms::menuContextuelVols);

    //==========Onglet Factures
    vueFactures = new QTableWidget(0, AeroDmsTypes::FactureTableElement_NB_COLONNES, this);;
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_INTITULE, new QTableWidgetItem("Intitulé"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_MONTANT, new QTableWidgetItem("Montant"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_PAYEUR, new QTableWidgetItem("Payeur"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_NOM_SORTIE, new QTableWidgetItem("Nom sortie"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_TYPE_SORTIE, new QTableWidgetItem("Type de dépense"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_DATE, new QTableWidgetItem("Date"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_ANNEE, new QTableWidgetItem("Année"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_SOUMIS_CE, new QTableWidgetItem("Soumis CE"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_NOM_FACTURE, new QTableWidgetItem("Nom facture"));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_FACTURE_ID, new QTableWidgetItem("Facture ID"));
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_FACTURE_ID, true);
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_NOM_FACTURE, true);
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_ANNEE, true);
    vueFactures->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vueFactures->setSelectionBehavior(QAbstractItemView::SelectRows);
    vueFactures->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vueFactures, QIcon("./ressources/file-document.svg"), "Factures");

    //==========Onglet Ajout dépense
    QHBoxLayout* ajoutVol = new QHBoxLayout(this);
    QWidget* widgetAjoutVol = new QWidget(this);
    widgetAjoutVol->setLayout(ajoutVol);
    mainTabWidget->addTab(widgetAjoutVol, QIcon("./ressources/file-document-minus.svg"), "Ajout dépense");

    pdfDocument = new QPdfDocument(this);
    QPdfView* pdfView = new QPdfView(this);
    pdfView->setDocument(pdfDocument);
    pdfView->setPageMode(QPdfView::PageMode::MultiPage);
    ajoutVol->addWidget(pdfView, 3);

    QTabWidget *depenseTabWidget = new QTabWidget(this);
    ajoutVol->addWidget(depenseTabWidget, 1);
    

    //==========Sous onglet vol de l'onglet "Ajout dépense"
    typeDeVol = new QComboBox(this);
    typeDeVol->addItems(db->recupererTypesDesVol());
    typeDeVol->setCurrentIndex(2);
    QLabel* typeDeVolLabel = new QLabel(tr("Type de vol : "), this);
    connect(typeDeVol, &QComboBox::currentIndexChanged, this, &AeroDms::changerInfosVolSurSelectionTypeVol);
    connect(typeDeVol, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    choixPilote = new QComboBox(this);
    QLabel* choixPiloteLabel = new QLabel(tr("Pilote : "), this);
    connect(choixPilote, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    aeroclubPiloteSelectionne = new QLineEdit(this);
    aeroclubPiloteSelectionne->setToolTip(tr("Nom de l'aéroclub renseigné pour le pilote séléctionné et au nom duquel le\n" 
                                              "remboursement sera émis. En cas de changement d'aéroclub, modifier\n"
                                              "l'aéroclub du pilote avant de générer les demandes de subvention."));
    aeroclubPiloteSelectionne->setEnabled(false);
    QLabel* aeroclubPiloteSelectionneLabel = new QLabel(tr("Aéroclub : "), this);

    dateDuVol = new QDateTimeEdit(this);
    dateDuVol->setDisplayFormat("dd/MM/yyyy");
    dateDuVol->setCalendarPopup(true);
    dateDuVol->setDate(QDate::currentDate());
    QLabel* dateDuVolLabel = new QLabel(tr("Date du vol : "), this);

    dureeDuVol = new QTimeEdit(this);
    QLabel* dureeDuVolLabel = new QLabel(tr("Durée du vol : "), this);
    connect(dureeDuVol, &QTimeEdit::timeChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    prixDuVol = new QDoubleSpinBox(this);
    prixDuVol->setRange(0.0, 2000.0);
    prixDuVol->setSingleStep(1.0);
    prixDuVol->setDecimals(2);
    prixDuVol->setSuffix("€");
    QLabel* prixDuVolLabel = new QLabel(tr("Prix du vol : "), this);
    connect(prixDuVol, &QDoubleSpinBox::valueChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    choixBalade = new QComboBox(this);
    QLabel* choixBaladeLabel = new QLabel(tr("Sortie : "), this);
    connect(choixBalade, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    remarqueVol = new QLineEdit(this);
    QLabel* remarqueVolLabel = new QLabel(tr("Remarque : "), this);

    validerLeVol = new QPushButton("Valider le vol", this);
    connect(validerLeVol, &QPushButton::clicked, this, &AeroDms::enregistrerUnVol);

    QGridLayout* infosVol = new QGridLayout(this);
    QWidget* widgetDepenseVol = new QWidget(this);
    widgetDepenseVol->setLayout(infosVol);
    depenseTabWidget->addTab(widgetDepenseVol, QIcon("./ressources/airplane-clock.svg"), "Heures de vol");
    infosVol->addWidget(typeDeVolLabel,                 0, 0);
    infosVol->addWidget(typeDeVol,                      0, 1);
    infosVol->addWidget(choixPiloteLabel,               1, 0);
    infosVol->addWidget(choixPilote,                    1, 1);
    infosVol->addWidget(aeroclubPiloteSelectionneLabel, 2, 0);
    infosVol->addWidget(aeroclubPiloteSelectionne,      2, 1);
    infosVol->addWidget(dateDuVolLabel,                 3, 0);
    infosVol->addWidget(dateDuVol,                      3, 1);
    infosVol->addWidget(dureeDuVolLabel,                4, 0);
    infosVol->addWidget(dureeDuVol,                     4, 1);
    infosVol->addWidget(prixDuVolLabel,                 5, 0);
    infosVol->addWidget(prixDuVol,                      5, 1);
    infosVol->addWidget(choixBaladeLabel,               6, 0);
    infosVol->addWidget(choixBalade,                    6, 1);
    infosVol->addWidget(remarqueVolLabel,               7, 0);
    infosVol->addWidget(remarqueVol,                    7, 1);
    infosVol->addWidget(validerLeVol,                   8, 0, 2, 0);

    //==========Sous onglet facture de l'onglet "Ajout dépense"
    choixPayeur = new QComboBox(this);
    QLabel* choixPayeurLabel = new QLabel(tr("Payeur : "), this);
    connect(choixPayeur, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    dateDeFacture = new QDateEdit(this);
    dateDeFacture->setDisplayFormat("dd/MM/yyyy");
    dateDeFacture->setCalendarPopup(true);
    dateDeFacture->setDate(QDate::currentDate());
    QLabel* dateDeFactureLabel = new QLabel(tr("Date de la facture : "), this);

    choixBaladeFacture = new QComboBox(this);
    QLabel* choixBaladeFactureLabel = new QLabel(tr("Sortie : "), this);
    connect(choixBaladeFacture, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    montantFacture = new QDoubleSpinBox(this);
    montantFacture->setRange(0.0, 2000.0);
    montantFacture->setSingleStep(1.0);
    montantFacture->setDecimals(2);
    montantFacture->setSuffix("€");
    QLabel* montantFactureLabel = new QLabel(tr("Montant facture : "), this);
    connect(montantFacture, &QDoubleSpinBox::valueChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    remarqueFacture = new QLineEdit(this);
    connect(remarqueFacture, &QLineEdit::textChanged, this, &AeroDms::prevaliderDonnnesSaisies);
    QLabel* remarqueFactureLabel = new QLabel(tr("Intitulé : "), this);

    validerLaFacture = new QPushButton("Valider la facture", this);
    validerLaFacture->setToolTip("Validation possible si : facture chargée, montant de la facture renseignée, payeur et sortie séléctionnés, intitulé saisi");
    connect(validerLaFacture, &QPushButton::clicked, this, &AeroDms::enregistrerUneFacture);

    QGridLayout* infosFacture = new QGridLayout(this);
    QWidget* widgetDepenseFacture = new QWidget(this);
    widgetDepenseFacture->setLayout(infosFacture);
    depenseTabWidget->addTab(widgetDepenseFacture, QIcon("./ressources/file-document.svg"), "Facture");
    infosFacture->addWidget(choixPayeurLabel, 1, 0);
    infosFacture->addWidget(choixPayeur, 1, 1);
    infosFacture->addWidget(dateDeFactureLabel, 2, 0);
    infosFacture->addWidget(dateDeFacture, 2, 1);
    infosFacture->addWidget(montantFactureLabel, 4, 0);
    infosFacture->addWidget(montantFacture, 4, 1);
    infosFacture->addWidget(choixBaladeFactureLabel, 5, 0);
    infosFacture->addWidget(choixBaladeFacture, 5, 1);
    infosFacture->addWidget(remarqueFactureLabel, 6, 0);
    infosFacture->addWidget(remarqueFacture, 6, 1);
    infosFacture->addWidget(validerLaFacture, 7, 0, 2, 0);

    //==========Onglet Ajout recette
    QHBoxLayout* ajoutRecette = new QHBoxLayout(this);
    QWidget* widgetAjoutRecette = new QWidget(this);
    widgetAjoutRecette->setLayout(ajoutRecette);
    mainTabWidget->addTab(widgetAjoutRecette, QIcon("./ressources/file-document-plus.svg"), "Ajout recette");

    listeBaladesEtSorties = new QListWidget(this);
    ajoutRecette->addWidget(listeBaladesEtSorties);

    typeDeRecette = new QComboBox(this);
    typeDeRecette->addItems(db->recupererTypesDesVol(true));
    connect(typeDeRecette, &QComboBox::currentIndexChanged, this, &AeroDms::chargerBaladesSorties);
    //typeDeRecette->setCurrentIndex(2);
    QLabel* typeDeRecetteLabel = new QLabel(tr("Type de vol : "), this);

    intituleRecette = new QLineEdit(this);
    intituleRecette->setToolTip(tr("Format recommandé : 'NOM EMETTEUR CHEQUE / Banque numéro de chèque'"));
    QLabel* intituleRecetteLabel = new QLabel(tr("Intitulé : "), this); 
    connect(intituleRecette, &QLineEdit::textChanged, this, &AeroDms::prevaliderDonnneesSaisiesRecette);

    montantRecette = new QDoubleSpinBox(this);
    montantRecette->setRange(0.0, 2000.0);
    montantRecette->setSingleStep(1.0);
    montantRecette->setDecimals(2);
    montantRecette->setSuffix("€");
    QLabel* montantRecetteLabel = new QLabel(tr("Montant : "), this);
    connect(montantRecette, &QDoubleSpinBox::valueChanged, this, &AeroDms::prevaliderDonnneesSaisiesRecette);

    validerLaRecette = new QPushButton("Valider la recette", this);
    connect(validerLaRecette, &QPushButton::clicked, this, &AeroDms::enregistrerUneRecette);

    QGridLayout* infosRecette = new QGridLayout(this);
    ajoutRecette->addLayout(infosRecette);
    infosRecette->addWidget(typeDeRecetteLabel, 0, 0);
    infosRecette->addWidget(typeDeRecette, 0, 1);
    infosRecette->addWidget(intituleRecetteLabel, 1, 0);
    infosRecette->addWidget(intituleRecette, 1, 1);
    infosRecette->addWidget(montantRecetteLabel, 2, 0);
    infosRecette->addWidget(montantRecette, 2, 1);
    infosRecette->addWidget(validerLaRecette, 5, 0, 2, 0);

    //=============Onglet graphiques
    initialiserOngletGraphiques();

    //=============General
    statusBar()->showMessage("Pret");

    setWindowTitle(tr("AeroDMS"));
    setMinimumSize(160, 160);
    showMaximized();
    //resize(480, 320);

    QToolBar* toolBar = addToolBar(tr(""));
    const QIcon iconeAjouterUnVol = QIcon("./ressources/airplane-plus.svg");
    QAction* bouttonAjouterUnVol = new QAction(iconeAjouterUnVol, tr("&Ajouter un vol/une dépense"), this);
    bouttonAjouterUnVol->setStatusTip(tr("Ajouter un vol/une dépense"));
    connect(bouttonAjouterUnVol, &QAction::triggered, this, &AeroDms::selectionnerUneFacture);
    toolBar->addAction(bouttonAjouterUnVol);

    const QIcon iconeAjouterPilote = QIcon("./ressources/account-tie-hat.svg");
    QAction* bouttonAjouterPilote = new QAction(iconeAjouterPilote, tr("&Ajouter un pilote"), this);
    bouttonAjouterPilote->setStatusTip(tr("Ajouter un pilote"));
    connect(bouttonAjouterPilote, &QAction::triggered, this, &AeroDms::ajouterUnPilote);
    toolBar->addAction(bouttonAjouterPilote);

    const QIcon iconeAjouterCotisation = QIcon("./ressources/ticket.svg");
    QAction* bouttonAjouterCotisation = new QAction(iconeAjouterCotisation, tr("&Ajouter une cotisation pour un pilote"), this);
    bouttonAjouterCotisation->setStatusTip(tr("Ajouter une cotisation pour un pilote"));
    connect(bouttonAjouterCotisation, &QAction::triggered, this, &AeroDms::ajouterUneCotisation);
    toolBar->addAction(bouttonAjouterCotisation);

    const QIcon iconeAjouterSortie = QIcon("./ressources/transit-connection-variant.svg");
    QAction* bouttonAjouterSortie = new QAction(iconeAjouterSortie, tr("&Ajouter une sortie"), this);
    bouttonAjouterSortie->setStatusTip(tr("Ajouter une sortie"));
    connect(bouttonAjouterSortie, &QAction::triggered, this, &AeroDms::ajouterUneSortie);
    toolBar->addAction(bouttonAjouterSortie);

    toolBar->addSeparator();

    const QIcon iconeGenerePdf = QIcon("./ressources/file-cog.svg");
    QAction* bouttonGenerePdf = new QAction(iconeGenerePdf, tr("&Générer les PDF de demande de subvention"), this);
    bouttonGenerePdf->setStatusTip(tr("Générer les PDF de demande de subvention"));
    connect(bouttonGenerePdf, &QAction::triggered, this, &AeroDms::genererPdf);
    toolBar->addAction(bouttonGenerePdf);

    const QIcon iconeGenerePdfRecapHdv = QIcon("./ressources/account-file-text.svg");
    bouttonGenerePdfRecapHdv = new QAction(iconeGenerePdfRecapHdv, tr("&Générer les PDF de recap HdV de l'année séléctionnée"), this);
    bouttonGenerePdfRecapHdv->setStatusTip(tr("Générer les PDF de recap HdV de l'année séléctionnée"));
    connect(bouttonGenerePdfRecapHdv, &QAction::triggered, this, &AeroDms::genererPdfRecapHdV);
    toolBar->addAction(bouttonGenerePdfRecapHdv);

    QToolBar* selectionToolBar = addToolBar(tr(""));

    listeDeroulanteAnnee = new QComboBox(this);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTablePilotes);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerStatistiques);
    selectionToolBar->addWidget(listeDeroulanteAnnee);

    listeDeroulantePilote = new QComboBox(this);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTablePilotes);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    selectionToolBar->addWidget(listeDeroulantePilote);

    listeDeroulanteStatistique = new QComboBox(this);
    listeDeroulanteStatistique->addItem("Statistiques mensuelles", AeroDmsTypes::Statistiques_HEURES_ANNUELLES);
    listeDeroulanteStatistique->addItem("Statistiques par pilote", AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE);
    listeDeroulanteStatistique->addItem("Statistiques par type de vol", AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL);
    connect(listeDeroulanteStatistique, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerStatistiques);
    selectionToolBar->addWidget(listeDeroulanteStatistique);

    //Fenêtres
    dialogueGestionPilote = new DialogueGestionPilote(db, this);
    connect(dialogueGestionPilote, SIGNAL(accepted()), this, SLOT(ajouterUnPiloteEnBdd()));

    dialogueAjouterCotisation = new DialogueAjouterCotisation( db, 
                                                               parametresMetiers.montantCotisationPilote, 
                                                               parametresMetiers.montantSubventionEntrainement, 
                                                               this);
    connect(dialogueAjouterCotisation, SIGNAL(accepted()), this, SLOT(ajouterUneCotisationEnBdd()));

    dialogueAjouterSortie = new DialogueAjouterSortie(this);
    connect(dialogueAjouterSortie, SIGNAL(accepted()), this, SLOT(ajouterUneSortieEnBdd()));

    //Dialogue de progression de génération PDF
    progressionGenerationPdf = new QProgressDialog("Génération PDF en cours", "Fermer", 0, 0, this);
    progressionGenerationPdf->setAutoClose(false);
    progressionGenerationPdf->setWindowModality(Qt::WindowModal);
    progressionGenerationPdf->close();
    progressionGenerationPdf->setAutoReset(false);

    connect(pdf, SIGNAL(mettreAJourNombreFacturesATraiter(int)), this, SLOT(ouvrirFenetreProgressionGenerationPdf(int)));
    connect(pdf, SIGNAL(mettreAJourNombreFacturesTraitees(int)), this, SLOT(mettreAJourFenetreProgressionGenerationPdf(int)));
    connect(pdf, SIGNAL(generationTerminee(QString)), this, SLOT(mettreAJourBarreStatusFinGenerationPdf(QString)));

    //========================Menu
    QMenu *helpMenu = menuBar()->addMenu(tr("Aide"));
    boutonModeDebug = new QAction(QIcon("./ressources/bug.svg"), tr("Activer le mode &debug"), this);
    helpMenu->addAction(boutonModeDebug);
    connect(boutonModeDebug, SIGNAL(triggered()), this, SLOT(switchModeDebug()));
    helpMenu->addSeparator();
    QAction *aboutQtAction = new QAction(QIcon("./ressources/qt-logo.svg"), tr("À propos de &Qt"), this);
    aboutQtAction->setStatusTip(tr("Voir la fenêtre à propos de Qt"));
    helpMenu->addAction(aboutQtAction);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QAction *aboutAction = new QAction(QIcon("./ressources/shield-airplane.svg"), tr("À propos de &AeroDms"), this);
    aboutAction->setStatusTip(tr("Voir la fenêtre à propos de cette application"));
    helpMenu->addAction(aboutAction);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aPropos()));

    //========================Initialisation des autres attributs
    piloteAEditer = "";
    volAEditer = -1;
    factureIdEnBdd = 0;

    peuplerListesPilotes();
    peuplerListeSorties();
    peuplerListeBaladesEtSorties();
    peuplerTablePilotes();
    peuplerTableVols();
    peuplerTableFactures();
    peuplerListeDeroulanteAnnee();
    prevaliderDonnnesSaisies();
    prevaliderDonnneesSaisiesRecette();
    changerInfosVolSurSelectionTypeVol();
    peuplerStatistiques();
}

void AeroDms::initialiserOngletGraphiques()
{
    graphiques = new QHBoxLayout(this);
    QWidget* widgetGraphiques = new QWidget(this);
    widgetGraphiques->setLayout(graphiques);
    mainTabWidget->addTab(widgetGraphiques, QIcon("./ressources/chart-areaspline.svg"), "Graphiques"); 
}

void AeroDms::peuplerStatistiques()
{
    if (m_activeWidget) {
        m_activeWidget->setVisible(false);
        m_activeWidget->deleteLater();
    }

    switch (listeDeroulanteStatistique->currentData().toInt())
    {
        case AeroDmsTypes::Statistiques_HEURES_ANNUELLES:
        {
            m_activeWidget = new StatistiqueHistogrammeEmpile(db, listeDeroulanteAnnee->currentData().toInt(), m_contentArea);
            break;
        }
        case AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget(db, listeDeroulanteAnnee->currentData().toInt(), AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE, m_contentArea);
            break;
        }
        case AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL:
        default:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget(db, listeDeroulanteAnnee->currentData().toInt(), AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL, m_contentArea);
            break;
        }
    }
    graphiques->addWidget(m_activeWidget);

    m_activeWidget->load();
    m_activeWidget->setVisible(true);
}

void AeroDms::ouvrirFenetreProgressionGenerationPdf(const int p_nombreDeFacturesATraiter)
{
    progressionGenerationPdf->setLabelText("Génération PDF en cours");
    progressionGenerationPdf->reset();
    progressionGenerationPdf->setMaximum(p_nombreDeFacturesATraiter);
    progressionGenerationPdf->setValue(0);
    progressionGenerationPdf->show();
}
void AeroDms::mettreAJourFenetreProgressionGenerationPdf(const int p_nombreDeFacturesTraitees)
{
    progressionGenerationPdf->setValue(p_nombreDeFacturesTraitees);
}
void AeroDms::mettreAJourBarreStatusFinGenerationPdf(const QString p_cheminDossier)
{
    //On met à jour la table des vols (champ Soumis CE)
    peuplerTableVols();

    //On met à jour la fenêtre de progression
    progressionGenerationPdf->setLabelText("Génération PDF terminée");
    const QString status = "Génération terminée. Fichiers disponibles sous "
                            +p_cheminDossier;
    statusBar()->showMessage(status);
}

AeroDms::~AeroDms()
{
}

void AeroDms::peuplerTablePilotes()
{
    const AeroDmsTypes::ListeSubventionsParPilotes listeSubventions = db->recupererSubventionsPilotes( listeDeroulanteAnnee->currentData().toInt(), 
                                                                                                       listeDeroulantePilote->currentData().toString());
    vuePilotes->setRowCount(listeSubventions.size());
    for (int i = 0; i < listeSubventions.size(); i++)
    {
        const AeroDmsTypes::SubventionsParPilote subvention = listeSubventions.at(i);
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_NOM, new QTableWidgetItem(subvention.nom));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_PRENOM, new QTableWidgetItem(subvention.prenom));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_ANNEE, new QTableWidgetItem(QString::number(subvention.annee)));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_HEURES_ENTRAINEMENT_SUBVENTIONNEES, new QTableWidgetItem(subvention.entrainement.heuresDeVol));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE, new QTableWidgetItem(QString::number(subvention.entrainement.montantRembourse).append(" €")));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_HEURES_BALADES_SUBVENTIONNEES, new QTableWidgetItem(subvention.balade.heuresDeVol));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_MONTANT_BALADES_SUBVENTIONNE, new QTableWidgetItem(QString::number(subvention.balade.montantRembourse).append(" €")));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_HEURES_SORTIES_SUBVENTIONNEES, new QTableWidgetItem(subvention.sortie.heuresDeVol));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_MONTANT_SORTIES_SUBVENTIONNE, new QTableWidgetItem(QString::number(subvention.sortie.montantRembourse).append(" €")));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_HEURES_TOTALES_SUBVENTIONNEES, new QTableWidgetItem(subvention.totaux.heuresDeVol));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_MONTANT_TOTAL_SUBVENTIONNE, new QTableWidgetItem(QString::number(subvention.totaux.montantRembourse).append(" €")));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_PILOTE_ID, new QTableWidgetItem(subvention.idPilote));
    }
    vuePilotes->resizeColumnsToContents();
}

void AeroDms::peuplerTableVols()
{
    AeroDmsTypes::ListeVols listeVols = db->recupererVols( listeDeroulanteAnnee->currentData().toInt(),
                                                           listeDeroulantePilote->currentData().toString());
    vueVols->setRowCount(listeVols.size());
    for (int i = 0; i < listeVols.size(); i++)
    {
        const AeroDmsTypes::Vol vol = listeVols.at(i);
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_DATE, new QTableWidgetItem(vol.date.toString("dd/MM/yyyy")));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_PILOTE, new QTableWidgetItem(QString(vol.prenomPilote).append(" ").append(vol.nomPilote)));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_SOUMIS_CE, new QTableWidgetItem(vol.estSoumisCe));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_DUREE, new QTableWidgetItem(vol.duree));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_COUT, new QTableWidgetItem(QString::number(vol.coutVol).append(" €")));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_SUBVENTION, new QTableWidgetItem(QString::number(vol.montantRembourse).append(" €")));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_TYPE_DE_VOL, new QTableWidgetItem(vol.typeDeVol));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_REMARQUE, new QTableWidgetItem(vol.remarque));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_VOL_ID, new QTableWidgetItem(QString::number(vol.volId)));
    }
    vueVols->resizeColumnsToContents();

    bouttonGenerePdfRecapHdv->setEnabled(true);
    //On desactive la génération du récap annuel si on est sur la séléction "Toutes les années"
    if (listeDeroulanteAnnee->currentData().toInt() == -1)
    {
        bouttonGenerePdfRecapHdv->setEnabled(false);
    }
}

void AeroDms::peuplerTableFactures()
{
    const AeroDmsTypes::ListeDemandeRemboursementFacture listeFactures = db->recupererToutesLesDemandesDeRemboursement(listeDeroulanteAnnee->currentData().toInt());
    vueFactures->setRowCount(listeFactures.size());
    for (int i = 0; i < listeFactures.size(); i++)
    {
        const AeroDmsTypes::DemandeRemboursementFacture facture = listeFactures.at(i);
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_INTITULE, new QTableWidgetItem(facture.intitule));
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_MONTANT, new QTableWidgetItem(QString::number(facture.montant).append(" €")));
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_PAYEUR, new QTableWidgetItem(facture.payeur));
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_NOM_SORTIE, new QTableWidgetItem(facture.nomSortie));
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_TYPE_SORTIE, new QTableWidgetItem(facture.typeDeSortie));
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_DATE, new QTableWidgetItem(facture.date.toString("dd/MM/yyyy")));
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_ANNEE, new QTableWidgetItem(QString::number(facture.annee)));
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_NOM_FACTURE, new QTableWidgetItem(facture.nomFacture));
        if(facture.soumisCe)
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_SOUMIS_CE, new QTableWidgetItem("Oui"));
        else
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_SOUMIS_CE, new QTableWidgetItem("Non"));
        vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_FACTURE_ID, new QTableWidgetItem(QString::number(facture.id)));
    }
    vueFactures->resizeColumnsToContents();
}

void AeroDms::peuplerListeDeroulanteAnnee()
{
    listeDeroulanteAnnee->clear();
    listeDeroulanteAnnee->addItem("Toutes les années", -1);
    QList<int> listeAnnees = db->recupererAnnees();
    for (int i = 0; i < listeAnnees.size(); i++)
    {
        listeDeroulanteAnnee->addItem(QString::number(listeAnnees.at(i)), listeAnnees.at(i));
    }
    //On affiche de base les infos de l'année courante
    listeDeroulanteAnnee->setCurrentText(QDate::currentDate().toString("yyyy"));
}

void AeroDms::ajouterUneCotisationEnBdd()
{
    //On contrôle que le pilote n'a pas déjà une cotisation pour cette année
    AeroDmsTypes::CotisationAnnuelle infosCotisation = dialogueAjouterCotisation->recupererInfosCotisationAAjouter();
    if (db->piloteEstAJourDeCotisation(infosCotisation.idPilote, infosCotisation.annee))
    {
        QMessageBox::critical(this, "Cotisation déja reglée", "Le pilote indiqué est déjà à jour de\nsa cotisation pour l'année saisie.");
    }
    //Si le pilote n'a pas deja une cotisation pour cette année la, on ajoute la cotisation en BDD
    else
    {
        db->ajouterCotisation(infosCotisation);
        //On met à jour la table des pilotes
        peuplerTablePilotes();
        //On met a jour la liste des années => permet de traiter le cas ou on ajoute un premier pilote sur l'année considerée
        peuplerListeDeroulanteAnnee();
        statusBar()->showMessage("Cotisation " + QString::number(infosCotisation.annee) + " ajoutée pour le pilote " + db->recupererNomPrenomPilote(infosCotisation.idPilote));
    }
}

void AeroDms::chargerBaladesSorties()
{
    peuplerListeBaladesEtSorties();
}

void AeroDms::ajouterUnPiloteEnBdd()
{
    const AeroDmsTypes::Pilote pilote = dialogueGestionPilote->recupererInfosPilote();
    const AeroDmsTypes::ResultatCreationPilote resultat = db->creerPilote(pilote);

    switch (resultat)
    {
        case AeroDmsTypes::ResultatCreationPilote_SUCCES:
        {
            if (pilote.idPilote == "")
            {
                statusBar()->showMessage("Pilote ajouté avec succès");
            }
            else
            {
                statusBar()->showMessage("Pilote modifié avec succès");
            } 
            break;
        } 
        case AeroDmsTypes::ResultatCreationPilote_PILOTE_EXISTE:
        {
            statusBar()->showMessage("Echec ajout pilote : le pilote existe déjà");
            QMessageBox::critical(this, "Echec ajoute pilote", "Un pilote existe avec ce nom\nexiste déjà. Ajout impossible.");
            break;
        }
        case AeroDmsTypes::ResultatCreationPilote_AUTRE:
        {
            statusBar()->showMessage("Echec ajout pilote : erreur indéterminée");
            QMessageBox::critical(this, "Echec ajoute pilote", "Une erreur indéterminée s'est\nproduite. Ajout du pilote impossible.");
            break;
        }
    }

    //On met à jour les listes de pilotes
    peuplerListesPilotes();
    dialogueAjouterCotisation->mettreAJourLeContenuDeLaFenetre();

    //Si on est sur une mise à jour, on met à jour les élements d'IMH susceptibles d'être impacté par des changements
    if (pilote.idPilote != "")
    {
        peuplerListeBaladesEtSorties();
        peuplerTablePilotes();
        peuplerTableVols();
    }
}

void AeroDms::ajouterUneSortieEnBdd()
{
    AeroDmsTypes::Sortie sortie = dialogueAjouterSortie->recupererInfosSortieAAjouter();

    db->creerSortie(sortie);
    statusBar()->showMessage("Sortie " + sortie.nom + " ajoutée");
    peuplerListeBaladesEtSorties();
    peuplerListeSorties();
}

void AeroDms::selectionnerUneFacture()
{
    QString fichier = QFileDialog::getOpenFileName(
        this,
        "Ouvrir une facture",
        cheminStockageFacturesATraiter,
        tr("Fichier PDF (*.pdf)"));

    if (!fichier.isNull())
    {
        chargerUneFacture(fichier);
        //Si on passe ici, on est pas en édition de vol
        volAEditer = -1;
        //On restaure le texte du bouton de validation (qui a changé si on était en édition)
        validerLeVol->setText("Valider le vol");
    }
}

void AeroDms::chargerUneFacture(QString p_fichier)
{
    cheminDeLaFactureCourante = p_fichier;
    factureIdEnBdd = -1;
    pdfDocument->load(p_fichier);

    choixPilote->setEnabled(true);
    dureeDuVol->setEnabled(true);
    prixDuVol->setEnabled(true);
    typeDeVol->setEnabled(true);

    choixPilote->setCurrentIndex(0);
    dureeDuVol->setTime(QTime::QTime(0, 0));
    prixDuVol->setValue(0);
    remarqueVol->clear();
    typeDeVol->setCurrentIndex(2);
    choixBalade->setCurrentIndex(0);
    
    //On affiche le widget qui contient la fonction d'ajout de vol
    mainTabWidget->setCurrentIndex(3);
}

void AeroDms::genererPdf()
{
    pdf->imprimerLesDemandesDeSubvention( parametresMetiers.nomTresorier,
                                          cheminSortieFichiersGeneres,
                                          cheminStockageFacturesTraitees);
}

void AeroDms::genererPdfRecapHdV()
{
    pdf->imprimerLeRecapitulatifDesHeuresDeVol( listeDeroulanteAnnee->currentData().toInt(),
                                                cheminSortieFichiersGeneres,
                                                cheminStockageFacturesTraitees);
}

void AeroDms::enregistrerUneFacture()
{
    bool estEnEchec = false;
    const int anneeRenseignee = dateDeFacture->date().year();
    const QString idPilote = choixPayeur->currentData().toString();
    const int idSortie = choixBaladeFacture->currentData().toInt();

    //On verifie si le pilote est a jour de sa cotisation pour l'année de la facture
    if (!db->piloteEstAJourDeCotisation(idPilote, anneeRenseignee))
    {
        estEnEchec = true;
        QMessageBox::critical(this, tr("AeroDMS"),
            tr("Le pilote n'est pas a jour de sa cotisation pour l'année du vol.\n"
               "Impossible d'enregistrer une facture à rembourser à son profit."), QMessageBox::Cancel);
    }

    //On effectue d'abord quelques contrôles pour savoir si la facture est enregistrable :
    //1) on a une facture chargée
    //2) les données (pilote, date du vol, durée, cout) sont renseignées
    //3) on est pas en echec sur une des étapes précédentes
    if (pdfDocument->status() == QPdfDocument::Status::Ready
        && !estEnEchec)
    {
        if (choixPayeur->isEnabled())
        {
            QString nomDeLaFacture = "";
            //Le comboBox choixPayeur est activé : la facture n'est pas encore soumise
            //1) on sauvegarde la facture avec les autres, en le renommant
            //2) on la référence en BDD
            pdfDocument->close();
            //On forme le nom du fichier sous forme annee.nomSortie.intitule.idFactureBdd.pdf
            int idFactureBdd = db->recupererProchainNumeroFacture();
            nomDeLaFacture.append(QString::number(anneeRenseignee));
            nomDeLaFacture.append(".");
            nomDeLaFacture.append(idPilote);
            nomDeLaFacture.append(".facture.");
            nomDeLaFacture.append(QString::number(idFactureBdd));
            nomDeLaFacture.append(".pdf");
            QString cheminComplet = cheminStockageFacturesTraitees + "/";
            cheminComplet.append(nomDeLaFacture);
            QFile gestionnaireDeFichier;
            if (gestionnaireDeFichier.copy(cheminDeLaFactureCourante, cheminComplet))
            {
                factureIdEnBdd = db->ajouterFacture(nomDeLaFacture);
                pdfDocument->load(cheminComplet);
                gestionnaireDeFichier.remove(cheminDeLaFactureCourante);
                cheminDeLaFactureCourante = cheminComplet;
            }
            else
            {
                statusBar()->showMessage("Impossible de déplacer la facture : arrêt.");
                estEnEchec = true;
            }
        }
        //On desactive le choix du payeur (la facture ne concerne qu'un seul et unique payeur)
        choixPayeur->setEnabled(false);

        //Ensuite :
        //3) on enregistre la paiement en BDD, si on est pas en echec
        if (!estEnEchec)
        {
            db->enregistrerUneFacture(idPilote,
                factureIdEnBdd,
                dateDeFacture->date(),
                montantFacture->value(),
                choixBaladeFacture->currentData().toInt(),
                remarqueFacture->text());

            statusBar()->showMessage(QString("Facture ")
                + remarqueFacture->text()
                + " du "
                + dateDeFacture->date().toString("dd/MM/yyyy")
                + " ("
                + QString::number(montantFacture->value())
                + "€) ajoutée.");

            montantFacture->setValue(0);
            remarqueFacture->clear();
        }
        else
        {
            statusBar()->showMessage("Erreur ajout");
        }
    }
    //On met à jour la vue
    peuplerTableFactures();
}

void AeroDms::enregistrerUnVol()
{
    bool estEnEchec = false;
    const int anneeRenseignee = dateDuVol->date().year();
    const QString idPilote = choixPilote->currentData().toString();

    //On verifie si le pilote est a jour de sa cotisation pour l'année du vol
    if (!db->piloteEstAJourDeCotisation(idPilote, anneeRenseignee))
    {
        estEnEchec = true;
        QMessageBox::critical(this, tr("AeroDMS"),
            tr("Le pilote n'est pas a jour de sa cotisation pour l'année du vol.\n"
                "Impossible d'enregistrer le vol."), QMessageBox::Cancel);
    }

    //On effectue d'abord quelques contrôles pour savoir si le vol est enregistrable :
    //1) on a une facture chargée
    //2) les données (pilote, date du vol, durée, cout) sont renseignées
    //3) on est pas en echec sur une des étapes précédentes
    if (pdfDocument->status() == QPdfDocument::Status::Ready
        && !estEnEchec )
    {
        QString nomDeLaFacture = "";
        if (choixPilote->isEnabled())
        {
            //Le comboBox choixPilote est activé : on est sur le premier vol de cette facture :
            //1) on sauvegarde la facture avec les autres, en le renommant
            //2) on la référence en BDD
            pdfDocument->close();
            //On forme le nom du fichier sous forme annee.idPilote.idFactureBdd.pdf
            int idFactureBdd = db->recupererProchainNumeroFacture();
            nomDeLaFacture.append(QString::number(anneeRenseignee));
            nomDeLaFacture.append(".");
            nomDeLaFacture.append(idPilote);
            nomDeLaFacture.append(".");
            nomDeLaFacture.append(QString::number(idFactureBdd));
            nomDeLaFacture.append(".pdf");
            QString cheminComplet = cheminStockageFacturesTraitees;
            cheminComplet.append(nomDeLaFacture);
            QFile gestionnaireDeFichier;
            if (gestionnaireDeFichier.copy(cheminDeLaFactureCourante, cheminComplet))
            {
                factureIdEnBdd = db->ajouterFacture(nomDeLaFacture);
                pdfDocument->load(cheminComplet);
                gestionnaireDeFichier.remove(cheminDeLaFactureCourante);
                cheminDeLaFactureCourante = cheminComplet;
            }
            else
            {
                statusBar()->showMessage("Impossible de déplacer la facture : arrêt.");
                estEnEchec = true;
            }
        }
        //On desactive le choix du pilote (la facture ne concerne qu'un seul et unique pilote)
        choixPilote->setEnabled(false);

        //Ensuite, si on est pas en echec :
        //1) on récupère la subvention restante pour le pilote
        //2) on calcul la subvention allouable pour le vol
        //3) on enregistre le vol en BDD    
		if (!estEnEchec)
		{
			float subventionRestante = db->recupererSubventionRestante(idPilote, anneeRenseignee);
			//On initialise le montant subventionné sur la base du prix du vol (cas vols type sortie)
			float montantSubventionne = prixDuVol->value();
			if (typeDeVol->currentText() == "Balade")
			{
				montantSubventionne = prixDuVol->value() * parametresMetiers.proportionRemboursementBalade;
			}
			//Si on est en vol d'entrainement, calculs spécifiques et enregistrement spécifique
			if (typeDeVol->currentText() == "Entrainement")
			{
				float coutHoraire = calculerCoutHoraire();
				if (coutHoraire > parametresMetiers.plafondHoraireRemboursementEntrainement)
				{
					coutHoraire = parametresMetiers.plafondHoraireRemboursementEntrainement;
				}
				montantSubventionne = (coutHoraire * (dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute()) / 60.0) * parametresMetiers.proportionRemboursementEntrainement;
           
                if (montantSubventionne > subventionRestante)
				{
					montantSubventionne = subventionRestante;
				}
                

				/*db->enregistrerUnVolSortieOuBalade(idPilote,
					typeDeVol->currentText(),
					dateDuVol->date(),
					dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute(),
					prixDuVol->value(),
					montantSubventionne,
					factureIdEnBdd,
                    choixBalade->currentData().toInt(),
					remarqueVol->text());*/

				subventionRestante = subventionRestante - montantSubventionne;
			}
			//Sinon on est balade ou sortie, on enregistre le vol avec la référence de balade/sortie
			//else
			//{
				db->enregistrerUnVol(idPilote,
					typeDeVol->currentText(),
					dateDuVol->date(),
					dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute(),
					prixDuVol->value(),
					montantSubventionne,
					factureIdEnBdd,
					choixBalade->currentData().toInt(),
					remarqueVol->text(),
                    volAEditer);

				
			//}

            statusBar()->showMessage(QString("Vol ")
                + typeDeVol->currentText() 
                + " de "
                + db->recupererNomPrenomPilote(idPilote)
                + " du " 
                + dateDuVol->date().toString("dd/MM/yyyy") 
                + " (" 
                + dureeDuVol->time().toString("hh:mm")
                + "/"
                + QString::number(prixDuVol->value())
                + "€) ajouté. Montant subvention : "
                + QString::number(montantSubventionne)
                + "€ / Subvention entrainement restante : "
                + QString::number(subventionRestante)
                + "€");

            dureeDuVol->setTime(QTime::QTime(0, 0));
            prixDuVol->setValue(0);
            remarqueVol->clear();

            //on réactive les éventuels élements d'IHM désactivés par une mise à jour de vol
            typeDeVol->setEnabled(true);
            dureeDuVol->setEnabled(true);
            prixDuVol->setEnabled(true);

            //On sort du mode édition, si on y etait...
            volAEditer = -1;
        }
        else
        {
            statusBar()->showMessage("Erreur ajout");
        }
    }
    //On met à jour la table des pilotes et celle des vols
    peuplerTablePilotes();
    peuplerTableVols();
    //On met a jour la liste des vols balades/sorties dans l'onglet des recettes
    peuplerListeBaladesEtSorties();

    //Et l'affichage des statistiques
    peuplerStatistiques();

    //On restaure le texte du bouton de validation (qui a changé si on était en édition)
    validerLeVol->setText("Valider le vol");
}

void AeroDms::enregistrerUneRecette()
{
    QStringList volsCoches;
    //On verifie si au moins un vol est coché
    for (int i = 0; i < listeBaladesEtSorties->count(); i++)
    {
        if (listeBaladesEtSorties->item(i)->checkState() == Qt::Checked)
        {
            volsCoches.append(listeBaladesEtSorties->item(i)->data(Qt::DisplayRole).toString());
        }
    }

    if (volsCoches.size() == 0)
    {
        QMessageBox::critical(this, "Vol non sélectionné", "La dépense doit être associée à au moins un vol. \nSélectionnez au moins un vol dans la partie gauche de la fenêtre.\n\nSaisie non prise en compte.");
    }
    else
    {
        db->ajouterUneRecetteAssocieeAVol( volsCoches,
                                           typeDeRecette->currentText(),
                                           intituleRecette->text(),
                                           montantRecette->value());
        statusBar()->showMessage("Recette ajoutee");
    }

    intituleRecette->clear();
    montantRecette->clear();

}

float AeroDms::calculerCoutHoraire()
{
    return (prixDuVol->value() / (dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute())) * 60.0;
}

void AeroDms::peuplerListesPilotes()
{
    const AeroDmsTypes::ListePilotes pilotes = db->recupererPilotes();

    listeDeroulantePilote->clear();
    choixPilote->clear();

    listeDeroulantePilote->addItem("Tous les pilotes", "*");
    choixPilote->addItem("", "aucun");
    choixPayeur->addItem("", "aucun");
    for (int i = 0; i < pilotes.size(); i++)
    {
        const AeroDmsTypes::Pilote pilote = pilotes.at(i);
        const QString nomPrenom = QString(pilote.prenom).append(" ").append(pilote.nom);
        listeDeroulantePilote->addItem(nomPrenom, pilote.idPilote);
        choixPilote->addItem(nomPrenom, pilote.idPilote);
        choixPayeur->addItem(nomPrenom, pilote.idPilote);
    }
}

void AeroDms::peuplerListeSorties()
{
    choixBalade->clear();
    choixBaladeFacture->clear();
    choixBalade->addItem("", -1);
    choixBaladeFacture->addItem("", -1);

    AeroDmsTypes::ListeSortie sorties;
    if (typeDeVol->currentText() == "Balade")
    {
        sorties = db->recupererListeBalade();
    }
    else if (typeDeVol->currentText() == "Sortie")
    {
        sorties = db->recupererListeSorties();
    }
    
    for (int i = 0; i < sorties.size(); i++)
    {
        AeroDmsTypes::Sortie sortie = sorties.at(i);
        sortie.nom.append(" ").append(sortie.date.toString("dd/MM/yyyy"));
        choixBalade->addItem(sortie.nom, sortie.id);
    }

    sorties = db->recupererListeDepensesPouvantAvoirUneFacture();
    for (int i = 0; i < sorties.size(); i++)
    {
        AeroDmsTypes::Sortie sortie = sorties.at(i);
        sortie.nom.append(" ").append(sortie.date.toString("dd/MM/yyyy"));
        choixBaladeFacture->addItem(sortie.nom, sortie.id);
    }

}

void AeroDms::peuplerListeBaladesEtSorties()
{
    listeBaladesEtSorties->clear();

    QStringList itemLabels = db->recupererBaladesEtSorties(typeDeRecette->currentText());
    QStringListIterator it(itemLabels);
    while (it.hasNext())
    {
        QListWidgetItem* itemBaladesEtSorties = new QListWidgetItem(it.next(), listeBaladesEtSorties);
        itemBaladesEtSorties->setCheckState(Qt::Unchecked);
        listeBaladesEtSorties->addItem(itemBaladesEtSorties);
    }
}

void AeroDms::prevaliderDonnnesSaisies()
{
    validerLeVol->setEnabled(true);
    validerLaFacture->setEnabled(true);

    if ( prixDuVol->value() == 0
         || dureeDuVol->time() == QTime::QTime(0,0)
         || choixPilote->currentIndex() == 0
         || pdfDocument->status() != QPdfDocument::Status::Ready
         || ( typeDeVol->currentText() != "Entrainement" && choixBalade->currentIndex() == 0))
    {
        validerLeVol->setEnabled(false);
    }

    if ( choixBaladeFacture->currentIndex() == 0
         || choixPayeur->currentIndex() == 0
         || montantFacture->value() == 0
         || remarqueFacture->text() == ""
         || pdfDocument->status() != QPdfDocument::Status::Ready )
    {
        validerLaFacture->setEnabled(false);
    }

    aeroclubPiloteSelectionne->setText(db->recupererAeroclub(choixPilote->currentData().toString()));
}

void AeroDms::prevaliderDonnneesSaisiesRecette()
{
    validerLaRecette->setEnabled(true);

    if ( montantRecette->value() == 0 
         || intituleRecette->text() == "")
    {
        validerLaRecette->setEnabled(false);
    }
}

void AeroDms::changerInfosVolSurSelectionTypeVol()
{
    peuplerListeSorties();
    if (typeDeVol->currentText() == "Entrainement")
    {
        choixBalade->setEnabled(false);
        choixBalade->setCurrentIndex(0);
    }
    else if (typeDeVol->currentText() == "Balade")
    {
        choixBalade->setEnabled(false);
        choixBalade->setCurrentIndex(1);
    }
    else if (typeDeVol->currentText() == "Sortie")
    {
        choixBalade->setEnabled(true);
        choixBalade->setCurrentIndex(0);
    }
}

void AeroDms::ajouterUnPilote()
{
    dialogueGestionPilote->exec();
}

void AeroDms::ajouterUneSortie()
{
    dialogueAjouterSortie->exec();
}

void AeroDms::ajouterUneCotisation()
{
    dialogueAjouterCotisation->exec();
}

void AeroDms::aPropos()
{
    QMessageBox::about(this, tr("À propos de AeroDms"),
        "<b>AeroDms v"+QApplication::applicationVersion() + "</b> < br />< br /> "
        "Logiciel de gestion de compta section d'une section aéronautique. <br /><br />"
        "Le code source de ce programme est disponible sous"
        " <a href=\"https://github.com/cvermot/AeroDMS\">GitHub</a>.<br />< br/>"
        "Les icones sont issues de <a href=\"https://pictogrammers.com/\">pictogrammers.com</a>.< br />< br />"
        "Mentions légales : <br />"
        " This program is free software: you can redistribute it and/or modify"
        " it under the terms of the GNU General Public License as published by"
        " the Free Software Foundation, either version 3 of the License, or"
        " (at your option) any later version."
        "<br />< br/>"
        "This program is distributed in the hope that it will be useful,"
        " but WITHOUT ANY WARRANTY; without even the implied warranty of"
        " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
        " GNU General Public License for more details."
        "<br />< br/>"
        "You should have received a copy of the GNU General Public License"
        " along with this program.  If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.");
}

void AeroDms::menuContextuelPilotes(const QPoint& pos)
{
    if (vuePilotes->itemAt(pos) != nullptr)
    {
        QMenu menuClicDroitPilote(tr("Menu contextuel"), this);
        const int ligneSelectionnee = vuePilotes->itemAt(pos)->row();
        piloteAEditer = vuePilotes->item( ligneSelectionnee, 
                                          AeroDmsTypes::PiloteTableElement_PILOTE_ID)->text();

        const QIcon iconeAjouterPilote = QIcon("./ressources/account-tie-hat.svg");
        QAction editer(iconeAjouterPilote,"Editer le pilote", this);
        connect(&editer, SIGNAL(triggered()), this, SLOT(editerPilote()));
        menuClicDroitPilote.addAction(&editer);

        //Afficher le menu sur la vue des pilotes
        menuClicDroitPilote.exec(vuePilotes->mapToParent(QCursor::pos()));
    }
}

void AeroDms::editerPilote()
{
    dialogueGestionPilote->preparerMiseAJourPilote(piloteAEditer);
    dialogueGestionPilote->exec();
}

void AeroDms::menuContextuelVols(const QPoint& pos)
{
    if (vueVols->itemAt(pos) != nullptr)
    {
        QMenu menuClicDroitVol(tr("Menu contextuel"), this);
        const int ligneSelectionnee = vueVols->itemAt(pos)->row();
        volAEditer = (vueVols->item( ligneSelectionnee,
                                    AeroDmsTypes::VolTableElement_VOL_ID)->text()).toInt();

        const bool leVolEstSupprimable = (vueVols->item(ligneSelectionnee, AeroDmsTypes::VolTableElement_SOUMIS_CE)->text() == "Non");
        volPartiellementEditable = !leVolEstSupprimable;

        QAction editerLeVol(QIcon("./ressources/airplane-edit.svg"), "Editer le vol", this);
        connect(&editerLeVol, SIGNAL(triggered()), this, SLOT(editerVol()));
        menuClicDroitVol.addAction(&editerLeVol);

        QAction supprimerLeVol(QIcon("./ressources/airplane-remove.svg"), "Supprimer le vol", this);
        connect(&supprimerLeVol, SIGNAL(triggered()), this, SLOT(supprimerVol()));
        menuClicDroitVol.addAction(&supprimerLeVol);
        supprimerLeVol.setEnabled(leVolEstSupprimable);

        //Afficher le menu sur la vue des vols
        menuClicDroitVol.exec(vueVols->mapToParent(QCursor::pos()));
    }
}

void AeroDms::editerVol()
{
    //On récupère le nom de la facture associée et on la charge
    const QString cheminComplet = cheminStockageFacturesTraitees + "/" + db->recupererNomFacture(volAEditer);
    chargerUneFacture(cheminComplet);
    qDebug() << cheminComplet;

    //On indique que c'est une facture déjà en BDD

    //On récupère les infos du vol pour les réintegrer dans l'IHM
    const AeroDmsTypes::Vol vol = db->recupererVol(volAEditer);
    choixPilote->setCurrentIndex(listeDeroulantePilote->findData(vol.idPilote));
    choixPilote->setEnabled(false);

    //Type de vol jamais éditable
    typeDeVol->setCurrentIndex(typeDeVol->findText(vol.typeDeVol));
    typeDeVol->setEnabled(false);

    dateDuVol->setDate(vol.date);

    QStringList hhmm = vol.duree.split("h");
    dureeDuVol->setTime(QTime(hhmm.at(0).toInt(), hhmm.at(1).toInt(),0));

    prixDuVol->setValue(vol.coutVol);

    choixBalade->setCurrentIndex(choixBalade->findData(vol.baladeId));
    choixBalade->setEnabled(false);

    remarqueVol->setText(vol.remarque);

    //Si le vol est déjà soumis au CE, on ne peut plus modifier les temps de vol et le coût du vol :
    //(modifierait des montants de subventions déjà soumises au CE)
    //On va chercher cette infos dans la table des vol
    if (volPartiellementEditable)
    {
        dureeDuVol->setEnabled(false);
        prixDuVol->setEnabled(false);
    }

    validerLeVol->setText("Modifier le vol");
    
}

void AeroDms::supprimerVol()
{
    QMessageBox demandeConfirmationSuppression;
    demandeConfirmationSuppression.setText("Voulez vous réellement supprimer le vol ?");
    demandeConfirmationSuppression.setWindowTitle("Suppression d'un vol");
    demandeConfirmationSuppression.setIcon(QMessageBox::Question);
    demandeConfirmationSuppression.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    
    const int ret = demandeConfirmationSuppression.exec();
    
    switch (ret) 
    {
        case QMessageBox::Yes:
        {
            if (db->supprimerUnVol(volAEditer))
            {
                statusBar()->showMessage("Vol supprimé avec succès.");
            }
            else
            {
                statusBar()->showMessage("Vol non supprimé : le vol est associé à une recette d'une sortie. Suppression impossible.");
            }
            peuplerTablePilotes();
            peuplerTableVols();
        }
        break;
        case QMessageBox::No:
        default:
        {
            //Rien à faire
        }
        break;
    }

    //On sort du mode suppression de vol
    volAEditer = -1;
}

void AeroDms::switchModeDebug()
{
    //Si le mode debug n'est pas actif
    if (boutonModeDebug->text() == "Activer le mode &debug")
    {
        boutonModeDebug->setText("Désactiver le mode &debug");
        boutonModeDebug->setIcon(QIcon("./ressources/bug-stop.svg"));
        vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_PILOTE_ID, false);
        vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_VOL_ID, false);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_FACTURE_ID, false);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_NOM_FACTURE, false);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_ANNEE, false);
    }
    //Sinon, le mode est actif, on desactive
    else
    {
        boutonModeDebug->setText("Activer le mode &debug");
        boutonModeDebug->setIcon(QIcon("./ressources/bug.svg"));
        vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_PILOTE_ID, true);
        vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_VOL_ID, true);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_FACTURE_ID, true);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_NOM_FACTURE, true);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_ANNEE, true);
    }
}


