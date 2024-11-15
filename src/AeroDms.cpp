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
#include "StatistiqueDonutCombineWidget.h"
#include "StatistiqueDonuts.h"

#include "DialogueEditionParametres.h"
#include "DialogueProgressionImpression.h"

#include <QtWidgets>
#include <QToolBar>
#include <QPdfPageNavigator>
#include <QtCore>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>

AeroDms::AeroDms(QWidget* parent) :QMainWindow(parent)
{
    ouvrirSplashscreen();

    initialiserBaseApplication();
    lireParametresEtInitialiserBdd();

    initialiserOngletPilotes();
    initialiserOngletVols();
    initialiserOngletFactures();
    initialiserOngletRecettes();
    initialiserOngletAjoutDepenses();
    initialiserOngletAjoutRecettes();
    initialiserOngletSubventionsDemandees();
    initialiserOngletGraphiques();
    connect(mainTabWidget, &QTabWidget::currentChanged, this, &AeroDms::gererChangementOnglet);

    initialiserRaccourcisClavierSansActionIhm();

    //=============Barres d'outils et boites de dialogue
    initialiserBarreDOutils();
    initialiserBarreDeFiltres();
    initialiserBoitesDeDialogues();

    //========================Menus
    initialiserMenuFichier();
    initialiserMenuOptions();
    initialiserMenuOutils();
    initialiserMenuAide();

    //=======================Peupler les vues tables, et initialiser les différents élements d'IHM dépendants
    // du contexte.
    peuplerListeDeroulanteAnnee();
    peuplerListesPilotes();
    peuplerListeSorties();
    peuplerListeBaladesEtSorties();
    peuplerTablePilotes();
    peuplerTableVols();
    peuplerTableFactures();
    peuplerTableRecettes();
    peuplerTableSubventionsDemandees();
    peuplerStatistiques();
    prevaliderDonnnesSaisies();
    prevaliderDonnneesSaisiesRecette();
    changerInfosVolSurSelectionTypeVol();
    verifierSignatureNumerisee();

    gererChangementOnglet();

    terminerMiseAJourApplication();
    verifierPresenceDeMiseAjour();

    statusBar()->showMessage("Prêt");

    demanderFermetureSplashscreen();
}

void AeroDms::initialiserBaseApplication()
{
    QApplication::setApplicationName("AeroDMS");
    QApplication::setApplicationVersion("6.7");
    QApplication::setWindowIcon(QIcon("./ressources/shield-airplane.svg"));
    mainTabWidget = new QTabWidget(this);
    setCentralWidget(mainTabWidget);

    setWindowTitle(tr("AeroDMS"));
    setMinimumSize(800, 600);
    showMaximized();

    installEventFilter(this);
    connect(this, &AeroDms::toucheEchapEstAppuyee, this, &AeroDms::deselectionnerVolDetecte);

    //========================Initialisation des autres attributs
    piloteAEditer = "";
    volAEditer = -1;
    factureIdEnBdd = 0;
}

void AeroDms::lireParametresEtInitialiserBdd()
{
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
        settings.setValue("objetChequesDisponibles", tr("[Section aéronautique] Chèques aéro"));
        settings.setValue("texteChequesDisponibles", tr("Bonjour,\n\nVos chèques sont disponibles. \n\nCordialement"));
        settings.setValue("objetSubventionRestante", tr("[Section aéronautique] Subvention entrainement"));
        settings.setValue("texteSubventionRestante", tr("Bonjour,\n\nSauf erreur de notre part, vous disposez encore d'un budget entrainement non consommé. \n\nCordialement"));
        settings.setValue("objetAutresMailings", tr("[Section aéronautique] "));
        settings.endGroup();
    }

    parametresMetiers.objetMailDispoCheques = settings.value("mailing/objetChequesDisponibles", "[Section aéronautique] Chèques aéro").toString();
    parametresMetiers.texteMailDispoCheques = settings.value("mailing/texteChequesDisponibles", "").toString();
    parametresMetiers.objetMailSubventionRestante = settings.value("mailing/objetSubventionRestante", "[Section aéronautique] Subvention entrainement").toString();
    parametresMetiers.texteMailSubventionRestante = settings.value("mailing/texteSubventionRestante", "").toString();
    parametresMetiers.objetMailAutresMailings = settings.value("mailing/objetAutresMailings", "[Section aéronautique] ").toString();

    if (settings.value("impression/imprimante", "") == "")
    {
        settings.beginGroup("impression");
        settings.setValue("imprimante", "");
        settings.setValue("couleur", 1);
        settings.setValue("resolution", 600);
        settings.setValue("forcageImpressionRecto", true);
        settings.endGroup();
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
        settingsMetier.setValue("proportionParticipationBalade", "0.375");
        settingsMetier.endGroup();
    }

    if (settingsMetier.value("parametresSysteme/delaisDeGardeDbEnMs", "") == "")
    {
        settingsMetier.beginGroup("parametresSysteme");
        settingsMetier.setValue("delaisDeGardeDbEnMs", "50");
        settingsMetier.setValue("margesHautBas", "20");
        settingsMetier.setValue("margesGaucheDroite", "20");
        settingsMetier.endGroup();
    }

    const QString database = settings.value("baseDeDonnees/chemin", "").toString() +
        QString("/") +
        settings.value("baseDeDonnees/nom", "").toString();
    const QString ressourcesHtml = settings.value("baseDeDonnees/chemin", "").toString() +
        QString("/ressources/HTML");

    parametresSysteme.cheminStockageBdd = settings.value("baseDeDonnees/chemin", "").toString();
    parametresSysteme.cheminStockageFacturesTraitees = settings.value("dossiers/facturesSaisies", "").toString();
    parametresSysteme.cheminStockageFacturesATraiter = settings.value("dossiers/facturesATraiter", "").toString();
    parametresSysteme.cheminSortieFichiersGeneres = settings.value("dossiers/sortieFichiersGeneres", "").toString();
    parametresSysteme.nomBdd = settings.value("baseDeDonnees/nom", "").toString();
    parametresSysteme.imprimante = settings.value("impression/imprimante", "").toString();
    parametresSysteme.modeCouleurImpression = static_cast<QPrinter::ColorMode>(settings.value("impression/couleur", "").toInt());
    parametresSysteme.resolutionImpression = settings.value("impression/resolution", 600).toInt();
    parametresSysteme.forcageImpressionRecto = settings.value("impression/forcageImpressionRecto", true).toBool();
    parametresSysteme.margesHautBas = settingsMetier.value("parametresSysteme/margesHautBas", "20").toInt();
    parametresSysteme.margesGaucheDroite = settingsMetier.value("parametresSysteme/margesGaucheDroite", "20").toInt();

    parametresMetiers.montantSubventionEntrainement = settingsMetier.value("parametresMetier/montantSubventionEntrainement", "750").toFloat();
    parametresMetiers.montantCotisationPilote = settingsMetier.value("parametresMetier/montantCotisationPilote", "15").toFloat();
    parametresMetiers.proportionRemboursementEntrainement = settingsMetier.value("parametresMetier/proportionRemboursementEntrainement", "0.5").toFloat();
    parametresMetiers.plafondHoraireRemboursementEntrainement = settingsMetier.value("parametresMetier/plafondHoraireRemboursementEntrainement", "150").toFloat();
    parametresMetiers.proportionRemboursementBalade = settingsMetier.value("parametresMetier/proportionRemboursementBalade", "0.875").toFloat();
    parametresMetiers.proportionParticipationBalade = settingsMetier.value("parametresMetier/proportionParticipationBalade", "0.375").toFloat();
    parametresMetiers.nomTresorier = settings.value("noms/nomTresorier", "").toString();
    parametresMetiers.delaisDeGardeBdd = settingsMetier.value("parametresSysteme/delaisDeGardeDbEnMs", "50").toInt();
    
    db = new ManageDb(database, parametresMetiers.delaisDeGardeBdd);
    pdf = new PdfRenderer(db,
        ressourcesHtml,
        QMarginsF(parametresSysteme.margesGaucheDroite,
            parametresSysteme.margesHautBas,
            parametresSysteme.margesGaucheDroite,
            parametresSysteme.margesHautBas));
}

