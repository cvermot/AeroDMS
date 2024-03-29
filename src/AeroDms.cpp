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
#include "AeroDmsServices.h"
#include "PdfRenderer.h"
#include "PdfExtractor.h"

#include "StatistiqueHistogrammeEmpile.h"
#include "StatistiqueDiagrammeCirculaireWidget.h"

#include <QtWidgets>
#include <QToolBar>
#include <QPdfPageNavigator>
#include <QtCore>

AeroDms::AeroDms(QWidget* parent) :QMainWindow(parent)
{
    QApplication::setApplicationName("AeroDms");
    QApplication::setApplicationVersion("3.7");
    QApplication::setWindowIcon(QIcon("./ressources/shield-airplane.svg"));
    mainTabWidget = new QTabWidget(this);
    setCentralWidget(mainTabWidget);

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "AeroDMS", "AeroDMS");

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
        settings.endGroup();
    }

    if (settings.value("mailing/texteChequesDisponibles", "") == "")
    {
        settings.beginGroup("mailing");
        settings.setValue("texteChequesDisponibles", "Bonjour,\n\nVos chèques sont disponibles. \n\nCordialement");
        settings.endGroup();
    }

    parametresMetiers.texteMailDispoCheques = settings.value("mailing/texteChequesDisponibles", "").toString();

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
        settingsMetier.setValue("proportionParticipationBalade", "0.375");
        settingsMetier.endGroup();
    }

    if (settingsMetier.value("parametresSysteme/delaisDeGardeDbEnMs", "") == "")
    {
        settingsMetier.beginGroup("parametresSysteme");
        settingsMetier.setValue("delaisDeGardeDbEnMs", "50");
        settingsMetier.endGroup();
    }

    const QString database = settings.value("baseDeDonnees/chemin", "").toString() +
        QString("/") +
        settings.value("baseDeDonnees/nom", "").toString();
    const QString ressourcesHtml = settings.value("baseDeDonnees/chemin", "").toString() +
        QString("/ressources/HTML");
    cheminStockageFacturesTraitees = settings.value("dossiers/facturesSaisies", "").toString();
    cheminStockageFacturesATraiter = settings.value("dossiers/facturesATraiter", "").toString();
    cheminSortieFichiersGeneres = settings.value("dossiers/sortieFichiersGeneres", "").toString();

    parametresMetiers.montantSubventionEntrainement = settingsMetier.value("parametresMetier/montantSubventionEntrainement", "750").toFloat();
    parametresMetiers.montantCotisationPilote = settingsMetier.value("parametresMetier/montantCotisationPilote", "15").toFloat();
    parametresMetiers.proportionRemboursementEntrainement = settingsMetier.value("parametresMetier/proportionRemboursementEntrainement", "0.5").toFloat();
    parametresMetiers.plafondHoraireRemboursementEntrainement = settingsMetier.value("parametresMetier/plafondHoraireRemboursementEntrainement", "150").toFloat();
    parametresMetiers.proportionRemboursementBalade = settingsMetier.value("parametresMetier/proportionRemboursementBalade", "0.875").toFloat();
    parametresMetiers.proportionParticipationBalade = settingsMetier.value("parametresMetier/proportionParticipationBalade", "0.375").toFloat();
    parametresMetiers.nomTresorier = settings.value("noms/nomTresorier", "").toString();
    parametresMetiers.delaisDeGardeBdd = settings.value("parametresSysteme/delaisDeGardeDbEnMs", "50").toInt();

    db = new ManageDb(database, parametresMetiers.delaisDeGardeBdd);
    pdf = new PdfRenderer(db,
        ressourcesHtml);

    installEventFilter(this);
    connect(this, &AeroDms::toucheEchapEstAppuyee, this, &AeroDms::deselectionnerVolDetecte);

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
    vueVols = new QTableWidget(0, AeroDmsTypes::VolTableElement_NB_COLONNES, this);
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_PILOTE, new QTableWidgetItem("Pilote"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_DATE, new QTableWidgetItem("Date"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_TYPE_DE_VOL, new QTableWidgetItem("Type"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_DUREE, new QTableWidgetItem("Durée"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_COUT, new QTableWidgetItem("Coût"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_SUBVENTION, new QTableWidgetItem("Subvention"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_SOUMIS_CE, new QTableWidgetItem("Soumis CE"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_REMARQUE, new QTableWidgetItem("Remarque"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_ACTIVITE, new QTableWidgetItem("Activité"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, new QTableWidgetItem("Durée en minutes"));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_VOL_ID, new QTableWidgetItem("ID"));
    vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_VOL_ID, true);
    vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, true);
    vueVols->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vueVols->setSelectionBehavior(QAbstractItemView::SelectRows);
    vueVols->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vueVols, QIcon("./ressources/airplane.svg"), "Vols");
    connect(vueVols, &QTableWidget::customContextMenuRequested, this, &AeroDms::menuContextuelVols);
    connect(vueVols, &QTableWidget::cellClicked, this, &AeroDms::volsSelectionnes);

    //==========Onglet Factures
    vueFactures = new QTableWidget(0, AeroDmsTypes::FactureTableElement_NB_COLONNES, this);
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

    //==========Onglet Recettes
    vueRecettes = new QTableWidget(0, AeroDmsTypes::RecetteTableElement_NB_COLONNES, this);
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_DATE, new QTableWidgetItem("Date"));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_TYPE_DE_RECETTE, new QTableWidgetItem("Type"));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_INTITULE, new QTableWidgetItem("Intitulé"));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_MONTANT, new QTableWidgetItem("Montant"));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_SOUMIS_CE, new QTableWidgetItem("Soumis CE"));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_ID, new QTableWidgetItem("ID"));
    vueRecettes->setColumnHidden(AeroDmsTypes::RecetteTableElement_ID, true);
    mainTabWidget->addTab(vueRecettes, QIcon("./ressources/cash-multiple.svg"), "Recettes");

    //==========Onglet Ajout dépense
    QHBoxLayout* ajoutVol = new QHBoxLayout(this);
    QWidget* widgetAjoutVol = new QWidget(this);
    widgetAjoutVol->setLayout(ajoutVol);
    mainTabWidget->addTab(widgetAjoutVol, QIcon("./ressources/file-document-minus.svg"), "Ajout dépense");

    pdfDocument = new QPdfDocument(this);
    pdfView = new QPdfView(this);
    pdfView->setDocument(pdfDocument);
    pdfView->setPageMode(QPdfView::PageMode::MultiPage);
    ajoutVol->addWidget(pdfView, 3);

    QTabWidget* depenseTabWidget = new QTabWidget(this);
    ajoutVol->addWidget(depenseTabWidget, 1);

    //==========Sous onglet vol de l'onglet "Ajout dépense"
    typeDeVol = new QComboBox(this);
    typeDeVol->addItems(db->recupererTypesDesVol());
    typeDeVol->setCurrentIndex(2);
    AeroDmsServices::ajouterIconesComboBox(*typeDeVol);
    QLabel* typeDeVolLabel = new QLabel(tr("Type de vol : "), this);
    connect(typeDeVol, &QComboBox::currentIndexChanged, this, &AeroDms::changerInfosVolSurSelectionTypeVol);
    connect(typeDeVol, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    choixPilote = new QComboBox(this);
    QLabel* choixPiloteLabel = new QLabel(tr("Pilote : "), this);
    connect(choixPilote, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);
    connect(choixPilote, &QComboBox::currentIndexChanged, this, &AeroDms::mettreAJourInfosSurSelectionPilote);

    aeroclubPiloteSelectionne = new QLineEdit(this);
    aeroclubPiloteSelectionne->setToolTip(tr("Nom de l'aéroclub renseigné pour le pilote sélectionné et au nom duquel le\n"
        "remboursement sera émis. En cas de changement d'aéroclub, modifier\n"
        "l'aéroclub du pilote avant de générer les demandes de subvention."));
    aeroclubPiloteSelectionne->setEnabled(false);
    QLabel* aeroclubPiloteSelectionneLabel = new QLabel(tr("Aéroclub : "), this);

    activite = new QComboBox(this);
    activite->setToolTip("L'activité est fournie à titre statistique uniquement.\n"
        "Ce champ est rempli automatiquement à partir de l'activité\n"
        "par défaut renseignée pour le pilote sélectionné.\n"
        "Il est modifiable ici pour les pilotes qui pratiqueraient plusieurs activités.");
    activite->addItems(db->recupererListeActivites());
    AeroDmsServices::ajouterIconesComboBox(*activite);
    QLabel* activiteLabel = new QLabel(tr("Activité : "), this);

    dateDuVol = new QDateTimeEdit(this);
    dateDuVol->setDisplayFormat("dd/MM/yyyy");
    dateDuVol->setCalendarPopup(true);
    dateDuVol->setDate(QDate::currentDate());
    QLabel* dateDuVolLabel = new QLabel(tr("Date du vol : "), this);
    connect(dateDuVol, &QDateEdit::dateChanged, this, &AeroDms::prevaliderDonnnesSaisies);

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
    validerLeVol->setToolTip(tr("Validation possible si :\n\
   -pilote sélectionné,\n\
   -durée de vol saisi,\n\
   -montant du vol saisi,\n\
   -si Type de vol est \"Sortie\", sortie sélectionnée,\n\
   -facture chargée."));
    connect(validerLeVol, &QPushButton::clicked, this, &AeroDms::enregistrerUnVol);

    QGridLayout* infosVol = new QGridLayout(this);
    QWidget* widgetDepenseVol = new QWidget(this);
    widgetDepenseVol->setLayout(infosVol);
    depenseTabWidget->addTab(widgetDepenseVol, QIcon("./ressources/airplane-clock.svg"), "Heures de vol");
    infosVol->addWidget(typeDeVolLabel, 0, 0);
    infosVol->addWidget(typeDeVol, 0, 1);
    infosVol->addWidget(choixPiloteLabel, 1, 0);
    infosVol->addWidget(choixPilote, 1, 1);
    infosVol->addWidget(aeroclubPiloteSelectionneLabel, 2, 0);
    infosVol->addWidget(aeroclubPiloteSelectionne, 2, 1);
    infosVol->addWidget(activiteLabel, 3, 0);
    infosVol->addWidget(activite, 3, 1);
    infosVol->addWidget(dateDuVolLabel, 4, 0);
    infosVol->addWidget(dateDuVol, 4, 1);
    infosVol->addWidget(dureeDuVolLabel, 5, 0);
    infosVol->addWidget(dureeDuVol, 5, 1);
    infosVol->addWidget(prixDuVolLabel, 6, 0);
    infosVol->addWidget(prixDuVol, 6, 1);
    infosVol->addWidget(choixBaladeLabel, 7, 0);
    infosVol->addWidget(choixBalade, 7, 1);
    infosVol->addWidget(remarqueVolLabel, 8, 0);
    infosVol->addWidget(remarqueVol, 8, 1);
    infosVol->addWidget(validerLeVol, 9, 0, 2, 0);

    initialiserTableauVolsDetectes(infosVol);

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
    validerLaFacture->setToolTip("Validation possible si : \n\
   -facture chargée,\n\
   -montant de la facture renseignée,\n\
   -payeur et sortie sélectionnés,\n\
   -intitulé saisi.");
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
    ajoutRecette->addWidget(listeBaladesEtSorties, 3);

    typeDeRecette = new QComboBox(this);
    typeDeRecette->addItems(db->recupererTypesDesVol(true));
    AeroDmsServices::ajouterIconesComboBox(*typeDeRecette);
    connect(typeDeRecette, &QComboBox::currentIndexChanged, this, &AeroDms::chargerBaladesSorties);
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
    validerLaRecette->setToolTip(tr("Validation possible si :\n\
   -montant de recette saisi (non nul),\n\
   -intitulé saisi,\n\
   -au moins un vol associé séléctionné."));
    connect(validerLaRecette, &QPushButton::clicked, this, &AeroDms::enregistrerUneRecette);

    QGridLayout* infosRecette = new QGridLayout(this);
    ajoutRecette->addLayout(infosRecette, 1);
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
    bouttonAjouterUnVol = new QAction(iconeAjouterUnVol, tr("&Ajouter un vol/une dépense"), this);
    bouttonAjouterUnVol->setStatusTip(tr("Ajouter un vol/une dépense"));
    connect(bouttonAjouterUnVol, &QAction::triggered, this, &AeroDms::selectionnerUneFacture);
    toolBar->addAction(bouttonAjouterUnVol);

    const QIcon iconeAjouterPilote = QIcon("./ressources/account-tie-hat.svg");
    bouttonAjouterPilote = new QAction(iconeAjouterPilote, tr("&Ajouter un pilote"), this);
    bouttonAjouterPilote->setStatusTip(tr("Ajouter un pilote"));
    connect(bouttonAjouterPilote, &QAction::triggered, this, &AeroDms::ajouterUnPilote);
    toolBar->addAction(bouttonAjouterPilote);

    const QIcon iconeAjouterCotisation = QIcon("./ressources/ticket.svg");
    bouttonAjouterCotisation = new QAction(iconeAjouterCotisation, tr("&Ajouter une cotisation pour un pilote"), this);
    bouttonAjouterCotisation->setStatusTip(tr("Ajouter une cotisation pour un pilote"));
    connect(bouttonAjouterCotisation, &QAction::triggered, this, &AeroDms::ajouterUneCotisation);
    toolBar->addAction(bouttonAjouterCotisation);

    const QIcon iconeAjouterSortie = QIcon("./ressources/transit-connection-variant.svg");
    bouttonAjouterSortie = new QAction(iconeAjouterSortie, tr("&Ajouter une sortie"), this);
    bouttonAjouterSortie->setStatusTip(tr("Ajouter une sortie"));
    connect(bouttonAjouterSortie, &QAction::triggered, this, &AeroDms::ajouterUneSortie);
    toolBar->addAction(bouttonAjouterSortie);

    toolBar->addSeparator();

    const QIcon iconeGenerePdf = QIcon("./ressources/file-cog.svg");
    bouttonGenerePdf = new QAction(iconeGenerePdf, tr("&Générer les PDF de demande de subvention"), this);
    bouttonGenerePdf->setStatusTip(tr("Générer les PDF de demande de subvention"));
    connect(bouttonGenerePdf, &QAction::triggered, this, &AeroDms::genererPdf);
    toolBar->addAction(bouttonGenerePdf);

    const QIcon iconeGenerePdfRecapHdv = QIcon("./ressources/account-file-text.svg");
    bouttonGenerePdfRecapHdv = new QAction(iconeGenerePdfRecapHdv, tr("&Générer les PDF de recap HdV de l'année sélectionnée"), this);
    bouttonGenerePdfRecapHdv->setStatusTip(tr("Générer les PDF de recap HdV de l'année sélectionnée"));
    connect(bouttonGenerePdfRecapHdv, &QAction::triggered, this, &AeroDms::genererPdfRecapHdV);
    toolBar->addAction(bouttonGenerePdfRecapHdv);

    QToolBar* selectionToolBar = addToolBar(tr(""));

    listeDeroulanteAnnee = new QComboBox(this);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTablePilotes);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableRecettes);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableFactures);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerStatistiques);
    selectionToolBar->addWidget(listeDeroulanteAnnee);

    listeDeroulantePilote = new QComboBox(this);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTablePilotes);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    selectionToolBar->addWidget(listeDeroulantePilote);

    listeDeroulanteStatistique = new QComboBox(this);
    listeDeroulanteStatistique->addItem("Statistiques mensuelles", AeroDmsTypes::Statistiques_HEURES_ANNUELLES);
    listeDeroulanteStatistique->setItemIcon(AeroDmsTypes::Statistiques_HEURES_ANNUELLES, QIcon("./ressources/chart-bar-stacked.svg"));
    listeDeroulanteStatistique->addItem("Statistiques par pilote", AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE);
    listeDeroulanteStatistique->setItemIcon(AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE, QIcon("./ressources/chart-pie.svg"));
    listeDeroulanteStatistique->addItem("Statistiques par type de vol", AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL);
    listeDeroulanteStatistique->setItemIcon(AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL, QIcon("./ressources/chart-pie.svg"));
    listeDeroulanteStatistique->addItem("Statistiques par activité", AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE);
    listeDeroulanteStatistique->setItemIcon(AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE, QIcon("./ressources/chart-pie.svg"));
    connect(listeDeroulanteStatistique, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerStatistiques);
    selectionToolBar->addWidget(listeDeroulanteStatistique);

    //Fenêtres
    dialogueGestionPilote = new DialogueGestionPilote(db, this);
    connect(dialogueGestionPilote, SIGNAL(accepted()), this, SLOT(ajouterUnPiloteEnBdd()));

    dialogueAjouterCotisation = new DialogueAjouterCotisation(db,
        parametresMetiers.montantCotisationPilote,
        parametresMetiers.montantSubventionEntrainement,
        this);
    connect(dialogueAjouterCotisation, SIGNAL(accepted()), this, SLOT(ajouterUneCotisationEnBdd()));

    dialogueAjouterSortie = new DialogueAjouterSortie(this);
    connect(dialogueAjouterSortie, SIGNAL(accepted()), this, SLOT(ajouterUneSortieEnBdd()));

    //Dialogue de progression de génération PDF
    progressionGenerationPdf = new QProgressDialog("Génération PDF en cours", "", 0, 0, this);
    boutonProgressionGenerationPdf = new QPushButton("Fermer", this);
    progressionGenerationPdf->setCancelButton(boutonProgressionGenerationPdf);
    progressionGenerationPdf->setAutoClose(false);
    progressionGenerationPdf->setWindowModality(Qt::WindowModal);
    progressionGenerationPdf->close();
    progressionGenerationPdf->setAutoReset(false);

    connect(pdf, SIGNAL(mettreAJourNombreFacturesATraiter(int)), this, SLOT(ouvrirFenetreProgressionGenerationPdf(int)));
    connect(pdf, SIGNAL(mettreAJourNombreFacturesTraitees(int)), this, SLOT(mettreAJourFenetreProgressionGenerationPdf(int)));
    connect(pdf, SIGNAL(generationTerminee(QString)), this, SLOT(mettreAJourBarreStatusFinGenerationPdf(QString)));

    //========================Menu Options
    QMenu* menuOption = menuBar()->addMenu(tr("Options"));

    QMenu* menuSignature = menuOption->addMenu(tr("Signature"));
    menuSignature->setIcon(QIcon("./ressources/file-sign.svg"));

    boutonAucuneSignature = new QAction(QIcon("./ressources/file-outline.svg"), tr("Signature manuelle"), this);
    menuSignature->addAction(boutonAucuneSignature);

    boutonSignatureManuelle = new QAction(QIcon("./ressources/draw-pen.svg"), tr("Utiliser l'image d'une signature"), this);
    menuSignature->addAction(boutonSignatureManuelle);

    boutonSignatureNumerique = new QAction(QIcon("./ressources/lock-check-outline.svg"), tr("Signature numérique avec Lex Community"), this);
    menuSignature->addAction(boutonSignatureNumerique);

    connect(boutonAucuneSignature, SIGNAL(triggered()), this, SLOT(changerModeSignature()));
    connect(boutonSignatureNumerique, SIGNAL(triggered()), this, SLOT(changerModeSignature()));
    connect(boutonSignatureManuelle, SIGNAL(triggered()), this, SLOT(changerModeSignature()));

    QMenu* menuFusionnerLesPdf = menuOption->addMenu(tr("Fusion des PDF"));
    menuFusionnerLesPdf->setIcon(QIcon("./ressources/paperclip.svg"));
    boutonFusionnerLesPdf = new QAction(QIcon("./ressources/paperclip-check.svg"), tr("Fusionner les PDF"), this);
    menuFusionnerLesPdf->addAction(boutonFusionnerLesPdf);
    boutonNePasFusionnerLesPdf = new QAction(QIcon("./ressources/paperclip-off.svg"), tr("Ne pas fusionner les PDF"), this);
    menuFusionnerLesPdf->addAction(boutonNePasFusionnerLesPdf);

    connect(boutonFusionnerLesPdf, SIGNAL(triggered()), this, SLOT(changerFusionPdf()));
    connect(boutonNePasFusionnerLesPdf, SIGNAL(triggered()), this, SLOT(changerFusionPdf()));

    QMenu* menuDemandesAGenerer = menuOption->addMenu(tr("Demandes à générer"));
    menuDemandesAGenerer->setIcon(QIcon("./ressources/file-cog.svg"));
    boutonDemandesAGenererToutes = new QAction(QIcon("./ressources/file-document-multiple.svg"), tr("Toutes"), this);
    menuDemandesAGenerer->addAction(boutonDemandesAGenererToutes);
    boutonDemandesAGenererRecettes = new QAction(QIcon("./ressources/file-document-plus.svg"), tr("Recettes uniquement"), this);
    menuDemandesAGenerer->addAction(boutonDemandesAGenererRecettes);

    boutonDemandesAGenererDepenses = new QAction(QIcon("./ressources/file-document-minus.svg"), tr("Dépenses uniquement"), this);
    menuDemandesAGenerer->addAction(boutonDemandesAGenererDepenses);

    connect(boutonDemandesAGenererToutes, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));
    connect(boutonDemandesAGenererRecettes, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));
    connect(boutonDemandesAGenererDepenses, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));

    menuOption->addSeparator();

    boutonActivationScanAutoFactures = new QAction(QIcon("./ressources/file-search.svg"), tr("Desactiver le scan automatique des factures"), this);
    boutonActivationScanAutoFactures->setStatusTip(tr("Convertir une heure sous forme décimale (X,y heures) en HH:mm"));
    menuOption->addAction(boutonActivationScanAutoFactures);
    connect(boutonActivationScanAutoFactures, SIGNAL(triggered()), this, SLOT(switchScanAutomatiqueDesFactures()));

    QFont font;
    font.setWeight(QFont::Bold);
    boutonDemandesAGenererToutes->setFont(font);
    typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_TOUTES;
    boutonAucuneSignature->setFont(font);
    signature = AeroDmsTypes::Signature_SANS;
    boutonFusionnerLesPdf->setFont(font);

    //========================Menu Outils
    QMenu* menuOutils = menuBar()->addMenu(tr("Outils"));

    menuOutils->addAction(bouttonAjouterUnVol);
    menuOutils->addAction(bouttonAjouterCotisation);
    menuOutils->addAction(bouttonAjouterPilote);
    menuOutils->addAction(bouttonAjouterSortie);
    menuOutils->addSeparator();
    menuOutils->addAction(bouttonGenerePdf);
    menuOutils->addAction(bouttonGenerePdfRecapHdv);

    menuOutils->addSeparator();

    QMenu* scanFacture = menuOutils->addMenu(tr("Scan automatique des factures"));
    scanFacture->setToolTip(tr("Scan une facture en se basant sur une des méthode\nutilisée par le logiciel pour un type de facture déjà connu."));
    scanFacture->setIcon(QIcon("./ressources/file-search.svg"));
    scanAutoOpenFlyer = new QAction(tr("OpenFlyer (CAPAM, ACB)"), this);
    scanAutoOpenFlyer->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoOpenFlyer);
    scanAutoAca = new QAction(tr("Aéroclub d'Andernos"), this);
    scanAutoAca->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoAca);
    scanAutoDaca = new QAction(tr("DACA"), this);
    scanAutoDaca->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoDaca);
    scanFacture->addSeparator();
    scanAutoGenerique = new QAction(tr("Générique"), this);
    scanAutoGenerique->setIcon(QIcon("./ressources/text-box-search.svg"));
    scanFacture->addAction(scanAutoGenerique);
    connect(scanAutoOpenFlyer, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoAca, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoDaca, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoGenerique, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));

    menuOutils->addSeparator();

    QMenu* mailing = menuOutils->addMenu(tr("Mailing"));
    mailing->setIcon(QIcon("./ressources/email-multiple.svg"));
    mailingPilotesAyantCotiseCetteAnnee = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes ayant cotisé cette année"), this);
    mailing->addAction(mailingPilotesAyantCotiseCetteAnnee);
    mailingPilotesActifsAyantCotiseCetteAnnee = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes ayant cotisé cette année (pilotes actifs seulement)"), this);
    mailing->addAction(mailingPilotesActifsAyantCotiseCetteAnnee);
    mailingPilotesActifsBrevetes = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes actifs brevetés"), this);
    mailing->addAction(mailingPilotesActifsBrevetes);
    mailingPilotesDerniereDemandeSubvention = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes concernés par la dernière demande de subvention"), this);
    mailing->addAction(mailingPilotesDerniereDemandeSubvention);
    connect(mailingPilotesAyantCotiseCetteAnnee, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesActifsAyantCotiseCetteAnnee, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesActifsBrevetes, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesDerniereDemandeSubvention, SIGNAL(triggered()), this, SLOT(envoyerMail()));

    menuOutils->addSeparator();

    QAction* boutonConversionHeureDecimalesVersHhMm = new QAction(QIcon("./ressources/clock-star-four-points.svg"), tr("Convertir une heure en décimal"), this);
    boutonConversionHeureDecimalesVersHhMm->setStatusTip(tr("Convertir une heure sous forme décimale (X,y heures) en HH:mm"));
    menuOutils->addAction(boutonConversionHeureDecimalesVersHhMm);
    connect(boutonConversionHeureDecimalesVersHhMm, SIGNAL(triggered()), this, SLOT(convertirHeureDecimalesVersHhMm()));

    //========================Menu Aide
    QMenu* helpMenu = menuBar()->addMenu(tr("Aide"));
    QAction* aideQtAction = new QAction(QIcon("./ressources/lifebuoy.svg"), tr("Aide en ligne"), this);
    aideQtAction->setStatusTip(tr("Ouvrir l'aide en ligne"));
    helpMenu->addAction(aideQtAction);
    connect(aideQtAction, SIGNAL(triggered()), this, SLOT(ouvrirAide()));
    helpMenu->addSeparator();
    boutonModeDebug = new QAction(QIcon("./ressources/bug.svg"), tr("Activer le mode &debug"), this);
    helpMenu->addAction(boutonModeDebug);
    connect(boutonModeDebug, SIGNAL(triggered()), this, SLOT(switchModeDebug()));
    helpMenu->addSeparator();
    QAction* aboutQtAction = new QAction(QIcon("./ressources/qt-logo.svg"), tr("À propos de &Qt"), this);
    aboutQtAction->setStatusTip(tr("Voir la fenêtre à propos de Qt"));
    helpMenu->addAction(aboutQtAction);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QAction* aboutAction = new QAction(QIcon("./ressources/shield-airplane.svg"), tr("À propos de &AeroDms"), this);
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
    peuplerTableRecettes();
    peuplerListeDeroulanteAnnee();
    peuplerStatistiques();
    prevaliderDonnnesSaisies();
    prevaliderDonnneesSaisiesRecette();
    changerInfosVolSurSelectionTypeVol();
    verifierSignatureNumerisee();

    verifierPresenceDeMiseAjour(settings.value("baseDeDonnees/chemin", "").toString());
}