void AeroDms::initialiserOngletPilotes()
{
    //==========Onglet Pilotes
    vuePilotes = new QTableWidget(0, AeroDmsTypes::PiloteTableElement_NB_COLONNES, this);
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_NOM, new QTableWidgetItem(tr("Nom")));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_PRENOM, new QTableWidgetItem(tr("Prénom")));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_ANNEE, new QTableWidgetItem(tr("Année")));

    QTableWidgetItem* headerHdVEntrainement = new QTableWidgetItem(tr("HdV"));
    headerHdVEntrainement->setIcon(AeroDmsServices::recupererIcone("Entrainement"));
    headerHdVEntrainement->setToolTip("Heures de vol d'entrainement");
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_ENTRAINEMENT_SUBVENTIONNEES, headerHdVEntrainement);
    QTableWidgetItem* headerSubventionEntrainement = new QTableWidgetItem(tr("Subvention"));
    headerSubventionEntrainement->setIcon(AeroDmsServices::recupererIcone("Entrainement"));
    headerSubventionEntrainement->setToolTip(tr("Montant de la subvention d'entrainement déjà allouée"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE, headerSubventionEntrainement);

    QTableWidgetItem* headerHdVBalade = new QTableWidgetItem(tr("HdV"));
    headerHdVBalade->setIcon(AeroDmsServices::recupererIcone("Balade"));
    headerHdVBalade->setToolTip((tr("Heures de vol de balade")));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_BALADES_SUBVENTIONNEES, headerHdVBalade);
    QTableWidgetItem* headerSubventionBalade = new QTableWidgetItem(tr("Subvention"));
    headerSubventionBalade->setIcon(AeroDmsServices::recupererIcone("Balade"));
    headerSubventionBalade->setToolTip(tr("Montant de la subvention balade déjà allouée"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_BALADES_SUBVENTIONNE, headerSubventionBalade);

    QTableWidgetItem* headerHdVSortie = new QTableWidgetItem((tr("HdV")));
    headerHdVSortie->setIcon(AeroDmsServices::recupererIcone("Sortie"));
    headerHdVSortie->setToolTip(tr("Heures de vol de sortie"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_SORTIES_SUBVENTIONNEES, headerHdVSortie);
    QTableWidgetItem* headerSubventionSortie = new QTableWidgetItem(tr("Subvention"));
    headerSubventionSortie->setIcon(AeroDmsServices::recupererIcone("Sortie"));
    headerSubventionSortie->setToolTip(tr("Montant de la subvention sortie déjà allouée"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_SORTIES_SUBVENTIONNE, headerSubventionSortie);

    QTableWidgetItem* headerHdVTotales = new QTableWidgetItem(tr("HdV"));
    headerHdVTotales->setIcon(AeroDmsServices::recupererIcone("Total"));
    headerHdVTotales->setToolTip(tr("Heures de vol totales"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_TOTALES_SUBVENTIONNEES, headerHdVTotales);
    QTableWidgetItem* headerSubventionTotale = new QTableWidgetItem(tr("Subvention"));
    headerSubventionTotale->setIcon(AeroDmsServices::recupererIcone("Total"));
    headerSubventionTotale->setToolTip(tr("Montant total de la subvention déjà allouée"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_TOTAL_SUBVENTIONNE, headerSubventionTotale);

    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_PILOTE_ID, new QTableWidgetItem(tr("Pilote Id (masqué)")));
    vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_PILOTE_ID, true);
    vuePilotes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vuePilotes->setSelectionBehavior(QAbstractItemView::SelectRows);
    vuePilotes->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vuePilotes, QIcon("./ressources/account-tie-hat.svg"), tr("Pilotes"));
    connect(vuePilotes, &QTableWidget::customContextMenuRequested, this, &AeroDms::menuContextuelPilotes);
}

void AeroDms::initialiserOngletVols()
{
    //==========Onglet Vols
    vueVols = new QTableWidget(0, AeroDmsTypes::VolTableElement_NB_COLONNES, this);
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_PILOTE, new QTableWidgetItem(tr("Pilote")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_DATE, new QTableWidgetItem(tr("Date")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_TYPE_DE_VOL, new QTableWidgetItem(tr("Type")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_DUREE, new QTableWidgetItem(tr("Durée")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_COUT, new QTableWidgetItem(tr("Coût")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_SUBVENTION, new QTableWidgetItem(tr("Subvention")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_SOUMIS_CE, new QTableWidgetItem(tr("Soumis CE")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_REMARQUE, new QTableWidgetItem(tr("Remarque")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_IMMAT, new QTableWidgetItem(tr("Immatriculation")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_ACTIVITE, new QTableWidgetItem(tr("Activité")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, new QTableWidgetItem(tr("Durée en minutes")));
    vueVols->setHorizontalHeaderItem(AeroDmsTypes::VolTableElement_VOL_ID, new QTableWidgetItem(tr("ID")));
    vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_VOL_ID, true);
    vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, true);
    vueVols->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vueVols->setSelectionBehavior(QAbstractItemView::SelectRows);
    vueVols->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vueVols, QIcon("./ressources/airplane.svg"), tr("Vols"));
    connect(vueVols, &QTableWidget::customContextMenuRequested, this, &AeroDms::menuContextuelVols);
    connect(vueVols, &QTableWidget::cellClicked, this, &AeroDms::volsSelectionnes);
}

void AeroDms::initialiserOngletFactures()
{
    //==========Onglet Factures
    vueFactures = new QTableWidget(0, AeroDmsTypes::FactureTableElement_NB_COLONNES, this);
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_INTITULE, new QTableWidgetItem(tr("Intitulé")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_MONTANT, new QTableWidgetItem(tr("Montant")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_PAYEUR, new QTableWidgetItem(tr("Payeur")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_NOM_SORTIE, new QTableWidgetItem(tr("Nom sortie")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_TYPE_SORTIE, new QTableWidgetItem(tr("Type de dépense")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_DATE, new QTableWidgetItem(tr("Date")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_ANNEE, new QTableWidgetItem(tr("Année")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_SOUMIS_CE, new QTableWidgetItem(tr("Soumis CE")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_NOM_FACTURE, new QTableWidgetItem(tr("Nom facture")));
    vueFactures->setHorizontalHeaderItem(AeroDmsTypes::FactureTableElement_FACTURE_ID, new QTableWidgetItem(tr("Facture ID")));
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_FACTURE_ID, true);
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_NOM_FACTURE, true);
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_ANNEE, true);
    vueFactures->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vueFactures->setSelectionBehavior(QAbstractItemView::SelectRows);
    vueFactures->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vueFactures, QIcon("./ressources/file-document.svg"), tr("Factures"));
}

void AeroDms::initialiserOngletRecettes()
{
    //==========Onglet Recettes
    vueRecettes = new QTableWidget(0, AeroDmsTypes::RecetteTableElement_NB_COLONNES, this);
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_DATE, new QTableWidgetItem(tr("Date")));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_TYPE_DE_RECETTE, new QTableWidgetItem(tr("Type")));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_INTITULE, new QTableWidgetItem(tr("Intitulé")));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_MONTANT, new QTableWidgetItem(tr("Montant")));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_SOUMIS_CE, new QTableWidgetItem(tr("Soumis CE")));
    vueRecettes->setHorizontalHeaderItem(AeroDmsTypes::RecetteTableElement_ID, new QTableWidgetItem(tr("ID")));
    vueRecettes->setColumnHidden(AeroDmsTypes::RecetteTableElement_ID, true);
    vueRecettes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vueRecettes->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainTabWidget->addTab(vueRecettes, QIcon("./ressources/cash-multiple.svg"), tr("Recettes"));
}

void AeroDms::initialiserOngletAjoutDepenses()
{
    //==========Onglet Ajout dépense
    QHBoxLayout* ajoutVol = new QHBoxLayout();
    widgetAjoutVol = new QWidget(this);
    widgetAjoutVol->setLayout(ajoutVol);
    mainTabWidget->addTab(widgetAjoutVol, QIcon("./ressources/file-document-minus.svg"), tr("Ajout dépense"));

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
    activite->setToolTip(tr("L'activité est fournie à titre statistique uniquement.\n"
        "Ce champ est rempli automatiquement à partir de l'activité\n"
        "par défaut renseignée pour le pilote sélectionné.\n"
        "Il est modifiable ici pour les pilotes qui pratiqueraient plusieurs activités."));
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

    immat = new QLineEdit(this);
    immat->setToolTip(tr("Facultatif : à des fins de statistiques uniquement"));
    QLabel* immatLabel = new QLabel(tr("Immatriculation : "), this);

    validerLeVol = new QPushButton("Valider le vol", this);
    validerLeVol->setToolTip(tr("Validation possible si :\n\
   -pilote sélectionné,\n\
   -durée de vol saisi,\n\
   -montant du vol saisi,\n\
   -si Type de vol est \"Sortie\", sortie sélectionnée,\n\
   -facture chargée."));
    connect(validerLeVol, &QPushButton::clicked, this, &AeroDms::enregistrerUnVol);

    QGridLayout* infosVol = new QGridLayout();
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
    infosVol->addWidget(immatLabel, 9, 0);
    infosVol->addWidget(immat, 9, 1);
    infosVol->addWidget(validerLeVol, 10, 0, 2, 0);

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
    validerLaFacture->setToolTip(tr("Validation possible si : \n\
   -facture chargée,\n\
   -montant de la facture renseignée,\n\
   -payeur et sortie sélectionnés,\n\
   -intitulé saisi."));
    connect(validerLaFacture, &QPushButton::clicked, this, &AeroDms::enregistrerUneFacture);

    QGridLayout* infosFacture = new QGridLayout();
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
}

void AeroDms::initialiserOngletAjoutRecettes()
{
    //==========Onglet Ajout recette
    QHBoxLayout* ajoutRecette = new QHBoxLayout();
    widgetAjoutRecette = new QWidget(this);
    widgetAjoutRecette->setLayout(ajoutRecette);
    mainTabWidget->addTab(widgetAjoutRecette, QIcon("./ressources/file-document-plus.svg"), tr("Ajout recette"));

    listeBaladesEtSorties = new QListWidget(this);
    ajoutRecette->addWidget(listeBaladesEtSorties, 3);

    typeDeRecette = new QComboBox(this);
    typeDeRecette->addItems(db->recupererTypesDesVol(true));
    AeroDmsServices::ajouterIconesComboBox(*typeDeRecette);
    connect(typeDeRecette, &QComboBox::currentIndexChanged, this, &AeroDms::chargerBaladesSorties);
    QLabel* typeDeRecetteLabel = new QLabel(tr("Type de vol : "), this);

    nomEmetteurChequeRecette = new QLineEdit(this);
    nomEmetteurChequeRecette->setToolTip(tr("Nom du titulaire du compte du chèque"));
    QLabel* nomEmetteurChequeRecetteLabel = new QLabel(tr("Nom émetteur chèque : "), this);
    connect(nomEmetteurChequeRecette, &QLineEdit::textChanged, this, &AeroDms::prevaliderDonnneesSaisiesRecette);

    banqueNumeroChequeRecette = new QLineEdit(this);
    banqueNumeroChequeRecette->setToolTip(tr("Initiales du nom de la banque et numéro du chèque"));
    QLabel* banqueNumeroChequeRecetteLabel = new QLabel(tr("Banque & n° chèque : "), this);
    connect(banqueNumeroChequeRecette, &QLineEdit::textChanged, this, &AeroDms::prevaliderDonnneesSaisiesRecette);

    montantRecette = new QDoubleSpinBox(this);
    montantRecette->setRange(0.0, 2000.0);
    montantRecette->setSingleStep(1.0);
    montantRecette->setDecimals(2);
    montantRecette->setSuffix("€");
    QLabel* montantRecetteLabel = new QLabel(tr("Montant : "), this);
    connect(montantRecette, &QDoubleSpinBox::valueChanged, this, &AeroDms::prevaliderDonnneesSaisiesRecette);

    validerLaRecette = new QPushButton(tr("Valider la recette"), this);
    validerLaRecette->setToolTip(tr("Validation possible si :\n\
   -montant de recette saisi (non nul),\n\
   -nom de l'émetteur du chèque saisi,\n\
   -nom banque et numéro du chèque saisi,\n\
   -au moins un vol associé sélectionné."));
    connect(validerLaRecette, &QPushButton::clicked, this, &AeroDms::enregistrerUneRecette);

    QGridLayout* infosRecette = new QGridLayout();
    ajoutRecette->addLayout(infosRecette, 1);
    infosRecette->addWidget(typeDeRecetteLabel, 0, 0);
    infosRecette->addWidget(typeDeRecette, 0, 1);
    infosRecette->addWidget(nomEmetteurChequeRecetteLabel, 1, 0);
    infosRecette->addWidget(nomEmetteurChequeRecette, 1, 1);
    infosRecette->addWidget(banqueNumeroChequeRecetteLabel, 2, 0);
    infosRecette->addWidget(banqueNumeroChequeRecette, 2, 1);
    infosRecette->addWidget(montantRecetteLabel, 3, 0);
    infosRecette->addWidget(montantRecette, 3, 1);
    infosRecette->addWidget(validerLaRecette, 5, 0, 2, 0);
}

void AeroDms::verifierPresenceDeMiseAjour()
{
    const QString dossierAVerifier = parametresSysteme.cheminStockageBdd + "/update/";
    //On vérifie dans les répertoire d'update si 2 fichiers ont bougé :
    //  -AeroDms.exe : mise à jour de l'application et eventuellement de ses librairies
    //  -Qt6Core.dll : mise à jour de Qt sans mise à jour de l'application (mise à jour
    // de sécurité/maintenance de Qt avec compatibilité des interfaces)
    //  -podofo.dll : mise à jour de la librairie PDF et des librairies associées
    if ( uneMaJEstDisponible(dossierAVerifier, "AeroDms.exe")
         || uneMaJEstDisponible(dossierAVerifier,"Qt6Core.dll")
         || uneMaJEstDisponible(dossierAVerifier, "podofo.dll"))
    {
        fermerSplashscreen();

        QMessageBox dialogueMiseAJourDisponible;
        dialogueMiseAJourDisponible.setText(QString(tr("Une mise à jour d'AeroDMS est disponible.\n"))
            + tr("Voulez vous l'exécuter maintenant ?"));
        dialogueMiseAJourDisponible.setInformativeText(tr("La liste des nouveautés est disponible sur <a href=\"https://github.com/cvermot/AeroDMS/compare/v") + QApplication::applicationVersion() + "...main\">GitHub</a>.");
        dialogueMiseAJourDisponible.setWindowTitle(QApplication::applicationName() + " - " + tr("Mise à jour disponible"));
        dialogueMiseAJourDisponible.setIcon(QMessageBox::Question);
        dialogueMiseAJourDisponible.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        const int ret = dialogueMiseAJourDisponible.exec();

        switch (ret)
        {
        case QMessageBox::Yes:
        {
            mettreAJourApplication(parametresSysteme.cheminStockageBdd + "/update/");
        }
        break;

        case QMessageBox::No:
        default:
        {
            if (!db->laBddEstALaVersionAttendue())
            {
                fermerSplashscreen();
                passerLeLogicielEnLectureSeule();

                QMessageBox dialogueErreurVersionBdd;
                dialogueErreurVersionBdd.setText(tr("Une mise à jour de l'application est disponible et doit être réalisée\n\
car la base de données a évoluée.\n\n\
L'application va passer en mode lecture seule.\
\n\nPour mettre à jour l'application, séléctionnez l'option \"Verifier la présence\n"
"de mise à jour\" du menu Aide."));
                dialogueErreurVersionBdd.setWindowTitle(QApplication::applicationName() + " - " + tr("Version de base données incompatible"));
                dialogueErreurVersionBdd.setIcon(QMessageBox::Critical);
                dialogueErreurVersionBdd.setStandardButtons(QMessageBox::Close);
                dialogueErreurVersionBdd.exec();
            }
        }
        break;
        }
    }
    else if (sender() == miseAJourAction)
    {
        QMessageBox::information(this,
            tr("Pas de mise à jour disponible"),
            tr("Aucune mise à jour ne semble disponible.\n\nSi vous attendiez une mise à jour, merci de\ncontacter la personne responsable de l'application."));
    }
    else if (!db->laBddEstALaVersionAttendue())
    {
        fermerSplashscreen();
        passerLeLogicielEnLectureSeule();

        QMessageBox dialogueErreurVersionBdd;
        dialogueErreurVersionBdd.setText(tr("La version de la base de données ne correspond pas à la version attendue par le logiciel.\n\n\
L'application va passer en mode lecture seule pour éviter tout risque d'endommagement de la BDD.\n\n\
Consultez le développeur / responsable de l'application pour plus d'informations."));
        dialogueErreurVersionBdd.setWindowTitle(QApplication::applicationName() + " - " + tr("Erreur de version de base de données"));
        dialogueErreurVersionBdd.setIcon(QMessageBox::Critical);
        dialogueErreurVersionBdd.setStandardButtons(QMessageBox::Close);
        dialogueErreurVersionBdd.exec();
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

void AeroDms::ouvrirSplashscreen()
{
    splash = new QSplashScreen(QPixmap("./ressources/splash.png"), Qt::WindowStaysOnTopHint);
    splash->show();
    splash->showMessage("Chargement en cours...", Qt::AlignCenter | Qt::AlignBottom, Qt::black);
}

void AeroDms::demanderFermetureSplashscreen()
{
    if (splash != nullptr)
    {
        QTimer::singleShot(500, this, &AeroDms::fermerSplashscreen);
    }   
}
void AeroDms::fermerSplashscreen()
{
    if (splash != nullptr)
    {
        splash->close();

        delete splash;
        splash = nullptr;
    }
}

void AeroDms::verifierSignatureNumerisee()
{
    if (!QFile("./ressources/signature.jpg").exists())
    {
        boutonSignatureManuelle->setEnabled(false);
    }
    else
    {
        boutonSignatureManuelle->activate(QAction::Trigger);
    }
}

void AeroDms::initialiserOngletGraphiques()
{
    //=============Onglet graphiques
    graphiques = new QHBoxLayout();
    widgetGraphiques = new QWidget(this);
    widgetGraphiques->setLayout(graphiques);
    mainTabWidget->addTab(widgetGraphiques, QIcon("./ressources/chart-areaspline.svg"), tr("Graphiques")); 
}

void AeroDms::initialiserOngletSubventionsDemandees()
{
    //=============Onglet Subventions demandées
    vueSubventions = new QTableWidget(0, AeroDmsTypes::SubventionDemandeeTableElementTableElement_NB_COLONNES, this);
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_DATE, new QTableWidgetItem("Date"));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_PILOTE, new QTableWidgetItem("Pilote"));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_BENEFICIAIRE, new QTableWidgetItem("Nom bénéficiaire"));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_TYPE_DEMANDE, new QTableWidgetItem("Type de demande"));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_MONTANT, new QTableWidgetItem("Montant"));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_MONTANT_VOL, new QTableWidgetItem("Montant vol"));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_ID_DEMANDE, new QTableWidgetItem("ID demande"));
    vueSubventions->setColumnHidden(AeroDmsTypes::SubventionDemandeeTableElement_ID_DEMANDE, true);
    vueSubventions->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vueSubventions->setSelectionBehavior(QAbstractItemView::SelectRows);
    vueSubventions->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vueSubventions, QIcon("./ressources/checkbook.svg"), "Subventions demandées");
}

void AeroDms::initialiserBarreDOutils()
{
    QToolBar* toolBar = addToolBar(tr("Outils"));
    const QIcon iconeAjouterUnVol = QIcon("./ressources/airplane-plus.svg");
    bouttonAjouterUnVol = new QAction(iconeAjouterUnVol, tr("Ajouter un &vol/une dépense"), this);
    bouttonAjouterUnVol->setStatusTip(tr("Ajouter un vol/une dépense"));
    bouttonAjouterUnVol->setShortcut(Qt::Key_F2);
    connect(bouttonAjouterUnVol, &QAction::triggered, this, &AeroDms::selectionnerUneFacture);
    toolBar->addAction(bouttonAjouterUnVol);

    const QIcon iconeAjouterPilote = QIcon("./ressources/account-tie-hat.svg");
    bouttonAjouterPilote = new QAction(iconeAjouterPilote, tr("Ajouter un &pilote"), this);
    bouttonAjouterPilote->setStatusTip(tr("Ajout d'un pilote"));
    bouttonAjouterPilote->setShortcut(Qt::Key_F3);
    connect(bouttonAjouterPilote, &QAction::triggered, this, &AeroDms::ajouterUnPilote);
    toolBar->addAction(bouttonAjouterPilote);

    const QIcon iconeAjouterCotisation = QIcon("./ressources/ticket.svg");
    bouttonAjouterCotisation = new QAction(iconeAjouterCotisation, tr("Ajouter une &cotisation pour un pilote"), this);
    bouttonAjouterCotisation->setStatusTip(tr("Ajouter une cotisation pour un pilote"));
    bouttonAjouterCotisation->setShortcut(Qt::Key_F4);
    connect(bouttonAjouterCotisation, &QAction::triggered, this, &AeroDms::ajouterUneCotisation);
    toolBar->addAction(bouttonAjouterCotisation);

    const QIcon iconeAjouterSortie = QIcon("./ressources/transit-connection-variant.svg");
    bouttonAjouterSortie = new QAction(iconeAjouterSortie, tr("Ajouter une &sortie"), this);
    bouttonAjouterSortie->setStatusTip(tr("Ajouter une sortie"));
    bouttonAjouterSortie->setShortcut(Qt::Key_F5);
    connect(bouttonAjouterSortie, &QAction::triggered, this, &AeroDms::ajouterUneSortie);
    toolBar->addAction(bouttonAjouterSortie);

    toolBar->addSeparator();

    const QIcon iconeGenerePdf = QIcon("./ressources/file-cog.svg");
    bouttonGenerePdf = new QAction(iconeGenerePdf, tr("&Générer les PDF de demande de subvention"), this);
    bouttonGenerePdf->setStatusTip(tr("Génère les fichiers de demandes : remplissage des formulaires et génération de PDF associant les factures"));
    bouttonGenerePdf->setShortcut(Qt::CTRL + Qt::Key_G);
    connect(bouttonGenerePdf, &QAction::triggered, this, &AeroDms::genererPdf);
    toolBar->addAction(bouttonGenerePdf);

    const QIcon iconeGenerePdfRecapHdv = QIcon("./ressources/account-file-text.svg");
    bouttonGenerePdfRecapHdv = new QAction(iconeGenerePdfRecapHdv, tr("Générer les PDF de récapitulatif &HdV de l'année sélectionnée"), this);
    bouttonGenerePdfRecapHdv->setStatusTip(tr("Générer les PDF de recap HdV de l'année sélectionnée"));
    bouttonGenerePdfRecapHdv->setShortcut(Qt::CTRL + Qt::Key_R);
    connect(bouttonGenerePdfRecapHdv, &QAction::triggered, this, &AeroDms::genererPdfRecapHdV);
    toolBar->addAction(bouttonGenerePdfRecapHdv);
}

void AeroDms::initialiserBarreDeFiltres()
{
    QToolBar* selectionToolBar = addToolBar(tr("Filtres"));
    listeDeroulanteAnnee = new QComboBox(this);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTablePilotes);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableRecettes);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableFactures);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerStatistiques);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerListeBaladesEtSorties);
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableSubventionsDemandees);
    actionListeDeroulanteAnnee = selectionToolBar->addWidget(listeDeroulanteAnnee);

    listeDeroulantePilote = new QComboBox(this);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTablePilotes);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableSubventionsDemandees);
    actionListeDeroulantePilote = selectionToolBar->addWidget(listeDeroulantePilote);

    listeDeroulanteType = new QComboBox(this);
    listeDeroulanteType->addItems(db->recupererTypesDesVol());
    AeroDmsServices::ajouterIconesComboBox(*listeDeroulanteType);
    listeDeroulanteType->addItem(AeroDmsServices::recupererIcone("Cotisation"), tr("Cotisation"), "Cotisation");
    listeDeroulanteType->addItem(AeroDmsServices::recupererIcone("Tous"), tr("Tous les types de vol"), "*");
    listeDeroulanteType->setCurrentIndex(4);

    connect(listeDeroulanteType, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulanteType, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableRecettes);
    connect(listeDeroulanteType, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableSubventionsDemandees);
    actionListeDeroulanteType = selectionToolBar->addWidget(listeDeroulanteType);

    listeDeroulanteElementsSoumis = new QComboBox(this);
    listeDeroulanteElementsSoumis->addItem(tr("Éléments soumis et non soumis au CSE"), 
        AeroDmsTypes::ElementSoumis_TOUS_LES_ELEMENTS);
    listeDeroulanteElementsSoumis->setItemIcon(AeroDmsTypes::ElementSoumis_TOUS_LES_ELEMENTS, 
        AeroDmsServices::recupererIcone("Tous"));
    listeDeroulanteElementsSoumis->addItem(tr("Éléments soumis au CSE"),
        AeroDmsTypes::ElementSoumis_ELEMENTS_SOUMIS);
    listeDeroulanteElementsSoumis->setItemIcon(AeroDmsTypes::ElementSoumis_ELEMENTS_SOUMIS, 
        AeroDmsServices::recupererIcone("Oui"));
    listeDeroulanteElementsSoumis->addItem(tr("Éléments non soumis au CSE"),
        AeroDmsTypes::ElementSoumis_ELEMENTS_NON_SOUMIS);
    listeDeroulanteElementsSoumis->setItemIcon(AeroDmsTypes::ElementSoumis_ELEMENTS_NON_SOUMIS, 
        AeroDmsServices::recupererIcone("Non"));

    connect(listeDeroulanteElementsSoumis, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulanteElementsSoumis, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableRecettes);
    connect(listeDeroulanteElementsSoumis, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableFactures);
    actionListeDeroulanteElementsSoumis = selectionToolBar->addWidget(listeDeroulanteElementsSoumis);

    listeDeroulanteStatistique = new QComboBox(this);
    listeDeroulanteStatistique->addItem(QIcon("./ressources/chart-bar-stacked.svg"),
        tr("Statistiques mensuelles"),
        AeroDmsTypes::Statistiques_HEURES_ANNUELLES);
    listeDeroulanteStatistique->addItem(QIcon("./ressources/chart-pie.svg"),
        tr("Statistiques par pilote"),
        AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE);
    listeDeroulanteStatistique->addItem(QIcon("./ressources/chart-pie.svg"),
        tr("Statistiques par type de vol"),
        AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL);
    listeDeroulanteStatistique->addItem(QIcon("./ressources/chart-pie.svg"),
        tr("Statistiques par activité"),
        AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE);
    listeDeroulanteStatistique->addItem(QIcon("./ressources/chart-donut-variant.svg"),
        tr("Statuts des pilotes"),
        AeroDmsTypes::Statistiques_STATUTS_PILOTES);
    listeDeroulanteStatistique->addItem(QIcon("./ressources/chart-donut-variant.svg"),
        tr("Types d'aéronefs"),
        AeroDmsTypes::Statistiques_AERONEFS);

    connect(listeDeroulanteStatistique, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerStatistiques);
    actionListeDeroulanteStatistique = selectionToolBar->addWidget(listeDeroulanteStatistique);
}

void AeroDms::initialiserBoitesDeDialogues()
{
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

    dialogueGestionAeronefs = new DialogueGestionAeronefs(db, this);

    //Gestion des signaux liés à la génération PDF
    connect(pdf, SIGNAL(mettreAJourNombreFacturesATraiter(int)), this, SLOT(ouvrirFenetreProgressionGenerationPdf(int)));
    connect(pdf, SIGNAL(mettreAJourNombreFacturesTraitees(int)), this, SLOT(mettreAJourFenetreProgressionGenerationPdf(int)));
    connect(pdf, SIGNAL(generationTerminee(QString, QString)), this, SLOT(mettreAJourBarreStatusFinGenerationPdf(QString, QString)));
    connect(pdf, SIGNAL(echecGeneration()), this, SLOT(mettreAJourEchecGenerationPdf()));
}

void AeroDms::initialiserMenuFichier()
{
    //========================Menu Fichier
    QMenu* menuFichier = menuBar()->addMenu(tr("&Fichier"));

    QAction* boutonOuvrirDossierDemandes = new QAction(QIcon("./ressources/folder-open.svg"), tr("&Ouvrir le dossier contenant les demandes de subventions"), this);
    menuFichier->addAction(boutonOuvrirDossierDemandes);
    boutonOuvrirDossierDemandes->setShortcut(QKeySequence::Open);
    connect(boutonOuvrirDossierDemandes, SIGNAL(triggered()), this, SLOT(ouvrirDossierDemandesSubventions()));

    QMenu* menuOuvrirPdfDemandeSubvention = menuFichier->addMenu(tr("Ouvrir un fichier de &demande de subventions"));
    menuOuvrirPdfDemandeSubvention->setIcon(QIcon("./ressources/file-pdf-box.svg"));

    QAction* boutonOuvrirDerniereDemande = new QAction(QIcon("./ressources/file-outline.svg"), tr("Ouvrir la &dernière demande"), this);
    menuOuvrirPdfDemandeSubvention->addAction(boutonOuvrirDerniereDemande);
    connect(boutonOuvrirDerniereDemande, SIGNAL(triggered()), this, SLOT(ouvrirPdfDemandeSuvbvention()));

    menuOuvrirAutreDemande = menuOuvrirPdfDemandeSubvention->addMenu(tr("Ouvrir un &autre fichier de demande de subventions"));
    menuOuvrirAutreDemande->setIcon(QIcon("./ressources/file-pdf-box.svg"));

    peuplerMenuAutreDemande();

    QAction* boutonImprimer = new QAction(QIcon("./ressources/printer.svg"), tr("&Imprimer la dernière demande"), this);
    boutonImprimer->setStatusTip(tr("Imprime la dernière demande de subvention en une seule fois (pas d'agrafage séparé)"));
    menuFichier->addAction(boutonImprimer);
    connect(boutonImprimer, SIGNAL(triggered()), this, SLOT(imprimerLaDerniereDemande()));

    QAction* boutonImprimerAgrafage = new QAction(QIcon("./ressources/printer.svg"), tr("Imprimer la dernière demande (avec &agrafage)"), this);
    boutonImprimerAgrafage->setStatusTip(tr("Permet d'imprimer chaque fichier séparément. Pour l'agrafage automatique par l'imprimante, pensez à\nsélectionner l'option adéquate dans les paramètres de l'imprimante."));
    boutonImprimerAgrafage->setShortcut(QKeySequence::Print);
    menuFichier->addAction(boutonImprimerAgrafage);
    connect(boutonImprimerAgrafage, SIGNAL(triggered()), this, SLOT(imprimerLaDerniereDemandeAgrafage()));
}

void AeroDms::initialiserMenuOptions()
{
    //========================Menu Options
    QMenu* menuOption = menuBar()->addMenu(tr("&Options"));

    QMenu* menuSignature = menuOption->addMenu(tr("&Signature"));
    menuSignature->setIcon(QIcon("./ressources/file-sign.svg"));

    boutonAucuneSignature = new QAction(QIcon("./ressources/file-outline.svg"), tr("Signature &manuelle"), this);
    boutonAucuneSignature->setStatusTip(tr("Ne pas remplir le champ signature. Chaque document sera signé au stylo par le trésorier."));
    boutonAucuneSignature->setCheckable(true);
    menuSignature->addAction(boutonAucuneSignature);

    boutonSignatureManuelle = new QAction(QIcon("./ressources/draw-pen.svg"), tr("Utiliser l'&image d'une signature"), this);
    boutonSignatureManuelle->setStatusTip(tr("Utiliser une image de signature pour remplir le champ signature."));
    boutonSignatureManuelle->setCheckable(true);
    menuSignature->addAction(boutonSignatureManuelle);

    boutonSignatureNumerique = new QAction(QIcon("./ressources/lock-check-outline.svg"), tr("Signature &numérique avec Lex Community"), this);
    boutonSignatureNumerique->setStatusTip(tr("Préparer le document pour une signature numérique via Lex Community."));
    boutonSignatureNumerique->setCheckable(true);
    menuSignature->addAction(boutonSignatureNumerique);

    connect(boutonAucuneSignature, SIGNAL(triggered()), this, SLOT(changerModeSignature()));
    connect(boutonSignatureNumerique, SIGNAL(triggered()), this, SLOT(changerModeSignature()));
    connect(boutonSignatureManuelle, SIGNAL(triggered()), this, SLOT(changerModeSignature()));

    QMenu* menuFusionnerLesPdf = menuOption->addMenu(tr("&Fusion des PDF"));
    menuFusionnerLesPdf->setIcon(QIcon("./ressources/paperclip.svg"));
    boutonFusionnerLesPdf = new QAction(QIcon("./ressources/paperclip-check.svg"), tr("&Fusionner les PDF"), this);
    boutonFusionnerLesPdf->setStatusTip(tr("Fusionne tous les PDF générés en un seul fichier PDF (les PDF \"unitaires\" restent disponibles dans le dossier de génération)"));
    boutonFusionnerLesPdf->setCheckable(true);
    menuFusionnerLesPdf->addAction(boutonFusionnerLesPdf);
    boutonNePasFusionnerLesPdf = new QAction(QIcon("./ressources/paperclip-off.svg"), tr("&Ne pas fusionner les PDF"), this);
    boutonNePasFusionnerLesPdf->setStatusTip(tr("Ne pas générer le PDF fusionné"));
    boutonNePasFusionnerLesPdf->setCheckable(true);
    menuFusionnerLesPdf->addAction(boutonNePasFusionnerLesPdf);

    connect(boutonFusionnerLesPdf, SIGNAL(triggered()), this, SLOT(changerFusionPdf()));
    connect(boutonNePasFusionnerLesPdf, SIGNAL(triggered()), this, SLOT(changerFusionPdf()));

    QMenu* menuDemandesAGenerer = menuOption->addMenu(tr("&Demandes à générer"));
    menuDemandesAGenerer->setIcon(QIcon("./ressources/file-cog.svg"));
    boutonDemandesAGenererToutes = new QAction(QIcon("./ressources/file-document-multiple.svg"), tr("&Toutes"), this);
    boutonDemandesAGenererToutes->setStatusTip(tr("Générer tout (recette et dépenses)"));
    boutonDemandesAGenererToutes->setCheckable(true);
    menuDemandesAGenerer->addAction(boutonDemandesAGenererToutes);
    boutonDemandesAGenererRecettes = new QAction(QIcon("./ressources/file-document-plus.svg"), tr("&Recettes uniquement"), this);
    boutonDemandesAGenererRecettes->setStatusTip(tr("Générer uniquement les documents de remises de recettes"));
    boutonDemandesAGenererRecettes->setCheckable(true);
    menuDemandesAGenerer->addAction(boutonDemandesAGenererRecettes);
    boutonDemandesAGenererRecettes->setCheckable(true);
    boutonDemandesAGenererDepenses = new QAction(QIcon("./ressources/file-document-minus.svg"), tr("&Dépenses uniquement"), this);
    boutonDemandesAGenererDepenses->setStatusTip(tr("Générer uniquement les documents de demande de subventions/remboursements"));
    menuDemandesAGenerer->addAction(boutonDemandesAGenererDepenses);

    QMenu* menuOptionsRecapAnnuel = menuOption->addMenu(QIcon("./ressources/account-file-text.svg"), tr("&Options du récapitulatif annuel"));
    boutonOptionRecapAnnuelRecettes = new QAction(QIcon("./ressources/table-plus.svg"), tr("Récapitulatif des &recettes"), this);
    menuOptionsRecapAnnuel->addAction(boutonOptionRecapAnnuelRecettes);
    boutonOptionRecapAnnuelRecettes->setCheckable(true);
    boutonOptionRecapAnnuelRecettes->setStatusTip(tr("Permet d'ajouter le récapitulatif des recettes dans le récap des heures de vol"));
    boutonOptionRecapAnnuelBaladesSorties = new QAction(QIcon("./ressources/airplane-search.svg"), tr("Récapitulatif des &balades et sorties"), this);
    menuOptionsRecapAnnuel->addAction(boutonOptionRecapAnnuelBaladesSorties);
    boutonOptionRecapAnnuelBaladesSorties->setCheckable(true);
    boutonOptionRecapAnnuelBaladesSorties->setStatusTip(tr("Permet d'ajouter le récapitulatif des balades et sorties (dates, durées, noms des passagers, couts et recettes...) dans le récap des heures de vol"));
    //Génération des graphiques
    QMenu* graphiquesDuRecapAnnuel = menuOptionsRecapAnnuel->addMenu(QIcon("./ressources/chart-areaspline.svg"), tr("&Graphiques"));
    graphiquesDuRecapAnnuel->setStatusTip(tr("Séléctionne les graphiques à ajouter au récapitulatif annuel des heures de vol"));

    boutonGraphRecapAnnuelHeuresAnnuelles = new QAction(QIcon("./ressources/chart-bar-stacked.svg"), tr("&Heures annuelles"), this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelHeuresAnnuelles);
    boutonGraphRecapAnnuelHeuresAnnuelles->setCheckable(true);

    boutonGraphRecapAnnuelHeuresParPilote = new QAction(QIcon("./ressources/chart-pie.svg"), tr("Heures par &pilote"), this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelHeuresParPilote);
    boutonGraphRecapAnnuelHeuresParPilote->setCheckable(true);

    boutonGraphRecapAnnuelHeuresParTypeDeVol = new QAction(QIcon("./ressources/chart-pie.svg"), tr("Heures par type de &vol"), this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelHeuresParTypeDeVol);
    boutonGraphRecapAnnuelHeuresParTypeDeVol->setCheckable(true);

    boutonGraphRecapAnnuelHeuresParActivite = new QAction(QIcon("./ressources/chart-pie.svg"), tr("Heures par &activite"), this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelHeuresParActivite);
    boutonGraphRecapAnnuelHeuresParActivite->setCheckable(true);

    boutonGraphRecapAnnuelStatutsDesPilotes = new QAction(QIcon("./ressources/chart-donut-variant.svg"), tr("&Statuts des pilotes"), this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelStatutsDesPilotes);
    boutonGraphRecapAnnuelStatutsDesPilotes->setCheckable(true);

    boutonGraphRecapAnnuelAeronefs = new QAction(QIcon("./ressources/chart-donut-variant.svg"), tr("Aérone&fs"), this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelAeronefs);
    boutonGraphRecapAnnuelAeronefs->setCheckable(true);

    //Résolutions
    QMenu* resolutionGraphiques = graphiquesDuRecapAnnuel->addMenu(QIcon("./ressources/chart-areaspline.svg"), tr("&Résolution des graphiques"));

    boutonGraphResolutionFullHd = new QAction(QIcon("./ressources/high-definition.svg"), tr("&Full HD (1920 × 1080)"), this);
    resolutionGraphiques->addAction(boutonGraphResolutionFullHd);
    boutonGraphResolutionFullHd->setCheckable(true);

    boutonGraphResolutionQhd = new QAction(QIcon("./ressources/high-definition.svg"), tr("&QHD (2560 × 1440)"), this);
    resolutionGraphiques->addAction(boutonGraphResolutionQhd);
    boutonGraphResolutionQhd->setCheckable(true);

    boutonGraphResolution4k = new QAction(QIcon("./ressources/ultra-high-definition.svg"), tr("&UHD (3840 × 2160)"), this);
    resolutionGraphiques->addAction(boutonGraphResolution4k);
    boutonGraphResolution4k->setCheckable(true);

    connect(boutonGraphResolutionFullHd, SIGNAL(triggered()), this, SLOT(changerResolutionExportGraphiques()));
    connect(boutonGraphResolutionQhd, SIGNAL(triggered()), this, SLOT(changerResolutionExportGraphiques()));
    connect(boutonGraphResolution4k, SIGNAL(triggered()), this, SLOT(changerResolutionExportGraphiques()));
    boutonGraphResolution4k->activate(QAction::Trigger);

    graphiquesDuRecapAnnuel->addSeparator();
    boutonGraphRecapAnnuelSelectionnerTousLesGraphs = new QAction(QIcon("./ressources/check-all.svg"), tr("&Sélectionner tous les graphs"), this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelSelectionnerTousLesGraphs);
    boutonGraphRecapAnnuelSelectionnerTousLesGraphs->setStatusTip(tr("Ajoute tous les graphiques au récap des heures de vol"));
    connect(boutonGraphRecapAnnuelSelectionnerTousLesGraphs, SIGNAL(triggered()), this, SLOT(selectionnerTousLesGraphsPourRecapAnnuel()));

    menuOptionsRecapAnnuel->addSeparator();
    boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap = new QAction(QIcon("./ressources/check-all.svg"), tr("&Sélectionner tous les graphs et tous les récaps"), this);
    menuOptionsRecapAnnuel->addAction(boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap);
    boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap->setStatusTip(tr("Ajoute tous les graphiques, le tableau des recettes et le tableau des balades au récap des heures de vol"));
    connect(boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap, SIGNAL(triggered()), this, SLOT(selectionnerTousLesGraphsPourRecapAnnuel()));

    connect(boutonDemandesAGenererToutes, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));
    connect(boutonDemandesAGenererRecettes, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));
    connect(boutonDemandesAGenererDepenses, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));

    menuOption->addSeparator();

    boutonActivationScanAutoFactures = new QAction(QIcon("./ressources/file-search.svg"), tr("Désactiver le scan &automatique des factures"), this);
    boutonActivationScanAutoFactures->setStatusTip(tr("Active/désactive le scan à l'ouverture des factures PDF. Cette option permet de désactiver le scan automatique si une facture fait planter le logiciel par exemple."));
    menuOption->addAction(boutonActivationScanAutoFactures);
    connect(boutonActivationScanAutoFactures, SIGNAL(triggered()), this, SLOT(switchScanAutomatiqueDesFactures()));

    menuOption->addSeparator();

    boutonParametresDuLogiciel = new QAction(QIcon("./ressources/cog.svg"), tr("&Paramètres du logiciel"), this);
    boutonParametresDuLogiciel->setStatusTip(tr("Ouvrir le panneau de configuration du logiciel"));
    menuOption->addAction(boutonParametresDuLogiciel);
    connect(boutonParametresDuLogiciel, SIGNAL(triggered()), this, SLOT(ouvrirDialogueParametresApplication()));

    QFont font;
    font.setWeight(QFont::Bold);
    boutonDemandesAGenererToutes->setFont(font);
    boutonDemandesAGenererToutes->setChecked(true);
    typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_TOUTES;
    boutonAucuneSignature->setFont(font);
    boutonAucuneSignature->setChecked(true);
    signature = AeroDmsTypes::Signature_SANS;
    boutonFusionnerLesPdf->setFont(font);
    boutonFusionnerLesPdf->setChecked(true);
}