void AeroDms::verifierPresenceDeMiseAjour(const QString p_chemin)
{
    const QString fichierAVerifier = p_chemin + "/update/AeroDms.exe";
    if (uneMaJEstDisponible(fichierAVerifier))
    {
        if (!db->laBddEstALaVersionAttendue())
        {
            passerLeLogicielEnLectureSeule();

            QMessageBox::critical(this, 
                "Une mise à jour est disponible", 
                "Une mise à jour de l'application est disponible et doit être réalisée\n\
car la base de données a évoluée.\n\n\
L'application va passer en mode lecture seule.\
\n\nPour mettre à jour l'application, recopier le fichier\n"
+ fichierAVerifier + "\ndans le repertoire de cette application.");
        }
        else
        {
            QMessageBox::information(this, 
                "Une mise à jour est disponible", 
                "Une mise à jour de l'application est disponible.\n\
Il est fortement recommandé d'effectuer cette mise à jour.\
\n\nPour mettre à jour l'application, recopier le fichier\n"
+ fichierAVerifier + "\ndans le repertoire de cette application.");
        }
    }
    else if (!db->laBddEstALaVersionAttendue())
    {
        passerLeLogicielEnLectureSeule();

        QMessageBox::critical(this,
            "Erreur de version de base de données",
            "La version de la base de données ne correspond pas à la version attendue par le logiciel.\n\n\
L'application va passer en mode lecture seule pour éviter tout risque d'endommagement de la BDD.\n\n\
Consultez le développeur/responsable de l'application pour plus d'informations.");
    }
}