void AeroDms::initialiserMenuOutils()
{
    //========================Menu Outils
    QMenu* menuOutils = menuBar()->addMenu(tr("Ou&tils"));
    menuOutils->setToolTipsVisible(true);

    menuOutils->addAction(bouttonAjouterUnVol);
    menuOutils->addAction(bouttonAjouterPilote);
    menuOutils->addAction(bouttonAjouterCotisation);
    menuOutils->addAction(bouttonAjouterSortie);
    menuOutils->addSeparator();
    menuOutils->addAction(bouttonGenerePdf);
    menuOutils->addAction(bouttonGenerePdfRecapHdv);

    menuOutils->addSeparator();

    QMenu* scanFacture = menuOutils->addMenu(tr("Scan automatique des &factures"));
    scanFacture->setToolTipsVisible(true);
    scanFacture->setStatusTip(tr("Scan une facture en se basant sur une des méthode utilisée par le logiciel pour un type de facture déjà connu."));
    scanFacture->setIcon(QIcon("./ressources/file-search.svg"));
    scanAutoOpenFlyer = new QAction(tr("&OpenFlyer (CAPAM, ACB)"), this);
    scanAutoOpenFlyer->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoOpenFlyer);
    scanAutoAerogest = new QAction(tr("&Aerogest (ACBA)"), this);
    scanAutoAerogest->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoAerogest);
    scanAutoAca = new QAction(tr("Aér&oclub d'Andernos"), this);
    scanAutoAca->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoAca);
    scanAutoDaca = new QAction(tr("&DACA"), this);
    scanAutoDaca->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoDaca);
    scanAutoSepavia = new QAction(tr("&SEPAVIA"), this);
    scanAutoSepavia->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoSepavia);
    scanAutoUaca = new QAction(tr("&UACA"), this);
    scanAutoUaca->setIcon(QIcon("./ressources/airplane-search.svg"));
    scanFacture->addAction(scanAutoUaca);
    scanFacture->addSeparator();
    scanAutoGenerique1Passe = new QAction(tr("&Générique (une passe)"), this);
    scanAutoGenerique1Passe->setIcon(QIcon("./ressources/text-box-search.svg"));
    scanFacture->addAction(scanAutoGenerique1Passe);
    scanAutoGenerique = new QAction(tr("Générique (&multi-passe)"), this);
    scanAutoGenerique->setIcon(QIcon("./ressources/text-box-search.svg"));
    scanFacture->addAction(scanAutoGenerique);
    scanFacture->addSeparator();
    scanAutoCsv = new QAction(tr("&Importer les vols depuis un fichier CSV"), this);
    scanAutoCsv->setIcon(QIcon("./ressources/file-delimited-outline.svg"));
    scanAutoCsv->setDisabled(true);
    scanAutoCsv->setStatusTip(tr("Importe les données associées à une facture depuis un fichier CSV. Nécessite de charger au préalable la facture qui servira de justificatif."));
    scanAutoCsv->setToolTip(tr("Format du CSV attendu :\nDate;Durée;Immat;Cout\n01/01/2000;1:30;F-ABCD;100,99 €"));
    scanFacture->addAction(scanAutoCsv);
    connect(scanAutoOpenFlyer, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoAerogest, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoAca, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoDaca, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoSepavia, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoUaca, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoGenerique1Passe, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoGenerique, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoCsv, SIGNAL(triggered()), this, SLOT(recupererVolDepuisCsv()));

    menuOutils->addSeparator();

    QMenu* mailing = menuOutils->addMenu(tr("&Mailing"));
    mailing->setIcon(QIcon("./ressources/email-multiple.svg"));
    mailingPilotesAyantCotiseCetteAnnee = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes ayant &cotisé cette année"), this);
    mailing->addAction(mailingPilotesAyantCotiseCetteAnnee);
    mailingPilotesActifsAyantCotiseCetteAnnee = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un &mail aux pilotes ayant cotisé cette année (pilotes actifs seulement)"), this);
    mailing->addAction(mailingPilotesActifsAyantCotiseCetteAnnee);
    mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes n'ayant pas épuisé leur subvention &entrainement"), this);
    mailing->addAction(mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement);
    mailingPilotesActifs = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes &actifs"), this);
    mailing->addAction(mailingPilotesActifs);
    mailingPilotesActifsBrevetes = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes actifs &brevetés"), this);
    mailing->addAction(mailingPilotesActifsBrevetes);
    mailingPilotesDerniereDemandeSubvention = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes concernés par la dernière &demande de subvention"), this);
    mailing->addAction(mailingPilotesDerniereDemandeSubvention);
    QMenu* menuMailDemandesSubvention = mailing->addMenu(QIcon("./ressources/email-multiple.svg"), tr("Envoyer un mail aux pilotes concernés par une demande de &subvention"));
    QList<QDate> datesDemandes = db->recupererDatesDesDemandesDeSubventions();
    for (int i = 0; i < datesDemandes.size(); i++)
    {
        QAction* action = new QAction(QIcon("./ressources/email-multiple.svg"), tr("Demande du ") + datesDemandes.at(i).toString("dd/MM/yyyy"), this);
        action->setData(datesDemandes.at(i).toString("yyyy-MM-dd"));
        menuMailDemandesSubvention->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    }
    connect(mailingPilotesAyantCotiseCetteAnnee, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesActifsAyantCotiseCetteAnnee, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesActifsBrevetes, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesDerniereDemandeSubvention, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesActifs, SIGNAL(triggered()), this, SLOT(envoyerMail()));

    menuOutils->addSeparator();
    QAction* boutonGestionAeronefs = new QAction(QIcon("./ressources/airplane-cog.svg"), tr("Gérer les aé&ronefs"), this);
    boutonGestionAeronefs->setStatusTip(tr("Permet d'indiquer le type associé à chaque immatriculation connue par le logiciel (à des fins statistiques)"));
    menuOutils->addAction(boutonGestionAeronefs);
    connect(boutonGestionAeronefs, SIGNAL(triggered()), this, SLOT(ouvrirGestionAeronefs()));

    boutonEditerLePiloteSelectionne = new QAction(QIcon("./ressources/account-edit.svg"), tr("É&diter le pilote sélectionné"), this);
    boutonEditerLePiloteSelectionne->setStatusTip(tr("Permet d'éditer le pilote actuellement sélectionné. Fonction disponible uniquement si un pilote est sélectionné dans la vue courante."));
    boutonEditerLePiloteSelectionne->setEnabled(false);
    menuOutils->addAction(boutonEditerLePiloteSelectionne);
    connect(boutonEditerLePiloteSelectionne, SIGNAL(triggered()), this, SLOT(editerPilote()));

    menuOutils->addSeparator();

    QAction* boutonConversionHeureDecimalesVersHhMm = new QAction(QIcon("./ressources/clock-star-four-points.svg"), tr("Convertir &une heure en décimal"), this);
    boutonConversionHeureDecimalesVersHhMm->setStatusTip(tr("Convertir une heure sous forme décimale (X,y heures) en HH:mm"));
    menuOutils->addAction(boutonConversionHeureDecimalesVersHhMm);
    connect(boutonConversionHeureDecimalesVersHhMm, SIGNAL(triggered()), this, SLOT(convertirHeureDecimalesVersHhMm()));
}

void AeroDms::initialiserMenuAide()
{
    //========================Menu Aide
    QMenu* helpMenu = menuBar()->addMenu(tr("&Aide"));

    QAction* aideQtAction = new QAction(QIcon("./ressources/lifebuoy.svg"), tr("Aide en &ligne"), this);
    aideQtAction->setShortcut(Qt::Key_F1);
    aideQtAction->setStatusTip(tr("Ouvrir l'aide en ligne"));
    helpMenu->addAction(aideQtAction);
    connect(aideQtAction, SIGNAL(triggered()), this, SLOT(ouvrirAide()));

    helpMenu->addSeparator();

    miseAJourAction = new QAction(QIcon("./ressources/download-box.svg"), tr("&Vérifier la présence de mise à jour"), this);
    miseAJourAction->setStatusTip(tr("Vérifie la présence de mise à jour et permet d'effectuer la mise à jour le cas échéant"));
    helpMenu->addAction(miseAJourAction);
    connect(miseAJourAction, SIGNAL(triggered()), this, SLOT(verifierPresenceDeMiseAjour()));

    boutonModeDebug = new QAction(QIcon("./ressources/bug.svg"), tr("Activer le mode &debug"), this);
    boutonModeDebug->setStatusTip(tr("Active/désactive le mode débug. Ce mode permet d'afficher des informations supplémentaires dans l'application et de modifier certains paramètres dans la fenêtre de configuration."));
    boutonModeDebug->setCheckable(true);
    helpMenu->addAction(boutonModeDebug);
    connect(boutonModeDebug, SIGNAL(triggered()), this, SLOT(switchModeDebug()));

    helpMenu->addSeparator();

    QAction* aboutQtAction = new QAction(QIcon("./ressources/qt-logo.svg"), tr("À propos de &Qt"), this);
    aboutQtAction->setStatusTip(tr("Voir la fenêtre à propos de Qt"));
    helpMenu->addAction(aboutQtAction);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QAction* aboutAction = new QAction(QIcon("./ressources/shield-airplane.svg"), tr("À propos de &AeroDMS"), this);
    aboutAction->setStatusTip(tr("Voir la fenêtre à propos de cette application"));
    helpMenu->addAction(aboutAction);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aPropos()));
}

void AeroDms::initialiserRaccourcisClavierSansActionIhm()
{
    ongletSuivantRaccourciClavier = new QShortcut(QKeySequence::MoveToNextPage, this);
    connect(ongletSuivantRaccourciClavier, SIGNAL(activated()), this, SLOT(switchOnglet()));

    ongletPrecedentRaccourciClavier = new QShortcut(QKeySequence::MoveToPreviousPage, this);
    connect(ongletPrecedentRaccourciClavier, SIGNAL(activated()), this, SLOT(switchOnglet()));
}

void AeroDms::changerModeSignature()
{
    QFont font;
    font.setWeight(QFont::Normal);
    boutonAucuneSignature->setFont(font);
    boutonSignatureManuelle->setFont(font);
    boutonSignatureNumerique->setFont(font);
    boutonAucuneSignature->setChecked(false);
    boutonSignatureManuelle->setChecked(false);
    boutonSignatureNumerique->setChecked(false);

    font.setWeight(QFont::Bold);
    if (sender() == boutonAucuneSignature)
    {
        boutonAucuneSignature->setFont(font);
        boutonAucuneSignature->setChecked(true);
        signature = AeroDmsTypes::Signature_SANS;
    }
    else if (sender() == boutonSignatureManuelle)
    {
        boutonSignatureManuelle->setFont(font);
        boutonSignatureManuelle->setChecked(true);
        signature = AeroDmsTypes::Signature_MANUSCRITE_IMAGE;
    }
    else if (sender() == boutonSignatureNumerique)
    {
        boutonSignatureNumerique->setFont(font);
        boutonSignatureNumerique->setChecked(true);
        signature = AeroDmsTypes::Signature_NUMERIQUE_LEX_COMMUNITY;
    }
}

void AeroDms::changerResolutionExportGraphiques()
{
    QFont font;
    font.setWeight(QFont::Normal);
    boutonGraphResolutionFullHd->setFont(font);
    boutonGraphResolutionQhd->setFont(font);
    boutonGraphResolution4k->setFont(font);

    boutonGraphResolutionFullHd->setChecked(false);
    boutonGraphResolutionQhd->setChecked(false);
    boutonGraphResolution4k->setChecked(false);

    font.setWeight(QFont::Bold);
    
    if (sender() == boutonGraphResolutionFullHd)
    {
        boutonGraphResolutionFullHd->setChecked(true);
        boutonGraphResolutionFullHd->setFont(font);
    }
    else if (sender() == boutonGraphResolutionQhd)
    {
        boutonGraphResolutionQhd->setChecked(true);
        boutonGraphResolutionQhd->setFont(font);
    }
    else if (sender() == boutonGraphResolution4k)
    {
        boutonGraphResolution4k->setChecked(true);
        boutonGraphResolution4k->setFont(font);
    }
}

void AeroDms::selectionnerTousLesGraphsPourRecapAnnuel()
{
    if (sender() == boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap)
    {
        boutonOptionRecapAnnuelRecettes->setChecked(true);
        boutonOptionRecapAnnuelBaladesSorties->setChecked(true);
    }

    boutonGraphRecapAnnuelHeuresAnnuelles->setChecked(true);
    boutonGraphRecapAnnuelHeuresParPilote->setChecked(true);
    boutonGraphRecapAnnuelHeuresParTypeDeVol->setChecked(true);
    boutonGraphRecapAnnuelHeuresParActivite->setChecked(true);
    boutonGraphRecapAnnuelStatutsDesPilotes->setChecked(true);
    boutonGraphRecapAnnuelAeronefs->setChecked(true);
}

void AeroDms::changerDemandesAGenerer()
{
    QFont font;
    font.setWeight(QFont::Normal);
    boutonDemandesAGenererToutes->setFont(font);
    boutonDemandesAGenererRecettes->setFont(font);
    boutonDemandesAGenererDepenses->setFont(font);
    boutonDemandesAGenererToutes->setChecked(false);
    boutonDemandesAGenererRecettes->setChecked(false);
    boutonDemandesAGenererDepenses->setChecked(false);

    font.setWeight(QFont::Bold);
    if (sender() == boutonDemandesAGenererToutes)
    {
        boutonDemandesAGenererToutes->setFont(font);
        boutonDemandesAGenererToutes->setChecked(true);
        typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_TOUTES;
    }
    else if (sender() == boutonDemandesAGenererRecettes)
    {
        boutonDemandesAGenererRecettes->setFont(font);
        boutonDemandesAGenererRecettes->setChecked(true);
        typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT;
    }
    else if (sender() == boutonDemandesAGenererDepenses)
    {
        boutonDemandesAGenererDepenses->setFont(font);
        boutonDemandesAGenererDepenses->setChecked(true);
        typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT;
    }
}

void AeroDms::changerFusionPdf()
{
    QFont font;
    font.setWeight(QFont::Normal);
    boutonFusionnerLesPdf->setFont(font);
    boutonNePasFusionnerLesPdf->setFont(font);
    boutonFusionnerLesPdf->setChecked(false);
    boutonNePasFusionnerLesPdf->setChecked(false);

    font.setWeight(QFont::Bold);
    if (sender() == boutonFusionnerLesPdf)
    {
        boutonFusionnerLesPdf->setFont(font);
        boutonFusionnerLesPdf->setChecked(true);
    }
    else if (sender() == boutonNePasFusionnerLesPdf)
    {
        boutonNePasFusionnerLesPdf->setFont(font);
        boutonNePasFusionnerLesPdf->setChecked(true);
    }
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
        case AeroDmsTypes::Statistiques_STATUTS_PILOTES:
        {
            m_activeWidget = new StatistiqueDonuts( db,
                                                    AeroDmsTypes::Statistiques_STATUTS_PILOTES,
                                                    m_contentArea);
            break;
        }
        case AeroDmsTypes::Statistiques_AERONEFS:
        {
            m_activeWidget = new StatistiqueDonutCombineWidget( db,
                                                                AeroDmsTypes::Statistiques_AERONEFS,
                                                                m_contentArea,
                                                                listeDeroulanteAnnee->currentData().toInt());
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
    //Dialogue de progression de génération PDF
    progressionGenerationPdf = new DialogueProgressionGenerationPdf(this);
    connect(progressionGenerationPdf, SIGNAL(accepted()), this, SLOT(ouvrirPdfGenere()));
    connect(progressionGenerationPdf, SIGNAL(rejected()), this, SLOT(detruireFenetreProgressionGenerationPdf()));
    connect(progressionGenerationPdf, SIGNAL(imprimer()), this, SLOT(imprimerLaDemande()));
    connect(progressionGenerationPdf, SIGNAL(imprimerAgrafage()), this, SLOT(imprimerLaDemandeAgrafage()));
    connect(progressionGenerationPdf, SIGNAL(ouvrirLeDossier()), this, SLOT(ouvrirDossierFichierVenantDEtreGenere()));

    //progressionGenerationPdf->show();
    progressionGenerationPdf->setMaximum(p_nombreDeFacturesATraiter);
    progressionGenerationPdf->setValue(0);

}

void AeroDms::mettreAJourFenetreProgressionGenerationPdf(const int p_nombreDeFacturesTraitees)
{
    progressionGenerationPdf->setValue(p_nombreDeFacturesTraitees);
}

void AeroDms::ouvrirFenetreProgressionImpression(const int p_nombreDeFichiersAImprimer)
{
    progressionImpression = new DialogueProgressionImpression(this);
    progressionImpression->setMaximumFichier(p_nombreDeFichiersAImprimer);

    connect(progressionImpression, SIGNAL(accepted()), this, SLOT(detruireFenetreProgressionImpression()));

    update();
}

void AeroDms::detruireFenetreProgressionImpression()
{
    delete progressionImpression;
    progressionImpression = nullptr;
}

void AeroDms::detruireFenetreProgressionGenerationPdf()
{
    if (progressionGenerationPdf != nullptr)
    {
        delete progressionGenerationPdf;
        progressionGenerationPdf = nullptr;
    }
}

void AeroDms::mettreAJourBarreStatusFinGenerationPdf(const QString p_cheminDossier, const QString p_cheminFichierPdfMerge)
{
    fichierAImprimer = p_cheminFichierPdfMerge;
    dossierSortieGeneration = p_cheminDossier;

    //On met à jour la table des vols et des recettes (champ Soumis CE)
    peuplerTableVols();
    peuplerTableRecettes();

    //On met à jour la table des subventions demandées
    peuplerTableSubventionsDemandees();

    //On repeuple le menu d'ouverture des fichiers générés 
    peuplerMenuAutreDemande();

    //On met à jour la fenêtre de progression
    const bool fichierMergeDisponible = (fichierAImprimer != "");
    progressionGenerationPdf->generationEstTerminee(fichierMergeDisponible);
    const QString status = "Génération terminée. Fichiers disponibles sous "
                            +p_cheminDossier;
    statusBar()->showMessage(status);
}

void AeroDms::mettreAJourEchecGenerationPdf()
{
    progressionGenerationPdf->close();
    statusBar()->showMessage("Echec de la génération");
    detruireFenetreProgressionGenerationPdf();
}

AeroDms::~AeroDms()
{
}

void AeroDms::peuplerTablePilotes()
{
    //On mette à jour l'info pilote a editer en cas de demande d'édition du pilote via le menu outils,
    //si on a sélectionné un pilote donné et pas "Tous les pilotes"
    boutonEditerLePiloteSelectionne->setEnabled(false);
    if (listeDeroulantePilote->currentData().toString() != "*")
    {
        piloteAEditer = listeDeroulantePilote->currentData().toString();
        boutonEditerLePiloteSelectionne->setEnabled(true);
    }  

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
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE, new QTableWidgetItem(QString::number(subvention.entrainement.montantRembourse, 'f', 2).append(" €")));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_HEURES_BALADES_SUBVENTIONNEES, new QTableWidgetItem(subvention.balade.heuresDeVol));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_MONTANT_BALADES_SUBVENTIONNE, new QTableWidgetItem(QString::number(subvention.balade.montantRembourse, 'f', 2).append(" €")));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_HEURES_SORTIES_SUBVENTIONNEES, new QTableWidgetItem(subvention.sortie.heuresDeVol));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_MONTANT_SORTIES_SUBVENTIONNE, new QTableWidgetItem(QString::number(subvention.sortie.montantRembourse, 'f', 2).append(" €")));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_HEURES_TOTALES_SUBVENTIONNEES, new QTableWidgetItem(subvention.totaux.heuresDeVol));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_MONTANT_TOTAL_SUBVENTIONNE, new QTableWidgetItem(QString::number(subvention.totaux.montantRembourse, 'f', 2).append(" €")));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_PILOTE_ID, new QTableWidgetItem(subvention.idPilote));

        QTableWidgetItem* item = vuePilotes->item(i, AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE);
        if (subvention.entrainement.montantRembourse < subvention.montantSubventionEntrainement)
        {
            item->setBackground(QBrush(QColor(140, 255, 135, 120)));
            item->setToolTip("Subvention entrainement restante pour ce pilote : " 
                + QString::number(subvention.montantSubventionEntrainement - subvention.entrainement.montantRembourse) 
                + " €");
        }
        else
        {
            item->setBackground(QBrush(QColor(255, 140, 135, 120)));
            item->setToolTip("Ce pilote a consommé la totalité de sa subvention d'entrainement");   
        }
    }
    vuePilotes->resizeColumnsToContents();
}

void AeroDms::peuplerTableVols()
{
    AeroDmsTypes::ListeVols listeVols = db->recupererVols( listeDeroulanteAnnee->currentData().toInt(),
                                                           listeDeroulantePilote->currentData().toString());
    const AeroDmsTypes::ElementSoumis volAAfficher = static_cast<AeroDmsTypes::ElementSoumis>(listeDeroulanteElementsSoumis->currentData().toInt());

    int nbItems = 0;

    vueVols->clearContents();
    vueVols->setRowCount(0);

    for (int i = 0; i < listeVols.size(); i++)
    {
        const AeroDmsTypes::Vol vol = listeVols.at(i);
        if ( ( (vol.estSoumis && volAAfficher == AeroDmsTypes::ElementSoumis_ELEMENTS_SOUMIS)
              || (!vol.estSoumis && volAAfficher == AeroDmsTypes::ElementSoumis_ELEMENTS_NON_SOUMIS)
              || volAAfficher == AeroDmsTypes::ElementSoumis_TOUS_LES_ELEMENTS)
            && ( listeDeroulanteType->currentData().toString() == "*"
                 || listeDeroulanteType->currentText() == vol.typeDeVol) )
        {
            vueVols->setRowCount(nbItems + 1);
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_DATE, new QTableWidgetItem(vol.date.toString("dd/MM/yyyy")));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_PILOTE, new QTableWidgetItem(QString(vol.prenomPilote).append(" ").append(vol.nomPilote)));
            QTableWidgetItem *twSoumisCe = new QTableWidgetItem(vol.estSoumisCe);
            twSoumisCe->setIcon(AeroDmsServices::recupererIcone(vol.estSoumisCe));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_SOUMIS_CE, twSoumisCe);
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_DUREE, new QTableWidgetItem(vol.duree));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_COUT, new QTableWidgetItem(QString::number(vol.coutVol, 'f', 2).append(" €")));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_SUBVENTION, new QTableWidgetItem(QString::number(vol.montantRembourse, 'f', 2).append(" €")));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_TYPE_DE_VOL, new QTableWidgetItem(vol.typeDeVol));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_REMARQUE, new QTableWidgetItem(vol.remarque));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_ACTIVITE, new QTableWidgetItem(vol.activite));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_IMMAT, new QTableWidgetItem(vol.immat));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, new QTableWidgetItem(QString::number(vol.dureeEnMinutes)));
            vueVols->setItem(nbItems, AeroDmsTypes::VolTableElement_VOL_ID, new QTableWidgetItem(QString::number(vol.volId)));
            nbItems++;
        }
    }
    vueVols->resizeColumnsToContents();

    bouttonGenerePdfRecapHdv->setEnabled(true);
    mailingPilotesAyantCotiseCetteAnnee->setEnabled(true);
    //On desactive la génération du récap annuel si on est sur la sélection "Toutes les années"
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
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_MONTANT, new QTableWidgetItem(QString::number(facture.coutDuVol, 'f', 2).append(" €")));
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_IMMAT, new QTableWidgetItem(facture.immat));
        //Par défaut => vol entrainement
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_TYPE, new QTableWidgetItem(typeDeVol->itemText(2)));
    }

    vueVolsDetectes->resizeColumnsToContents();
    
}

void AeroDms::peuplerTableFactures()
{
    const AeroDmsTypes::ElementSoumis elementAAfficher = static_cast<AeroDmsTypes::ElementSoumis>(listeDeroulanteElementsSoumis->currentData().toInt());

    const AeroDmsTypes::ListeDemandeRemboursementFacture listeFactures = db->recupererToutesLesDemandesDeRemboursement(listeDeroulanteAnnee->currentData().toInt());
    vueFactures->setRowCount(0);
    for (int i = 0; i < listeFactures.size(); i++)
    {
        const AeroDmsTypes::DemandeRemboursementFacture facture = listeFactures.at(i);

        if (facture.soumisCe && elementAAfficher == AeroDmsTypes::ElementSoumis_ELEMENTS_SOUMIS
            || !facture.soumisCe && elementAAfficher == AeroDmsTypes::ElementSoumis_ELEMENTS_NON_SOUMIS
            || elementAAfficher == AeroDmsTypes::ElementSoumis_TOUS_LES_ELEMENTS)
        {
            vueFactures->setRowCount((vueFactures->rowCount()) + 1);

            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_INTITULE, new QTableWidgetItem(facture.intitule));
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_MONTANT, new QTableWidgetItem(QString::number(facture.montant, 'f', 2).append(" €")));
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_PAYEUR, new QTableWidgetItem(facture.payeur));
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_NOM_SORTIE, new QTableWidgetItem(facture.nomSortie));
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_TYPE_SORTIE, new QTableWidgetItem(facture.typeDeSortie));
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_DATE, new QTableWidgetItem(facture.date.toString("dd/MM/yyyy")));
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_ANNEE, new QTableWidgetItem(QString::number(facture.annee)));
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_NOM_FACTURE, new QTableWidgetItem(facture.nomFacture));
            if (facture.soumisCe)
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Oui");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone("Oui"));
                vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_SOUMIS_CE, twSoumisCe);
            }
            else
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Non");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone("Non"));
                vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_SOUMIS_CE, twSoumisCe);
            }
            vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_FACTURE_ID, new QTableWidgetItem(QString::number(facture.id)));
        }  
    }
    vueFactures->resizeColumnsToContents();
}

void AeroDms::peuplerTableSubventionsDemandees()
{
    vueSubventions->clearContents();
    vueSubventions->setRowCount(0);

    const AeroDmsTypes::ListeDemandesRemboursementSoumises listeFactures = db->recupererDemandesRemboursementSoumises( listeDeroulanteAnnee->currentData().toInt(),                                                                                                                 listeDeroulantePilote->currentData().toString());
    
    int nbItems = 0;

    for (int i = 0; i < listeFactures.size(); i++)
    {
        const AeroDmsTypes::DemandeRemboursementSoumise facture = listeFactures.at(i);
        if ( (listeDeroulanteType->currentData().toString() == "*"
            || listeDeroulanteType->currentText() == facture.typeDeVol) )
        {
            vueSubventions->setRowCount(nbItems+1);

            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_DATE, new QTableWidgetItem(facture.dateDemande.toString("dd/MM/yyyy")));
            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_PILOTE, new QTableWidgetItem(facture.nomPilote));
            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_BENEFICIAIRE, new QTableWidgetItem(facture.nomBeneficiaire));
            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_TYPE_DEMANDE, new QTableWidgetItem(facture.typeDeVol));
            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_MONTANT, new QTableWidgetItem(QString::number(facture.montant, 'f', 2).append(" €")));
            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_MONTANT_VOL, new QTableWidgetItem(QString::number(facture.coutTotalVolAssocies, 'f', 2).append(" €")));
            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_ID_DEMANDE, new QTableWidgetItem(QString::number(facture.id)));

            nbItems++;
        }
    }
    vueSubventions->resizeColumnsToContents();

}