void AeroDms::passerLeLogicielEnLectureSeule()
{
    bouttonAjouterUnVol->setEnabled(false);
    bouttonAjouterCotisation->setEnabled(false);
    bouttonAjouterPilote->setEnabled(false);
    bouttonAjouterSortie->setEnabled(false);
    bouttonGenerePdf->setEnabled(false);

    logicielEnModeLectureSeule = true;
}

void AeroDms::verifierSignatureNumerisee()
{
    if (!QFile("./ressources/signature.jpg").exists())
    {
        boutonSignatureManuelle->setEnabled(false);
    }
}

void AeroDms::initialiserOngletGraphiques()
{
    graphiques = new QHBoxLayout(this);
    QWidget* widgetGraphiques = new QWidget(this);
    widgetGraphiques->setLayout(graphiques);
    mainTabWidget->addTab(widgetGraphiques, QIcon("./ressources/chart-areaspline.svg"), "Graphiques"); 
}

void AeroDms::changerModeSignature()
{
    QFont font;
    font.setWeight(QFont::Normal);
    boutonAucuneSignature->setFont(font);
    boutonSignatureManuelle->setFont(font);
    boutonSignatureNumerique->setFont(font);

    font.setWeight(QFont::Bold);
    if (sender() == boutonAucuneSignature)
    {
        boutonAucuneSignature->setFont(font);
        signature = AeroDmsTypes::Signature_SANS;
    }
    else if (sender() == boutonSignatureManuelle)
    {
        boutonSignatureManuelle->setFont(font);
        signature = AeroDmsTypes::Signature_MANUSCRITE_IMAGE;
    }
    else if (sender() == boutonSignatureNumerique)
    {
        boutonSignatureNumerique->setFont(font);
        signature = AeroDmsTypes::Signature_NUMERIQUE_LEX_COMMUNITY;
    }
}