void AeroDms::peuplerTableRecettes()
{
    const AeroDmsTypes::ListeRecetteDetail listeRecettesCotisations = db->recupererRecettesCotisations(listeDeroulanteAnnee->currentData().toInt());
    const AeroDmsTypes::ListeRecetteDetail listeRecettesHorsCotisations = db->recupererRecettesHorsCotisation(listeDeroulanteAnnee->currentData().toInt());

    const AeroDmsTypes::ElementSoumis elementAAfficher = static_cast<AeroDmsTypes::ElementSoumis>(listeDeroulanteElementsSoumis->currentData().toInt());

    vueRecettes->clearContents();
    vueRecettes->setRowCount(0);

    for (int i = 0; i < listeRecettesCotisations.size(); i++)
    {
        AeroDmsTypes::RecetteDetail recette = listeRecettesCotisations.at(i);

        if ( ( (recette.estSoumisCe && elementAAfficher == AeroDmsTypes::ElementSoumis_ELEMENTS_SOUMIS )
                || (!recette.estSoumisCe && elementAAfficher == AeroDmsTypes::ElementSoumis_ELEMENTS_NON_SOUMIS)
                || elementAAfficher == AeroDmsTypes::ElementSoumis_TOUS_LES_ELEMENTS)
            &&  (listeDeroulanteType->currentData().toString() == "*"
                 || listeDeroulanteType->currentText() == recette.typeDeRecette) )
        {
            const int position = vueRecettes->rowCount();
            vueRecettes->setRowCount(vueRecettes->rowCount() + 1);

            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_INTITULE, new QTableWidgetItem(recette.intitule));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_ID, new QTableWidgetItem(QString::number(recette.id)));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_MONTANT, new QTableWidgetItem(QString::number(recette.montant, 'f', 2) + " €"));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_DATE, new QTableWidgetItem(QString::number(recette.annee)));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_TYPE_DE_RECETTE, new QTableWidgetItem(recette.typeDeRecette));
            if (recette.estSoumisCe)
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Oui");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone("Oui"));
                vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, twSoumisCe);
            }
            else
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Non");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone("Non"));
                vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, twSoumisCe);
            }   
        }      
    }

    for (int i = 0; i < listeRecettesHorsCotisations.size(); i++)
    {
        AeroDmsTypes::RecetteDetail recette = listeRecettesHorsCotisations.at(i); 

        if ( ( (recette.estSoumisCe && elementAAfficher == AeroDmsTypes::ElementSoumis_ELEMENTS_SOUMIS)
                || (!recette.estSoumisCe && elementAAfficher == AeroDmsTypes::ElementSoumis_ELEMENTS_NON_SOUMIS)
                || elementAAfficher == AeroDmsTypes::ElementSoumis_TOUS_LES_ELEMENTS)
            && (listeDeroulanteType->currentData().toString() == "*"
                || listeDeroulanteType->currentText() == recette.typeDeRecette) )
        {
            const int position = vueRecettes->rowCount();
            vueRecettes->setRowCount(vueRecettes->rowCount() + 1);

            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_INTITULE, new QTableWidgetItem(recette.intitule));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_ID, new QTableWidgetItem(QString::number(recette.id)));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_MONTANT, new QTableWidgetItem(QString::number(recette.montant, 'f', 2) + " €"));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_DATE, new QTableWidgetItem(QString::number(recette.annee)));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_TYPE_DE_RECETTE, new QTableWidgetItem(recette.typeDeRecette));
            if (recette.estSoumisCe)
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Oui");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone("Oui"));
                vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, twSoumisCe);
            }
            else
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Non");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone("Non"));
                vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, twSoumisCe);
            }
        }   
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
    const AeroDmsTypes::CotisationAnnuelle infosCotisation = dialogueAjouterCotisation->recupererInfosCotisationAAjouter();
    if (!infosCotisation.estEnEdition)
    {
        //On contrôle que le pilote n'a pas déjà une cotisation pour cette année
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
            
            peuplerTableRecettes();
            //On peut avoir réactivé un pilote inactif : on réélabore les listes de pilotes
            peuplerListesPilotes();

            statusBar()->showMessage("Cotisation " + QString::number(infosCotisation.annee) + " ajoutée pour le pilote " + db->recupererNomPrenomPilote(infosCotisation.idPilote));
        }
    }
    else
    {
        db->ajouterCotisation(infosCotisation);
        statusBar()->showMessage("Cotisation " + QString::number(infosCotisation.annee) + " mise à jour pour le pilote " + db->recupererNomPrenomPilote(infosCotisation.idPilote));

        //On met à jour la table des pilotes (en cas de changement de couleur)
        peuplerTablePilotes();
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

    //Si on est sur une mise à jour, on met à jour les élements d'IHM susceptibles d'être impacté par des changements
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
    peuplerListeBaladesEtSorties();
    peuplerListeSorties();

    statusBar()->showMessage("Sortie " + sortie.nom + " ajoutée");
}

void AeroDms::selectionnerUneFacture()
{
    QString fichier = QFileDialog::getOpenFileName(
        this,
        QApplication::applicationName() + " - " + "Ouvrir une facture",
        parametresSysteme.cheminStockageFacturesATraiter,
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
    if (sender() == scanAutoAerogest)
    {
        aeroclub = AeroDmsTypes::Aeroclub_Generique_Aerogest;
    }
    else if (sender() == scanAutoAca)
    {
        aeroclub = AeroDmsTypes::Aeroclub_ACAndernos;
    }
    else if (sender() == scanAutoDaca)
    {
        aeroclub = AeroDmsTypes::Aeroclub_DACA;
    }
    else if (sender() == scanAutoSepavia)
    {
        aeroclub = AeroDmsTypes::Aeroclub_SEPAVIA;
    }
    else if (sender() == scanAutoUaca)
    {
        aeroclub = AeroDmsTypes::Aeroclub_UACA;
    }
    else if (sender() == scanAutoGenerique)
    {
        aeroclub = AeroDmsTypes::Aeroclub_GENERIQUE;
    }
    else if (sender() == scanAutoGenerique1Passe)
    {
        aeroclub = AeroDmsTypes::Aeroclub_GENERIQUE_1_PASSE;
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

void AeroDms::recupererVolDepuisCsv()
{
    QString fichier = QFileDialog::getOpenFileName(
        this,
        QApplication::applicationName() + " - " + "Ouvrir un fichier CSV de récapitulatif de vol",
        parametresSysteme.cheminStockageFacturesATraiter,
        tr("Fichier CSV (*.csv)"));

    if (!fichier.isNull())
    {
        //On masque par défaut... on reaffiche si le scan est effectué
        //et qu'il ne retourne par une liste vide
        validerLesVols->setHidden(true);
        vueVolsDetectes->setHidden(true);
        supprimerLeVolSelectionne->setHidden(true);

        factures = PdfExtractor::recupererLesDonneesDuCsv(fichier);
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
    immat->clear();
    typeDeVol->setCurrentIndex(2);
    choixBalade->setCurrentIndex(0);
    scanAutoCsv->setEnabled(true);
    
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

    QString recapHdv = "Heures annuelles<br />seulement";
    if (boutonOptionRecapAnnuelRecettes->isChecked()
        && boutonOptionRecapAnnuelBaladesSorties->isChecked())
    {
        recapHdv = "Heures annuelles,<br />recettes, balades et sorties";
    }
    else if (boutonOptionRecapAnnuelRecettes->isChecked())
    {
        recapHdv = "Heures annuelles,<br />recettes";
    }
    else if (boutonOptionRecapAnnuelBaladesSorties->isChecked())
    {
        recapHdv = "Heures annuelles,<br />balades et sorties";
    }

    QMessageBox demandeConfirmationGeneration;
    demandeConfirmationGeneration.setText(QString("Voulez vous générer les PDF de demande de subventions ? <br /><br />")
        + "La génération sera réalisée avec les options suivantes : <br />"
        + texteSignature + "<br />"
        + texteDemande + "<br />"
        + "<b>Fusion des PDF</b> : " + fusionnerLesPdf + "<br />"
        + "<b>Récapitulatif annuel des heures de vol</b> : " + recapHdv + "<br />"
        + "<br />Nombre de demandes à générer :<br />"
        + "Subventions heures de vol : " + nbSubventions + "<br />"
        + "Remboursement factures : " + nbFactures + "<br />"
        + "Remise chèques cotisations : " + nbCotisations + "<br />"
        + "Remise chèques balades et sorties : " + nbBaladesSorties + "<br />");
    demandeConfirmationGeneration.setWindowTitle(QApplication::applicationName() + " - " + "Génération des PDF de demande");
    demandeConfirmationGeneration.setIcon(QMessageBox::Question);
    demandeConfirmationGeneration.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    if ( (db->recupererLesSubventionsAEmettre().size()
         + db->recupererLesDemandesDeRembousementAEmettre().size()
         + db->recupererLesCotisationsAEmettre().size()
         + db->recupererLesRecettesBaladesEtSortiesAEmettre().size()) == 0)
    {
        demandeConfirmationGeneration.setInformativeText("Rien à génerer. Génération indisponible.");
        demandeConfirmationGeneration.setStandardButtons( QMessageBox::Close);
    }

    const int ret = demandeConfirmationGeneration.exec();

    switch (ret)
    {
        case QMessageBox::Yes:
        {
            pdf->imprimerLesDemandesDeSubvention(parametresMetiers.nomTresorier,
                parametresSysteme.cheminSortieFichiersGeneres,
                parametresSysteme.cheminStockageFacturesTraitees,
                typeGenerationPdf,
                signature,
                boutonFusionnerLesPdf->font().bold(),
                boutonOptionRecapAnnuelRecettes->isChecked(),
                boutonOptionRecapAnnuelBaladesSorties->isChecked(),
                calculerValeurGraphAGenererPdf());
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
                                                parametresSysteme.cheminSortieFichiersGeneres,
                                                parametresSysteme.cheminStockageFacturesTraitees,
                                                signature,
                                                calculerValeurGraphAGenererPdf());
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
            QString cheminComplet = parametresSysteme.cheminStockageFacturesTraitees + "/";
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
                + QString::number(montantFacture->value(), 'f', 2)
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
    //2) les données (pilote, date du vol, durée, cout) sont renseignées (OK si le bouton est actif)
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
            QString cheminComplet = parametresSysteme.cheminStockageFacturesTraitees;
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
                    immat->text(),
                    activite->currentText(),
                    volAEditer);
                
                QString volAjouteModifie = "ajouté";
                if (volAEditer != -1)
                {
                    volAjouteModifie = "modifié";
                    //On sort du mode édition
                    volAEditer = -1;
                    //Et on recharge la liste déroulante de sélection des pilotes dans laquelle on avait remis les pilotes
                    //inactifs...
                    peuplerListesPilotes();
                }

                //On supprime la vol de la liste des vols détectés si on en avait chargé un
                //On fait ceci avant la mise à jour de la statiusBar car supprimerLeVolDeLaVueVolsDetectes()
                //en fait également une. De cette façon on masque le status de suppression et on affiche
                //que le status d'ajout du vol
                supprimerLeVolDeLaVueVolsDetectes();

            statusBar()->showMessage(QString("Vol ")
                + typeDeVol->currentText() 
                + " de "
                + db->recupererNomPrenomPilote(idPilote)
                + " du " 
                + dateDuVol->date().toString("dd/MM/yyyy") 
                + " (" 
                + dureeDuVol->time().toString("hh:mm")
                + "/"
                + QString::number(prixDuVol->value(), 'f', 2)
                + "€) "
                + volAjouteModifie
                + ". Montant subvention : "
                + QString::number(montantSubventionne, 'f', 2)
                + "€ / Subvention entrainement restante : "
                + QString::number(subventionRestante, 'f', 2)
                + "€");

            //On rince les données de vol
            dureeDuVol->setTime(QTime::QTime(0, 0));
            prixDuVol->setValue(0);
            remarqueVol->clear();
            immat->clear();
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
    chargerUnVolDetecte(0, 0);
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

        statusBar()->showMessage(tr("Vol supprimé avec succès de la liste des vols detectés"));
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
                                           nomEmetteurChequeRecette->text() + " / " + banqueNumeroChequeRecette->text(),
                                           montantRecette->value());
        statusBar()->showMessage("Recette ajoutee");

        nomEmetteurChequeRecette->clear();
        banqueNumeroChequeRecette->clear();
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

    //On sauvegarde le pilote sélectionné pour le rester le cas échéant
    const QString piloteSelectionne = listeDeroulantePilote->currentData().toString();

    listeDeroulantePilote->clear();
    choixPilote->clear();
    choixPayeur->clear();

    listeDeroulantePilote->addItem(AeroDmsServices::recupererIcone("Tous"), "Tous les pilotes", "*");
    choixPilote->addItem("", "aucun");
    choixPayeur->addItem("", "aucun");
    for (int i = 0; i < pilotes.size(); i++)
    {
        const AeroDmsTypes::Pilote pilote = pilotes.at(i);
        const QString nomPrenom = QString(pilote.prenom).append(" ").append(pilote.nom);
        QString nomPrenomNorm = nomPrenom;
        AeroDmsServices::normaliser(nomPrenomNorm);
        listeDeroulantePilote->addItem(AeroDmsServices::recupererIcone(nomPrenomNorm.at(0)), nomPrenom, pilote.idPilote);
        //Si le pilote est actif, on le met dans la liste...
        //Et on met tous les pilotes si on est en mode édition de vol
        //(permet d'éditer le vol d'un pilote inactif)
        if (pilote.estActif
            || volAEditer != -1)
        {
            choixPilote->addItem(AeroDmsServices::recupererIcone(nomPrenomNorm.at(0)), nomPrenom, pilote.idPilote);
            choixPayeur->addItem(AeroDmsServices::recupererIcone(nomPrenomNorm.at(0)), nomPrenom, pilote.idPilote);
        }
    }

    //Si le pilote précédément sélectionné existe toujours, on le restaure
    const int index = listeDeroulantePilote->findData(piloteSelectionne);
    if (index != -1)
    {
        listeDeroulantePilote->setCurrentIndex(index);
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
                                                                                     parametresMetiers.proportionParticipationBalade,
                                                                                     listeDeroulanteAnnee->currentData().toInt());
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
         || nomEmetteurChequeRecette->text() == ""
         || banqueNumeroChequeRecette->text() == "")
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

void AeroDms::ouvrirGestionAeronefs()
{
    dialogueGestionAeronefs->peuplerListeAeronefs();
    dialogueGestionAeronefs->exec();
}

void AeroDms::aPropos()
{
    QMessageBox::about(this, tr("À propos de ")+ QApplication::applicationName(),
        "<b>"+ QApplication::applicationName() + " v" + QApplication::applicationVersion() + "</b> < br />< br /> "
        "Logiciel de gestion de compta d'une section aéronautique. <br /><br />"
        "Le code source de ce programme est disponible sous"
        " <a href=\"https://github.com/cvermot/AeroDMS\">GitHub</a>.<br />< br/>"
        "Ce programme utilise la libraire <a href=\"https://github.com/podofo/podofo\">PoDoFo</a> 0.10.3.<br />< br/>"
        "Les icones sont issues de <a href=\"https://pictogrammers.com/\">pictogrammers.com</a>.< br />< br />"
        "L'image de démarrage est génerée avec <a href=\"https://designer.microsoft.com/\">Microsoft Designer</a>.< br />< br />"
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
        montantSubventionDejaAlloue = vuePilotes->item( ligneSelectionnee,
                                                         AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE)->text().remove(" €").toFloat();
        anneeAEditer = vuePilotes->item( ligneSelectionnee,
                                         AeroDmsTypes::PiloteTableElement_ANNEE)->text().toInt();

        QAction editer(QIcon("./ressources/account-tie-hat.svg"),"Éditer le pilote", this);
        connect(&editer, SIGNAL(triggered()), this, SLOT(editerPilote()));
        menuClicDroitPilote.addAction(&editer);

        QAction editerCotisation(QIcon("./ressources/ticket.svg"), "Modifier la cotisation", this);
        connect(&editerCotisation, SIGNAL(triggered()), this, SLOT(editerCotisation()));
        menuClicDroitPilote.addAction(&editerCotisation);

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

void AeroDms::editerCotisation()
{
    dialogueAjouterCotisation->editerLaCotisation( piloteAEditer, 
                                                   anneeAEditer,
                                                   montantSubventionDejaAlloue);
    dialogueAjouterCotisation->exec();
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
                + QString::number(coutTotal, 'f', 2)
                + " € / Montant subventionné total : "
                + QString::number(montantTotalSubventionne, 'f', 2)
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

        QAction editerLeVol(QIcon("./ressources/airplane-edit.svg"), "Éditer le vol", this);
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
    const QString cheminComplet = parametresSysteme.cheminStockageFacturesTraitees + "/" + db->recupererNomFacture(volAEditer);
    chargerUneFacture(cheminComplet);

    //On indique que c'est une facture déjà en BDD

    //On récupère les infos du vol pour les réintegrer dans l'IHM
    const AeroDmsTypes::Vol vol = db->recupererVol(volAEditer);

    //On remet les pilotes inactifs dans la listes des pilotes, pour le cas ou on édite un vol
    //d'un pilote inactif
    peuplerListesPilotes();
    choixPilote->setCurrentIndex(choixPilote->findData(vol.idPilote));
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
    immat->setText(vol.immat);

    factureIdEnBdd = vol.facture;

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
    demandeConfirmationSuppression.setWindowTitle(QApplication::applicationName() + " - " + "Suppression d'un vol");
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
    bool masquageEstDemande = false;

    //Si le mode debug n'est pas actif
    if (boutonModeDebug->text() == "Activer le mode &debug")
    {
        boutonModeDebug->setText("Désactiver le mode &debug");
        boutonModeDebug->setIcon(QIcon("./ressources/bug-stop.svg"));
        masquageEstDemande = false;
    }
    //Sinon, le mode est actif, on desactive
    else
    {
        boutonModeDebug->setText("Activer le mode &debug");
        boutonModeDebug->setIcon(QIcon("./ressources/bug.svg"));
        masquageEstDemande = true;
    }

    vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_PILOTE_ID, masquageEstDemande);
    vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_VOL_ID, masquageEstDemande);
    vueVols->setColumnHidden(AeroDmsTypes::VolTableElement_DUREE_EN_MINUTES, masquageEstDemande);
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_FACTURE_ID, masquageEstDemande);
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_NOM_FACTURE, masquageEstDemande);
    vueFactures->setColumnHidden(AeroDmsTypes::FactureTableElement_ANNEE, masquageEstDemande);
    vueRecettes->setColumnHidden(AeroDmsTypes::RecetteTableElement_ID, masquageEstDemande);
    vueSubventions->setColumnHidden(AeroDmsTypes::SubventionDemandeeTableElement_ID_DEMANDE, masquageEstDemande);
}

void AeroDms::switchOnglet()
{
    if (sender() == ongletSuivantRaccourciClavier)
    {
        if (mainTabWidget->currentIndex() < mainTabWidget->count() - 1)
        {
            mainTabWidget->setCurrentIndex(mainTabWidget->currentIndex() + 1);
        }
        else
        {
            mainTabWidget->setCurrentIndex(0);
        }
    }
    else if (sender() == ongletPrecedentRaccourciClavier)
    {
        if (mainTabWidget->currentIndex() > 0)
        {
            mainTabWidget->setCurrentIndex(mainTabWidget->currentIndex() - 1);
        }
        else
        {
            mainTabWidget->setCurrentIndex(mainTabWidget->count() - 1);
        }
    }
    
}

void AeroDms::switchScanAutomatiqueDesFactures()
{
    scanAutomatiqueDesFacturesEstActif = !scanAutomatiqueDesFacturesEstActif;
    if (scanAutomatiqueDesFacturesEstActif)
    {
        boutonActivationScanAutoFactures->setText(tr("Désactiver le scan automatique des factures"));
    }
    else
    {
        boutonActivationScanAutoFactures->setText(tr("Activer le scan automatique des factures"));
    }
    
}

void AeroDms::ouvrirDialogueParametresApplication()
{
    DialogueEditionParametres editionParametres(parametresMetiers,
        parametresSysteme,
        boutonModeDebug->isChecked() ,
        this);
    connect(&editionParametres,
        SIGNAL(envoyerParametres(AeroDmsTypes::ParametresMetier, AeroDmsTypes::ParametresSysteme)),
        this,
        SLOT(enregistrerParametresApplication(AeroDmsTypes::ParametresMetier, AeroDmsTypes::ParametresSysteme)));
    editionParametres.exec();
}

void AeroDms::enregistrerParametresApplication( AeroDmsTypes::ParametresMetier p_parametresMetiers,
                                                AeroDmsTypes::ParametresSysteme p_parametresSysteme)
{
    parametresMetiers = p_parametresMetiers;
    parametresSysteme = p_parametresSysteme;

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "AeroDMS", "AeroDMS");

    settings.beginGroup("baseDeDonnees");
    settings.setValue("chemin", parametresSysteme.cheminStockageBdd);
    settings.setValue("nom", parametresSysteme.nomBdd);
    settings.endGroup();

    settings.beginGroup("dossiers");
    settings.setValue("facturesATraiter", parametresSysteme.cheminStockageFacturesATraiter);
    settings.setValue("facturesSaisies", parametresSysteme.cheminStockageFacturesTraitees);
    settings.setValue("sortieFichiersGeneres", parametresSysteme.cheminSortieFichiersGeneres);
    settings.endGroup();

    settings.beginGroup("noms");
    settings.setValue("nomTresorier", parametresMetiers.nomTresorier);
    settings.endGroup();
    
    settings.beginGroup("mailing");
    settings.setValue("texteChequesDisponibles", parametresMetiers.texteMailDispoCheques);
    settings.setValue("texteSubventionRestante", parametresMetiers.texteMailSubventionRestante);

    settings.setValue("objetChequesDisponibles", parametresMetiers.objetMailDispoCheques);
    settings.setValue("objetSubventionRestante", parametresMetiers.objetMailSubventionRestante);
    settings.setValue("objetAutresMailings", parametresMetiers.objetMailAutresMailings);
    settings.endGroup();

    settings.beginGroup("impression");
    settings.setValue("imprimante", parametresSysteme.imprimante);
    settings.setValue("couleur", parametresSysteme.modeCouleurImpression);
    settings.setValue("resolution", parametresSysteme.resolutionImpression);
    settings.setValue("forcageImpressionRecto", parametresSysteme.forcageImpressionRecto);
    settings.endGroup();

    parametresMetiers.texteMailDispoCheques = settings.value("mailing/texteChequesDisponibles", "").toString();
    parametresMetiers.texteMailSubventionRestante = settings.value("mailing/texteSubventionRestante", "").toString();

    //Fichier de conf commun => le fichier AeroDMS.ini est mis au même endroit que la BDD SQLite
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, settings.value("baseDeDonnees/chemin", "").toString() + QString("/"));
    QSettings settingsMetier(QSettings::IniFormat, QSettings::SystemScope, "AeroDMS");
    settingsMetier.beginGroup("parametresMetier");
    settingsMetier.setValue("montantSubventionEntrainement", parametresMetiers.montantSubventionEntrainement);
    settingsMetier.setValue("montantCotisationPilote", parametresMetiers.montantCotisationPilote);
    settingsMetier.setValue("proportionRemboursementEntrainement", parametresMetiers.proportionRemboursementEntrainement);
    settingsMetier.setValue("plafondHoraireRemboursementEntrainement", parametresMetiers.plafondHoraireRemboursementEntrainement);
    settingsMetier.setValue("proportionRemboursementBalade", parametresMetiers.proportionRemboursementBalade);
    settingsMetier.setValue("proportionParticipationBalade", parametresMetiers.proportionParticipationBalade);
    settingsMetier.endGroup();
 
    settingsMetier.beginGroup("parametresSysteme");
    settingsMetier.setValue("delaisDeGardeDbEnMs", parametresMetiers.delaisDeGardeBdd);
    settingsMetier.setValue("margesHautBas", parametresSysteme.margesHautBas);
    settingsMetier.setValue("margesGaucheDroite", parametresSysteme.margesGaucheDroite);
    settingsMetier.endGroup();

    //On met à jour les marges...
    pdf->mettreAJourMarges(QMarginsF(parametresSysteme.margesGaucheDroite,
            parametresSysteme.margesHautBas,
            parametresSysteme.margesGaucheDroite,
            parametresSysteme.margesHautBas));
}