void AeroDms::changerDemandesAGenerer()
{
    QFont font;
    font.setWeight(QFont::Normal);
    boutonDemandesAGenererToutes->setFont(font);
    boutonDemandesAGenererRecettes->setFont(font);
    boutonDemandesAGenererDepenses->setFont(font);

    font.setWeight(QFont::Bold);
    if (sender() == boutonDemandesAGenererToutes)
    {
        boutonDemandesAGenererToutes->setFont(font);
        typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_TOUTES;
    }
    else if (sender() == boutonDemandesAGenererRecettes)
    {
        boutonDemandesAGenererRecettes->setFont(font);
        typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT;
    }
    else if (sender() == boutonDemandesAGenererDepenses)
    {
        boutonDemandesAGenererDepenses->setFont(font);
        typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT;
    }
}

void AeroDms::changerFusionPdf()
{
    QFont font;
    font.setWeight(QFont::Normal);
    boutonFusionnerLesPdf->setFont(font);
    boutonNePasFusionnerLesPdf->setFont(font);

    font.setWeight(QFont::Bold);
    if (sender() == boutonFusionnerLesPdf)
    {
        boutonFusionnerLesPdf->setFont(font);
    }
    else if (sender() == boutonNePasFusionnerLesPdf)
    {
        boutonNePasFusionnerLesPdf->setFont(font);
    }
}

void AeroDms::peuplerStatistiques()
{
    //On affiche le widget qui contient la fonction d'ajout de vol
    if (sender() == listeDeroulanteStatistique)
    {
        mainTabWidget->setCurrentIndex(AeroDmsTypes::Onglet_STATISTIQUES);
    }

    if (m_activeWidget) {
        m_activeWidget->setVisible(false);
        m_activeWidget->deleteLater();
    }

    switch (listeDeroulanteStatistique->currentData().toInt())
    {
        case AeroDmsTypes::Statistiques_HEURES_ANNUELLES:
        {
            m_activeWidget = new StatistiqueHistogrammeEmpile( db, 
                                                               listeDeroulanteAnnee->currentData().toInt(), 
                                                               m_contentArea);
            break;
        }
        case AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget( db, 
                                                                       listeDeroulanteAnnee->currentData().toInt(), 
                                                                       AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE, 
                                                                       m_contentArea);
            break;
        }
        case AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget( db, 
                                                                       listeDeroulanteAnnee->currentData().toInt(), 
                                                                       AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE, 
                                                                       m_contentArea);
            break;
        }
        case AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL:
        default:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget( db, 
                                                                       listeDeroulanteAnnee->currentData().toInt(), 
                                                                       AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL, 
                                                                       m_contentArea);
            break;
        }
    }
    graphiques->addWidget(m_activeWidget);

    m_activeWidget->load();
    m_activeWidget->setVisible(true);
}

void AeroDms::ouvrirFenetreProgressionGenerationPdf(const int p_nombreDeFacturesATraiter)
{
    boutonProgressionGenerationPdf->setDisabled(true);
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
    boutonProgressionGenerationPdf->setDisabled(false);
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
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_ACTIVITE, new QTableWidgetItem(vol.activite));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, new QTableWidgetItem(QString::number(vol.dureeEnMinutes)));
        vueVols->setItem(i, AeroDmsTypes::VolTableElement_VOL_ID, new QTableWidgetItem(QString::number(vol.volId)));
    }
    vueVols->resizeColumnsToContents();

    bouttonGenerePdfRecapHdv->setEnabled(true);
    mailingPilotesAyantCotiseCetteAnnee->setEnabled(true);
    //On desactive la génération du récap annuel si on est sur la séléction "Toutes les années"
    //et également le bouton d'envoi des mails aux pilotes de l'année
    if (listeDeroulanteAnnee->currentData().toInt() == -1)
    {
        bouttonGenerePdfRecapHdv->setEnabled(false);
        mailingPilotesAyantCotiseCetteAnnee->setEnabled(false);
    }
}

void AeroDms::peuplerTableVolsDetectes(const AeroDmsTypes::ListeDonneesFacture p_factures)
{
    vueVolsDetectes->setRowCount(p_factures.size());

    for (int i = 0; i < p_factures.size(); i++)
    {
        AeroDmsTypes::DonneesFacture facture = p_factures.at(i);
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_DATE, new QTableWidgetItem(facture.dateDuVol.toString("dd/MM/yyyy")));
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_DUREE, new QTableWidgetItem(facture.dureeDuVol.toString("hh:mm")));
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_MONTANT, new QTableWidgetItem(QString::number(facture.coutDuVol).append(" €")));
        //Par défaut => vol entrainement
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_TYPE, new QTableWidgetItem(typeDeVol->itemText(2)));
    }

    vueVolsDetectes->resizeColumnsToContents();
    
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

void AeroDms::peuplerTableRecettes()
{
    const AeroDmsTypes::ListeRecetteDetail listeRecettesCotisations = db->recupererRecettesCotisations(listeDeroulanteAnnee->currentData().toInt());
    const AeroDmsTypes::ListeRecetteDetail listeRecettesHorsCotisations = db->recupererRecettesHorsCotisation(listeDeroulanteAnnee->currentData().toInt());

    vueRecettes->setRowCount( listeRecettesCotisations.size()
                              + listeRecettesHorsCotisations.size());

    for (int i = 0; i < listeRecettesCotisations.size(); i++)
    {
        AeroDmsTypes::RecetteDetail recette = listeRecettesCotisations.at(i);
        vueRecettes->setItem(i, AeroDmsTypes::RecetteTableElement_INTITULE, new QTableWidgetItem(recette.intitule));
        vueRecettes->setItem(i, AeroDmsTypes::RecetteTableElement_ID, new QTableWidgetItem(recette.id));
        vueRecettes->setItem(i, AeroDmsTypes::RecetteTableElement_MONTANT, new QTableWidgetItem(QString::number(recette.montant) + " €"));
        vueRecettes->setItem(i, AeroDmsTypes::RecetteTableElement_DATE, new QTableWidgetItem(QString::number(recette.annee)));
        vueRecettes->setItem(i, AeroDmsTypes::RecetteTableElement_TYPE_DE_RECETTE, new QTableWidgetItem(recette.typeDeRecette));
        if (recette.estSoumisCe)
            vueRecettes->setItem(i, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, new QTableWidgetItem("Oui"));
        else
            vueRecettes->setItem(i, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, new QTableWidgetItem("Non"));
    }

    for (int i = 0; i < listeRecettesHorsCotisations.size(); i++)
    {
        AeroDmsTypes::RecetteDetail recette = listeRecettesHorsCotisations.at(i);

        const int position = listeRecettesCotisations.size() + i;

        vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_INTITULE, new QTableWidgetItem(recette.intitule));
        vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_ID, new QTableWidgetItem(recette.id));
        vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_MONTANT, new QTableWidgetItem(QString::number(recette.montant) + " €"));
        vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_DATE, new QTableWidgetItem(QString::number(recette.annee)));
        vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_TYPE_DE_RECETTE, new QTableWidgetItem(recette.typeDeRecette));
        if (recette.estSoumisCe)
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, new QTableWidgetItem("Oui"));
        else
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, new QTableWidgetItem("Non"));
    }

    vueRecettes->resizeColumnsToContents();
    vueRecettes->sortItems(AeroDmsTypes::RecetteTableElement_DATE);
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

        peuplerTableRecettes();
        //On peut avoir réactivé un pilote inactif : on réélabore les listes de pilotes
        peuplerListesPilotes();
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
        idFactureDetectee = -1;

        //On masque par défaut... on reaffiche si le scan est effectué
        //et qu'il ne retourne par une liste vide
        validerLesVols->setHidden(true);
        vueVolsDetectes->setHidden(true);
        supprimerLeVolSelectionne->setHidden(true);
        if (scanAutomatiqueDesFacturesEstActif)
        {
            factures = PdfExtractor::recupererLesDonneesDuPdf(fichier);
            if (factures.size() != 0)
            {
                peuplerTableVolsDetectes(factures);
                validerLesVols->setHidden(false);
                vueVolsDetectes->setHidden(false);
                supprimerLeVolSelectionne->setHidden(false);
            }
        }
        
        //Si on passe ici, on est pas en édition de vol
        volAEditer = -1;
        //On restaure le texte du bouton de validation (qui a changé si on était en édition)
        validerLeVol->setText("Valider le vol");
    }
}