void AeroDms::convertirHeureDecimalesVersHhMm()
{
    bool ok;
    double heureDecimale = QInputDialog::getDouble(this, tr("Conversion heure décimale"),
        tr("Saisir l'heure au format décimal : "), 0, 0, 10, 2, &ok,
        Qt::WindowFlags(), 1);
    
    if (ok)
    {
        dureeDuVol->setTime(AeroDmsServices::convertirHeuresDecimalesEnQTime(heureDecimale));
    }
}

void AeroDms::envoyerMail()
{
    if (sender() == mailingPilotesAyantCotiseCetteAnnee)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailPilotes(listeDeroulanteAnnee->currentData().toInt(), AeroDmsTypes::MailPilotes_AYANT_COTISE)
            + "?subject="+ parametresMetiers.objetMailAutresMailings +"&body=", QUrl::TolerantMode));
    }
    else if (sender() == mailingPilotesActifsAyantCotiseCetteAnnee)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailPilotes(listeDeroulanteAnnee->currentData().toInt(), AeroDmsTypes::MailPilotes_ACTIF_AYANT_COTISE)
            + "?subject=" + parametresMetiers.objetMailAutresMailings + "&body=", QUrl::TolerantMode));
    }
    else if (sender() == mailingPilotesActifsBrevetes)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailPilotes(listeDeroulanteAnnee->currentData().toInt(), AeroDmsTypes::MailPilotes_ACTIFS_ET_BREVETES)
            + "?subject=" + parametresMetiers.objetMailAutresMailings + "&body=", QUrl::TolerantMode));
    }
    else if (sender() == mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailPilotes(listeDeroulanteAnnee->currentData().toInt(), AeroDmsTypes::MailPilotes_SUBVENTION_NON_CONSOMMEE)
            + "?subject=" + parametresMetiers.objetMailSubventionRestante + "&body="
            + parametresMetiers.texteMailSubventionRestante, QUrl::TolerantMode));
    }
    else if (sender() == mailingPilotesActifs)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailPilotes(listeDeroulanteAnnee->currentData().toInt(), AeroDmsTypes::MailPilotes_ACTIFS)
            + "?subject=" + parametresMetiers.objetMailAutresMailings + "&body=", QUrl::TolerantMode));
    }
    else if (sender() == mailingPilotesDerniereDemandeSubvention)
    {
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailDerniereDemandeDeSubvention()
            + "?subject=" + parametresMetiers.objetMailDispoCheques + "&body="
            + parametresMetiers.texteMailDispoCheques, QUrl::TolerantMode));
    }
    else
    {
        QAction* action = qobject_cast<QAction*>(sender());
        QDesktopServices::openUrl(QUrl("mailto:"
            + db->recupererMailDerniereDemandeDeSubvention(action->data().toString())
            + "?subject=" + parametresMetiers.objetMailDispoCheques + "&body="
            + parametresMetiers.texteMailDispoCheques, QUrl::TolerantMode));
    }  
}