void AeroDms::scannerUneFactureSelonMethodeChoisie()
{
    AeroDmsTypes::Aeroclub aeroclub = AeroDmsTypes::Aeroclub_INCONNU;

    if (sender() == scanAutoOpenFlyer)
    {
        aeroclub = AeroDmsTypes::Aeroclub_Generique_OpenFlyer;
    }
    else if (sender() == scanAutoAca)
    {
        aeroclub = AeroDmsTypes::Aeroclub_ACAndernos;
    }
    else if (sender() == scanAutoDaca)
    {
        aeroclub = AeroDmsTypes::Aeroclub_DACA;
    }
    else if (sender() == scanAutoGenerique)
    {
        aeroclub = AeroDmsTypes::Aeroclub_GENERIQUE;
    }

    if (pdfDocument->status() == QPdfDocument::Status::Ready)
    {
        factures = PdfExtractor::recupererLesDonneesDuPdf( cheminDeLaFactureCourante,
                                                           aeroclub);
        //On masque par défaut... on reaffiche si le scan est effectué
        //et qu'il ne retourne par une liste vide
        validerLesVols->setHidden(true);
        vueVolsDetectes->setHidden(true);
        supprimerLeVolSelectionne->setHidden(true);
        if (factures.size() != 0)
        {
            peuplerTableVolsDetectes(factures);
            validerLesVols->setHidden(false);
            vueVolsDetectes->setHidden(false);
            supprimerLeVolSelectionne->setHidden(false);
        }
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
    mainTabWidget->setCurrentIndex(AeroDmsTypes::Onglet_AJOUT_DEPENSES);
}

void AeroDms::genererPdf()
{
    QString texteSignature = "<b>Signature : </b>";
    switch (signature)
    {
        case AeroDmsTypes::Signature_MANUSCRITE_IMAGE:
        {
            texteSignature += "Image d'une signature manuscrite";
        }
        break;
        case AeroDmsTypes::Signature_NUMERIQUE_LEX_COMMUNITY:
        {
            texteSignature += "Signature numérique Lex Community";
        }
        break;
        case AeroDmsTypes::Signature_SANS:
        default:
        {
            texteSignature += "Sans (signature manuscrite)";
        }
        break;
    }

    QString nbSubventions = QString::number(db->recupererLesSubventionsAEmettre().size());
    QString nbFactures = QString::number(db->recupererLesDemandesDeRembousementAEmettre().size());
    QString nbCotisations = QString::number(db->recupererLesCotisationsAEmettre().size());
    QString nbBaladesSorties = QString::number(db->recupererLesRecettesBaladesEtSortiesAEmettre().size());

    QString texteDemande = "<b>Demandes à générer : </b>";
    switch (typeGenerationPdf)
    {
        case AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT:
        {
            texteDemande += "Recettes seulement";
            nbSubventions = "Aucune (recettes seulement)";
            nbFactures = "Aucune (recettes seulement)";
        }
        break;
        case AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT:
        {
            texteDemande += "Dépenses seulement";
            nbCotisations = "Aucune (dépenses seulement)";
            nbBaladesSorties = "Aucune (dépenses seulement)";
        }
        break;
        case AeroDmsTypes::TypeGenerationPdf_TOUTES:
        default:
        {
            texteDemande += "Toutes (recettes et dépenses)";
        }
        break;
    }

    QString fusionnerLesPdf = "Oui";
    if (boutonNePasFusionnerLesPdf->font().bold() == true)
    {
        fusionnerLesPdf = "Non";
    }

    QMessageBox demandeConfirmationGeneration;
    demandeConfirmationGeneration.setText(QString("Voulez vous générer les PDF de demande de subventions ? <br /><br />")
        + "La génération sera réalisée avec les options suivantes : <br />"
        + texteSignature + "<br />"
        + texteDemande + "<br />"
        + "<b>Fusion des PDF</b> : " + fusionnerLesPdf + "<br />"
        + "<br />Nombre de demandes à générer :<br />"
        + "Subventions heures de vol : " + nbSubventions + "<br />"
        + "Remboursement factures : " + nbFactures + "<br />"
        + "Remise chèques cotisations : " + nbCotisations + "<br />"
        + "Remise chèques balades et sorties : " + nbBaladesSorties + "<br />");
    demandeConfirmationGeneration.setWindowTitle("Génération des PDF de demande");
    demandeConfirmationGeneration.setIcon(QMessageBox::Question);
    demandeConfirmationGeneration.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    const int ret = demandeConfirmationGeneration.exec();

    switch (ret)
    {
        case QMessageBox::Yes:
        {
            pdf->imprimerLesDemandesDeSubvention(parametresMetiers.nomTresorier,
                cheminSortieFichiersGeneres,
                cheminStockageFacturesTraitees,
                typeGenerationPdf,
                signature,
                boutonFusionnerLesPdf->font().bold());
        }
        break;

        case QMessageBox::No:
        default:
        {
            //Rien à faire
        }
        break;
    }
    
}

void AeroDms::genererPdfRecapHdV()
{
    pdf->imprimerLeRecapitulatifDesHeuresDeVol( listeDeroulanteAnnee->currentData().toInt(),
                                                cheminSortieFichiersGeneres,
                                                cheminStockageFacturesTraitees,
                                                signature);
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
    //Si le pilote n'est pas à jour de sa cotisation => echec immediat
    bool estEnEchec = !lePiloteEstAJourDeCotisation();

    //On effectue d'abord quelques contrôles pour savoir si le vol est enregistrable :
    //1) on a une facture chargée
    //2) les données (pilote, date du vol, durée, cout) sont renseignées
    //3) on est pas en echec sur une des étapes précédentes
    if (pdfDocument->status() == QPdfDocument::Status::Ready
        && !estEnEchec )
    {
        const int anneeRenseignee = dateDuVol->date().year();
        const QString idPilote = choixPilote->currentData().toString();

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
                

				subventionRestante = subventionRestante - montantSubventionne;
			}
				db->enregistrerUnVol(idPilote,
					typeDeVol->currentText(),
					dateDuVol->date(),
					dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute(),
					prixDuVol->value(),
					montantSubventionne,
					factureIdEnBdd,
					choixBalade->currentData().toInt(),
					remarqueVol->text(),
                    activite->currentText(),
                    volAEditer);

                QString volAjouteModifie = "ajouté";
                if (volAEditer != -1)
                {
                    volAjouteModifie = "modifié";
                }

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
                + "€) "
                + volAjouteModifie
                + ". Montant subvention : "
                + QString::number(montantSubventionne)
                + "€ / Subvention entrainement restante : "
                + QString::number(subventionRestante)
                + "€");

            //On sort du mode édition, si on y etait...
            volAEditer = -1;

            //Et on supprime la vol de la liste des vols détectés si on en avait chargé un
            supprimerLeVolDeLaVueVolsDetectes();

            //On rince les données de vol
            dureeDuVol->setTime(QTime::QTime(0, 0));
            prixDuVol->setValue(0);
            remarqueVol->clear();
            //La mise à jour de ces données provoque la réélaboration de l'état des boutons de validation

            //on réactive les éventuels élements d'IHM désactivés par une mise à jour de vol
            typeDeVol->setEnabled(true);
            dureeDuVol->setEnabled(true);
            prixDuVol->setEnabled(true);
        }
        else
        {
            statusBar()->showMessage("Erreur ajout");
        }
    }
    //On met à jour la table des pilotes et celle des vols
    peuplerTablePilotes();
    peuplerTableVols();
    //On met a jour la liste des vols balades/sorties dans l'onglet ajout des recettes
    peuplerListeBaladesEtSorties();

    //Et l'affichage des statistiques
    peuplerStatistiques();

    //On restaure le texte du bouton de validation (qui a changé si on était en édition)
    validerLeVol->setText("Valider le vol");
}

void AeroDms::enregistrerLesVols()
{
    //On ne peut enregistrer les vols que si le pilote est à jour de cotisation
    //Cela sera verifie dans la methode enregistrerUnVol() cependant on le prévérifie
    //ici pour éviter d'avoir N fois la notification d'echec si le pilote n'est 
    //pas à jour de sa cotisation
    if (lePiloteEstAJourDeCotisation())
    {
        while (!factures.isEmpty())
        {
            //On charge le premier vol de la ligne
            chargerUnVolDetecte(0, 0);

            //On demande l'enregistrement
            enregistrerUnVol();
        }
        //La liste sera vide => on desactive le bouton d'enregistrement du vol
        validerLesVols->setEnabled(false);
        supprimerLeVolSelectionne->setEnabled(false);
    }
}

void AeroDms::supprimerLeVolDeLaVueVolsDetectes()
{
    if (idFactureDetectee != -1)
    {
        factures.remove(idFactureDetectee);
        idFactureDetectee = -1;
        peuplerTableVolsDetectes(factures);

        vueVolsDetectes->clearSelection();
        supprimerLeVolSelectionne->setEnabled(false);
    }
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
        QMessageBox::critical( this, 
                               "Vol non sélectionné", 
                               "La dépense doit être associée à au moins un vol. \n\
Sélectionnez au moins un vol dans la partie gauche de la fenêtre.\n\n\
Saisie non prise en compte.");
    }
    else
    {
        db->ajouterUneRecetteAssocieeAVol( volsCoches,
                                           typeDeRecette->currentText(),
                                           intituleRecette->text(),
                                           montantRecette->value());
        statusBar()->showMessage("Recette ajoutee");

        intituleRecette->clear();
        montantRecette->clear();
    }

    peuplerTableRecettes();
    peuplerListeBaladesEtSorties();
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
        if (pilote.estActif)
        {
            choixPilote->addItem(nomPrenom, pilote.idPilote);
            choixPayeur->addItem(nomPrenom, pilote.idPilote);
        }
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

    AeroDmsTypes::ListeVolSortieOuBalade itemLabels = db->recupererBaladesEtSorties( typeDeRecette->currentText(), 
                                                                                     parametresMetiers.proportionParticipationBalade);
    QListIterator it(itemLabels);
    while (it.hasNext())
    {
        const AeroDmsTypes::VolSortieOuBalade & vol = it.next();
        QListWidgetItem* itemBaladesEtSorties = new QListWidgetItem(vol.nomVol, listeBaladesEtSorties);
        itemBaladesEtSorties->setCheckState(Qt::Unchecked);
        if (vol.montantSubventionAttendu != 0)
        {
            itemBaladesEtSorties->setToolTip("Montant participation attendu : " + QString::number(vol.montantSubventionAttendu, 'f', 2) + " €");
        }  
        if (!vol.volAAuMoinsUnPaiement)
        {
            itemBaladesEtSorties->setForeground(Qt::red);
        }
        listeBaladesEtSorties->addItem(itemBaladesEtSorties);
    }
}

void AeroDms::prevaliderDonnnesSaisies()
{
    validerLeVol->setEnabled(true);
    validerLaFacture->setEnabled(true);
    validerLesVols->setEnabled(true);

    if ( prixDuVol->value() == 0
         || dureeDuVol->time() == QTime::QTime(0,0)
         || choixPilote->currentIndex() == 0
         || pdfDocument->status() != QPdfDocument::Status::Ready
         || ( typeDeVol->currentText() != "Entrainement" && choixBalade->currentIndex() == 0))
    {
        validerLeVol->setEnabled(false);
    }

    if ( choixPilote->currentIndex() == 0
         || pdfDocument->status() != QPdfDocument::Status::Ready
         || (typeDeVol->currentText() != "Entrainement" )
         || idFactureDetectee != -1)
    {
        validerLesVols->setEnabled(false);
    }

    if ( choixBaladeFacture->currentIndex() == 0
         || choixPayeur->currentIndex() == 0
         || montantFacture->value() == 0
         || remarqueFacture->text() == ""
         || pdfDocument->status() != QPdfDocument::Status::Ready
         || logicielEnModeLectureSeule)
    {
        validerLaFacture->setEnabled(false);
    }    
}

void AeroDms::mettreAJourInfosSurSelectionPilote()
{
    if (choixPilote->currentIndex() != 0)
    {
        aeroclubPiloteSelectionne->setText(db->recupererAeroclub(choixPilote->currentData().toString()));
        QString cotisation = "(Cotisation payée pour l'année "
            + QString::number(dateDuVol->date().year())
            + ")";
        if (!db->piloteEstAJourDeCotisation(choixPilote->currentData().toString(), dateDuVol->date().year()))
        {
            cotisation = "(Cotisation non payée pour l'année "
                + QString::number(dateDuVol->date().year())
                + ")";
        }
        statusBar()->showMessage("Subvention restante pour ce pilote, pour l'année "
            + QString::number(dateDuVol->date().year())
            + " : " + QString::number(db->recupererSubventionRestante(choixPilote->currentData().toString(), dateDuVol->date().year()))
            + " € "
            + cotisation
            + ".");
        activite->setCurrentIndex(activite->findText(db->recupererActivitePrincipale(choixPilote->currentData().toString())));
    }
    else
    {
        aeroclubPiloteSelectionne->setText("");
        statusBar()->clearMessage();
    }
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
        "Ce programme utilise la libraire <a href=\"https://github.com/podofo/podofo\">PoDoFo</a> 0.10.3.<br />< br/>"
        "Les icones sont issues de <a href=\"https://pictogrammers.com/\">pictogrammers.com</a>.< br />< br />"
        "Ce programme inclu du code source inspiré de code sous licence BSD-3-Clause (Copyright (C) The Qt Company Ltd).<br />< br/>"
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

void AeroDms::ouvrirAide()
{
    QDesktopServices::openUrl(QUrl("https://github.com/cvermot/AeroDMS/wiki", QUrl::TolerantMode));
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
        if (logicielEnModeLectureSeule)
        {
            menuClicDroitPilote.setEnabled(false);
        }

        //Afficher le menu sur la vue des pilotes
        menuClicDroitPilote.exec(vuePilotes->mapToParent(QCursor::pos()));
    }
}

void AeroDms::editerPilote()
{
    dialogueGestionPilote->preparerMiseAJourPilote(piloteAEditer);
    dialogueGestionPilote->exec();
}

void AeroDms::volsSelectionnes()
{
    int nombreDeColonnes = 0;
    for (AeroDmsTypes::VolTableElement element = AeroDmsTypes::VolTableElement_PILOTE ; 
        element < AeroDmsTypes::VolTableElement_NB_COLONNES ; 
        element = static_cast<AeroDmsTypes::VolTableElement>(element + 1))
    {
        if (!vueVols->isColumnHidden(element))
        {
            nombreDeColonnes++;
        }
    }

    if (vueVols->selectedItems().size() / nombreDeColonnes > 1)
    {
        QItemSelectionModel* selection = vueVols->selectionModel();

        QModelIndexList listesLignesSelectionnees = selection->selectedRows();

        double montantTotalSubventionne = 0;
        double coutTotal = 0;
        int hdvTotales = 0;
        
        while(listesLignesSelectionnees.size())
        {
            int numeroLigne = listesLignesSelectionnees.takeFirst().row();
            
            QString data = vueVols->item(numeroLigne,AeroDmsTypes::VolTableElement_COUT)->data(0).toString();
            data.chop(2);
            coutTotal = coutTotal + data.toDouble();

            data = vueVols->item(numeroLigne, AeroDmsTypes::VolTableElement_SUBVENTION)->data(0).toString();
            data.chop(2);
            montantTotalSubventionne = montantTotalSubventionne + data.toDouble();

            hdvTotales = hdvTotales + vueVols->item(numeroLigne, AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES)->data(0).toInt();

            statusBar()->showMessage("Vols sélectionnés : Coût total : "
                + QString::number(coutTotal)
                + " € / Montant subventionné total : "
                + QString::number(montantTotalSubventionne)
                + " € / Nombres d'heures de vol totales : "
                + AeroDmsServices::convertirMinutesEnHeuresMinutes(hdvTotales));
        }
    }
    else
    {
        statusBar()->clearMessage();
    }
    
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

        if (logicielEnModeLectureSeule)
        {
            editerLeVol.setEnabled(false);
            supprimerLeVol.setEnabled(false);
        }

        //Afficher le menu sur la vue des vols
        menuClicDroitVol.exec(vueVols->mapToParent(QCursor::pos()));
    }
}

void AeroDms::editerVol()
{
    //On récupère le nom de la facture associée et on la charge
    const QString cheminComplet = cheminStockageFacturesTraitees + "/" + db->recupererNomFacture(volAEditer);
    chargerUneFacture(cheminComplet);

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

    activite->setCurrentIndex(activite->findText(vol.activite));

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

    //On masque l'éventuelle table des infos de vol récupéré automatiquement de la facture précédement chargée
    validerLesVols->setHidden(true);
    vueVolsDetectes->setHidden(true);
    supprimerLeVolSelectionne->setHidden(true);
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
        vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, false);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_FACTURE_ID, false);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_NOM_FACTURE, false);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_ANNEE, false);
        vueRecettes->setColumnHidden(AeroDmsTypes::RecetteTableElement_ID, false);
    }
    //Sinon, le mode est actif, on desactive
    else
    {
        boutonModeDebug->setText("Activer le mode &debug");
        boutonModeDebug->setIcon(QIcon("./ressources/bug.svg"));
        vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_PILOTE_ID, true);
        vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_VOL_ID, true);
        vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, true);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_FACTURE_ID, true);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_NOM_FACTURE, true);
        vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_ANNEE, true);
        vueRecettes->setColumnHidden(AeroDmsTypes::RecetteTableElement_ID, true);
    }
}