bool AeroDms::uneMaJEstDisponible(const QString p_chemin, const QString p_fichier)
{ 
    QString fichier = p_chemin + p_fichier;
    if (QFile().exists(fichier))
    {
        QFile fichierDistant(fichier);
        if (fichierDistant.open(QFile::ReadOnly))
        {
            QString hashFichierDistant = "";
            QCryptographicHash hash(QCryptographicHash::Sha1);
            if (hash.addData(&fichierDistant))
            {
                hashFichierDistant = hash.result().toHex();
            }
            hash.reset();

            //Calcul de la somme de controle du fichier courant
            QFile fichierCourant(QCoreApplication::applicationDirPath()+"/"+p_fichier);

            if (fichierCourant.open(QFile::ReadOnly))
            {
                QString hashFichierCourant = "";
                if (hash.addData(&fichierCourant))
                {
                    hashFichierCourant = hash.result().toHex();

                    if (hashFichierDistant != hashFichierCourant)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void AeroDms::mettreAJourApplication(const QString p_chemin)
{  
    //Dialogue progression mise à jour
    QProgressDialog* progressionMiseAJour = new QProgressDialog("Mise à jour en cours...", "", 0, 0, this);
    QPushButton* boutonProgressionMiseAJour = new QPushButton("Quitter AeroDMS", this);
    boutonProgressionMiseAJour->setDisabled(true);
    progressionMiseAJour->setCancelButton(boutonProgressionMiseAJour);
    progressionMiseAJour->setAutoClose(false);
    progressionMiseAJour->setWindowModality(Qt::WindowModal);
    progressionMiseAJour->close();
    progressionMiseAJour->setAutoReset(false);
    progressionMiseAJour->setMinimumSize(QSize(300, 150));
    connect(boutonProgressionMiseAJour, &QPushButton::clicked, this, &QCoreApplication::quit);

    //Pour chaque élement présente dans p_chemin, on vérifie s'il existe dans l'aborescence locale
    //Si c'est le cas on renomme le fichier local en suffixant par xxx_
    //Ensuite dans tous les cas on recopie le fichier distant vers le dossier local

    QDirIterator it(p_chemin, QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);

    int nombreDeFichiers = 0;
    while (it.hasNext())
    {
        it.next();
        nombreDeFichiers++;
    }

    progressionMiseAJour->setMaximum(nombreDeFichiers);
    progressionMiseAJour->setValue(0);
    progressionMiseAJour->show();

    QThread::sleep(2);

    //Pas de fonction reset de l'iterateur dans QDirIterator... on redémarre via un nouveau scope
    {
        int etapeMiseAJour = 0;

        QDirIterator it(p_chemin, QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);

        QRegularExpression ini("^.*\\.ini$");
        QRegularExpression sqlite("^.*\\.sqlite");

        while (it.hasNext())
        {
            QString fichier = it.next();
            //On ne replace pas les éventuels fichiers .ini ou .sqlite qui seraient présents dans le répertoire d'update
            if (!fichier.contains(ini)
                && !fichier.contains(sqlite))
            {
                progressionMiseAJour->setLabelText("Mise à jour de : "+it.fileName());

                QString fichierLocal = "./" + fichier;
                QString fichierDistant = fichier;
                fichierLocal.replace(p_chemin, "");
                QFileInfo infosFichierLocal(fichierLocal);
                QString nouveauNomFichierLocal = infosFichierLocal.path() + "/xxx_" + infosFichierLocal.fileName();
                
                QFile::rename(fichierLocal, nouveauNomFichierLocal);
                QFile::copy(fichierDistant, fichierLocal);

                progressionMiseAJour->setValue(etapeMiseAJour);
                etapeMiseAJour++;
            }
        }
    }
    progressionMiseAJour->setValue(nombreDeFichiers);
    progressionMiseAJour->setLabelText(tr("Mise à jour terminée.\nAeroDMS doit redémarrer.\n\n AeroDMS va quitter. Veuillez relancer AeroDMS\npour que la mise à jour soit effective."));
    boutonProgressionMiseAJour->setDisabled(false);
}

void AeroDms::terminerMiseAJourApplication()
{
    //Cette méthode termine une mise à jour précédement démarrée : elle supprime l'ensemble des fichiers xxx_*
    QString dossierCourantApplication = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();

    QDirIterator it(dossierCourantApplication, QStringList() << "xxx_*", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString cheminFichier = it.next();
        QFile fichier(cheminFichier);
        fichier.remove();
    }
}

void AeroDms::initialiserTableauVolsDetectes(QGridLayout* p_infosVol)
{
    vueVolsDetectes = new QTableWidget(0, AeroDmsTypes::VolsDetectesTableElement_NB_COLONNES, this);
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_DATE, new QTableWidgetItem("Date"));
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_DUREE, new QTableWidgetItem("Durée"));
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_MONTANT, new QTableWidgetItem("Montant"));
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_IMMAT, new QTableWidgetItem("Immat"));
    vueVolsDetectes->setHorizontalHeaderItem(AeroDmsTypes::VolsDetectesTableElement_TYPE, new QTableWidgetItem("Type"));
    vueVolsDetectes->setSelectionBehavior(QAbstractItemView::SelectRows);

    vueVolsDetectes->resizeColumnsToContents();
    vueVolsDetectes->setHidden(true);

    p_infosVol->addWidget(vueVolsDetectes, 12, 0, 2, 0);

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
    p_infosVol->addWidget(validerLesVols, 14, 0, 2, 0);
    p_infosVol->addWidget(supprimerLeVolSelectionne, 15, 0, 2, 0);
}

void AeroDms::chargerUnVolDetecte(int row, int column)
{
    idFactureDetectee = row;
    prixDuVol->setValue(factures.at(idFactureDetectee).coutDuVol);
    dureeDuVol->setTime(factures.at(idFactureDetectee).dureeDuVol);
    dateDuVol->setDate(factures.at(idFactureDetectee).dateDuVol);
    immat->setText(factures.at(idFactureDetectee).immat);
    pdfView->pageNavigator()->jump(factures.at(idFactureDetectee).pageDansLeFichierPdf, QPoint());

    supprimerLeVolSelectionne->setEnabled(true);

    statusBar()->showMessage(tr("Appuyez sur Echap pour désélectionner le vol"));
}

void AeroDms::ouvrirDossierDemandesSubventions()
{
    QDesktopServices::openUrl(QUrl(parametresSysteme.cheminSortieFichiersGeneres, QUrl::TolerantMode));
}

void AeroDms::ouvrirDossierFichierVenantDEtreGenere()
{
    QDesktopServices::openUrl(QUrl(dossierSortieGeneration, QUrl::TolerantMode));
    detruireFenetreProgressionGenerationPdf();
}

void AeroDms::ouvrirPdfDemandeSuvbvention()
{
    const QString fichier = rechercherDerniereDemande();

    if (fichier != "")
    {
        QDesktopServices::openUrl(QUrl(fichier, QUrl::TolerantMode));
    }
    else
    {
        QMessageBox::warning(this, tr("AeroDMS"),
                                   tr("Fichier non trouvé\n"
                                      "Aucun fichier fusionné de demande de subvention trouvé."), QMessageBox::Cancel);
    }
}

QString AeroDms::rechercherDerniereDemande()
{
    //Recherche de la dernière demande
    QDir fichierSortie(parametresSysteme.cheminSortieFichiersGeneres);
    fichierSortie.setSorting(QDir::Name | QDir::Reversed | QDir::DirsFirst);
    const QStringList fichiers = fichierSortie.entryList();

    QString fichier = "";

    if (fichiers.size() >= 2)
    {
        //On recherche un fichier fusionné = un fichier dont le nom contient FichiersAssembles
        const QStringList filtre = QStringList("*FichiersAssembles.pdf");
        int i = 0;
        bool fichierTrouve = false;
 
        while (i < fichiers.size() - 2 && !fichierTrouve)
        {
            //Indice n-1 et n-2 contiennent . et ..
            fichier = parametresSysteme.cheminSortieFichiersGeneres + "/" + fichiers.at(i) + "/";
            const QDir dirCourant(fichier);
            const QStringList fichierFusionne = dirCourant.entryList(filtre);
            if (fichierFusionne.size() > 0)
            {
                fichier = fichier + fichierFusionne.at(0);
                fichierTrouve = true;
            }
            i++;
        }
    }

    return fichier;
}

void AeroDms::peuplerMenuAutreDemande()
{
    //On supprime les précédents menus
    menuOuvrirAutreDemande->clear();

    //On ajoute les 10 dernières dates auxquelles on a généré des fichiers
    QDir fichierSortie(parametresSysteme.cheminSortieFichiersGeneres);
    fichierSortie.setSorting(QDir::Name | QDir::Reversed | QDir::DirsFirst);
    const QStringList fichiers = fichierSortie.entryList();
    if (fichiers.size() >= 2)
    {
        int i = 0;
        //les 2 dernières cases ontiennent . et ..
        while (i < fichiers.size() - 2 && i < 10)
        {
            QMenu *menuFichier = menuOuvrirAutreDemande->addMenu(QIcon("./ressources/folder.svg"), fichiers.at(i));
            const QString dossier = parametresSysteme.cheminSortieFichiersGeneres + "/" + fichiers.at(i) + "/";
            const QDir dirCourant(dossier);
            const QStringList listeFichiers = dirCourant.entryList(QStringList("*.pdf"));
            if (listeFichiers.size() > 0)
            {
                QAction* actionDossier = new QAction(QIcon("./ressources/folder-open.svg"), "Ouvrir le dossier", this);
                actionDossier->setData(dossier);
                menuFichier->addAction(actionDossier);
                menuFichier->addSeparator();
                connect(actionDossier, &QAction::triggered, this, &AeroDms::ouvrirUnFichierDeDemandeDeSubvention);

                //Boucle While pour ajouter l'accès à chaque fichier
                int idFichier = 0;
                while (idFichier < listeFichiers.size())
                {
                    QAction* action = new QAction(QIcon("./ressources/file.svg"), listeFichiers.at(idFichier), this);
                    action->setData(dossier+listeFichiers.at(idFichier));
                    menuFichier->addAction(action);
                    connect(action, &QAction::triggered, this, &AeroDms::ouvrirUnFichierDeDemandeDeSubvention);
                    idFichier++;
                }
                
            }
            i++;
        }
    }
}

void AeroDms::ouvrirUnFichierDeDemandeDeSubvention()
{
    if (sender() != nullptr)
    {
        QAction* action = qobject_cast<QAction*>(sender());
        QDesktopServices::openUrl(QUrl(action->data().toString(), QUrl::TolerantMode));
    } 
}

void AeroDms::ouvrirPdfGenere()
{
    const int nombreElements = menuOuvrirAutreDemande->actions().at(0)->menu()->actions().size();  
    QDesktopServices::openUrl(QUrl(menuOuvrirAutreDemande->actions().at(0)->menu()->actions().at(nombreElements - 1)->data().toString(), QUrl::TolerantMode));

    detruireFenetreProgressionGenerationPdf();
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
        immat->clear();
        //La mise à jour de ces données provoque la réélaboration de l'état des boutons de validation => a faire
        //imperativement après le rincage de idFactureDetectee car cette donnée ne redeclenche pas ce traitement

        vueVolsDetectes->clearSelection();
        supprimerLeVolSelectionne->setEnabled(false);

        statusBar()->clearMessage();
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

void AeroDms::gererChangementOnglet()
{
    actionListeDeroulanteElementsSoumis->setVisible(false);
    actionListeDeroulanteStatistique->setVisible(false);
    actionListeDeroulanteAnnee->setVisible(true);
    actionListeDeroulantePilote->setVisible(true);
    actionListeDeroulanteType->setVisible(false);
    listeDeroulanteType->setItemText(4, tr("Tous les types de vols"));
    //On affiche la ligne "Entrainement"
    static_cast<QListView*>(listeDeroulanteType->view())->setRowHidden(2, false);
    //On masque la ligne "Cotisation" qui n'existe qu'en recette
    static_cast<QListView*>(listeDeroulanteType->view())->setRowHidden(3, true);

    //Pour la liste type, on peut avoir des types qui n'existent que dans certaines vues
    //(Cotisation en vue recette)
    //=> on rince le cas échéant
    if (listeDeroulanteType->currentData().toString() == "Cotisation")
    {
        //On repasse a tous les types
        listeDeroulanteType->setCurrentIndex(4);
    }

    if (mainTabWidget->currentWidget() == vueVols)
    {
        actionListeDeroulanteElementsSoumis->setVisible(true);
        actionListeDeroulanteType->setVisible(true);
    }
    else if (mainTabWidget->currentWidget() == vueSubventions)
    {
        listeDeroulanteType->setItemText(4, tr("Tous les types de demandes"));
        actionListeDeroulanteType->setVisible(true);
    }
    else if (mainTabWidget->currentWidget() == vueFactures)
    {
        actionListeDeroulanteElementsSoumis->setVisible(true);
    }
    else if (mainTabWidget->currentWidget() == vueRecettes)
    {
        actionListeDeroulanteElementsSoumis->setVisible(true);
        actionListeDeroulanteType->setVisible(true);

        listeDeroulanteType->setItemText(4, tr("Tous les types de recettes"));
        //On masque la ligne "Entrainement" qui n'existe pas en recette
        static_cast<QListView*>(listeDeroulanteType->view())->setRowHidden(2, true);
        //On affiche la ligne "Entrainement" qui n'existe qu'en recette
        static_cast<QListView*>(listeDeroulanteType->view())->setRowHidden(3, false);

        if (listeDeroulanteType->currentText() == "Entrainement")
        {
            //On repasse a tous les types
            listeDeroulanteType->setCurrentIndex(4);
        }
    }
    else if ( mainTabWidget->currentWidget() == widgetAjoutRecette)
    {
        actionListeDeroulantePilote->setVisible(false);
    }
    else if (mainTabWidget->currentWidget() == widgetAjoutVol)
    {
        actionListeDeroulantePilote->setVisible(false);
        actionListeDeroulanteAnnee->setVisible(false);
    }
    else if (mainTabWidget->currentWidget() == widgetGraphiques)
    {
        actionListeDeroulanteStatistique->setVisible(true);
        actionListeDeroulantePilote->setVisible(false);
    }
}

void AeroDms::imprimerLaDerniereDemande()
{
    fichierAImprimer = rechercherDerniereDemande();
    imprimerLaDemande();
}

void AeroDms::imprimerLaDemande()
{
    QPrinter imprimante;
    if (selectionnerImprimante(imprimante))
    {
        //On demande l'affichage de la fenêtre de génération
        ouvrirFenetreProgressionImpression(1);

        QThread::usleep(500);

        imprimer(imprimante);

        progressionImpression->traitementFichierSuivant();
    }
}

void AeroDms::imprimerLaDerniereDemandeAgrafage()
{
    dossierSortieGeneration = QFileInfo(rechercherDerniereDemande()).absolutePath();
    imprimerLaDemandeAgrafage();
}

void AeroDms::imprimerLaDemandeAgrafage()
{
    QPrinter imprimante;
    if (selectionnerImprimante(imprimante))
    {   
        //On compte les fichiers
        QDir repertoire(dossierSortieGeneration, "*.pdf", QDir::QDir::Name, QDir::Files);

        QFileInfoList liste = repertoire.entryInfoList();
        //Le fichier assemblé est forcément le dernier de la liste car tous les fichiers sont suffixés
        //par un chiffre dans l'ordre de génération. Si la fichier assemblé est présent, on le supprime 
        //de la liste
        if (liste.last().filePath().contains("FichiersAssembles.pdf"))
        {
            liste.removeLast();
        }

        //On demande l'affichage de la fenêtre de génération
        ouvrirFenetreProgressionImpression(liste.size());

        QThread::usleep(500);
        
        for (QFileInfo fichier : liste)
        {
            //On imprime tout 
            fichierAImprimer = fichier.filePath();
            imprimer(imprimante);
        }
        progressionImpression->traitementFichierSuivant();
    }
}

bool AeroDms::selectionnerImprimante(QPrinter &p_printer)
{
    p_printer.setPrinterName(parametresSysteme.imprimante);
    p_printer.setColorMode(parametresSysteme.modeCouleurImpression);
    p_printer.setResolution(parametresSysteme.resolutionImpression);

    QPrintDialog dialog(&p_printer, this);
    dialog.setOption(QAbstractPrintDialog::PrintSelection, false);
    dialog.setOption(QAbstractPrintDialog::PrintPageRange, false);
    dialog.setOption(QAbstractPrintDialog::PrintCollateCopies, false);
    dialog.setOption(QAbstractPrintDialog::PrintToFile, false);
    dialog.setOption(QAbstractPrintDialog::PrintShowPageSize, false);
    dialog.setWindowTitle(QApplication::applicationName() + " - " + tr("Imprimer la demande de subvention"));

    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    //p_printer.setResolution(p_printer.supportedResolutions().last());
    p_printer.setResolution(parametresSysteme.resolutionImpression);
    //En mode forcage recto on force le mode à recto-verso : on inserera ensuite
    //un page blanche entre chaque page qui fera que le rendu sera un recto simple
    if (parametresSysteme.forcageImpressionRecto)
    {
        p_printer.setDuplex(QPrinter::DuplexLongSide);
    }

    return true;
}
void AeroDms::imprimer(QPrinter& p_printer)
{
    progressionImpression->traitementFichierSuivant();

    QPdfDocument *doc = new QPdfDocument(this);
    doc->load(fichierAImprimer);

    int attenteChargementFichier = 0;
    while ( doc->status() != QPdfDocument::Status::Ready
            || attenteChargementFichier > 500)
    {
        attenteChargementFichier++;
        QThread::usleep(10);
    }

    if (doc->status() == QPdfDocument::Status::Ready)
    {
        progressionImpression->setMaximumPage(doc->pageCount());
        
        QPainter painter;
        painter.begin(&p_printer);

        for (int i = 0; i < doc->pageCount(); i++)
        {
            progressionImpression->traitementPageSuivante();
            QSizeF size = doc->pagePointSize(i);
            QImage image = doc->render(i,
                QSize(size.width() * p_printer.resolution() / AeroDmsTypes::K_DPI_PAR_DEFAUT,
                    size.height() * p_printer.resolution() / AeroDmsTypes::K_DPI_PAR_DEFAUT));

            //Si la page du PDF est en paysage, on retourne l'image pour la place en portrait
            //pour l'impression
            if (size.width() > size.height())
            {
                QTransform transformation;
                transformation.rotate(270);
                image = image.transformed(transformation);
                painter.drawImage(0, 0, image);
            }
            else
            {
                painter.drawImage(0, 0, image);
            }

            //S'il reste des pages derrière... on démarre une nouvelle page
            if (i + 1 < doc->pageCount())
            {
                p_printer.newPage();

                //Et si on est en mode forcage Recto, on ajoute une page blanche
                if (parametresSysteme.forcageImpressionRecto)
                {
                    p_printer.newPage();
                }  
            } 
        }
        progressionImpression->traitementPageSuivante();

        painter.end();
    }
    delete doc;
}

int AeroDms::calculerValeurGraphAGenererPdf()
{
    int valeur = 0;

    if (boutonGraphRecapAnnuelHeuresAnnuelles->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_HEURES_ANNUELLES;
    }
    if (boutonGraphRecapAnnuelHeuresParPilote->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE;
    }
    if (boutonGraphRecapAnnuelHeuresParTypeDeVol->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL;
    }
    if (boutonGraphRecapAnnuelHeuresParActivite->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE;
    }
    if (boutonGraphRecapAnnuelStatutsDesPilotes->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_STATUTS_PILOTES;
    }
    if (boutonGraphRecapAnnuelAeronefs->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_AERONEFS;
    }

    if (boutonGraphResolutionFullHd->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Resolution_Full_HD;
    }
    else if (boutonGraphResolutionQhd->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Resolution_QHD;
    }
    else if (boutonGraphResolution4k->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Resolution_4K;
    }

    return valeur;
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