void AeroDms::switchScanAutomatiqueDesFactures()
{
    scanAutomatiqueDesFacturesEstActif = !scanAutomatiqueDesFacturesEstActif;
    if (scanAutomatiqueDesFacturesEstActif)
    {
        boutonActivationScanAutoFactures->setText(tr("Desactiver le scan automatique des factures"));
    }
    else
    {
        boutonActivationScanAutoFactures->setText(tr("Activer le scan automatique des factures"));
    }
    
}


void AeroDms::convertirHeureDecimalesVersHhMm()
{
    bool ok;
    double heureDecimale = QInputDialog::getDouble(this, tr("Conversion heure décimale"),
        tr("Saisir l'heure au format décimal : "), 0, 0, 10, 2, &ok,
        Qt::WindowFlags(), 1);
    
    if (ok)
    {
        QTime heureHhmm;
        int heure = floor(heureDecimale);
        int minutes = 60 * (heureDecimale - floor(heureDecimale));
        heureHhmm.setHMS(heure, minutes, 0);
        dureeDuVol->setTime(heureHhmm);
    }
}

void AeroDms::envoyerMail()
{
    if (sender() == mailingPilotesAyantCotiseCetteAnnee)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailPilotes(listeDeroulanteAnnee->currentData().toInt(), false)
            + "?subject=[Section aéronautique]&body=", QUrl::TolerantMode));
    }
    else if (sender() == mailingPilotesActifsAyantCotiseCetteAnnee)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailPilotes(listeDeroulanteAnnee->currentData().toInt(), true)
            + "?subject=[Section aéronautique]&body=", QUrl::TolerantMode));
    }
    else if (sender() == mailingPilotesActifsBrevetes)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailPilotes(listeDeroulanteAnnee->currentData().toInt(), true, true)
            + "?subject=[Section aéronautique]&body=", QUrl::TolerantMode));
    }
    else if (sender() == mailingPilotesDerniereDemandeSubvention)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailDerniereDemandeDeSubvention()
            + "?subject=[Section aéronautique] Chèques aéro&body="
            + parametresMetiers.texteMailDispoCheques, QUrl::TolerantMode));
    }
    
}

bool AeroDms::uneMaJEstDisponible(const QString p_chemin)
{ 
    if (QFile().exists(p_chemin))
    {
        QFile fichierDistant(p_chemin);
        if (fichierDistant.open(QFile::ReadOnly))
        {
            QString hashFichierDistant = "";
            QCryptographicHash hash(QCryptographicHash::Sha1);
            if (hash.addData(&fichierDistant))
            {
                //QString hashString = QString::fromStdString(hash.result().toStdString());
                //QMessageBox::critical(this, "Fichier présent", hash.result().toHex());
                hashFichierDistant = hash.result().toHex();
            }
            hash.reset();

            //Calcul de la somme de controle de l'application courante
            QFile fichierCourant(QCoreApplication::applicationFilePath());
            //qDebug() << "courant" << QCoreApplication::applicationFilePath();
            if (fichierCourant.open(QFile::ReadOnly))
            {
                QString hashFichierCourant = "";
                if (hash.addData(&fichierCourant))
                {
                    //QString hashString = QString::fromStdString(hash.result().toStdString());
                    //QMessageBox::critical(this, "Fichier présent", hash.result().toHex() + "\n" + hashFichierDistant);
                    hashFichierCourant = hash.result().toHex();

                    if (hashFichierDistant != hashFichierCourant)
                    {
                        //QFile::copy(fichierAVerifier, QCoreApplication::applicationDirPath()+"/AeroDms_new.exe");
                        //QApplication::quit();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void AeroDms::initialiserTableauVolsDetectes(QGridLayout* p_infosVol)
{
    vueVolsDetectes = new QTableWidget(0, AeroDmsTypes::VolsDetectesTableElement_NB_COLONNES, this);
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_DATE, new QTableWidgetItem("Date"));
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_DUREE, new QTableWidgetItem("Durée"));
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_MONTANT, new QTableWidgetItem("Montant"));
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_TYPE, new QTableWidgetItem("Type"));
    vueVolsDetectes->setSelectionBehavior(QAbstractItemView::SelectRows);

    vueVolsDetectes->resizeColumnsToContents();
    vueVolsDetectes->setHidden(true);

    p_infosVol->addWidget(vueVolsDetectes, 11, 0, 2, 0);

    validerLesVols = new QPushButton("Valider les vols", this);
    validerLesVols->setHidden(true);
    validerLesVols->setEnabled(false);
    validerLesVols->setToolTip(tr("Validation possible si :\n\
   -pilote sélectionné,\n\
   -type de vol sélectionné vaut Entrainement,\n\
   -Aucun vol sélectionné dans la liste pour modification.\n\
Note : tous les vols enregistrés via ce bouton seront enregistrés en tant que vol d'entrainement.\n\
Les vols d'une autre catégorie doivent être saisis via modification manuelle en cliquant sur le vol\n\
puis en complétant les informations via la fenêtre de saisie."));
    supprimerLeVolSelectionne = new QPushButton("Supprimer le vol sélectionné", this);
    supprimerLeVolSelectionne->setHidden(true);
    supprimerLeVolSelectionne->setEnabled(false);
    connect(validerLesVols, &QPushButton::clicked, this, &AeroDms::enregistrerLesVols);
    connect(supprimerLeVolSelectionne, &QPushButton::clicked, this, &AeroDms::supprimerLeVolDeLaVueVolsDetectes);
    connect(vueVolsDetectes, &QTableWidget::cellClicked, this, &AeroDms::chargerUnVolDetecte);
    p_infosVol->addWidget(validerLesVols, 13, 0, 2, 0);
    p_infosVol->addWidget(supprimerLeVolSelectionne, 14, 0, 2, 0);
}

void AeroDms::chargerUnVolDetecte(int row, int column)
{
    idFactureDetectee = row;
    prixDuVol->setValue(factures.at(idFactureDetectee).coutDuVol);
    dureeDuVol->setTime(factures.at(idFactureDetectee).dureeDuVol);
    dateDuVol->setDate(factures.at(idFactureDetectee).dateDuVol);
    pdfView->pageNavigator()->jump(factures.at(idFactureDetectee).pageDansLeFichierPdf, QPoint());

    supprimerLeVolSelectionne->setEnabled(true);
}

void AeroDms::deselectionnerVolDetecte()
{
    if (idFactureDetectee != -1)
    {
        idFactureDetectee = -1;
        //On rince les données de vol
        dureeDuVol->setTime(QTime::QTime(0, 0));
        prixDuVol->setValue(0);
        remarqueVol->clear();
        //La mise à jour de ces données provoque la réélaboration de l'état des boutons de validation => a faire
        //imperativement après le rincage de idFactureDetectee car cette donnée ne redeclenche pas ce traitement

        vueVolsDetectes->clearSelection();
        supprimerLeVolSelectionne->setEnabled(false);
    } 
}

bool AeroDms::lePiloteEstAJourDeCotisation()
{
    const int anneeRenseignee = dateDuVol->date().year();
    const QString idPilote = choixPilote->currentData().toString();

    //On verifie si le pilote est a jour de sa cotisation pour l'année du vol
    if (!db->piloteEstAJourDeCotisation(idPilote, anneeRenseignee))
    {
        QMessageBox::critical(this, tr("AeroDMS"),
            tr("Le pilote n'est pas a jour de sa cotisation pour l'année du vol.\n"
                "Impossible d'enregistrer le vol."), QMessageBox::Cancel);
        return false;
    }
    return true;
}

bool AeroDms::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) 
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            emit toucheEchapEstAppuyee();
        }
        return true;
    }
    else
        return false;
}


