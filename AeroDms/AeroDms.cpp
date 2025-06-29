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
#include "AeroDms.h"
#include "AeroDmsTypes.h"
#include "AeroDmsServices.h"
#include "PdfRenderer.h"
#include "PdfExtractor.h"
#include "PdfPrinter.h"
#include "PdfDownloader.h"
#include "AixmParser.h"
#include "ZipExtractorWorker.h"

#include "RccGenerator.h"

#include "StatistiqueHistogrammeEmpile.h"
#include "StatistiqueDiagrammeCirculaireWidget.h"
#include "StatistiqueDonutCombineWidget.h"
#include "StatistiqueDonuts.h"

#include "DialogueEditionParametres.h"

#include "resource.h"

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
    peuplerListesEtTables();
    prevaliderDonnnesSaisies();
    prevaliderDonnneesSaisiesRecette();
    changerInfosVolSurSelectionTypeVol();
    verifierSignatureNumerisee();

    gererChangementOnglet();
    switchAffichageInfosComplementaires();

    terminerMiseAJourApplication();
    verifierPresenceDeMiseAjour();
    initialiserGestionnaireTelechargement();
   
    preparerStatusBar();
    demanderFermetureSplashscreen();

    gererBddDistante();
}

void AeroDms::initialiserBaseApplication()
{
    QApplication::setApplicationName(VER_PRODUCTNAME_STR);
    QApplication::setApplicationVersion(VER_PRODUCTVERSION);
    QApplication::setWindowIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_ICONE_APPLICATION));
    mainTabWidget = new QTabWidget(this);
    setCentralWidget(mainTabWidget);

    setWindowTitle(QApplication::applicationName());
    setMinimumSize(800, 600);
    showMaximized();

    installEventFilter(this);
    connect(this, &AeroDms::toucheEchapEstAppuyee, this, &AeroDms::deselectionnerVolDetecte);

    //========================Initialisation des autres attributs
    piloteAEditer = "";
    volAEditer = AeroDmsTypes::K_INIT_INT_INVALIDE;
    factureIdEnBdd = 0;
    subventionAAnoter.id = AeroDmsTypes::K_INIT_INT_INVALIDE;
    subventionAAnoter.texteActuel = "";
}

void AeroDms::peuplerListesEtTables()
{
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
}

void AeroDms::lireParametresEtInitialiserBdd()
{
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationName(), QApplication::applicationName());

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

    if (settings.value("mailing/objetVirementSubvention", "") == "")
    {
        settings.beginGroup("mailing");
        settings.setValue("objetVirementSubvention", tr("[Section aéronautique] Versement de votre subvention par virement"));
        settings.setValue("texteVirementSubvention", tr("Bonjour,\n\nLe CSE vient de verser votre subvention par virement sur le compte de votre aéroclub.\nAfin que vous puissiez saisir ces virement dans votre compte pilote de votre aéroclub, voici le(s) montant(s) du(des) virement(s) réalisé(s) :\n#listeVirements \n\nCordialement"));
        settings.endGroup();
    }

    if (settings.value("modeFonctionnementLogiciel/modeFonctionnement", "") == "")
    {
        settings.beginGroup("modeFonctionnementLogiciel");
        settings.setValue("modeFonctionnement", AeroDmsTypes::ModeFonctionnementLogiciel_INTERNE_UNIQUEMENT);
        settings.setValue("adresse", "");
        settings.setValue("login", "");
        settings.setValue("password", "");
        settings.endGroup();
    }

    parametresMetiers.objetMailDispoCheques = settings.value("mailing/objetChequesDisponibles", "[Section aéronautique] Chèques aéro").toString();
    parametresMetiers.texteMailDispoCheques = settings.value("mailing/texteChequesDisponibles", "").toString();
    parametresMetiers.objetMailSubventionRestante = settings.value("mailing/objetSubventionRestante", "[Section aéronautique] Subvention entrainement").toString();
    parametresMetiers.texteMailSubventionRestante = settings.value("mailing/texteSubventionRestante", "").toString();
    parametresMetiers.objetMailVirementSubvention = settings.value("mailing/objetVirementSubvention", "[Section aéronautique] Subvention entrainement").toString();
    parametresMetiers.texteMailVirementSubvention = settings.value("mailing/texteVirementSubvention", "").toString();
    parametresMetiers.objetMailAutresMailings = settings.value("mailing/objetAutresMailings", "[Section aéronautique] ").toString();

    if (settings.value("impression/imprimante") == "")
    {
        settings.beginGroup("impression");
        settings.setValue("imprimante", "Imprimante par défaut");
        settings.setValue("couleur", 1);
        settings.setValue("resolution", 600);
        settings.setValue("forcageImpressionRecto", true);
        settings.endGroup();
    }

    //Fichier de conf commun => le fichier AeroDMS.ini est mis au même endroit que la BDD SQLite
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, settings.value("baseDeDonnees/chemin", "").toString() + QString("/"));
    QSettings settingsMetier(QSettings::IniFormat, QSettings::SystemScope, QApplication::applicationName());

    if (settingsMetier.value("parametresSysteme/delaisDeGardeDbEnMs", "") == "")
    {
        settingsMetier.beginGroup("parametresSysteme");
        settingsMetier.setValue("delaisDeGardeDbEnMs", "50");
        settingsMetier.setValue("margesHautBas", "20");
        settingsMetier.setValue("margesGaucheDroite", "20");
        settingsMetier.setValue("utiliserRessourcesHtmlInternes", true);
        settingsMetier.endGroup();
    }

    settings.beginGroup("");

    parametresSysteme.cheminStockageBdd = settings.value("baseDeDonnees/chemin", "").toString();
    parametresSysteme.cheminStockageFacturesTraitees = settings.value("dossiers/facturesSaisies", "").toString();
    parametresSysteme.cheminStockageFacturesATraiter = settings.value("dossiers/facturesATraiter", "").toString();
    parametresSysteme.cheminSortieFichiersGeneres = settings.value("dossiers/sortieFichiersGeneres", "").toString();
    parametresSysteme.nomBdd = settings.value("baseDeDonnees/nom", "").toString();
    parametresSysteme.parametresImpression.imprimante = settings.value("impression/imprimante", "Imprimante par défaut").toString();
    parametresSysteme.parametresImpression.modeCouleurImpression = static_cast<QPrinter::ColorMode>(settings.value("impression/couleur", "").toInt());
    parametresSysteme.parametresImpression.resolutionImpression = settings.value("impression/resolution", 600).toInt();
    parametresSysteme.parametresImpression.forcageImpressionRecto = settings.value("impression/forcageImpressionRecto", true).toBool();
    parametresSysteme.margesHautBas = settingsMetier.value("parametresSysteme/margesHautBas", "20").toInt();
    parametresSysteme.margesGaucheDroite = settingsMetier.value("parametresSysteme/margesGaucheDroite", "20").toInt();
    parametresSysteme.utiliserRessourcesHtmlInternes = settingsMetier.value("parametresSysteme/utiliserRessourcesHtmlInternes", true).toBool();
    parametresSysteme.modeFonctionnementLogiciel = static_cast<AeroDmsTypes::ModeFonctionnementLogiciel>(settings.value("modeFonctionnementLogiciel/modeFonctionnement", AeroDmsTypes::ModeFonctionnementLogiciel_INTERNE_UNIQUEMENT).toInt());
    parametresSysteme.adresseServeurModeExterne = settings.value("modeFonctionnementLogiciel/adresse", "").toString();
    parametresSysteme.loginServeurModeExterne = settings.value("modeFonctionnementLogiciel/login", "").toString();
    parametresSysteme.loginSiteDaca = settings.value("siteDaca/login", "").toString();
    parametresSysteme.periodiciteVerificationPresenceFactures = settings.value("siteDaca/periodiciteVerification", 3).toInt();

    parametresMetiers.nomTresorier = settings.value("noms/nomTresorier", "").toString();
    parametresMetiers.delaisDeGardeBdd = settingsMetier.value("parametresSysteme/delaisDeGardeDbEnMs", "50").toInt();

    parametresSysteme.motDePasseSiteDaca = settings.value("siteDaca/password", "").toString();
    parametresSysteme.motDePasseServeurModeExterne = settings.value("modeFonctionnementLogiciel/password", "").toString();
    
    //Permet de fournir un fichier .ini avec les mots de passe en clair pour la conf initiale
    //Si on est pas à 1, cela veut dire que le mot de passe était chiffré => on le déchiffre
    if (settings.value("modeFonctionnementLogiciel/motsDePasseAChiffrer", "0").toBool() == false)
    {
        parametresSysteme.motDePasseSiteDaca = AeroDmsServices::dechiffrerDonnees(parametresSysteme.motDePasseSiteDaca);
        parametresSysteme.motDePasseServeurModeExterne = AeroDmsServices::dechiffrerDonnees(parametresSysteme.motDePasseServeurModeExterne);
        
    }
    //Sinon, on réenregistre le fichier de conf avec le mot de passe chiffré à la fin de la méthode
    
    gestionnaireDonneesEnLigne = new GestionnaireDonneesEnLigne(parametresSysteme);
    connect(gestionnaireDonneesEnLigne, SIGNAL(zipMiseAJourDisponible()), 
        this, SLOT(traiterZipMiseAJourDispo()));
    connect(gestionnaireDonneesEnLigne, SIGNAL(notifierProgressionTelechargement(const qint64, const qint64)),
        this, SLOT(afficherProgressionTelechargementMaJ(const qint64, const qint64)));

    const QString database = settings.value("baseDeDonnees/chemin", "").toString() +
        QString("/") +
        settings.value("baseDeDonnees/nom", "").toString();

    db = new ManageDb(parametresMetiers.delaisDeGardeBdd,
        parametresMetiers.nomTresorier,
        gestionnaireDonneesEnLigne);

    connect(db, SIGNAL(erreurOuvertureBdd()), this, SLOT(fermerSplashscreen()));
    connect(db, SIGNAL(erreurOuvertureBdd()), this, SLOT(gererEchecOuvertureBdd()));

    db->ouvrirLaBdd(database);
    db->lireParametres(parametresMetiers, parametresSysteme);

    pdf = new PdfRenderer(db,
        elaborerCheminRessourcesHtml(),
        QMarginsF(parametresSysteme.margesGaucheDroite,
            parametresSysteme.margesHautBas,
            parametresSysteme.margesGaucheDroite,
            parametresSysteme.margesHautBas));

    enregistrerParametresApplication(parametresMetiers, parametresSysteme);
}

void AeroDms::initialiserOngletPilotes()
{
    //==========Onglet Pilotes
    vuePilotes = new QTableWidget(0, AeroDmsTypes::PiloteTableElement_NB_COLONNES, this);
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_NOM, new QTableWidgetItem(tr("Nom")));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_PRENOM, new QTableWidgetItem(tr("Prénom")));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_ANNEE, new QTableWidgetItem(tr("Année")));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_AEROCLUB, new QTableWidgetItem(tr("Aéroclub")));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_TERRAIN, new QTableWidgetItem(tr("Terrain")));

    QTableWidgetItem* headerHdVEntrainement = new QTableWidgetItem(tr("HdV"));
    headerHdVEntrainement->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_ENTRAINEMENT));
    headerHdVEntrainement->setToolTip("Heures de vol d'entrainement");
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_ENTRAINEMENT_SUBVENTIONNEES, headerHdVEntrainement);
    QTableWidgetItem* headerSubventionEntrainement = new QTableWidgetItem(tr("Subvention"));
    headerSubventionEntrainement->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_ENTRAINEMENT));
    headerSubventionEntrainement->setToolTip(tr("Montant de la subvention d'entrainement déjà allouée"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE, headerSubventionEntrainement);

    QTableWidgetItem* headerHdVBalade = new QTableWidgetItem(tr("HdV"));
    headerHdVBalade->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_BALADE));
    headerHdVBalade->setToolTip((tr("Heures de vol de balade")));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_BALADES_SUBVENTIONNEES, headerHdVBalade);
    QTableWidgetItem* headerSubventionBalade = new QTableWidgetItem(tr("Subvention"));
    headerSubventionBalade->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_BALADE));
    headerSubventionBalade->setToolTip(tr("Montant de la subvention balade déjà allouée"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_BALADES_SUBVENTIONNE, headerSubventionBalade);

    QTableWidgetItem* headerHdVSortie = new QTableWidgetItem((tr("HdV")));
    headerHdVSortie->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SORTIE));
    headerHdVSortie->setToolTip(tr("Heures de vol de sortie"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_SORTIES_SUBVENTIONNEES, headerHdVSortie);
    QTableWidgetItem* headerSubventionSortie = new QTableWidgetItem(tr("Subvention"));
    headerSubventionSortie->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SORTIE));
    headerSubventionSortie->setToolTip(tr("Montant de la subvention sortie déjà allouée"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_SORTIES_SUBVENTIONNE, headerSubventionSortie);

    QTableWidgetItem* headerHdVTotales = new QTableWidgetItem(tr("HdV"));
    headerHdVTotales->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TOTAL));
    headerHdVTotales->setToolTip(tr("Heures de vol totales"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_HEURES_TOTALES_SUBVENTIONNEES, headerHdVTotales);
    QTableWidgetItem* headerSubventionTotale = new QTableWidgetItem(tr("Subvention"));
    headerSubventionTotale->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TOTAL));
    headerSubventionTotale->setToolTip(tr("Montant total de la subvention déjà allouée"));
    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_MONTANT_TOTAL_SUBVENTIONNE, headerSubventionTotale);

    vuePilotes->setHorizontalHeaderItem(AeroDmsTypes::PiloteTableElement_PILOTE_ID, new QTableWidgetItem(tr("Pilote Id (masqué)")));
    vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_PILOTE_ID, true);
    vuePilotes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vuePilotes->setSelectionBehavior(QAbstractItemView::SelectRows);
    vuePilotes->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vuePilotes, AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_PILOTE), tr("Pilotes"));
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
    mainTabWidget->addTab(vueVols, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_AVION), 
        tr("Vols"));
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
    mainTabWidget->addTab(vueFactures, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FACTURE), 
        tr("Factures"));
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
    mainTabWidget->addTab(vueRecettes, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FINANCIER), 
        tr("Recettes"));
}

void AeroDms::initialiserOngletAjoutDepenses()
{
    //==========Onglet Ajout dépense
    QHBoxLayout* ajoutVol = new QHBoxLayout();
    widgetAjoutVol = new QWidget(this);
    widgetAjoutVol->setLayout(ajoutVol);
    mainTabWidget->addTab(widgetAjoutVol, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_DEPENSE), 
        tr("Ajout dépense"));

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
        "remboursement sera émis. En cas de changement d'aéroclub, modifier l'aéroclub\n"
        "du pilote avant de générer les demandes de subvention.\n"
        "Changement réalisable via l'option \"Éditer le pilote sélectionné\" du menu Options."));
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
    depenseTabWidget->addTab(widgetDepenseVol, QIcon(":/AeroDms/ressources/airplane-clock.svg"), "Heures de vol");
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
    depenseTabWidget->addTab(widgetDepenseFacture, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FACTURE), 
        tr("Facture"));
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
    mainTabWidget->addTab(widgetAjoutRecette, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_RECETTE), 
        tr("Ajout recette"));

    listeBaladesEtSorties = new QListWidget(this);
    ajoutRecette->addWidget(listeBaladesEtSorties, 3);

    typeDeRecette = new QComboBox(this);
    typeDeRecette->addItems(db->recupererTypesDesVol(true));
    AeroDmsServices::ajouterIconesComboBox(*typeDeRecette);
    connect(typeDeRecette, &QComboBox::currentIndexChanged, this, &AeroDms::chargerBaladesSorties);
    QLabel* typeDeRecetteLabel = new QLabel(tr("Type de vol : "), this);

    nomEmetteurChequeRecette = new QLineEdit(this);
    nomEmetteurChequeRecette->setToolTip(tr("Nom du titulaire du compte du chèque"));
    QLabel* nomEmetteurChequeRecetteLabel = new QLabel(tr("Nom émetteur paiement : "), this);
    connect(nomEmetteurChequeRecette, &QLineEdit::textChanged, this, &AeroDms::prevaliderDonnneesSaisiesRecette);

    paiementCarteBancaire = new QCheckBox(this);
    paiementCarteBancaire->setToolTip(tr("Cocher cette case si le paiement est réalisé par carte bancaire"));
    QLabel* paiementCarteBancaireLabel = new QLabel(tr("Paiement par carte : "), this);
    connect(paiementCarteBancaire, &QCheckBox::checkStateChanged, this, &AeroDms::changementCochePaiementCb);

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
    infosRecette->addWidget(paiementCarteBancaireLabel, 2, 0);
    infosRecette->addWidget(paiementCarteBancaire, 2, 1);
    infosRecette->addWidget(banqueNumeroChequeRecetteLabel, 3, 0);
    infosRecette->addWidget(banqueNumeroChequeRecette, 3, 1);
    infosRecette->addWidget(montantRecetteLabel, 4, 0);
    infosRecette->addWidget(montantRecette, 4, 1);
    infosRecette->addWidget(validerLaRecette, 5, 0, 2, 0);
}

void AeroDms::verifierPresenceDeMiseAjour()
{
    const QString dossierAVerifier = parametresSysteme.cheminStockageBdd + "/update/";
    //On vérifie dans les répertoire d'update si 2 fichiers ont bougé :
    //  -AeroDms.exe : mise à jour de l'application
    //  -AeroDmsLauncher.exe : mise à jour du lanceur de l'application
    //  -Qt6Core.dll : mise à jour de Qt sans mise à jour de l'application (mise à jour
    // de sécurité/maintenance de Qt avec compatibilité des interfaces)
    //  -podofo.dll : mise à jour de la librairie PDF et des librairies associées
    if ( uneMaJEstDisponible(dossierAVerifier, "AeroDms.exe")
         || uneMaJEstDisponible(dossierAVerifier, "AeroDmsLauncher.exe")
         || uneMaJEstDisponible(dossierAVerifier, "Aero.exe")
         || uneMaJEstDisponible(dossierAVerifier, "Qt6Core.dll")
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
            mettreAJourApplication(dossierAVerifier);
            miseAJourApplicationEstEnCours = true;
        }
        break;

        case QMessageBox::No:
        default:
        {
            if (!db->laBddEstALaVersionAttendue())
            {
                passerLeLogicielEnLectureSeule(true, false, true);

                QMessageBox dialogueErreurVersionBdd;
                dialogueErreurVersionBdd.setText(tr("Une mise à jour de l'application est disponible et doit être réalisée\n\
car la base de données a évoluée.\n\n\
L'application va passer en mode lecture seule.\
\n\nPour mettre à jour l'application, sélectionnez l'option \"Vérifier la présence\n"
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
            QApplication::applicationName() + " - " + tr("Pas de mise à jour disponible"),
            tr("Aucune mise à jour ne semble disponible.\n\nSi vous attendiez une mise à jour, merci de\ncontacter la personne responsable de l'application."));
    }
    else if (!db->laBddEstALaVersionAttendue())
    {
        fermerSplashscreen();
        passerLeLogicielEnLectureSeule(true, false, true);

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

void AeroDms::mettreAJourVersionMiniExigee()
{
    bool ok = false;
    const QString numeroDeVersion = QInputDialog::getText(this, 
        QApplication::applicationName() + " - " + tr("Nouvelle version minimale exigée - Numéro de version"),
        tr("Le numéro de version saisi ici doit correspondre à un tag vX.Y(.Z) dans le dépot https://github.com/cvermot/AeroDMS. \n\n")
            + tr("Nouvelle version X.Y(.Z) minimale d'") + QApplication::applicationName() + tr(" exigée :"),
        QLineEdit::Normal,
        db->recupererVersionLogicielleMinimale().toString(), 
        &ok);

    if (ok)
    {
        const QVersionNumber version = QVersionNumber::fromString(numeroDeVersion);

        if (version.isNull())
        {
            QMessageBox dialogueSaisieVersionInvalide;
            dialogueSaisieVersionInvalide.setText(tr("La saisie ne correspond pas à un numéro de version valide (nommage attendu : X.Y(.Z))."));
            dialogueSaisieVersionInvalide.setWindowTitle(QApplication::applicationName() + " - " + tr("La saisie ne correspond pas à un numéro de version"));
            dialogueSaisieVersionInvalide.setIcon(QMessageBox::Critical);
            dialogueSaisieVersionInvalide.setStandardButtons(QMessageBox::Close);
            dialogueSaisieVersionInvalide.exec();
        }
        else
        {
            const QString url = "https://github.com/cvermot/AeroDMS/releases/tag/v" + version.toString() + "/";

            const QString nomDuFichier = QInputDialog::getText(this,
                QApplication::applicationName() + " - " + tr("Nouvelle version minimale exigée - Nom du fichier"),
                tr("Le nom de fichier saisi ici doit correspondre à un fichier ZIP disponible dans les assets présent<br />à l'adresse <a href=\"")
                + url
                + "\">"
                + url
                + "</a>.<br /><br />"
                + tr("Nouveau nom du fichier ZIP à télécharger :"),
                QLineEdit::Normal,
                db->recupererNomFichierMiseAJour(),
                &ok);
            if (ok)
            {
                if (nomDuFichier.isEmpty())
                {
                    QMessageBox dialogueSaisieNomFichierInvalide;
                    dialogueSaisieNomFichierInvalide.setText(tr("Le nom de fichier saisi est vide. <br/><br/>Les nouvelles exigences en termes de version minimales ne seront pas enregistrées."));
                    dialogueSaisieNomFichierInvalide.setWindowTitle(QApplication::applicationName() + " - " + tr("Nom de fichier invalide"));
                    dialogueSaisieNomFichierInvalide.setIcon(QMessageBox::Critical);
                    dialogueSaisieNomFichierInvalide.setStandardButtons(QMessageBox::Close);
                    dialogueSaisieNomFichierInvalide.exec();
                }
                else
                {
                    db->enregistrerNouvelleVersionLogicielleMinimale(version, nomDuFichier);
                    db->demanderEnvoiBdd();
                }
            }
        }
    } 
}

void AeroDms::traiterZipMiseAJourDispo()
{
    const QString zip = parametresSysteme.cheminStockageBdd + "/update.zip";
    const QString sortie = parametresSysteme.cheminStockageBdd + "/update/";

    auto* worker = new ZipExtractorWorker(zip, sortie, this);

    connect(worker, &ZipExtractorWorker::progress, this, [this](int currentPage, int totalPages) {
        afficherProgressionDecompressionMaJ(currentPage, totalPages);
        });

    connect(worker, &ZipExtractorWorker::finished, this, [this, worker]() {
        statusBar()->showMessage(tr("Téléchargement de la mise à jour terminé"));
        QTimer::singleShot(5000, this, &AeroDms::masquerBarreDeProgressionDeLaStatusBar);
        verifierPresenceDeMiseAjour();
        worker->deleteLater();
        });

    connect(worker, &ZipExtractorWorker::error, this, [this](const QString& message) {
        QMessageBox::critical(nullptr, tr("Erreur de décompression"), message);
        });

    // Lance le thread
    worker->start();    
}

void AeroDms::afficherProgressionTelechargementMaJ(const qint64 p_nbOctetsRecus,
    const qint64 p_nbOctetsTotal)
{
    statusBar()->showMessage(tr("Une mise à jour du logiciel est nécessaire. Téléchargement en cours... (")
    + QString::number(p_nbOctetsRecus / 1024.0 / 1024.0 , 'f', 2)
    + "/"
    + QString::number(p_nbOctetsTotal / 1024.0 / 1024.0, 'f', 2)
    + " Mo)");

    barreDeProgressionStatusBar->setMaximum(p_nbOctetsTotal);
    barreDeProgressionStatusBar->setValue(p_nbOctetsRecus);
    barreDeProgressionStatusBar->show();
}

void AeroDms::afficherProgressionDecompressionMaJ(uint64_t p_nbOctetsDecompresses, uint64_t p_nbOctetsTotaux)
{
    statusBar()->showMessage(tr("Une mise à jour du logiciel est nécessaire. Décompression en cours... (")
        + QString::number(p_nbOctetsDecompresses / 1024.0 / 1024.0, 'f', 2)
        + "/"
        + QString::number(p_nbOctetsTotaux / 1024.0 / 1024.0, 'f', 2)
        + " Mo)");

    barreDeProgressionStatusBar->setMaximum(p_nbOctetsTotaux);
    barreDeProgressionStatusBar->setValue(p_nbOctetsDecompresses);
    barreDeProgressionStatusBar->show();
}

void AeroDms::initialiserGestionnaireTelechargement()
{
    pdfdl = new PdfDownloader(parametresSysteme.cheminStockageFacturesATraiter, db);
    connect(pdfdl, SIGNAL(etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures)), this, SLOT(gererChargementDonneesSitesExternes(AeroDmsTypes::EtatRecuperationDonneesFactures)));
}

void AeroDms::verifierPresenceFacturesDaca()
{
    const QSettings settingsDaca(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationName(), "DACA");
    const QDate derniereVerificationDaca = settingsDaca.value("DACA/dateDerniereVerification", QDate::currentDate().toString("yyyy-MM-dd")).toDate();

    //Si on a dépassé le délais entre 2 vérifications, et que l'application n'est pas en mode mise à jour ou en lecture seule,
    //on vérifie si présence de nouvelle facture sur le site du DACA.
    if (derniereVerificationDaca.daysTo(QDate::currentDate()) >= parametresSysteme.periodiciteVerificationPresenceFactures
        && !miseAJourApplicationEstEnCours
        && !logicielEnModeLectureSeule)
    {
        chargerListeFacturesDaca();
    }
}

void AeroDms::passerLeLogicielEnLectureSeule()
{
    passerLeLogicielEnLectureSeule(true, true);
}

void AeroDms::gererEchecOuvertureBdd()
{
    //passerLeLogicielEnLectureSeule(true, true);
    //statusBar()->showMessage(tr("Impossible de (re)charger la base de données. Relancez le logiciel."));
}

void AeroDms::sortirLeLogicielDeLectureSeule()
{
    //Si etapeFermetureEnCours vaut FERMETURE_BDD, c'est qu'on est sur un envoi de BDD
    //consécutif à une demande de fermeture => on termine la cloture du logiciel
    if (etapeFermetureEnCours == EtapeFermeture_FERMETURE_BDD)
    {
        etapeFermetureEnCours = EtapeFermeture_BDD_FERMEE;
        QThread::msleep(parametresMetiers.delaisDeGardeBdd);
        QCoreApplication::quit();
    }
    else
    {
        passerLeLogicielEnLectureSeule(false, false);
    }

    //Une fois arrivé ici, on est sur que la BDD est dans un état "stable" :
    // -on a vérifié si la BDD en ligne a besoin d'être téléchargée
    // -on l'a téléchargée si besoin
    // -on a mis le verrou
    // -on a envoyé la base lockée en ligne

    //si on est sur le premier envoi de BDD, on vérifie si on a des
    //factures PDF qui n'existe pas en local
    //Et on en profite pour relire les paramètres stockés en BDD
    if (!verificationDeNouvelleFacturesAChargerEnLigneEstEffectue)
    {
        verificationDeNouvelleFacturesAChargerEnLigneEstEffectue = true;

        db->lireParametres(parametresMetiers, parametresSysteme);

        //pour chaque facture de la table "fichiersFacture",
        //on vérifie si la facture existe dans le répertoire local des factures
        //s'il n'existe pas, on demande son téléchargement
        const QStringList listeFactures = db->recupererListeFichiersPdfFactures();
        for (const QString& facture : listeFactures) 
        {
            const QString cheminFichier = parametresSysteme.cheminStockageFacturesTraitees + "/" + facture;
            if (!QFile::exists(cheminFichier)) 
            {
                gestionnaireDonneesEnLigne->telechargerFacture(facture);
            }
        }
    }

    //Si on est sur un cas de chargement BDD, on indique la fin
    if (barreDeProgressionStatusBar->maximum() == AeroDmsTypes::EtapeChargementBdd_TERMINE)
    {
        afficherEtapesChargementBdd(AeroDmsTypes::EtapeChargementBdd_TERMINE);
    }
}

void AeroDms::passerLeLogicielEnLectureSeule(const bool p_lectureSeuleEstDemandee,
    const bool p_figerLesListes,
    const bool p_interdireRetourEnModeLectureEcriture)
{
    if (!retourEnModeLectureEcritureEstInterdit)
    {
        boutonAjouterUnVol->setEnabled(!p_lectureSeuleEstDemandee);
        boutonAjouterCotisation->setEnabled(!p_lectureSeuleEstDemandee);
        boutonAjouterPilote->setEnabled(!p_lectureSeuleEstDemandee);
        boutonAjouterSortie->setEnabled(!p_lectureSeuleEstDemandee);
        boutonAjouterUnAeroclub->setEnabled(!p_lectureSeuleEstDemandee);
        boutonGenerePdf->setEnabled(!p_lectureSeuleEstDemandee);
        facturesDaca->setEnabled(!p_lectureSeuleEstDemandee);

        mettreAJourDonneesVersionMiniAction->setEnabled(!p_lectureSeuleEstDemandee);

        boutonEditerUnAeroclub->setEnabled(!p_lectureSeuleEstDemandee);
        boutonGestionAeronefs->setEnabled(!p_lectureSeuleEstDemandee);
        boutonMettreAJourAerodromes->setEnabled(!p_lectureSeuleEstDemandee);

        listeBaladesEtSorties->setDisabled(p_figerLesListes);
        listeDeroulanteAnnee->setDisabled(p_figerLesListes);
        listeDeroulanteElementsSoumis->setDisabled(p_figerLesListes);
        listeDeroulantePilote->setDisabled(p_figerLesListes);
        listeDeroulanteStatistique->setDisabled(p_figerLesListes);
        listeDeroulanteType->setDisabled(p_figerLesListes);

        logicielEnModeLectureSeule = p_lectureSeuleEstDemandee;
        retourEnModeLectureEcritureEstInterdit = p_interdireRetourEnModeLectureEcriture 
            && p_lectureSeuleEstDemandee;
    }

    //Hors site, on interdit la génération des demandes de subventions (fichiers PDF associés non envoyées en ligne)
    if (parametresSysteme.modeFonctionnementLogiciel == AeroDmsTypes::ModeFonctionnementLogiciel_EXERNE_AUTORISE_MODE_EXTERNE)
    {
        boutonGenerePdf->setEnabled(false);
        boutonGenerePdf->setToolTip(tr("La génération des demandes de subventions n'est pas possible hors site."));
    }
}

void AeroDms::ouvrirSplashscreen()
{
    splash = new QSplashScreen(QPixmap(":/AeroDms/ressources/splash.webp"), Qt::WindowStaysOnTopHint);
    splash->show();
    splash->showMessage(tr("Chargement en cours..."), Qt::AlignCenter | Qt::AlignBottom, Qt::black);
}

void AeroDms::preparerStatusBar()
{
    barreDeProgressionStatusBar = new QProgressBar(this);
    statusBar()->addPermanentWidget(barreDeProgressionStatusBar);
    barreDeProgressionStatusBar->hide();
    statusBar()->showMessage(tr("Prêt"));
}

void AeroDms::demanderFermetureSplashscreen()
{
    if (splash != nullptr)
    {
        QTimer::singleShot(100, this, &AeroDms::fermerSplashscreen);
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

void AeroDms::gererBddDistante()
{
    //Si on est pas en mode fonctionnement interne pur, on réalise des actions supplémentaires
    if (parametresSysteme.modeFonctionnementLogiciel != AeroDmsTypes::ModeFonctionnementLogiciel_INTERNE_UNIQUEMENT)
    {
        statusBar()->showMessage(tr("Vérification de la base de données en ligne en cours... Patientez"));

        gestionnaireDonneesEnLigne->activer();

        connect(db, SIGNAL(signalerChargementBaseSuiteTelechargement()), this, SLOT(peuplerListesEtTables()));
        connect(db, SIGNAL(signalerChargementBaseSuiteTelechargement()), this, SLOT(verifierVersionBddSuiteChargement()));
        connect(db, SIGNAL(signalerDebutTelechargementBdd()), this, SLOT(afficherStatusDebutTelechargementBdd()));
        connect(db, SIGNAL(passerLogicielEnLectureSeuleDurantEnvoiBdd()), this, SLOT(passerLeLogicielEnLectureSeule()));
        connect(db, SIGNAL(sortirDuModeLectureSeule()), this, SLOT(sortirLeLogicielDeLectureSeule()));
        connect(db, SIGNAL(signalerBddBloqueeParUnAutreUtilisateur(const QString, const QDateTime, const QDateTime)), this, SLOT(signalerBaseDeDonneesBloqueeParUnAutreUtilisateur(const QString, const QDateTime, const QDateTime)));

        connect(db, SIGNAL(notifierEtapeChargementBdd(const AeroDmsTypes::EtapeChargementBdd)), this, SLOT(afficherEtapesChargementBdd(const AeroDmsTypes::EtapeChargementBdd)));
        connect(gestionnaireDonneesEnLigne, SIGNAL(notifierEtapeChargementBdd(const AeroDmsTypes::EtapeChargementBdd)), this, SLOT(afficherEtapesChargementBdd(const AeroDmsTypes::EtapeChargementBdd)));

        //On passe systématiquement en lecture seule => si la base est identique a celle dispo en ligne,
        //on recevra le signal signalerChargementBaseSuiteTelechargement() ce qui permettra de repasse
        //le logiciel en lecture-ecriture.
        passerLeLogicielEnLectureSeule(true, false);

        //On affiche la barre de progression
        barreDeProgressionStatusBar->show();
        barreDeProgressionStatusBar->setMaximum(AeroDmsTypes::EtapeChargementBdd_TERMINE);
        barreDeProgressionEstAMettreAJourPourBddInitialeOuFinale = true;

        //On fait la demande au gestionnaire de données en ligne.
        //La réponse sera traitée directement par le gestionnaire de BDD  
        barreDeProgressionStatusBar->setValue(AeroDmsTypes::EtapeChargementBdd_DEMANDE_SHA256);
        gestionnaireDonneesEnLigne->recupererSha256Bdd();
    }
    //Sinon, on vérifie tout de suite la présence de factures du DACA
    else
    {
        verifierPresenceFacturesDaca();
    }
}


void AeroDms::verifierSignatureNumerisee()
{
    if (AeroDmsServices::recupererCheminFichierImageSignature() == "")
    {
        boutonSignatureManuelle->setEnabled(false);
    }
    else
    {
        QString cheminOut = QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + "/signature.rcc";

        RCCResourceLibrary library;
        library.readFiles(AeroDmsServices::recupererCheminFichierImageSignature());
        library.output(cheminOut);

        if (QResource::registerResource(cheminOut))
        {
            boutonSignatureManuelle->activate(QAction::Trigger);
        }
        else
        {
            boutonSignatureManuelle->setEnabled(false);
            QMessageBox::critical(this,
                QApplication::applicationName() + " - " + tr("Fichier de signature"),
                tr("Le fichier ressource RCC de signature n'a pas pu être chargé.\nLa signature numérisée n'est pas disponible."));
        }
    }
}

const QString AeroDms::elaborerCheminRessourcesHtml()
{
    return parametresSysteme.utiliserRessourcesHtmlInternes ?
        ":/AeroDms/ressources/HTML/" :
        parametresSysteme.cheminStockageBdd + QString("/ressources/HTML/");
}

void AeroDms::initialiserOngletGraphiques()
{
    //=============Onglet graphiques
    graphiques = new QHBoxLayout();
    widgetGraphiques = new QWidget(this);
    widgetGraphiques->setLayout(graphiques);
    mainTabWidget->addTab(widgetGraphiques, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS), 
        tr("Graphiques"));
}

void AeroDms::initialiserOngletSubventionsDemandees()
{
    //=============Onglet Subventions demandées
    vueSubventions = new QTableWidget(0, AeroDmsTypes::SubventionDemandeeTableElementTableElement_NB_COLONNES, this);
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_DATE, new QTableWidgetItem(tr("Date")));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_PILOTE, new QTableWidgetItem(tr("Pilote")));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_BENEFICIAIRE, new QTableWidgetItem(tr("Nom bénéficiaire")));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_TYPE_DEMANDE, new QTableWidgetItem(tr("Type de demande")));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_MONTANT, new QTableWidgetItem(tr("Montant")));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_MONTANT_VOL, new QTableWidgetItem(tr("Montant vol")));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_MODE_DE_REGLEMENT, new QTableWidgetItem(tr("Mode de réglement")));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_NOTE, new QTableWidgetItem(tr("Note")));
    vueSubventions->setHorizontalHeaderItem(AeroDmsTypes::SubventionDemandeeTableElement_ID_DEMANDE, new QTableWidgetItem(tr("ID demande")));
    vueSubventions->setColumnHidden(AeroDmsTypes::SubventionDemandeeTableElement_ID_DEMANDE, true);
    vueSubventions->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vueSubventions->setSelectionBehavior(QAbstractItemView::SelectRows);
    vueSubventions->setContextMenuPolicy(Qt::CustomContextMenu);
    mainTabWidget->addTab(vueSubventions, QIcon(":/AeroDms/ressources/checkbook.svg"), tr("Subventions demandées"));

    connect(vueSubventions, &QTableWidget::customContextMenuRequested, this, &AeroDms::menuContextuelSubvention);
}

void AeroDms::initialiserBarreDOutils()
{
    QToolBar* toolBar = addToolBar(tr("Outils"));

    boutonAjouterUnVol = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_AJOUTER_VOL),
        tr("Ajouter un &vol/une dépense"), 
        this);
    boutonAjouterUnVol->setStatusTip(tr("Ajouter un vol/une dépense"));
    boutonAjouterUnVol->setShortcut(Qt::Key_F2);
    connect(boutonAjouterUnVol, &QAction::triggered, this, &AeroDms::selectionnerUneFacture);
    toolBar->addAction(boutonAjouterUnVol);

    boutonAjouterPilote = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_PILOTE), 
        tr("Ajouter un &pilote"), 
        this);
    boutonAjouterPilote->setStatusTip(tr("Ajout d'un pilote"));
    boutonAjouterPilote->setShortcut(Qt::Key_F3);
    connect(boutonAjouterPilote, &QAction::triggered, this, &AeroDms::ajouterUnPilote);
    toolBar->addAction(boutonAjouterPilote);

    boutonAjouterUnAeroclub = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_AJOUTER_AEROCLUB),
        tr("Ajouter un &aéroclub"),
        this);
    boutonAjouterUnAeroclub->setStatusTip(tr("Ajouter un aéroclub"));
    boutonAjouterUnAeroclub->setShortcut(Qt::Key_F6);
    connect(boutonAjouterUnAeroclub, &QAction::triggered, this, &AeroDms::ajouterUnAeroclub);
    toolBar->addAction(boutonAjouterUnAeroclub);

    boutonAjouterCotisation = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_COTISATION), 
        tr("Ajouter une &cotisation pour un pilote"), 
        this);
    boutonAjouterCotisation->setStatusTip(tr("Ajouter une cotisation pour un pilote"));
    boutonAjouterCotisation->setShortcut(Qt::Key_F4);
    connect(boutonAjouterCotisation, &QAction::triggered, this, &AeroDms::ajouterUneCotisation);
    toolBar->addAction(boutonAjouterCotisation);

    boutonAjouterSortie = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SORTIE), 
        tr("Ajouter une &sortie"), 
        this);
    boutonAjouterSortie->setStatusTip(tr("Ajouter une sortie"));
    boutonAjouterSortie->setShortcut(Qt::Key_F5);
    connect(boutonAjouterSortie, &QAction::triggered, this, &AeroDms::ajouterUneSortie);
    toolBar->addAction(boutonAjouterSortie);

    fichierPrecedent = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FICHIER_PRECEDENT),
        tr("Télécharger la facture précédente sur le site du DACA"),
        this);
    fichierPrecedent->setStatusTip(tr("Charger la facture précédente sur le site du DACA"));
    //fichierPrecedent->setShortcut(Qt::Key_F5);
    connect(fichierPrecedent, &QAction::triggered, this, &AeroDms::demanderTelechagementFactureSuivanteOuPrecedente);
    toolBar->addAction(fichierPrecedent);

    fichierSuivant = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FICHIER_SUIVANT),
        tr("Télécharger la facture suivante sur le site du DACA"),
        this);
    fichierSuivant->setStatusTip(tr("Charger la facture suivante sur le site du DACA"));
    //fichierSuivant->setShortcut(Qt::Key_F5);
    connect(fichierSuivant, &QAction::triggered, this, &AeroDms::demanderTelechagementFactureSuivanteOuPrecedente);
    toolBar->addAction(fichierSuivant);

    fichierPrecedent->setVisible(false);
    fichierSuivant->setVisible(false);
    
    toolBar->addSeparator();

    boutonGenerePdf = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_GENERE_DEMANDE_SUBVENTIONS),
        tr("&Générer les PDF de demande de subvention"), 
        this);
    boutonGenerePdf->setStatusTip(tr("Génère les fichiers de demandes : remplissage des formulaires et génération de PDF associant les factures"));
    boutonGenerePdf->setShortcut(Qt::CTRL + Qt::Key_G);
    connect(boutonGenerePdf, &QAction::triggered, this, &AeroDms::genererPdf);
    toolBar->addAction(boutonGenerePdf);

    boutonGenerePdfRecapHdv = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_GENERE_RECAP_HDV), 
        tr("Générer les PDF de récapitulatif &HdV de l'année sélectionnée"), 
        this);
    boutonGenerePdfRecapHdv->setStatusTip(tr("Générer les PDF de récapitulatif des HdV de l'année sélectionnée"));
    boutonGenerePdfRecapHdv->setShortcut(Qt::CTRL + Qt::Key_R);
    connect(boutonGenerePdfRecapHdv, &QAction::triggered, this, &AeroDms::genererPdfRecapHdV);
    toolBar->addAction(boutonGenerePdfRecapHdv);
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
    connect(listeDeroulanteAnnee, &QComboBox::currentIndexChanged, this, &AeroDms::traiterClicSurVolBaladesEtSorties);
    actionListeDeroulanteAnnee = selectionToolBar->addWidget(listeDeroulanteAnnee);

    listeDeroulantePilote = new QComboBox(this);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTablePilotes);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulantePilote, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableSubventionsDemandees);
    actionListeDeroulantePilote = selectionToolBar->addWidget(listeDeroulantePilote);

    listeDeroulanteType = new QComboBox(this);
    listeDeroulanteType->addItems(db->recupererTypesDesVol());
    AeroDmsServices::ajouterIconesComboBox(*listeDeroulanteType);
    listeDeroulanteType->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_COTISATION), tr("Cotisation"), "Cotisation");
    listeDeroulanteType->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TOUS), tr("Tous les types de vol"), "*");
    listeDeroulanteType->setCurrentIndex(4);

    connect(listeDeroulanteType, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulanteType, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableRecettes);
    connect(listeDeroulanteType, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableSubventionsDemandees);
    actionListeDeroulanteType = selectionToolBar->addWidget(listeDeroulanteType);

    listeDeroulanteElementsSoumis = new QComboBox(this);
    listeDeroulanteElementsSoumis->addItem(tr("Éléments soumis et non soumis au CSE"), 
        AeroDmsTypes::ElementSoumis_TOUS_LES_ELEMENTS);
    listeDeroulanteElementsSoumis->setItemIcon(AeroDmsTypes::ElementSoumis_TOUS_LES_ELEMENTS, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TOUS));
    listeDeroulanteElementsSoumis->addItem(tr("Éléments soumis au CSE"),
        AeroDmsTypes::ElementSoumis_ELEMENTS_SOUMIS);
    listeDeroulanteElementsSoumis->setItemIcon(AeroDmsTypes::ElementSoumis_ELEMENTS_SOUMIS, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_OUI));
    listeDeroulanteElementsSoumis->addItem(tr("Éléments non soumis au CSE"),
        AeroDmsTypes::ElementSoumis_ELEMENTS_NON_SOUMIS);
    listeDeroulanteElementsSoumis->setItemIcon(AeroDmsTypes::ElementSoumis_ELEMENTS_NON_SOUMIS, 
        AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_NON));

    connect(listeDeroulanteElementsSoumis, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableVols);
    connect(listeDeroulanteElementsSoumis, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableRecettes);
    connect(listeDeroulanteElementsSoumis, &QComboBox::currentIndexChanged, this, &AeroDms::peuplerTableFactures);
    actionListeDeroulanteElementsSoumis = selectionToolBar->addWidget(listeDeroulanteElementsSoumis);

    listeDeroulanteStatistique = new QComboBox(this);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_BARRES_EMPILEES),
        tr("Statistiques mensuelles"),
        AeroDmsTypes::Statistiques_HEURES_ANNUELLES);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Heures par pilote"),
        AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Subventions par pilote"),
        AeroDmsTypes::Statistiques_EUROS_PAR_PILOTE);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Heures par type de vol"),
        AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Subventions par type de vol"),
        AeroDmsTypes::Statistiques_EUROS_PAR_TYPE_DE_VOL);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Heures par activité"),
        AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Subventions par activité"),
        AeroDmsTypes::Statistiques_EUROS_PAR_ACTIVITE);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_DONUT),
        tr("Statuts des pilotes"),
        AeroDmsTypes::Statistiques_STATUTS_PILOTES);
    listeDeroulanteStatistique->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_DONUT),
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
    connect(dialogueGestionPilote, SIGNAL(demandeAjoutAeroclub()), this, SLOT(ajouterUnAeroclub()));

    dialogueGestionAeroclub = new DialogueGestionAeroclub(db, this);
    connect(dialogueGestionAeroclub, SIGNAL(accepted()), this, SLOT(ajouterUnAeroclubEnBdd()));

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

    QAction* boutonOuvrirDossierDemandes = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_OUVRIR_DOSSIER), 
        tr("&Ouvrir le dossier contenant les demandes de subventions"), 
        this);
    menuFichier->addAction(boutonOuvrirDossierDemandes);
    boutonOuvrirDossierDemandes->setShortcut(QKeySequence::Open);
    boutonOuvrirDossierDemandes->setStatusTip(tr("Ouvrir le dossier où sont génerées les PDF de demande de subventions"));
    connect(boutonOuvrirDossierDemandes, SIGNAL(triggered()), this, SLOT(ouvrirDossierDemandesSubventions()));

    QMenu* menuOuvrirPdfDemandeSubvention = menuFichier->addMenu(tr("Ouvrir un fichier de &demande de subventions"));
    menuOuvrirPdfDemandeSubvention->setIcon(QIcon(":/AeroDms/ressources/file-pdf-box.svg"));
    menuOuvrirPdfDemandeSubvention->setStatusTip(tr("Ouvrir directement un PDF ou un dossier d'une demande particulière"));

    QAction* boutonOuvrirDerniereDemande = new QAction(QIcon(":/AeroDms/ressources/file-outline.svg"), tr("Ouvrir la &dernière demande"), this);
    menuOuvrirPdfDemandeSubvention->addAction(boutonOuvrirDerniereDemande);
    connect(boutonOuvrirDerniereDemande, SIGNAL(triggered()), this, SLOT(ouvrirPdfDemandeSuvbvention()));

    menuOuvrirAutreDemande = menuOuvrirPdfDemandeSubvention->addMenu(tr("Ouvrir un &autre fichier de demande de subventions"));
    menuOuvrirAutreDemande->setIcon(QIcon(":/AeroDms/ressources/file-pdf-box.svg"));

    peuplerMenuAutreDemande();

    QAction* boutonImprimer = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_IMPRIMER),
        tr("&Imprimer la dernière demande"), 
        this);
    boutonImprimer->setStatusTip(tr("Imprime la dernière demande de subvention en une seule fois (pas d'agrafage séparé)"));
    menuFichier->addAction(boutonImprimer);
    connect(boutonImprimer, SIGNAL(triggered()), this, SLOT(imprimerLaDerniereDemande()));

    QAction* boutonImprimerAgrafage = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_IMPRIMER),
        tr("Imprimer la dernière demande (avec &agrafage)"), 
        this);
    boutonImprimerAgrafage->setStatusTip(tr("Permet d'imprimer chaque fichier séparément. Pour l'agrafage automatique par l'imprimante, pensez à\nsélectionner l'option adéquate dans les paramètres de l'imprimante."));
    boutonImprimerAgrafage->setShortcut(QKeySequence::Print);
    menuFichier->addAction(boutonImprimerAgrafage);
    connect(boutonImprimerAgrafage, SIGNAL(triggered()), this, SLOT(imprimerLaDerniereDemandeAgrafage()));
}

void AeroDms::initialiserMenuOptions()
{
    //========================Menu Options
    menuOption = menuBar()->addMenu(tr("&Options"));

    QMenu* menuSignature = menuOption->addMenu(tr("&Signature"));
    menuSignature->setIcon(QIcon(":/AeroDms/ressources/file-sign.svg"));

    boutonAucuneSignature = new QAction(QIcon(":/AeroDms/ressources/file-outline.svg"), tr("Signature &manuelle"), this);
    boutonAucuneSignature->setStatusTip(tr("Ne pas remplir le champ signature. Chaque document sera signé au stylo par le trésorier."));
    boutonAucuneSignature->setCheckable(true);
    menuSignature->addAction(boutonAucuneSignature);

    boutonSignatureManuelle = new QAction(QIcon(":/AeroDms/ressources/draw-pen.svg"), tr("Utiliser l'&image d'une signature"), this);
    boutonSignatureManuelle->setStatusTip(tr("Utiliser une image de signature pour remplir le champ signature."));
    boutonSignatureManuelle->setCheckable(true);
    menuSignature->addAction(boutonSignatureManuelle);

    boutonSignatureNumerique = new QAction(QIcon(":/AeroDms/ressources/lock-check-outline.svg"), tr("Signature &numérique avec Lex Community"), this);
    boutonSignatureNumerique->setStatusTip(tr("Préparer le document pour une signature numérique via Lex Community."));
    boutonSignatureNumerique->setCheckable(true);
    menuSignature->addAction(boutonSignatureNumerique);

    connect(boutonAucuneSignature, SIGNAL(triggered()), this, SLOT(changerModeSignature()));
    connect(boutonSignatureNumerique, SIGNAL(triggered()), this, SLOT(changerModeSignature()));
    connect(boutonSignatureManuelle, SIGNAL(triggered()), this, SLOT(changerModeSignature()));

    QMenu* menuFusionnerLesPdf = menuOption->addMenu(tr("&Fusion des PDF"));
    menuFusionnerLesPdf->setIcon(QIcon(":/AeroDms/ressources/paperclip.svg"));
    boutonFusionnerLesPdf = new QAction(QIcon(":/AeroDms/ressources/paperclip-check.svg"), tr("&Fusionner les PDF"), this);
    boutonFusionnerLesPdf->setStatusTip(tr("Fusionne tous les PDF générés en un seul fichier PDF (les PDF \"unitaires\" restent disponibles dans le dossier de génération)"));
    boutonFusionnerLesPdf->setCheckable(true);
    menuFusionnerLesPdf->addAction(boutonFusionnerLesPdf);
    boutonNePasFusionnerLesPdf = new QAction(QIcon(":/AeroDms/ressources/paperclip-off.svg"), tr("&Ne pas fusionner les PDF"), this);
    boutonNePasFusionnerLesPdf->setStatusTip(tr("Ne pas générer le PDF fusionné"));
    boutonNePasFusionnerLesPdf->setCheckable(true);
    menuFusionnerLesPdf->addAction(boutonNePasFusionnerLesPdf);

    connect(boutonFusionnerLesPdf, SIGNAL(triggered()), this, SLOT(changerFusionPdf()));
    connect(boutonNePasFusionnerLesPdf, SIGNAL(triggered()), this, SLOT(changerFusionPdf()));

    QMenu* menuDemandesAGenerer = menuOption->addMenu(tr("&Demandes à générer"));
    menuDemandesAGenerer->setIcon(QIcon(":/AeroDms/ressources/file-cog.svg"));
    boutonDemandesAGenererToutes = new QAction(QIcon(":/AeroDms/ressources/file-document-multiple.svg"), tr("&Toutes"), this);
    boutonDemandesAGenererToutes->setStatusTip(tr("Générer tout (recette et dépenses)"));
    boutonDemandesAGenererToutes->setCheckable(true);
    menuDemandesAGenerer->addAction(boutonDemandesAGenererToutes);
    boutonDemandesAGenererRecettes = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_RECETTE), 
        tr("&Recettes uniquement"), 
        this);
    boutonDemandesAGenererRecettes->setStatusTip(tr("Générer uniquement les documents de remises de recettes"));
    boutonDemandesAGenererRecettes->setCheckable(true);
    menuDemandesAGenerer->addAction(boutonDemandesAGenererRecettes);
    boutonDemandesAGenererRecettes->setCheckable(true);
    boutonDemandesAGenererDepenses = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_DEPENSE), 
        tr("&Dépenses uniquement"), 
        this);
    boutonDemandesAGenererDepenses->setStatusTip(tr("Générer uniquement les documents de demande de subventions/remboursements"));
    menuDemandesAGenerer->addAction(boutonDemandesAGenererDepenses);
	boutonDemandesAGenererDepenses->setCheckable(true);
	menuDemandesAGenererAnnees = menuDemandesAGenerer->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_CHOIX_ANNEE),
        tr("&Année"));

    menuOptionsRecapAnnuel = menuOption->addMenu(QIcon(":/AeroDms/ressources/account-file-text.svg"), tr("&Options du récapitulatif annuel"));
    boutonOptionRecapAnnuelRecettes = new QAction(QIcon(":/AeroDms/ressources/table-plus.svg"), 
        tr("Récapitulatif des &recettes"), 
        this);
    menuOptionsRecapAnnuel->addAction(boutonOptionRecapAnnuelRecettes);
    boutonOptionRecapAnnuelRecettes->setCheckable(true);
    boutonOptionRecapAnnuelRecettes->setStatusTip(tr("Permet d'ajouter le récapitulatif des recettes dans le récap des heures de vol"));
    boutonOptionRecapAnnuelBaladesSorties = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_VOL),
        tr("Récapitulatif des &balades et sorties"), 
        this);
    menuOptionsRecapAnnuel->addAction(boutonOptionRecapAnnuelBaladesSorties);
    boutonOptionRecapAnnuelBaladesSorties->setCheckable(true);
    boutonOptionRecapAnnuelBaladesSorties->setStatusTip(tr("Permet d'ajouter le récapitulatif des balades et sorties (dates, durées, noms des passagers, couts et recettes...) dans le récap des heures de vol"));
    //Génération des graphiques
    graphiquesDuRecapAnnuel = menuOptionsRecapAnnuel->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS), 
        tr("&Graphiques"));
    graphiquesDuRecapAnnuel->setStatusTip(tr("Sélectionne les graphiques à ajouter au récapitulatif annuel des heures de vol"));

    boutonGraphRecapAnnuelHeuresAnnuelles = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_BARRES_EMPILEES), 
        tr("&Heures annuelles"), 
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelHeuresAnnuelles);
    boutonGraphRecapAnnuelHeuresAnnuelles->setCheckable(true);

    boutonGraphRecapAnnuelHeuresParPilote = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT), 
        tr("Heures par &pilote"), 
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelHeuresParPilote);
    boutonGraphRecapAnnuelHeuresParPilote->setCheckable(true);

    boutonGraphRecapAnnuelEurosParPilote = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Subventions par pilote"),
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelEurosParPilote);
    boutonGraphRecapAnnuelEurosParPilote->setCheckable(true);

    boutonGraphRecapAnnuelHeuresParTypeDeVol = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT), 
        tr("Heures par type de &vol"), 
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelHeuresParTypeDeVol);
    boutonGraphRecapAnnuelHeuresParTypeDeVol->setCheckable(true);

    boutonGraphRecapAnnuelEurosParTypeDeVol = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Subventions par type de vol"),
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelEurosParTypeDeVol);
    boutonGraphRecapAnnuelEurosParTypeDeVol->setCheckable(true);

    boutonGraphRecapAnnuelHeuresParActivite = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT), 
        tr("Heures par &activité"), 
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelHeuresParActivite);
    boutonGraphRecapAnnuelHeuresParActivite->setCheckable(true);

    boutonGraphRecapAnnuelEurosParActivite = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_CAMEMBERT),
        tr("Subventions par activité"),
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelEurosParActivite);
    boutonGraphRecapAnnuelEurosParActivite->setCheckable(true);

    boutonGraphRecapAnnuelStatutsDesPilotes = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_DONUT), 
        tr("&Statuts des pilotes"), 
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelStatutsDesPilotes);
    boutonGraphRecapAnnuelStatutsDesPilotes->setCheckable(true);

    boutonGraphRecapAnnuelAeronefs = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS_DONUT), 
        tr("Aérone&fs"), 
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelAeronefs);
    boutonGraphRecapAnnuelAeronefs->setCheckable(true);

    connect(boutonGraphRecapAnnuelHeuresAnnuelles, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelHeuresParPilote, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelEurosParPilote, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelEurosParPilote, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelHeuresParTypeDeVol, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelEurosParTypeDeVol, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelHeuresParActivite, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelEurosParActivite, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelStatutsDesPilotes, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
    connect(boutonGraphRecapAnnuelAeronefs, SIGNAL(triggered()), this, SLOT(maintenirMenuSelectionGraphsOuvert()));
        
    //Résolutions
    resolutionGraphiques = graphiquesDuRecapAnnuel->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS), 
        tr("&Résolution des graphiques"));

    boutonGraphResolutionFullHd = new QAction(QIcon(":/AeroDms/ressources/standard-definition.svg"), 
        tr("&Full HD (1920 × 1080)"), 
        this);
    resolutionGraphiques->addAction(boutonGraphResolutionFullHd);
    boutonGraphResolutionFullHd->setCheckable(true);

    boutonGraphResolutionQhd = new QAction(QIcon(":/AeroDms/ressources/high-definition.svg"), 
        tr("&QHD (2560 × 1440)"), 
        this);
    resolutionGraphiques->addAction(boutonGraphResolutionQhd);
    boutonGraphResolutionQhd->setCheckable(true);

    boutonGraphResolution4k = new QAction(QIcon(":/AeroDms/ressources/ultra-high-definition.svg"), 
        tr("&UHD (3840 × 2160)"),
        this);
    resolutionGraphiques->addAction(boutonGraphResolution4k);
    boutonGraphResolution4k->setCheckable(true);

    connect(boutonGraphResolutionFullHd, SIGNAL(triggered()), this, SLOT(changerResolutionExportGraphiques()));
    connect(boutonGraphResolutionQhd, SIGNAL(triggered()), this, SLOT(changerResolutionExportGraphiques()));
    connect(boutonGraphResolution4k, SIGNAL(triggered()), this, SLOT(changerResolutionExportGraphiques()));

    resolutionGraphiques->addSeparator();

    boutonGraphRatioIso216 = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_RATIO),
        tr("Ratio ISO 216"),
        this);
    boutonGraphRatioIso216->setStatusTip(tr("Ratio d'une page A4"));
    resolutionGraphiques->addAction(boutonGraphRatioIso216);
    boutonGraphRatioIso216->setCheckable(true);

    boutonGraphRatio16x9 = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_RATIO),
        tr("Ratio 16/9"),
        this);
    boutonGraphRatio16x9->setStatusTip(tr("Ratio d'un écran"));
    resolutionGraphiques->addAction(boutonGraphRatio16x9);
    boutonGraphRatio16x9->setCheckable(true);

    connect(boutonGraphRatioIso216, SIGNAL(triggered()), this, SLOT(changerRatioExportGraphiques()));
    connect(boutonGraphRatio16x9, SIGNAL(triggered()), this, SLOT(changerRatioExportGraphiques()));

    boutonGraphResolution4k->activate(QAction::Trigger);
    boutonGraphRatioIso216->activate(QAction::Trigger);

    graphiquesDuRecapAnnuel->addSeparator();
    boutonGraphRecapAnnuelSelectionnerTousLesGraphs = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TOUT_COCHER),
        tr("&Sélectionner tous les graphs"), 
        this);
    graphiquesDuRecapAnnuel->addAction(boutonGraphRecapAnnuelSelectionnerTousLesGraphs);
    boutonGraphRecapAnnuelSelectionnerTousLesGraphs->setStatusTip(tr("Ajoute tous les graphiques au récap des heures de vol"));
    connect(boutonGraphRecapAnnuelSelectionnerTousLesGraphs, SIGNAL(triggered()), this, SLOT(selectionnerTousLesGraphsPourRecapAnnuel()));

    menuOptionsRecapAnnuel->addSeparator();
    boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TOUT_COCHER),
        tr("&Sélectionner tous les graphs et tous les récaps"), 
        this);
    menuOptionsRecapAnnuel->addAction(boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap);
    boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap->setStatusTip(tr("Ajoute tous les graphiques, le tableau des recettes et le tableau des balades au récap des heures de vol"));
    connect(boutonGraphRecapAnnuelSelectionnerTousLesGraphsEtTousLesRecap, SIGNAL(triggered()), this, SLOT(selectionnerTousLesGraphsPourRecapAnnuel()));

    connect(boutonDemandesAGenererToutes, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));
    connect(boutonDemandesAGenererRecettes, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));
    connect(boutonDemandesAGenererDepenses, SIGNAL(triggered()), this, SLOT(changerDemandesAGenerer()));

    QMenu *menuOptionsGraphiques = menuOption->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_STATS), 
        tr("Options des &graphiques"));
    boutonGraphiquesVolsSubventionnesUniquement = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FINANCIER),
        tr("Vols avec subvention uniquement"),
        this);
    boutonGraphiquesVolsSubventionnesUniquement->setStatusTip(tr("Ne prendre en compte que les vols ayant effectivement bénéficié d'une subvention pour l'élaboration des graphiques"));
    menuOptionsGraphiques->addAction(boutonGraphiquesVolsSubventionnesUniquement);
    boutonGraphiquesVolsSubventionnesUniquement->setCheckable(true);
    boutonGraphiquesVolsSubventionnesUniquement->setChecked(true);
    connect(boutonGraphiquesVolsSubventionnesUniquement, &QAction::toggled, this, &AeroDms::peuplerStatistiques);

    boutonGraphiquesExclureAvion = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_AVION),
        tr("Exclure vols avions"),
        this);
    boutonGraphiquesExclureAvion->setStatusTip(tr("Ne pas prendre en compte les vols avion dans les graphiques"));
    menuOptionsGraphiques->addAction(boutonGraphiquesExclureAvion);
    boutonGraphiquesExclureAvion->setCheckable(true);
    connect(boutonGraphiquesExclureAvion, &QAction::toggled, this, &AeroDms::peuplerStatistiques);

    boutonGraphiquesExclureHelico = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_HELICOPTERE),
        tr("Exclure vols hélicoptères"),
        this);
    boutonGraphiquesExclureHelico->setStatusTip(tr("Ne pas prendre en compte les vols hélicoptère dans les graphiques"));
    menuOptionsGraphiques->addAction(boutonGraphiquesExclureHelico);
    boutonGraphiquesExclureHelico->setCheckable(true);
    connect(boutonGraphiquesExclureHelico, &QAction::toggled, this, &AeroDms::peuplerStatistiques);

    boutonGraphiquesExclurePlaneur = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_PLANEUR),
        tr("Exclure vols planeurs"),
        this);
    boutonGraphiquesExclurePlaneur->setStatusTip(tr("Ne pas prendre en compte les vols planeurs dans les graphiques"));
    menuOptionsGraphiques->addAction(boutonGraphiquesExclurePlaneur);
    boutonGraphiquesExclurePlaneur->setCheckable(true);
    connect(boutonGraphiquesExclurePlaneur, &QAction::toggled, this, &AeroDms::peuplerStatistiques);

    boutonGraphiquesExclureUlm = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_ULM),
        tr("Exclure vols ULM"),
        this);
    boutonGraphiquesExclureUlm->setStatusTip(tr("Ne pas prendre en compte les vols ULM dans les graphiques"));
    menuOptionsGraphiques->addAction(boutonGraphiquesExclureUlm);
    boutonGraphiquesExclureUlm->setCheckable(true);
    connect(boutonGraphiquesExclureUlm, &QAction::toggled, this, &AeroDms::peuplerStatistiques);

    menuOption->addSeparator();

    boutonActivationScanAutoFactures = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_FACTURE), 
        tr("Désactiver le scan &automatique des factures"), 
        this);
    boutonActivationScanAutoFactures->setStatusTip(tr("Active/désactive le scan à l'ouverture des factures PDF. Cette option permet de désactiver le scan automatique si une facture fait planter le logiciel par exemple."));
    menuOption->addAction(boutonActivationScanAutoFactures);
    connect(boutonActivationScanAutoFactures, SIGNAL(triggered()), this, SLOT(switchScanAutomatiqueDesFactures()));

    selecteurFiltreInformationsSupplementaires = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_INFOS_COMPLEMENTAIRES),
        tr("Afficher/masquer les informations supplémentaires"),
        this);
    selecteurFiltreInformationsSupplementaires->setCheckable(true);
    selecteurFiltreInformationsSupplementaires->setChecked(false);
    menuOption->addAction(selecteurFiltreInformationsSupplementaires);

    connect(selecteurFiltreInformationsSupplementaires, &QAction::triggered, this, &AeroDms::switchAffichageInfosComplementaires);

    menuOption->addSeparator();

    boutonParametresDuLogiciel = new QAction(QIcon(":/AeroDms/ressources/cog.svg"), tr("&Paramètres du logiciel"), this);
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

    menuOutils->addAction(boutonAjouterUnVol);
    menuOutils->addAction(boutonAjouterPilote);
    menuOutils->addAction(boutonAjouterCotisation);
    menuOutils->addAction(boutonAjouterSortie);
    menuOutils->addAction(boutonAjouterUnAeroclub);
    menuOutils->addSeparator();
    menuOutils->addAction(boutonGenerePdf);
    menuOutils->addAction(boutonGenerePdfRecapHdv);

    menuOutils->addSeparator();

    QMenu* scanFacture = menuOutils->addMenu(tr("Scan automatique des &factures"));
    scanFacture->setToolTipsVisible(true);
    scanFacture->setStatusTip(tr("Scan une facture en se basant sur une des méthode utilisée par le logiciel pour un type de facture déjà connu."));
    scanFacture->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_FACTURE));
    scanAutoOpenFlyer = new QAction(tr("&OpenFlyer (CAPAM, ACB)"), this);
    scanAutoOpenFlyer->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_VOL));
    scanFacture->addAction(scanAutoOpenFlyer);
    scanAutoAerogest = new QAction(tr("&Aerogest (ACBA)"), this);
    scanAutoAerogest->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_VOL));
    scanFacture->addAction(scanAutoAerogest);
    scanAutoAca = new QAction(tr("Aéroclub d'A&ndernos"), this);
    scanAutoAca->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_VOL));
    scanFacture->addAction(scanAutoAca);
    scanAutoDaca = new QAction(tr("&DACA"), this);
    scanAutoDaca->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_VOL));
    scanFacture->addAction(scanAutoDaca);
    scanAutoSepavia = new QAction(tr("&SEPAVIA"), this);
    scanAutoSepavia->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_VOL));
    scanFacture->addAction(scanAutoSepavia);
    scanAutoUaca = new QAction(tr("&UACA"), this);
    scanAutoUaca->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_VOL));
    scanFacture->addAction(scanAutoUaca);
    scanAutoAtvv = new QAction(tr("AT&VV"), this);
    scanAutoAtvv->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_VOL));
    scanFacture->addAction(scanAutoAtvv);
    scanFacture->addSeparator();
    scanAutoGenerique1Passe = new QAction(tr("&Générique (une passe)"), this);
    scanAutoGenerique1Passe->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_FACTURE_GENERIQUE));
    scanFacture->addAction(scanAutoGenerique1Passe);
    scanAutoGenerique = new QAction(tr("Générique (&multi-passe)"), this);
    scanAutoGenerique->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_SCAN_AUTO_FACTURE_GENERIQUE));
    scanFacture->addAction(scanAutoGenerique);
    scanFacture->addSeparator();
    scanAutoCsv = new QAction(tr("&Importer les vols depuis un fichier CSV"), this);
    scanAutoCsv->setIcon(QIcon(":/AeroDms/ressources/file-delimited-outline.svg"));
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
    connect(scanAutoAtvv, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoGenerique1Passe, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoGenerique, SIGNAL(triggered()), this, SLOT(scannerUneFactureSelonMethodeChoisie()));
    connect(scanAutoCsv, SIGNAL(triggered()), this, SLOT(recupererVolDepuisCsv()));

    facturesDaca = menuOutils->addMenu(texteTitreQMenuFacturesDaca);
    facturesDaca->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TELECHARGER_CLOUD));
    boutonChargerFacturesDaca = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TELECHARGER_CLOUD), 
        tr("Charger la liste des factures"), 
        this);
    facturesDaca->setToolTipsVisible(true);
    boutonChargerFacturesDaca->setStatusTip(tr("Charge la liste des factures disponibles sur le site du DACA"));
    facturesDaca->addAction(boutonChargerFacturesDaca);
    connect(boutonChargerFacturesDaca, SIGNAL(triggered()), this, SLOT(chargerListeFacturesDaca()));

    verifierDispoIdentifiantsDaca();

    menuOutils->addSeparator();

    QMenu* mailing = menuOutils->addMenu(tr("&Mailing"));
    mailing->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING));
    mailingPilotesAyantCotiseCetteAnnee = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING), tr("Envoyer un mail aux pilotes ayant &cotisé cette année"), this);
    mailing->addAction(mailingPilotesAyantCotiseCetteAnnee);
    mailingPilotesActifsAyantCotiseCetteAnnee = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Envoyer un &mail aux pilotes ayant cotisé cette année (pilotes actifs seulement)"), 
        this);
    mailing->addAction(mailingPilotesActifsAyantCotiseCetteAnnee);
    mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING), 
        tr("Envoyer un mail aux pilotes n'ayant pas épuisé leur subvention &entrainement"), 
        this);
    mailing->addAction(mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement);
    mailingPilotesActifs = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Envoyer un mail aux pilotes &actifs"), 
        this);
    mailing->addAction(mailingPilotesActifs);
    mailingPilotesActifsBrevetes = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Envoyer un mail aux pilotes actifs &brevetés"), 
        this);
    mailing->addAction(mailingPilotesActifsBrevetes);
    mailingPilotesDerniereDemandeSubvention = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Envoyer un mail aux pilotes concernés par la dernière &demande de subvention"), 
        this);
    mailing->addAction(mailingPilotesDerniereDemandeSubvention);
    menuMailDemandesSubvention = mailing->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Envoyer un mail aux pilotes concernés par une demande de &subvention"));
    menuMailPilotesSubventionVerseeParVirement = mailing->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Envoyer un mail aux pilotes concernés par un &virement"));
    peuplerMenuMailDemandesSubvention();
   
    menuMailPilotesDUnAerodrome = mailing->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Envoyer un mail aux pilotes d'un aérodrome"));
    menuMailPilotesDUnAerodromeActif = menuMailPilotesDUnAerodrome->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Pilotes actifs"));
    menuMailPilotesDUnAerodromeActifBreveteMoteur = menuMailPilotesDUnAerodrome->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
        tr("Pilotes actifs, brevetés et vol moteur"));
    peuplerMenuMailPilotesDUnAerodrome();

    connect(mailingPilotesAyantCotiseCetteAnnee, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesActifsAyantCotiseCetteAnnee, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesActifsBrevetes, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesDerniereDemandeSubvention, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesNAyantPasEpuiseLeurSubventionEntrainement, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    connect(mailingPilotesActifs, SIGNAL(triggered()), this, SLOT(envoyerMail()));

    menuOutils->addSeparator();

    boutonEditerLePiloteSelectionne = new QAction(QIcon(":/AeroDms/ressources/account-edit.svg"), tr("É&diter le pilote sélectionné"), this);
    boutonEditerLePiloteSelectionne->setStatusTip(tr("Permet d'éditer le pilote actuellement sélectionné. Fonction disponible uniquement si un pilote est sélectionné dans la vue courante."));
    boutonEditerLePiloteSelectionne->setEnabled(false);
    menuOutils->addAction(boutonEditerLePiloteSelectionne);
    connect(boutonEditerLePiloteSelectionne, SIGNAL(triggered()), this, SLOT(editerPilote()));

    boutonEditerUnAeroclub = new QAction(QIcon(":/AeroDms/ressources/account-multiple.svg"), tr("Éditer un aéroclub"), this);
    boutonEditerUnAeroclub->setStatusTip(tr("Permet d'éditer un des aéroclub connus du logiciel"));
    menuOutils->addAction(boutonEditerUnAeroclub);
    connect(boutonEditerUnAeroclub, SIGNAL(triggered()), this, SLOT(editerAeroclub()));

    menuOutils->addSeparator();

    boutonGestionAeronefs = new QAction(QIcon(":/AeroDms/ressources/airplane-cog.svg"), tr("Gérer les aé&ronefs"), this);
    boutonGestionAeronefs->setStatusTip(tr("Permet d'indiquer le type associé à chaque immatriculation connue par le logiciel (à des fins statistiques)"));
    menuOutils->addAction(boutonGestionAeronefs);
    connect(boutonGestionAeronefs, SIGNAL(triggered()), this, SLOT(ouvrirGestionAeronefs()));

    boutonMettreAJourAerodromes = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_ENTRAINEMENT), tr("Mettre à jour la liste des aérodromes"), this);
    boutonMettreAJourAerodromes->setStatusTip(tr("Permet de mettre à jour la liste des aérodromes à partir d'un fichier AIXM 4.5"));
    menuOutils->addAction(boutonMettreAJourAerodromes);
    connect(boutonMettreAJourAerodromes, SIGNAL(triggered()), this, SLOT(mettreAJourAerodromes()));

    menuOutils->addSeparator();

    QAction* boutonConversionHeureDecimalesVersHhMm = new QAction(QIcon(":/AeroDms/ressources/clock-star-four-points.svg"), tr("Convertir &une heure en décimal"), this);
    boutonConversionHeureDecimalesVersHhMm->setStatusTip(tr("Convertir une heure sous forme décimale (X,y heures) en HH:mm"));
    menuOutils->addAction(boutonConversionHeureDecimalesVersHhMm);
    connect(boutonConversionHeureDecimalesVersHhMm, SIGNAL(triggered()), this, SLOT(convertirHeureDecimalesVersHhMm()));
}

void AeroDms::initialiserMenuAide()
{
    //========================Menu Aide
    QMenu* helpMenu = menuBar()->addMenu(tr("&Aide"));

    QAction* aideQtAction = new QAction(QIcon(":/AeroDms/ressources/lifebuoy.svg"), tr("Aide en &ligne"), this);
    aideQtAction->setShortcut(Qt::Key_F1);
    aideQtAction->setStatusTip(tr("Ouvrir l'aide en ligne"));
    helpMenu->addAction(aideQtAction);
    connect(aideQtAction, SIGNAL(triggered()), this, SLOT(ouvrirAide()));

    helpMenu->addSeparator();

    miseAJourAction = new QAction(QIcon(":/AeroDms/ressources/download-box.svg"), tr("&Vérifier la présence de mise à jour"), this);
    miseAJourAction->setStatusTip(tr("Vérifie la présence de mise à jour et permet d'effectuer la mise à jour le cas échéant"));
    helpMenu->addAction(miseAJourAction);
    connect(miseAJourAction, SIGNAL(triggered()), this, SLOT(verifierPresenceDeMiseAjour()));

    miseAJourUpdateLocale = new QAction(QIcon(":/AeroDms/ressources/download-network.svg"), tr("&Deployer la mise à jour distante pour usages internes"), this);
    miseAJourUpdateLocale->setStatusTip(tr("Deploie la mise à jour GitHub pour l'usage interne"));
    helpMenu->addAction(miseAJourUpdateLocale);
    connect(miseAJourUpdateLocale, SIGNAL(triggered()), this, SLOT(demanderTelechargementMiseAJourLogiciel()));
    //De base on masque : fonction accessible uniquement en mode debug
    miseAJourUpdateLocale->setVisible(false);

    mettreAJourDonneesVersionMiniAction = new QAction(QIcon(":/AeroDms/ressources/cog-clockwise.svg"), tr("&Modifier les informations de version minimale"), this);
    mettreAJourDonneesVersionMiniAction->setStatusTip(tr("Met à jour les exigences de version minimale acceptable par le logiciel"));
    helpMenu->addAction(mettreAJourDonneesVersionMiniAction);
    connect(mettreAJourDonneesVersionMiniAction, SIGNAL(triggered()), this, SLOT(mettreAJourVersionMiniExigee()));
    //De base on masque : fonction accessible uniquement en mode debug
    mettreAJourDonneesVersionMiniAction->setVisible(false);

    boutonModeDebug = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_DEBUG), 
        texteBoutonActiverModeDebogage,
        this);
    boutonModeDebug->setStatusTip(tr("Active/désactive le mode débogage. Ce mode permet d'afficher des informations supplémentaires dans l'application et de modifier certains paramètres dans la fenêtre de configuration."));
    boutonModeDebug->setCheckable(true);
    helpMenu->addAction(boutonModeDebug);
    connect(boutonModeDebug, SIGNAL(triggered()), this, SLOT(switchModeDebug()));

    QAction *afficherInfosSsl = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_CHIFFREMENT),
        tr("Afficher les informations de support SSL/TLS"),
        this);
    afficherInfosSsl->setStatusTip(tr("Afficher les informations sur le support SSL/TLS par cette application. Informations utiles pour déboguer les soucis de connexion aux sites des aéroclubs."));
    helpMenu->addAction(afficherInfosSsl);
    QObject::connect(afficherInfosSsl, &QAction::triggered, this, []() {
        QMessageBox::information(0,
            QApplication::applicationName() + " - " + tr("Informations de débogage SSL/TLS"),
            tr("Support OpenSSL : ")
            + (QSslSocket::supportsSsl() ? "Oui" : "Non")
            + "<br />"
            + tr("Version librairie SSL : ")
            + QSslSocket::sslLibraryVersionString()
            + "<br />"
            + tr("Build librairie SSL : ")
            + QSslSocket::sslLibraryBuildVersionString());
        });

    QAction* afficherInfosRessourcesInternes = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_RESSOURCE),
        tr("Afficher les informations sur les ressources internes"),
        this);
    afficherInfosRessourcesInternes->setStatusTip(tr("Affiche l'espace occupé par les ressources internes de l'application (icones, images...)"));
    helpMenu->addAction(afficherInfosRessourcesInternes);
    QObject::connect(afficherInfosRessourcesInternes, &QAction::triggered, this, []() {
        AeroDmsTypes::TailleFichiers tailleFichiers;
        AeroDmsServices::calculerTailleQResources(tailleFichiers);

        QMessageBox::information(0, QApplication::applicationName() + " - " + tr("Ressources internes"),
            tr("<br/><br/><b>Information sur les ressources internes à l'application</b><br/>")
            + tr("Cette fenêtre fournit des informations sur les ressources occupées par les données internes à l'application.")
            + "<table border='1' cellspacing='0' cellpadding='3'>"
            + "<tr><th>Element</th><th>Taille compressée</th><th>Taille non compressée</th><th>Taux</th></tr>"
            + "<tr><td>SVG</td><td>" + QString::number((double)tailleFichiers.svg.compresse / 1024, 'f', 2) + " ko</td><td>" + QString::number((double)tailleFichiers.svg.nonCompresse / 1024, 'f', 2) + " ko</td><td>" + QString::number(((double)(tailleFichiers.svg.nonCompresse - tailleFichiers.svg.compresse) / tailleFichiers.svg.nonCompresse) * 100, 'f', 2) + " %</td></tr>"
            + "<tr><td>PNG/WEBP</td><td>" + QString::number((double)tailleFichiers.png.compresse / 1024, 'f', 2) + " ko</td><td>" + QString::number((double)tailleFichiers.png.nonCompresse / 1024, 'f', 2) + " ko</td><td>" + QString::number(((double)(tailleFichiers.png.nonCompresse - tailleFichiers.png.compresse) / tailleFichiers.png.nonCompresse) * 100, 'f', 2) + " %</td></tr>"
            + "<tr><td>HTML</td><td>" + QString::number((double)tailleFichiers.html.compresse / 1024, 'f', 2) + " ko</td><td>" + QString::number((double)tailleFichiers.html.nonCompresse / 1024, 'f', 2) + " ko</td><td>" + QString::number(((double)(tailleFichiers.html.nonCompresse - tailleFichiers.html.compresse) / tailleFichiers.html.nonCompresse) * 100, 'f', 2) + " %</td></tr>"
            + "<tr><td>Autres</td><td>" + QString::number((double)tailleFichiers.autres.compresse / 1024, 'f', 2) + " ko</td><td>" + QString::number((double)tailleFichiers.autres.nonCompresse / 1024, 'f', 2) + " ko</td><td>" + QString::number(((double)(tailleFichiers.autres.nonCompresse - tailleFichiers.autres.compresse) / tailleFichiers.autres.nonCompresse) * 100, 'f', 2) + " %</td></tr>"
            + "<tr><td><b>Total</b></td><td>" + QString::number((double)tailleFichiers.total.compresse / 1024, 'f', 2) + " ko</td><td>" + QString::number((double)tailleFichiers.total.nonCompresse / 1024, 'f', 2) + " ko</td><td>" + QString::number(((double)(tailleFichiers.total.nonCompresse - tailleFichiers.total.compresse) / tailleFichiers.total.nonCompresse) * 100, 'f', 2) + " %</td></tr>"
            + "</table>");
        });

    helpMenu->addSeparator();

    QAction* aboutQtAction = new QAction(QIcon(":/AeroDms/ressources/qt-logo.svg"), tr("À propos de &Qt"), this);
    aboutQtAction->setStatusTip(tr("Voir la fenêtre à propos de Qt"));
    helpMenu->addAction(aboutQtAction);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QAction* aboutAction = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_ICONE_APPLICATION), 
        tr("À propos de &AeroDMS"), 
        this);
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

    //Si splash vaut nullptr c'est qu'on est hors phase de démarrage
    //=> on demande la réouverture du menu pour permettre la sélection de plusieurs options
    //sans refaire tout le chemin
    if (splash == nullptr)
    {
        menuOption->show();
        menuOptionsRecapAnnuel->show();
        graphiquesDuRecapAnnuel->show();
        resolutionGraphiques->show();
    }
}

void AeroDms::maintenirMenuSelectionGraphsOuvert()
{
    menuOption->show();
    menuOptionsRecapAnnuel->show();
    graphiquesDuRecapAnnuel->show();
}

void AeroDms::changerRatioExportGraphiques()
{
    QFont font;
    font.setWeight(QFont::Normal);
    boutonGraphRatioIso216->setFont(font);
    boutonGraphRatio16x9->setFont(font);

    boutonGraphRatioIso216->setChecked(false);
    boutonGraphRatio16x9->setChecked(false);

    font.setWeight(QFont::Bold);

    if (sender() == boutonGraphRatioIso216)
    {
        boutonGraphRatioIso216->setChecked(true);
        boutonGraphRatioIso216->setFont(font);
    }
    else
    {
        boutonGraphRatio16x9->setChecked(true);
        boutonGraphRatio16x9->setFont(font);
    }

    //Si splash vaut nullptr c'est qu'on est hors phase de démarrage
    //=> on demande la réouverture du menu pour permettre la sélection de plusieurs options
    //sans refaire tout le chemin
    if (splash == nullptr)
    {
        menuOption->show();
        menuOptionsRecapAnnuel->show();
        graphiquesDuRecapAnnuel->show();
        resolutionGraphiques->show();
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
    boutonGraphRecapAnnuelEurosParPilote->setChecked(true);
    boutonGraphRecapAnnuelHeuresParTypeDeVol->setChecked(true);
    boutonGraphRecapAnnuelEurosParTypeDeVol->setChecked(true);
    boutonGraphRecapAnnuelHeuresParActivite->setChecked(true);
    boutonGraphRecapAnnuelEurosParActivite->setChecked(true);
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
     
    int options = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS;
    if (boutonGraphiquesVolsSubventionnesUniquement->isChecked())
    {
        options = options + AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT;
    }
    if (boutonGraphiquesExclurePlaneur->isChecked())
    {
        options = options + AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_PLANEUR;
    }
    if (boutonGraphiquesExclureUlm->isChecked())
    {
        options = options + AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_ULM;
    }
    if (boutonGraphiquesExclureAvion->isChecked())
    {
        options = options + AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_AVION;
    }
    if (boutonGraphiquesExclureHelico->isChecked())
    {
        options = options + AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_HELICOPTERE;
    }

    switch (listeDeroulanteStatistique->currentData().toInt())
    {
        case AeroDmsTypes::Statistiques_HEURES_ANNUELLES:
        {
            m_activeWidget = new StatistiqueHistogrammeEmpile( db, 
                listeDeroulanteAnnee->currentData().toInt(), 
                m_contentArea,
                options);
            break;
        }
        case AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget( db, 
                listeDeroulanteAnnee->currentData().toInt(), 
                AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE, 
                m_contentArea,
                options);
            break;
        }
        case AeroDmsTypes::Statistiques_EUROS_PAR_PILOTE:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget(db,
                listeDeroulanteAnnee->currentData().toInt(),
                AeroDmsTypes::Statistiques_EUROS_PAR_PILOTE,
                m_contentArea,
                options);
            break;
        }
        case AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget( db, 
                listeDeroulanteAnnee->currentData().toInt(), 
                AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE, 
                m_contentArea,
                options);
            break;
        }
        case AeroDmsTypes::Statistiques_EUROS_PAR_ACTIVITE:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget(db,
                listeDeroulanteAnnee->currentData().toInt(),
                AeroDmsTypes::Statistiques_EUROS_PAR_ACTIVITE,
                m_contentArea,
                options);
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
                listeDeroulanteAnnee->currentData().toInt(),
                options);
            break;
        }
        case AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget( db, 
                listeDeroulanteAnnee->currentData().toInt(), 
                AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL, 
                m_contentArea,
                options);
            break;
        }
        case AeroDmsTypes::Statistiques_EUROS_PAR_TYPE_DE_VOL:
        default:
        {
            m_activeWidget = new StatistiqueDiagrammeCirculaireWidget(db,
                listeDeroulanteAnnee->currentData().toInt(),
                AeroDmsTypes::Statistiques_EUROS_PAR_TYPE_DE_VOL,
                m_contentArea, 
                options);
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

    progressionGenerationPdf->setMaximum(p_nombreDeFacturesATraiter);
    progressionGenerationPdf->setValue(0);
}

void AeroDms::mettreAJourFenetreProgressionGenerationPdf(const int p_nombreDeFacturesTraitees)
{
    progressionGenerationPdf->setValue(p_nombreDeFacturesTraitees);
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
    const QString status = tr("Génération terminée. Fichiers disponibles sous ")
                            +p_cheminDossier;
    statusBar()->showMessage(status);

    db->demanderEnvoiBdd();
}

void AeroDms::mettreAJourEchecGenerationPdf()
{
    progressionGenerationPdf->close();
    statusBar()->showMessage(tr("Échec de la génération des demandes de subventions"));
    detruireFenetreProgressionGenerationPdf();
}

AeroDms::~AeroDms()
{
}

void AeroDms::peuplerTablePilotes()
{
    gererBoutonEditionPilote();

    const AeroDmsTypes::ListeSubventionsParPilotes listeSubventions = db->recupererSubventionsPilotes( listeDeroulanteAnnee->currentData().toInt(), 
        listeDeroulantePilote->currentData().toString(),
        AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS,
        false);
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

        const AeroDmsTypes::Club clubDuPilote = db->recupererInfosAeroclubDuPilote(subvention.idPilote);
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_AEROCLUB, new QTableWidgetItem(clubDuPilote.aeroclub));
        vuePilotes->setItem(i, AeroDmsTypes::PiloteTableElement_TERRAIN, new QTableWidgetItem(clubDuPilote.aerodrome));

        QTableWidgetItem* item = vuePilotes->item(i, AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE);
        const double proportionConsommationSubvention = subvention.entrainement.montantRembourse / subvention.montantSubventionEntrainement;
        item->setToolTip(tr("Subvention entrainement restante pour ce pilote : ")
            + QString::number(subvention.montantSubventionEntrainement - subvention.entrainement.montantRembourse)
            + tr(" € (subvention consommée : ")
            + QString::number(proportionConsommationSubvention*100, 'f', 2)
            + " %)");
        if (proportionConsommationSubvention < 0.05)
        {
            item->setBackground(QBrush(QColor(133, 194, 255, 120)));
        }
        else if (proportionConsommationSubvention < 0.75)
        {
            item->setBackground(QBrush(QColor(140, 255, 135, 120))); 
        }
        else if (proportionConsommationSubvention < 0.95)
        {
            item->setBackground(QBrush(QColor(255, 255, 133, 120)));
        }
        else if (proportionConsommationSubvention < 1.0)
        {
            item->setBackground(QBrush(QColor(255, 194, 133, 120)));
        }
        else
        {
            item->setBackground(QBrush(QColor(255, 140, 135, 120)));
            item->setToolTip(tr("Ce pilote a consommé la totalité de sa subvention d'entrainement"));   
        }
    }
    vuePilotes->resizeColumnsToContents();
}

void AeroDms::gererBoutonEditionPilote()
{
    //Edition possible du pilote uniquement si logiciel pas en lecture seule 
    if (!logicielEnModeLectureSeule)
    {
        //On met à jour l'info pilote a éditer en cas de demande d'édition du pilote via le menu outils,
        //si on a sélectionné un pilote donné et pas "Tous les pilotes"
        boutonEditerLePiloteSelectionne->setEnabled(false);
        if (mainTabWidget->currentWidget() == widgetAjoutVol)
        {
            if (choixPilote->currentIndex() != 0)
            {
                piloteAEditer = choixPilote->currentData().toString();
                boutonEditerLePiloteSelectionne->setEnabled(true);
            }
        }
        else if (listeDeroulantePilote->currentData().toString() != "*")
        {
            piloteAEditer = listeDeroulantePilote->currentData().toString();
            boutonEditerLePiloteSelectionne->setEnabled(true);
        }
    } 
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

            if (vol.soumissionEstDelayee)
            {
                for (int i = 0; i < vueVols->columnCount(); i++)
                {
                    vueVols->item(nbItems, i)->setBackground(QBrush(QColor(255, 140, 135, 120)));
                    vueVols->item(nbItems, i)->setToolTip("Ce vol est marqué comme à ne pas soumettre au CSE.\nPour soumettre ce vol au CSE, faire un clic droit sur ce vol et utilisez la fonction \"Marquer/démarquer le vol comme à ne pas soumettre au CSE\".");
                }
            }
            nbItems++;
        }
    }
    vueVols->resizeColumnsToContents();

    boutonGenerePdfRecapHdv->setEnabled(true);
    mailingPilotesAyantCotiseCetteAnnee->setEnabled(true);
    //On désactive la génération du récap annuel si on est sur la sélection "Toutes les années"
    //et également le bouton d'envoi des mails aux pilotes de l'année
    if (listeDeroulanteAnnee->currentData().toInt() == AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        boutonGenerePdfRecapHdv->setEnabled(false);
        mailingPilotesAyantCotiseCetteAnnee->setEnabled(false);
    }
}

void AeroDms::peuplerTableVolsDetectes(const AeroDmsTypes::ListeDonneesFacture p_factures)
{
    vueVolsDetectes->setRowCount(p_factures.size());

    for (int i = 0; i < p_factures.size(); i++)
    {
        AeroDmsTypes::DonneesFacture facture = p_factures.at(i);
        QTableWidgetItem *date = new QTableWidgetItem(facture.dateDuVol.toString("dd/MM/yyyy"));
        date->setData(Qt::UserRole, facture.dateDuVol.toString("yyyy-MM-dd"));
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_DATE, date);
        QTableWidgetItem* duree = new QTableWidgetItem(facture.dureeDuVol.toString("hh:mm"));
        duree->setData(Qt::UserRole, facture.dureeDuVol.hour() * 60 + facture.dureeDuVol.minute());
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_DUREE, duree);
        QTableWidgetItem* montant = new QTableWidgetItem(QString::number(facture.coutDuVol, 'f', 2).append(" €"));
        montant->setData(Qt::UserRole, facture.coutDuVol);
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_MONTANT, montant);
        QTableWidgetItem* immat = new QTableWidgetItem(facture.immat);
        immat->setData(Qt::UserRole, facture.immat);
        vueVolsDetectes->setItem(i, AeroDmsTypes::VolsDetectesTableElement_IMMAT, immat);
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
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_OUI));
                vueFactures->setItem(i, AeroDmsTypes::FactureTableElement_SOUMIS_CE, twSoumisCe);
            }
            else
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Non");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_NON));
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
            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_NOTE, new QTableWidgetItem(facture.note));
            vueSubventions->item(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_NOTE)->setToolTip(facture.note);
            if (facture.modeDeReglement == AeroDmsTypes::ModeDeReglement_VIREMENT)
            {
                vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_MODE_DE_REGLEMENT, new QTableWidgetItem(tr("Virement")));
            }
            else
            {
                vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_MODE_DE_REGLEMENT, new QTableWidgetItem(tr("Chèque")));
            }
            vueSubventions->setItem(nbItems, AeroDmsTypes::SubventionDemandeeTableElement_ID_DEMANDE, new QTableWidgetItem(QString::number(facture.id)));

            nbItems++;
        }
    }
    vueSubventions->resizeColumnsToContents();

    ajusterTableSubventionsDemandeesAuContenu();
}

void AeroDms::ajusterTableSubventionsDemandeesAuContenu()
{
    const int tailleColonnesHorsNote =
        vueSubventions->columnWidth(AeroDmsTypes::SubventionDemandeeTableElement_DATE)
        + vueSubventions->columnWidth(AeroDmsTypes::SubventionDemandeeTableElement_DATE)
        + vueSubventions->columnWidth(AeroDmsTypes::SubventionDemandeeTableElement_PILOTE)
        + vueSubventions->columnWidth(AeroDmsTypes::SubventionDemandeeTableElement_BENEFICIAIRE)
        + vueSubventions->columnWidth(AeroDmsTypes::SubventionDemandeeTableElement_TYPE_DEMANDE)
        + vueSubventions->columnWidth(AeroDmsTypes::SubventionDemandeeTableElement_MONTANT)
        + vueSubventions->columnWidth(AeroDmsTypes::SubventionDemandeeTableElement_MONTANT_VOL)
        + vueSubventions->columnWidth(AeroDmsTypes::SubventionDemandeeTableElement_MODE_DE_REGLEMENT);
    vueSubventions->setColumnWidth(AeroDmsTypes::SubventionDemandeeTableElement_NOTE,
        vueSubventions->width() - tailleColonnesHorsNote);
}

void AeroDms::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if(vueSubventions != nullptr)
        ajusterTableSubventionsDemandeesAuContenu();
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

            QTableWidgetItem *inituleCotisation = new QTableWidgetItem(recette.intitule);
            if (recette.remarque != "")
            {
                inituleCotisation->setToolTip("Remarque : " + recette.remarque);
            }

            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_INTITULE, inituleCotisation);
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_ID, new QTableWidgetItem(QString::number(recette.id)));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_MONTANT, new QTableWidgetItem(QString::number(recette.montant, 'f', 2) + " €"));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_DATE, new QTableWidgetItem(QString::number(recette.annee)));
            vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_TYPE_DE_RECETTE, new QTableWidgetItem(recette.typeDeRecette));
            if (recette.estSoumisCe)
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Oui");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_OUI));
                vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, twSoumisCe);
            }
            else
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Non");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_NON));
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
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_OUI));
                vueRecettes->setItem(position, AeroDmsTypes::RecetteTableElement_SOUMIS_CE, twSoumisCe);
            }
            else
            {
                QTableWidgetItem* twSoumisCe = new QTableWidgetItem("Non");
                twSoumisCe->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_NON));
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
    menuDemandesAGenererAnnees->clear();
    listeDeroulanteAnnee->addItem("Toutes les années", 
        AeroDmsTypes::K_INIT_INT_INVALIDE);

    QAction* actionToutesLesAnnees = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TOUTES_LES_ANNEES), 
        tr("&Toutes"), 
        this);
    actionToutesLesAnnees->setStatusTip(tr("Générer les demandes pour toutes les années pour lesquelles il existe des entrées en base de données"));
    actionToutesLesAnnees->setCheckable(true);
    actionToutesLesAnnees->setChecked(true);
    actionToutesLesAnnees->setData(AeroDmsTypes::K_INIT_INT_INVALIDE);
    menuDemandesAGenererAnnees->addAction(actionToutesLesAnnees);
    connect(actionToutesLesAnnees, SIGNAL(triggered()), this, SLOT(gererSelectionAnneeAGenerer()));

    QList<int> listeAnnees = db->recupererAnnees();
    for (int i = 0; i < listeAnnees.size(); i++)
    {
        listeDeroulanteAnnee->addItem(QString::number(listeAnnees.at(i)), listeAnnees.at(i));

        QAction* actionAnnees = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_UNE_ANNEE), QString::number(listeAnnees.at(i)), this);
        actionAnnees->setStatusTip(tr("Générer uniquement les demandes pour l'année ") + QString::number(listeAnnees.at(i)));
        actionAnnees->setCheckable(true);
        actionAnnees->setData(listeAnnees.at(i));
        menuDemandesAGenererAnnees->addAction(actionAnnees);
        connect(actionAnnees, SIGNAL(triggered()), this, SLOT(gererSelectionAnneeAGenerer()));
    }
    //On affiche de base les infos de l'année courante, et, si pas encore de cotisation sur l'année courante,
    //on affiche la dernière année disponible
	const int indexAnneeCourante = listeDeroulanteAnnee->findText(QDate::currentDate().toString("yyyy"));
	if (indexAnneeCourante != AeroDmsTypes::K_INIT_INT_INVALIDE)
	{
		listeDeroulanteAnnee->setCurrentIndex(indexAnneeCourante);
	}
	else
	{
		listeDeroulanteAnnee->setCurrentIndex(listeDeroulanteAnnee->count() - 1);
	}
}

void AeroDms::gererSelectionAnneeAGenerer()
{
	QAction* actionSelectionnee = qobject_cast<QAction*>(sender());
	if (actionSelectionnee != nullptr)
	{
		for (QAction* action : menuDemandesAGenererAnnees->actions())
		{
            if (action != actionSelectionnee)
            {
                action->setChecked(false);
            }
            else
			{
				action->setChecked(true);
			}
		}
	}
}

const int AeroDms::recupererAnneeAGenerer()
{
    for (QAction* action : menuDemandesAGenererAnnees->actions())
    {
        if (action->isChecked())
        {
            return action->data().toInt();
        }
    }
    return AeroDmsTypes::K_INIT_INT_INVALIDE;
}

void AeroDms::ajouterUneCotisationEnBdd()
{
    const AeroDmsTypes::CotisationAnnuelle infosCotisation = dialogueAjouterCotisation->recupererInfosCotisationAAjouter();
    if (!infosCotisation.estEnEdition)
    {
        //On contrôle que le pilote n'a pas déjà une cotisation pour cette année
        if (db->piloteEstAJourDeCotisation(infosCotisation.idPilote, infosCotisation.annee))
        {
            QMessageBox::critical(this, 
                QApplication::applicationName() + " - " + tr("Cotisation déja reglée"), 
                tr("Le pilote indiqué est déjà à jour de\nsa cotisation pour l'année saisie."));
        }
        //Si le pilote n'a pas déjà une cotisation pour cette année la, on ajoute la cotisation en BDD
        else
        {
            db->ajouterCotisation(infosCotisation);
            //On met à jour la table des pilotes
            peuplerTablePilotes();
            //On met a jour la liste des années => permet de traiter le cas ou on ajoute un premier pilote sur l'année considérée
            peuplerListeDeroulanteAnnee();
            
            peuplerTableRecettes();
            //On peut avoir réactivé un pilote inactif : on réélabore les listes de pilotes
            peuplerListesPilotes();

            db->demanderEnvoiBdd();

            statusBar()->showMessage(tr("Cotisation ") + QString::number(infosCotisation.annee) + tr(" ajoutée pour le pilote ") + db->recupererNomPrenomPilote(infosCotisation.idPilote));
        }
    }
    else
    {
        db->ajouterCotisation(infosCotisation);
        statusBar()->showMessage(tr("Cotisation ") + QString::number(infosCotisation.annee) + tr(" mise à jour pour le pilote ") + db->recupererNomPrenomPilote(infosCotisation.idPilote));

        //On met à jour la table des pilotes (en cas de changement de couleur)
        peuplerTablePilotes();

        db->demanderEnvoiBdd();
    }
}

void AeroDms::chargerBaladesSorties()
{
    peuplerListeBaladesEtSorties();
}

void AeroDms::ajouterUnPiloteEnBdd()
{
    const AeroDmsTypes::Pilote pilote = dialogueGestionPilote->recupererInfosPilote();
    const AeroDmsTypes::ResultatCreationBdd resultat = db->creerPilote(pilote);

    //On met à jour les listes de pilotes
    peuplerListesPilotes();
    dialogueAjouterCotisation->mettreAJourLeContenuDeLaFenetre();

    //On met également à jour la liste des aérodromes pour le mailing vers les pilotes d'un aérodrome
    peuplerMenuMailPilotesDUnAerodrome();

    //Si on est sur une mise à jour, on met à jour les éléments d'IHM susceptibles d'être impacté par des changements
    if (pilote.idPilote != "")
    {
        peuplerListeBaladesEtSorties();
        peuplerTablePilotes();
        peuplerTableVols();

        //Si on est en ajout de vol, on resélectionne le pilote...
        if (mainTabWidget->currentWidget() == widgetAjoutVol)
        {
            choixPilote->setCurrentIndex(choixPilote->findData(pilote.idPilote));
            aeroclubPiloteSelectionne->setText(db->recupererAeroclub(choixPilote->currentData().toString()));
        }
    }

    switch (resultat)
    {
        case AeroDmsTypes::ResultatCreationBdd_SUCCES:
        {
            if (pilote.idPilote == "")
            {
                statusBar()->showMessage(tr("Pilote ajouté avec succès"));
            }
            else
            {
                statusBar()->showMessage(tr("Pilote modifié avec succès"));
            } 

            db->demanderEnvoiBdd();
            break;
        } 
        case AeroDmsTypes::ResultatCreationBdd_ELEMENT_EXISTE:
        {
            statusBar()->showMessage(tr("Échec ajout pilote : le pilote existe déjà"));
            QMessageBox::critical(this, 
                QApplication::applicationName() + " - " + tr("Échec ajoute pilote"), 
                tr("Un pilote existe avec ce nom\nexiste déjà. Ajout impossible."));
            break;
        }
        case AeroDmsTypes::ResultatCreationBdd_AUTRE:
        {
            statusBar()->showMessage(tr("Échec ajout pilote : erreur indéterminée"));
            QMessageBox::critical(this, 
                QApplication::applicationName() + " - " + tr("Échec ajoute pilote"), 
                tr("Une erreur indéterminée s'est\nproduite. Ajout du pilote impossible."));
            break;
        }
    }
}

void AeroDms::ajouterUnAeroclubEnBdd()
{
    const AeroDmsTypes::Club club = dialogueGestionAeroclub->recupererInfosClub();
    const AeroDmsTypes::ResultatCreationBdd resultat = db->creerAeroclub(club);

    //On met à jour la liste des clubs
    dialogueGestionPilote->peuplerListeAeroclub();
    dialogueGestionAeroclub->peuplerListeAeroclub();

    //On repeuple la liste des mailings des pilotes d'un terrain
    peuplerMenuMailPilotesDUnAerodrome();

    switch (resultat)
    {
    case AeroDmsTypes::ResultatCreationBdd_SUCCES:
        {
            if (club.idAeroclub == AeroDmsTypes::K_INIT_INT_INVALIDE)
            {
                statusBar()->showMessage(tr("Aéroclub ajouté avec succès"));
            }
            else
            {
                statusBar()->showMessage(tr("Aéroclub modifié avec succès"));
            }
            db->demanderEnvoiBdd();

            break;
        }
        case AeroDmsTypes::ResultatCreationBdd_ELEMENT_EXISTE:
	    {
		    statusBar()->showMessage(tr("Échec ajout aéroclub : l'aéroclub existe déjà"));
		    QMessageBox::critical(this,
			    QApplication::applicationName() + " - " + tr("Échec ajoute aéroclub"),
			    tr("Un aéroclub existe avec ce nom\nexiste déjà. Ajout impossible."));
		    break;
	    }
        case AeroDmsTypes::ResultatCreationBdd_AUTRE:
        default:
        {
            statusBar()->showMessage(tr("Échec ajout aéroclub : erreur indéterminée"));
            QMessageBox::critical(this,
                QApplication::applicationName() + " - " + tr("Échec ajoute aéroclub"),
                tr("Une erreur indéterminée s'est\nproduite. Ajout de l'aéroclub impossible."));
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

    statusBar()->showMessage(tr("Sortie ") + sortie.nom + tr(" ajoutée"));

    db->demanderEnvoiBdd();
}

void AeroDms::selectionnerUneFacture()
{
    QStringList fichiers = QFileDialog::getOpenFileNames(
            this,
            QApplication::applicationName() + " - " + tr("Ouvrir une facture"),
            parametresSysteme.cheminStockageFacturesATraiter,
            tr("Fichier PDF (*.pdf)"));

    if (fichiers.size() == 1)
    {
        fichierPrecedent->setVisible(false);
        fichierSuivant->setVisible(false);

        chargerUneFactureAvecScan(fichiers.at(0));
    }
	else if (fichiers.size() > 1)
	{
		fichierPrecedent->setVisible(false);
		fichierSuivant->setVisible(false);

        const QString cheminFactureMergee =  AeroDmsServices::mergerPdf("",
            QFileInfo(fichiers.at(0)).path() + "/facturesMergees_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmm") + ".pdf",
            fichiers,
            parametresMetiers.nomTresorier,
            "Factures fusionnnées",
            "Factures fusionnnées");

        for (int i = 0; i < fichiers.size(); i++)
        {
            QFile::remove(fichiers.at(i));
        }

		chargerUneFactureAvecScan(cheminFactureMergee);
	}
}

void AeroDms::chargerUneFactureAvecScan(const QString p_fichier, const bool p_laFactureAChargerEstTelechargeeDInternet)
{
    if (!p_fichier.isNull())
    {
        chargerUneFacture(p_fichier, p_laFactureAChargerEstTelechargeeDInternet);
        idFactureDetectee = AeroDmsTypes::K_INIT_INT_INVALIDE;

        //On masque par défaut... on réaffiche si le scan est effectué
        //et qu'il ne retourne par une liste vide
        validerLesVols->setHidden(true);
        vueVolsDetectes->setHidden(true);
        supprimerLeVolSelectionne->setHidden(true);
        if (scanAutomatiqueDesFacturesEstActif)
        {
            factures = PdfExtractor::recupererLesDonneesDuPdf(p_fichier);
            if (factures.size() != 0)
            {
                peuplerTableVolsDetectes(factures);
                validerLesVols->setHidden(false);
                vueVolsDetectes->setHidden(false);
                supprimerLeVolSelectionne->setHidden(false);

                //On met la date à la première date de la facture
                //(permet d'avoir l'année correspondante à la facture
                //lors de la séléction du pilote, et donc d'afficher
                //l'état de la cotisation pour l'année concernée par la facture)
                dateDuVol->setDate(factures.at(0).dateDuVol);
            }
        }
        
        //Si on passe ici, on est pas en édition de vol
        volAEditer = AeroDmsTypes::K_INIT_INT_INVALIDE;
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
    else if (sender() == scanAutoAtvv)
    {
        aeroclub = AeroDmsTypes::Aeroclub_ATVV;
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
        //On masque par défaut... on réaffiche si le scan est effectué
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
        //On masque par défaut... on réaffiche si le scan est effectué
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

void AeroDms::chargerUneFacture(const QString p_fichier, const bool p_laFactureAChargerEstTelechargeeDInternet)
{
    
    factureIdEnBdd = AeroDmsTypes::K_INIT_INT_INVALIDE;
    pdfDocument->load(p_fichier);
    if (factureRecupereeEnLigneEstNonTraitee == true)
    {
        factureRecupereeEnLigneEstNonTraitee = false;
        QFile::remove(cheminDeLaFactureCourante);
    }
    factureRecupereeEnLigneEstNonTraitee = p_laFactureAChargerEstTelechargeeDInternet;
    cheminDeLaFactureCourante = p_fichier;

    choixPilote->setEnabled(true);
    choixPayeur->setEnabled(true);
    dureeDuVol->setEnabled(true);
    prixDuVol->setEnabled(true);
    typeDeVol->setEnabled(true);

    choixPilote->setCurrentIndex(0);
    choixPayeur->setCurrentIndex(0);
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

    const int anneeAGenerer = recupererAnneeAGenerer();
	QString anneeAGenererTexte = tr("Toutes");
	if (anneeAGenerer != AeroDmsTypes::K_INIT_INT_INVALIDE)
	{
		anneeAGenererTexte = QString::number(anneeAGenerer) + tr(" seulement");
	}

    QString nbSubventions = QString::number(db->recupererLesSubventionsAEmettre(anneeAGenerer).size());
    QString nbFactures = QString::number(db->recupererLesDemandesDeRembousementAEmettre(anneeAGenerer).size());
    QString nbCotisations = QString::number(db->recupererLesCotisationsAEmettre(anneeAGenerer).size());
    QString nbBaladesSorties = QString::number(db->recupererLesRecettesBaladesEtSortiesAEmettre(anneeAGenerer).size());

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
		+ "<b>Année</b> : " + anneeAGenererTexte + "<br />"
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

    if ( (db->recupererLesSubventionsAEmettre(anneeAGenerer).size()
         + db->recupererLesDemandesDeRembousementAEmettre(anneeAGenerer).size()
         + db->recupererLesCotisationsAEmettre(anneeAGenerer).size()
         + db->recupererLesRecettesBaladesEtSortiesAEmettre(anneeAGenerer).size()) == 0)
    {
        demandeConfirmationGeneration.setInformativeText(tr("Rien à générer. Génération indisponible."));
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
                parametresSysteme.autoriserReglementParVirement,
                calculerValeurGraphAGenererPdf(),
                anneeAGenerer);
        }
        break;

        case QMessageBox::No:
        default:
        {
            //Rien à faire
        }
        break;
    }
    
	//On met à jour le menu des mailing pour les demandes de subvention
    peuplerMenuMailDemandesSubvention();
}

void AeroDms::genererPdfRecapHdV()
{
    pdf->imprimerLeRecapitulatifDesHeuresDeVol( listeDeroulanteAnnee->currentData().toInt(),
                                                parametresSysteme.cheminSortieFichiersGeneres,
                                                parametresSysteme.cheminStockageFacturesTraitees,
                                                signature,
                                                calculerValeurGraphAGenererPdf(),
                                                parametresMetiers.nomTresorier);
}

void AeroDms::enregistrerUneFacture()
{
    bool estEnEchec = false;
    const int anneeRenseignee = dateDeFacture->date().year();
    const QString idPilote = choixPayeur->currentData().toString();
    const int idSortie = choixBaladeFacture->currentData().toInt();

    //On vérifie si le pilote est a jour de sa cotisation pour l'année de la facture
    if (!db->piloteEstAJourDeCotisation(idPilote, anneeRenseignee))
    {
        estEnEchec = true;
        QMessageBox::critical(this, QApplication::applicationName() + " - " + tr("Cotisation non payée"),
            tr("Le pilote n'est pas a jour de sa cotisation pour l'année du vol.\n"
               "Impossible d'enregistrer une facture à rembourser à son profit."), QMessageBox::Cancel);
    }

    //On effectue d'abord quelques contrôles pour savoir si la facture est enregistrable :
    //1) on a une facture chargée
    //2) les données (pilote, date du vol, durée, cout) sont renseignées
    //3) on est pas en échec sur une des étapes précédentes
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

                gestionnaireDonneesEnLigne->envoyerFichier(cheminDeLaFactureCourante);

                factureRecupereeEnLigneEstNonTraitee = false;
            }
            else
            {
                statusBar()->showMessage(tr("Impossible de déplacer la facture : arrêt"));
                estEnEchec = true;
            }
        }
        //On désactive le choix du payeur (la facture ne concerne qu'un seul et unique payeur)
        choixPayeur->setEnabled(false);

        //Ensuite :
        //3) on enregistre la paiement en BDD, si on est pas en échec
        if (!estEnEchec)
        {
            db->enregistrerUneFacture(idPilote,
                factureIdEnBdd,
                dateDeFacture->date(),
                montantFacture->value(),
                choixBaladeFacture->currentData().toInt(),
                remarqueFacture->text());

            statusBar()->showMessage(QString(tr("Facture "))
                + remarqueFacture->text()
                + tr(" du ")
                + dateDeFacture->date().toString("dd/MM/yyyy")
                + " ("
                + QString::number(montantFacture->value(), 'f', 2)
                + tr("€) ajoutée."));

            montantFacture->setValue(0);
            remarqueFacture->clear();

            //On met à jour la vue
            peuplerTableFactures();

            db->demanderEnvoiBdd();
        }
        else
        {
            statusBar()->showMessage(tr("Erreur d'ajout de la facture"));
        }
    }
}

void AeroDms::enregistrerUnVol()
{
    //Si le pilote n'est pas à jour de sa cotisation => échec immédiat
    bool estEnEchec = !lePiloteEstAJourDeCotisation();
    bool bddAEnvoyer = false;

    //On effectue d'abord quelques contrôles pour savoir si le vol est enregistrable :
    //1) on a une facture chargée
    //2) les données (pilote, date du vol, durée, cout) sont renseignées (OK si le bouton est actif)
    //3) on est pas en échec sur une des étapes précédentes
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

                gestionnaireDonneesEnLigne->envoyerFichier(cheminDeLaFactureCourante);

                factureRecupereeEnLigneEstNonTraitee = false;
            }
            else
            {
                statusBar()->showMessage(tr("Impossible de déplacer la facture : arrêt"));
                estEnEchec = true;
            }
        }
        //On désactive le choix du pilote (la facture ne concerne qu'un seul et unique pilote)
        choixPilote->setEnabled(false);

        //Ensuite, si on est pas en échec :
        //1) on récupère la subvention restante pour le pilote
        //2) on calcul la subvention allouable pour le vol
        //3) on enregistre le vol en BDD    
		if (!estEnEchec)
		{
			double subventionRestante = db->recupererSubventionRestante(idPilote, anneeRenseignee);
			//On initialise le montant subventionné sur la base du prix du vol (cas vols type sortie)
			double montantSubventionne = prixDuVol->value();
			if (typeDeVol->currentText() == "Balade")
			{
				montantSubventionne = prixDuVol->value() * parametresMetiers.proportionRemboursementBalade;
			}
			//Si on est en vol d'entrainement, calculs spécifiques et enregistrement spécifique
			if (typeDeVol->currentText() == "Entrainement")
			{
				double coutHoraire = calculerCoutHoraire();
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
                if (volAEditer != AeroDmsTypes::K_INIT_INT_INVALIDE)
                {
                    volAjouteModifie = "modifié";
                    //On sort du mode édition
                    volAEditer = AeroDmsTypes::K_INIT_INT_INVALIDE;
                    //Et on recharge la liste déroulante de sélection des pilotes dans laquelle on avait remis les pilotes
                    //inactifs...
                    peuplerListesPilotes();
                }

                //Si on enregistre un vol seul, 
                //   ou si on enregistre le dernier vol d'une série de vol détéctés
                //il faudra envoyer la BDD en ligne à l'issue
                if (idFactureDetectee == AeroDmsTypes::K_INIT_INT_INVALIDE
                    ||(idFactureDetectee != AeroDmsTypes::K_INIT_INT_INVALIDE
                        && factures.size() == 1))
                {
                    bddAEnvoyer = true;
                }

                //On supprime la vol de la liste des vols détectés si on en avait chargé un
                //On fait ceci avant la mise à jour de la statusBar car supprimerLeVolDeLaVueVolsDetectes()
                //en fait également une. De cette façon on masque le status de suppression et on affiche
                //que le status d'ajout du vol
                supprimerLeVolDeLaVueVolsDetectes();

            statusBar()->showMessage(QString(tr("Vol "))
                + typeDeVol->currentText() 
                + tr(" de ")
                + db->recupererNomPrenomPilote(idPilote)
                + tr(" du ")
                + dateDuVol->date().toString("dd/MM/yyyy") 
                + " (" 
                + dureeDuVol->time().toString("hh:mm")
                + "/"
                + QString::number(prixDuVol->value(), 'f', 2)
                + "€) "
                + volAjouteModifie
                + tr(". Montant subvention : ")
                + QString::number(montantSubventionne, 'f', 2)
                + tr("€ / Subvention entrainement restante : ")
                + QString::number(subventionRestante, 'f', 2)
                + "€");

            //On rince les données de vol
            dureeDuVol->setTime(QTime::QTime(0, 0));
            prixDuVol->setValue(0);
            remarqueVol->clear();
            immat->clear();
            //La mise à jour de ces données provoque la réélaboration de l'état des boutons de validation

            //on réactive les éventuels éléments d'IHM désactivés par une mise à jour de vol
            typeDeVol->setEnabled(true);
            dureeDuVol->setEnabled(true);
            prixDuVol->setEnabled(true);
        }
        else
        {
            statusBar()->showMessage(tr("Erreur d'ajout du vol"));
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
    
    if (bddAEnvoyer)
    {
        db->demanderEnvoiBdd();
    }
}

void AeroDms::enregistrerLesVols()
{
    //On ne peut enregistrer les vols que si le pilote est à jour de cotisation
    //Cela sera vérifié dans la méthode enregistrerUnVol() cependant on le prévérifie
    //ici pour éviter d'avoir N fois la notification d'échec si le pilote n'est 
    //pas à jour de sa cotisation
    chargerUnVolDetecte(0, 0);
    if (lePiloteEstAJourDeCotisation())
    {
		barreDeProgressionStatusBar->setMaximum(factures.size());
		barreDeProgressionStatusBar->setValue(0);
        barreDeProgressionStatusBar->show();
        while (!factures.isEmpty())
        {
			barreDeProgressionStatusBar->setValue(barreDeProgressionStatusBar->value() + 1);
            //On charge le premier vol de la ligne
            chargerUnVolDetecte(0, 0);

            //On demande l'enregistrement
            enregistrerUnVol();
        }
        //La liste sera vide => on désactive le bouton d'enregistrement du vol
        validerLesVols->setEnabled(false);
        supprimerLeVolSelectionne->setEnabled(false);
    }

    QTimer::singleShot(5000, this, &AeroDms::masquerBarreDeProgressionDeLaStatusBar);
}

void AeroDms::supprimerLeVolDeLaVueVolsDetectes()
{
    if (idFactureDetectee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        factures.remove(idFactureDetectee);
        idFactureDetectee = AeroDmsTypes::K_INIT_INT_INVALIDE;
        peuplerTableVolsDetectes(factures);

        vueVolsDetectes->clearSelection();
        supprimerLeVolSelectionne->setEnabled(false);

        statusBar()->showMessage(tr("Vol supprimé avec succès de la liste des vols détectés"));
    }
}

void AeroDms::enregistrerUneRecette()
{
    QStringList volsCoches;
    //On vérifie si au moins un vol est coché
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
            QApplication::applicationName() + " - " + tr("Vol non sélectionné"),
                               tr("La dépense doit être associée à au moins un vol. \n\
Sélectionnez au moins un vol dans la partie gauche de la fenêtre.\n\n\
Saisie non prise en compte."));
    }
    else
    {
        db->ajouterUneRecetteAssocieeAVol( volsCoches,
                                           typeDeRecette->currentText(),
                                           nomEmetteurChequeRecette->text() + " / " + banqueNumeroChequeRecette->text(),
                                           montantRecette->value(),
                                           paiementCarteBancaire->isChecked());
        statusBar()->showMessage(tr("Recette ajoutée"));

        nomEmetteurChequeRecette->clear();
        banqueNumeroChequeRecette->clear();
        paiementCarteBancaire->setChecked(false);
        montantRecette->clear();

        peuplerTableRecettes();
        peuplerListeBaladesEtSorties();

        db->demanderEnvoiBdd();
    }
}

const double AeroDms::calculerCoutHoraire()
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

    listeDeroulantePilote->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TOUS), "Tous les pilotes", "*");
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
            || volAEditer != AeroDmsTypes::K_INIT_INT_INVALIDE)
        {
            choixPilote->addItem(AeroDmsServices::recupererIcone(nomPrenomNorm.at(0)), nomPrenom, pilote.idPilote);
            choixPayeur->addItem(AeroDmsServices::recupererIcone(nomPrenomNorm.at(0)), nomPrenom, pilote.idPilote);
        }
    }

    //Si le pilote précédemment sélectionné existe toujours, on le restaure
    const int index = listeDeroulantePilote->findData(piloteSelectionne);
    if (index != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        listeDeroulantePilote->setCurrentIndex(index);
    }  
}

void AeroDms::peuplerListeSorties()
{
    choixBalade->clear();
    choixBaladeFacture->clear();
    choixBalade->addItem("", 
        AeroDmsTypes::K_INIT_INT_INVALIDE);
    choixBaladeFacture->addItem("", 
        AeroDmsTypes::K_INIT_INT_INVALIDE);

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

void AeroDms::peuplerMenuMailDemandesSubvention()
{
    menuMailDemandesSubvention->clear();
    menuMailPilotesSubventionVerseeParVirement->clear();

    QList<QDate> datesDemandes = db->recupererDatesDesDemandesDeSubventions();
    for (int i = 0; i < datesDemandes.size(); i++)
    {
        QAction* action = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
            tr("Demande du ") + datesDemandes.at(i).toString("dd/MM/yyyy"),
            this);
        AeroDmsTypes::DonneesMailing data;
        data.typeMailing = AeroDmsTypes::DonnesMailingType_DEMANDE_DE_SUBVENTION;
        data.donneeComplementaire = datesDemandes.at(i).toString("yyyy-MM-dd");
        action->setData(QVariant::fromValue(data));
        menuMailDemandesSubvention->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    }

    datesDemandes = db->recupererDatesDesDemandesDeSubventionsVerseesParVirement();
    for (int i = 0; i < datesDemandes.size(); i++)
    {
        QAction* action = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
            tr("Demande du ") + datesDemandes.at(i).toString("dd/MM/yyyy"),
            this);
        AeroDmsTypes::DonneesMailing data;
        data.typeMailing = AeroDmsTypes::DonnesMailingType_SUBVENTION_VERSEE_PAR_VIREMENT;
        data.donneeComplementaire = datesDemandes.at(i).toString("yyyy-MM-dd");
        action->setData(QVariant::fromValue(data));
        menuMailPilotesSubventionVerseeParVirement->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    }

}

void AeroDms::peuplerMenuMailPilotesDUnAerodrome()
{
    menuMailPilotesDUnAerodromeActif->clear();

    AeroDmsTypes::ListeAerodromes aerodromes = db->recupererAerodromesAvecPilotesActifs(AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_D_UN_AERODROME);
    for (AeroDmsTypes::Aerodrome aerodrome : aerodromes)
    {
        QAction* action = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
            aerodrome.nom + " - " + aerodrome.indicatifOaci,
            this);
        
        AeroDmsTypes::DonneesMailing data;
        data.typeMailing = AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_D_UN_AERODROME;
        data.donneeComplementaire = aerodrome.indicatifOaci;
        action->setData(QVariant::fromValue(data));
        menuMailPilotesDUnAerodromeActif->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(envoyerMail()));
    }

    menuMailPilotesDUnAerodromeActifBreveteMoteur->clear();

    aerodromes = db->recupererAerodromesAvecPilotesActifs(AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_BREVETES_VOL_MOTEUR_D_UN_AERODROME);
    for (AeroDmsTypes::Aerodrome aerodrome : aerodromes)
    {
        QAction* action = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_MAILING),
            aerodrome.nom + " - " + aerodrome.indicatifOaci,
            this);

        AeroDmsTypes::DonneesMailing data;
        data.typeMailing = AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_BREVETES_VOL_MOTEUR_D_UN_AERODROME;
        data.donneeComplementaire = aerodrome.indicatifOaci;
        action->setData(QVariant::fromValue(data));
        menuMailPilotesDUnAerodromeActifBreveteMoteur->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(envoyerMail()));
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
            itemBaladesEtSorties->setData(Qt::UserRole, vol.montantSubventionAttendu);
        }  
        if (!vol.volAAuMoinsUnPaiement) 
        {
            itemBaladesEtSorties->setForeground(Qt::red);
        }
        listeBaladesEtSorties->addItem(itemBaladesEtSorties);
        connect(listeBaladesEtSorties, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(traiterClicSurVolBaladesEtSorties()));
    }
}

void AeroDms::traiterClicSurVolBaladesEtSorties()
{
    double montantRecetteAttendu = 0;

    //On somme les montants des vols cochés
    for (int i = 0; i < listeBaladesEtSorties->count(); i++)
    {
        if (listeBaladesEtSorties->item(i)->checkState() == Qt::Checked)
        {
            montantRecetteAttendu = montantRecetteAttendu + listeBaladesEtSorties->item(i)->data(Qt::UserRole).toDouble();
        }
    }

	montantRecette->setValue(montantRecetteAttendu);
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
         || idFactureDetectee != AeroDmsTypes::K_INIT_INT_INVALIDE)
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

    if (choixPilote->currentIndex() != 0 
        && !vueVolsDetectes->isHidden())
    {
        const QString texteVolSembleExisterEnDb = tr("Ce vol semble déjà exister en base de données");

        for (int i = 0; i < vueVolsDetectes->rowCount(); i++)
        {
            const int dureeDuVol = vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DUREE)->data(Qt::UserRole).toInt();
            const double coutDuVol = vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_MONTANT)->data(Qt::UserRole).toDouble();
            const QString date = vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DATE)->data(Qt::UserRole).toString();
            const bool volSembleExisterEnDb = db->volSembleExistantEnBdd(choixPilote->currentData().toString(), dureeDuVol, date, coutDuVol);
            
            if (volSembleExisterEnDb)
            {
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DATE)->setBackground(QBrush(QColor(255, 140, 135, 120)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DATE)->setToolTip(texteVolSembleExisterEnDb);
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DUREE)->setBackground(QBrush(QColor(255, 140, 135, 120)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DUREE)->setToolTip(texteVolSembleExisterEnDb);
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_IMMAT)->setBackground(QBrush(QColor(255, 140, 135, 120)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_IMMAT)->setToolTip(texteVolSembleExisterEnDb);
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_MONTANT)->setBackground(QBrush(QColor(255, 140, 135, 120)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_MONTANT)->setToolTip(texteVolSembleExisterEnDb);
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_TYPE)->setBackground(QBrush(QColor(255, 140, 135, 120)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_TYPE)->setToolTip(texteVolSembleExisterEnDb);
            }
            else
            {
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DATE)->setBackground(QBrush(QColor(255, 255, 255, 255)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DATE)->setToolTip("");
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DUREE)->setBackground(QBrush(QColor(255, 255, 255, 255)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_DUREE)->setToolTip("");
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_IMMAT)->setBackground(QBrush(QColor(255, 255, 255, 255)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_IMMAT)->setToolTip("");
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_MONTANT)->setBackground(QBrush(QColor(255, 255, 255, 255)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_MONTANT)->setToolTip("");
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_TYPE)->setBackground(QBrush(QColor(255, 255, 255, 255)));
                vueVolsDetectes->item(i, AeroDmsTypes::VolsDetectesTableElement_TYPE)->setToolTip("");
            }
        }
    }
}

void AeroDms::mettreAJourInfosSurSelectionPilote()
{
    if (choixPilote->currentIndex() != 0)
    {
        aeroclubPiloteSelectionne->setText(db->recupererAeroclub(choixPilote->currentData().toString()));
        
        statusBar()->showMessage(texteSubventionRestante());
        activite->setCurrentIndex(activite->findText(db->recupererActivitePrincipale(choixPilote->currentData().toString())));
    }
    else
    {
        aeroclubPiloteSelectionne->setText("");
        statusBar()->clearMessage();
    }

    gererBoutonEditionPilote();
}

const QString AeroDms::texteSubventionRestante()
{
    QString cotisation = tr("(Cotisation payée pour l'année ")
        + QString::number(dateDuVol->date().year())
        + ")";
    if (!db->piloteEstAJourDeCotisation(choixPilote->currentData().toString(), dateDuVol->date().year()))
    {
        cotisation = tr("(Cotisation non payée pour l'année ")
            + QString::number(dateDuVol->date().year())
            + ")";
    }

    return tr("Subvention restante pour ce pilote, pour l'année ")
        + QString::number(dateDuVol->date().year())
        + " : " + QString::number(db->recupererSubventionRestante(choixPilote->currentData().toString(), dateDuVol->date().year()))
        + " € "
        + cotisation
        + ".";
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

void AeroDms::changementCochePaiementCb()
{
    banqueNumeroChequeRecette->setDisabled(paiementCarteBancaire->isChecked());
    banqueNumeroChequeRecette->setText("");

    if (paiementCarteBancaire->isChecked())
    {
        banqueNumeroChequeRecette->setText("CB");
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

void AeroDms::ajouterUnAeroclub()
{
    dialogueGestionAeroclub->ouvrirFenetre(false);
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
    QDate date = QDate::fromString(__DATE__, "MMM dd yyyy");
    if (!date.isValid())
    {
        date = QDate::fromString(__DATE__, "MMM  d yyyy");
    }
    const QTime heure = QTime::fromString(__TIME__);

    QMessageBox::about(this, tr("À propos de ") + QApplication::applicationName(),
        "<b>" + QApplication::applicationName() + " v" + QApplication::applicationVersion() + "</b> <br/>" 
        + "Version compilée le " + date.toString("dd/MM/yyyy") + " à " + heure.toString("hh'h'mm") + " <br/><br/> "
        + tr("Logiciel de gestion de compta d'une section aéronautique.") + "<br/><br/>"
        + tr("Le code source de ce programme est disponible sous")
        + " <a href=\"https://github.com/cvermot/AeroDMS\">GitHub</a>.<br/><br/>"
        + tr("Ce programme utilise les libraires :<ul>")
        + "<li><a href = \"https://www.qt.io\">Qt</a> "+ qVersion() +"</li>"
        + "<li><a href = \"https://github.com/podofo/podofo\">PoDoFo</a> 1.0.0</li>"
        + "<li><a href = \"https://github.com/rikyoz/bit7z\">bit7z</a> 4.0.10</li>"
        + "</ul>"
        + tr("Les icones sont issues de")
        + " <a href = \"https://pictogrammers.com/\">pictogrammers.com</a>.< br />< br />"
        + tr("L'image de démarrage est générée avec")
        + " <a href = \"https://designer.microsoft.com/\">Microsoft Designer</a>.< br />< br />"
        + tr("Ce programme inclut du code source inspiré de code sous licence BSD-3-Clause (Copyright (C) The Qt Company Ltd).<br />< br/>")
        + tr("Mentions légales : <br />")
        + " This program is free software: you can redistribute it and/or modify"
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
                                                         AeroDmsTypes::PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE)->text().remove(" €").toDouble();
        anneeAEditer = vuePilotes->item( ligneSelectionnee,
                                         AeroDmsTypes::PiloteTableElement_ANNEE)->text().toInt();

        QAction editer(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_PILOTE),
            tr("Éditer le pilote"), 
            this);
        connect(&editer, SIGNAL(triggered()), this, SLOT(editerPilote()));
        menuClicDroitPilote.addAction(&editer);

        QAction editerCotisation(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_COTISATION), tr("Modifier la cotisation"), this);
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

void AeroDms::editerAeroclub()
{
    dialogueGestionAeroclub->ouvrirFenetre(true);
}

void AeroDms::editerCotisation()
{
    dialogueAjouterCotisation->editerLaCotisation( piloteAEditer, 
                                                   anneeAEditer,
                                                   montantSubventionDejaAlloue);
    dialogueAjouterCotisation->exec();
}

void AeroDms::menuContextuelSubvention(const QPoint& pos)
{
    if (vueSubventions->itemAt(pos) != nullptr)
    {
        QMenu menuClicDroitSubvention(tr("Menu contextuel"), this);
        const int ligneSelectionnee = vueSubventions->itemAt(pos)->row();
        subventionAAnoter.id = vueSubventions->item(ligneSelectionnee, 
            AeroDmsTypes::SubventionDemandeeTableElement_ID_DEMANDE)->text().toInt();
        subventionAAnoter.texteActuel = vueSubventions->item(ligneSelectionnee,
            AeroDmsTypes::SubventionDemandeeTableElement_NOTE)->text();

        QAction ajouterNote(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_AJOUTER_NOTE),
            tr("Ajouter une note"),
            this);
        connect(&ajouterNote, SIGNAL(triggered()), this, SLOT(ajouterUneNoteSubvention()));
        menuClicDroitSubvention.addAction(&ajouterNote);

        if (logicielEnModeLectureSeule)
        {
            menuClicDroitSubvention.setEnabled(false);
        }

        //Afficher le menu sur la vue des pilotes
        menuClicDroitSubvention.exec(vueSubventions->mapToParent(QCursor::pos()));
    }
}

void AeroDms::ajouterUneNoteSubvention()
{
    QInputDialog dialogueNote;
    dialogueNote.setInputMode(QInputDialog::TextInput);
    dialogueNote.setWindowTitle(QApplication::applicationName() + " - " + tr("Ajout d'une note sur une subvention demandée"));
    dialogueNote.setLabelText(tr("Cette note est destinée à un usge interne et permet de tracer par exemple à\nqui a été remis le chèque s'il n'a pas été remis en direct à son destinataire.\n\nNote à ajouter à cette demande de subvention :"));

    dialogueNote.setTextValue(subventionAAnoter.texteActuel);
    
    switch (dialogueNote.exec())
    {
        case QDialog::Accepted:
        {
            db->ajouterNoteSubvention(subventionAAnoter.id, dialogueNote.textValue());

            //On repeuple la table des subventions demandées pour afficher la nouvelle note
            peuplerTableSubventionsDemandees();

            db->demanderEnvoiBdd();
            break;
        }
        case QDialog::Rejected:
        default:
        {
            //On ne fais rien
            break;
        }
    }
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

            statusBar()->showMessage(tr("Vols sélectionnés : Coût total : ")
                + QString::number(coutTotal, 'f', 2)
                + tr(" € / Montant subventionné total : ")
                + QString::number(montantTotalSubventionne, 'f', 2)
                + tr(" € / Nombres d'heures de vol totales : ")
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

        QAction editerLeVol(QIcon(":/AeroDms/ressources/airplane-edit.svg"), 
            tr("Éditer le vol"), 
            this);
        connect(&editerLeVol, SIGNAL(triggered()), this, SLOT(editerVol()));
        menuClicDroitVol.addAction(&editerLeVol);

        QAction supprimerLeVol(QIcon(":/AeroDms/ressources/airplane-remove.svg"), 
            tr("Supprimer le vol"), 
            this);
        connect(&supprimerLeVol, SIGNAL(triggered()), this, SLOT(supprimerVol()));
        menuClicDroitVol.addAction(&supprimerLeVol);
        supprimerLeVol.setEnabled(leVolEstSupprimable);

        QAction soumettreLeVolAuCsePlusTard(QIcon(":/AeroDms/ressources/airplane-remove.svg"),
            tr("Marquer/démarquer le vol comme à ne pas soumettre au CSE"),
            this);
        connect(&soumettreLeVolAuCsePlusTard, SIGNAL(triggered()), this, SLOT(switchMarquageVolASoumettrePlusTard()));
        menuClicDroitVol.addAction(&soumettreLeVolAuCsePlusTard);
        soumettreLeVolAuCsePlusTard.setEnabled(leVolEstSupprimable);

        if (logicielEnModeLectureSeule)
        {
            editerLeVol.setEnabled(false);
            supprimerLeVol.setEnabled(false);
            soumettreLeVolAuCsePlusTard.setEnabled(false);
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

    //On récupère les infos du vol pour les réintégrer dans l'IHM
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

    //On masque l'éventuelle table des infos de vol récupéré automatiquement de la facture précédemment chargée
    validerLesVols->setHidden(true);
    vueVolsDetectes->setHidden(true);
    supprimerLeVolSelectionne->setHidden(true);
}

void AeroDms::supprimerVol()
{
    QMessageBox demandeConfirmationSuppression;
    demandeConfirmationSuppression.setText(tr("Voulez vous réellement supprimer le vol ?"));
    demandeConfirmationSuppression.setWindowTitle(QApplication::applicationName() + " - " + tr("Suppression d'un vol"));
    demandeConfirmationSuppression.setIcon(QMessageBox::Question);
    demandeConfirmationSuppression.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    
    const int ret = demandeConfirmationSuppression.exec();
    
    switch (ret) 
    {
        case QMessageBox::Yes:
        {
            if (db->supprimerUnVol(volAEditer))
            {
                statusBar()->showMessage(tr("Vol supprimé avec succès"));

                peuplerTablePilotes();
                peuplerTableVols();

                db->demanderEnvoiBdd();
            }
            else
            {
                statusBar()->showMessage(tr("Vol non supprimé : le vol est associé à une recette d'une sortie. Suppression impossible."));
            }
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
    volAEditer = AeroDmsTypes::K_INIT_INT_INVALIDE;
}

void AeroDms::switchMarquageVolASoumettrePlusTard()
{
    db->switcherVolANePasSoumettreAuCse(volAEditer);
    //On sort du mode suppression de vol
    volAEditer = AeroDmsTypes::K_INIT_INT_INVALIDE;

    peuplerTableVols();
}

void AeroDms::switchModeDebug()
{
    bool masquageEstDemande = false;

    //Si le mode debug n'est pas actif
    if (boutonModeDebug->text() == texteBoutonActiverModeDebogage)
    {
        boutonModeDebug->setText("Désactiver le mode &debug");
        boutonModeDebug->setIcon(QIcon(":/AeroDms/ressources/bug-stop.svg"));
        masquageEstDemande = false;
    }
    //Sinon, le mode est actif, on désactive
    else
    {
        boutonModeDebug->setText(texteBoutonActiverModeDebogage);
        boutonModeDebug->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_DEBUG));
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
    mettreAJourDonneesVersionMiniAction->setVisible(!masquageEstDemande);

    //fonction de mise à jour update locale non nécessaire en mode externe à l'extérieur de l'organisation
    if (parametresSysteme.modeFonctionnementLogiciel != AeroDmsTypes::ModeFonctionnementLogiciel_EXERNE_AUTORISE_MODE_EXTERNE)
    {
        miseAJourUpdateLocale->setVisible(!masquageEstDemande);
    }
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

void AeroDms::switchAffichageInfosComplementaires()
{
    vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_AEROCLUB, !selecteurFiltreInformationsSupplementaires->isChecked());
    vuePilotes->setColumnHidden(AeroDmsTypes::PiloteTableElement_TERRAIN, !selecteurFiltreInformationsSupplementaires->isChecked());
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

void AeroDms::enregistrerParametresApplication( const AeroDmsTypes::ParametresMetier p_parametresMetiers,
                                                const AeroDmsTypes::ParametresSysteme p_parametresSysteme)
{
    parametresMetiers = p_parametresMetiers;
    parametresSysteme = p_parametresSysteme;

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(), QCoreApplication::applicationName());

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
    settings.setValue("texteVirementSubvention", parametresMetiers.texteMailVirementSubvention);

    settings.setValue("objetChequesDisponibles", parametresMetiers.objetMailDispoCheques);
    settings.setValue("objetSubventionRestante", parametresMetiers.objetMailSubventionRestante);
    settings.setValue("objetVirementSubvention", parametresMetiers.objetMailVirementSubvention);
    settings.setValue("objetAutresMailings", parametresMetiers.objetMailAutresMailings);
    settings.endGroup();

    settings.beginGroup("impression");
    settings.setValue("imprimante", parametresSysteme.parametresImpression.imprimante);
    settings.setValue("couleur", parametresSysteme.parametresImpression.modeCouleurImpression);
    settings.setValue("resolution", parametresSysteme.parametresImpression.resolutionImpression);
    settings.setValue("forcageImpressionRecto", parametresSysteme.parametresImpression.forcageImpressionRecto);
    settings.endGroup();

    settings.beginGroup("modeFonctionnementLogiciel");
    settings.setValue("modeFonctionnement", parametresSysteme.modeFonctionnementLogiciel);
    settings.setValue("adresse", parametresSysteme.adresseServeurModeExterne);
    settings.setValue("login", parametresSysteme.loginServeurModeExterne);
    settings.setValue("password", AeroDmsServices::chiffrerDonnees(parametresSysteme.motDePasseServeurModeExterne));
    settings.setValue("motsDePasseAChiffrer", 0);
    settings.endGroup();

    settings.beginGroup("siteDaca");
    settings.setValue("login", parametresSysteme.loginSiteDaca);
    settings.setValue("password", AeroDmsServices::chiffrerDonnees(parametresSysteme.motDePasseSiteDaca));
    settings.setValue("periodiciteVerification", parametresSysteme.periodiciteVerificationPresenceFactures);
    settings.endGroup();

    parametresMetiers.texteMailDispoCheques = settings.value("mailing/texteChequesDisponibles", "").toString();
    parametresMetiers.texteMailSubventionRestante = settings.value("mailing/texteSubventionRestante", "").toString();

    //Fichier de conf commun => le fichier AeroDMS.ini est mis au même endroit que la BDD SQLite
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, settings.value("baseDeDonnees/chemin", "").toString() + QString("/"));
    QSettings settingsMetier(QSettings::IniFormat, QSettings::SystemScope, "AeroDMS");
    settingsMetier.beginGroup("parametresSysteme");
    settingsMetier.setValue("delaisDeGardeDbEnMs", parametresMetiers.delaisDeGardeBdd);
    settingsMetier.setValue("margesHautBas", parametresSysteme.margesHautBas);
    settingsMetier.setValue("margesGaucheDroite", parametresSysteme.margesGaucheDroite);
    settingsMetier.setValue("utiliserRessourcesHtmlInternes", parametresSysteme.utiliserRessourcesHtmlInternes);
    settingsMetier.endGroup();

    //On sauvegarde les paramètres qu'on enregistre en BDD
    db->enregistrerParametres(parametresMetiers, parametresSysteme);
    db->demanderEnvoiBdd();

    //On met à jour les marges...
    pdf->mettreAJourMarges(QMarginsF(parametresSysteme.margesGaucheDroite,
            parametresSysteme.margesHautBas,
            parametresSysteme.margesGaucheDroite,
            parametresSysteme.margesHautBas));
    //... et le chemin des templates
    pdf->mettreAJourCheminRessourcesHtml(elaborerCheminRessourcesHtml());

    verifierDispoIdentifiantsDaca();
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

        if (action->data().canConvert<AeroDmsTypes::DonneesMailing>()) {
            AeroDmsTypes::DonneesMailing donnnesMailing = action->data().value<AeroDmsTypes::DonneesMailing>();
            switch (donnnesMailing.typeMailing)
            {
            case AeroDmsTypes::DonnesMailingType_DEMANDE_DE_SUBVENTION:
            {
                QDesktopServices::openUrl(QUrl("mailto:"
                    + db->recupererMailDerniereDemandeDeSubvention(donnnesMailing.donneeComplementaire)
                    + "?subject=" + parametresMetiers.objetMailDispoCheques + "&body="
                    + parametresMetiers.texteMailDispoCheques, QUrl::TolerantMode));
            }
            break;
            case AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_D_UN_AERODROME:
            {
                QDesktopServices::openUrl(QUrl("mailto:"
                    + db->recupererMailPilotesDUnAerodrome(donnnesMailing.donneeComplementaire, 
                        AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_D_UN_AERODROME)
                    + "?subject=" + parametresMetiers.objetMailAutresMailings + "&body=", QUrl::TolerantMode));
            }
            break;
            case AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_BREVETES_VOL_MOTEUR_D_UN_AERODROME:
            {
                QDesktopServices::openUrl(QUrl("mailto:"
                    + db->recupererMailPilotesDUnAerodrome(donnnesMailing.donneeComplementaire, 
                        AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_BREVETES_VOL_MOTEUR_D_UN_AERODROME)
                    + "?subject=" + parametresMetiers.objetMailAutresMailings + "&body=", QUrl::TolerantMode));
            }
            break;
            case AeroDmsTypes::DonnesMailingType_SUBVENTION_VERSEE_PAR_VIREMENT:
            {
                const AeroDmsTypes::ListeMailsEtVirements listeVirements = db->recupererMailsVirements(donnnesMailing.donneeComplementaire);
                for (int pilote = 0; pilote < listeVirements.size(); pilote++)
                {
                    QString stringListeVirement = "";
                    QString stringListeVols = "";
                    for (int virement = 0; virement < listeVirements.at(pilote).listeMontantsVirements.size(); virement++)
                    {
                        stringListeVirement = stringListeVirement 
                            + tr("Montant virement ") 
                            + QString::number(virement + 1) 
                            + " : " 
                            + QString::number(listeVirements.at(pilote).listeMontantsVirements.at(virement), 'f', 2)
                            + "€\n";
                        const AeroDmsTypes::ListeVols listeVols = db->recupererVolsParDemandeDeRemboursement(listeVirements.at(pilote).listeIdVirements.at(virement));
                        qDebug() << listeVirements.at(pilote).mail << listeVirements.at(pilote).listeIdVirements.at(virement);
                        for (const AeroDmsTypes::Vol vol : listeVols)
                        {
                            stringListeVols = stringListeVols + "\t\tVol du " + vol.date.toString("dd/MM/yyyy") + " d'une durée de " + vol.duree + " pour un coût de vol de " + QString::number(vol.coutVol, 'f', 2) + "€ (subvention allouée pour ce vol : "+ QString::number(vol.montantRembourse, 'f', 2)  + "€)\n";
                        }
                    }
                    stringListeVirement = stringListeVirement + tr("\nPour information : subvention entrainement restante pour l'année ");
                    stringListeVirement = stringListeVirement + QString::number(QDate::currentDate().year());
                    stringListeVirement = stringListeVirement + " : ";
                    stringListeVirement = stringListeVirement + QString::number(db->recupererSubventionRestante(listeVirements.at(pilote).pilote, QDate::currentDate().year()), 'f', 2);
                    stringListeVirement = stringListeVirement + " €";
                    if (!db->piloteEstAJourDeCotisation(listeVirements.at(pilote).pilote, QDate::currentDate().year()))
                    {
                        stringListeVirement = stringListeVirement + tr(" (cotisation non payée pour l'année ");
                        stringListeVirement = stringListeVirement + QString::number(QDate::currentDate().year());
                        stringListeVirement = stringListeVirement + ")";
                    }
                    stringListeVirement = stringListeVirement + ".\n\n";

                    stringListeVirement = stringListeVirement + "Ce remboursement couvre le ou les vol(s) suivant(s) :\n" + stringListeVols;

                    QString texteMail = parametresMetiers.texteMailVirementSubvention;
                    texteMail = texteMail.replace("#listeVirements", stringListeVirement);
                    QDesktopServices::openUrl(QUrl("mailto:"
                        + listeVirements.at(pilote).mail
                        + "?subject=" + parametresMetiers.objetMailVirementSubvention + "&body=" + texteMail, QUrl::TolerantMode));
                    QThread::sleep(1.0);
                }
            }
            break;
            }

        }
    }
}

const bool AeroDms::uneMaJEstDisponible(const QString p_chemin, const QString p_fichier)
{ 
    const QString cheminFichierDistant = p_chemin + p_fichier;
    const QString cheminFichierLocal = QCoreApplication::applicationDirPath() + "/" + p_fichier;

    if (QFile().exists(cheminFichierDistant))
    {
        //Si le fichier existant en distant, mais pas en local, alors
        //il faut mettre à jour
        if (!QFile().exists(cheminFichierLocal))
        {
            return true;
        }

        //Sinon, on calcul les checksum pour voir s'il faut mettre à jour
        QFile fichierDistant(cheminFichierDistant);
        if (fichierDistant.open(QFile::ReadOnly))
        {
            QString hashFichierDistant = "";
            QCryptographicHash hash(QCryptographicHash::Sha1);
            if (hash.addData(&fichierDistant))
            {
                hashFichierDistant = hash.result().toHex();
            }
            hash.reset();

            //Calcul de la somme de contrôle du fichier courant
            QFile fichierLocal(cheminFichierLocal);

            if (fichierLocal.open(QFile::ReadOnly))
            {
                QString hashFichierCourant = "";
                if (hash.addData(&fichierLocal))
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
    QProgressDialog* progressionMiseAJour = new QProgressDialog(QApplication::applicationName() + " - " + tr("Mise à jour"), tr("Vérification des fichiers a mettre à jour..."), 0, 0, this);
    QPushButton* boutonProgressionMiseAJour = new QPushButton("Quitter AeroDMS", this);
    boutonProgressionMiseAJour->setDisabled(true);
    progressionMiseAJour->setCancelButton(boutonProgressionMiseAJour);
    progressionMiseAJour->setAutoClose(false);
    progressionMiseAJour->setWindowModality(Qt::WindowModal);
    progressionMiseAJour->close();
    progressionMiseAJour->setAutoReset(false);
    progressionMiseAJour->setMinimumSize(QSize(300, 150));
    connect(boutonProgressionMiseAJour, &QPushButton::clicked, this, &QCoreApplication::quit);

    int nombreDeFichiers = 0;
    progressionMiseAJour->setMaximum(1);
    progressionMiseAJour->setValue(0);
    progressionMiseAJour->show();

    const QRegularExpression ini("^.*\\.ini$");
    const QRegularExpression sqlite("^.*\\.sqlite$");
    const QRegularExpression pdf("^.*\\.pdf$");
    const QRegularExpression signature("^.*signature\.(jpg|png|jpeg|svg)$");

    //1) On fait du ménage : tous les fichiers locaux qui ne sont 
    //   ni .sqlite ni .ini ni .pdf ni signature.jpg/png/jpeg/svf 
    //   sont préfixés en xxx_
    //   Ils seront supprimés au lancement suivant d'AeroDMS
    {
        QDirIterator it("./",
            QStringList() << "*",
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            QString fichier = it.next();
            //On ne replace pas les éventuels fichiers .ini, .pdf, .sqlite ou signature 
            //qui seraient présents dans le répertoire d'update
            if (!fichier.contains(ini)
                && !fichier.contains(sqlite)
                && !fichier.contains(pdf)
                && !fichier.contains(signature))
            {
                QFileInfo infosFichierLocal(fichier);
                QString nouveauNomFichierLocal = infosFichierLocal.path() + "/xxx_" + infosFichierLocal.fileName();

                QFile::rename(fichier, nouveauNomFichierLocal);
            }
        }
    }

    //2) Comptage des fichiers a recopier
    {
        QDirIterator it(p_chemin,
            QStringList() << "*",
            QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        
        while (it.hasNext())
        {
            it.next();
            nombreDeFichiers++;
        }
    }
    
    progressionMiseAJour->setMaximum(nombreDeFichiers);
    progressionMiseAJour->setValue(0);

    //3) on effectue la recopie
    //Pas de fonction reset de l'itérateur dans QDirIterator... on redémarre via un nouveau scope
    {
        int etapeMiseAJour = 0;

        QDirIterator it(p_chemin, 
            QStringList() << "*", 
            QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot, 
            QDirIterator::Subdirectories);

        //Pour chaque élément présente dans p_chemin, on vérifie s'il existe dans l'arborescence locale
        //Si c'est le cas on renomme le fichier local en suffixant par xxx_
        //Ensuite dans tous les cas on recopie le fichier distant vers le dossier local
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

                //Si c'est un dossier, on vérifie son existence, et on le créé si absent.
                if (it.fileInfo().isDir())
                {
                    if (!QDir(fichierLocal).exists())
                    {
                        QDir().mkdir(fichierLocal);
                    }
                }
                //Sinon, si c'est un fichier, on renomme le précédent et on copie le nouveau
                else if (it.fileInfo().isFile())
                {
                    //QFileInfo infosFichierLocal(fichierLocal);
                    //QString nouveauNomFichierLocal = infosFichierLocal.path() + "/xxx_" + infosFichierLocal.fileName();

                    //QFile::rename(fichierLocal, nouveauNomFichierLocal);
                    QFile::copy(fichierDistant, fichierLocal);
                }

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
    //Cette méthode termine une mise à jour précédemment démarrée : elle supprime l'ensemble des fichiers xxx_*
    QString dossierCourantApplication = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();

    QDirIterator it(dossierCourantApplication, QStringList() << "xxx_*", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString cheminFichier = it.next();
        QFile fichier(cheminFichier);
        fichier.remove();
    }

    //Si on est en mode externe, on fait du ménage
    if (parametresSysteme.modeFonctionnementLogiciel == AeroDmsTypes::ModeFonctionnementLogiciel_EXERNE_AUTORISE_MODE_EXTERNE)
    {
        QFile::remove(parametresSysteme.cheminStockageBdd + "/update.zip");

        QDir repertoire(parametresSysteme.cheminStockageBdd + "/update/");
        repertoire.removeRecursively();
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
        QMessageBox::warning(this, QApplication::applicationName() + " - " + tr("Fichier non trouvé"),
                                   tr("Fichier non trouvé\n"
                                      "Aucun fichier fusionné de demande de subvention trouvé."), QMessageBox::Cancel);
    }
}

const QString AeroDms::rechercherDerniereDemande()
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
            //Indice nAeroDmsTypes::K_INIT_INT_INVALIDEet n-2 contiennent . et ..
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
        //les 2 dernières cases contiennent . et ..
        while (i < fichiers.size() - 2 && i < 10)
        {
            QMenu *menuFichier = menuOuvrirAutreDemande->addMenu(QIcon(":/AeroDms/ressources/folder.svg"), fichiers.at(i));
            const QString dossier = parametresSysteme.cheminSortieFichiersGeneres + "/" + fichiers.at(i) + "/";
            const QDir dirCourant(dossier);
            const QStringList listeFichiers = dirCourant.entryList(QStringList("*.pdf"));
            if (listeFichiers.size() > 0)
            {
                QAction* actionDossier = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_OUVRIR_DOSSIER), 
                    tr("Ouvrir le dossier"), 
                    this);
                actionDossier->setData(dossier);
                menuFichier->addAction(actionDossier);
                menuFichier->addSeparator();
                connect(actionDossier, &QAction::triggered, this, &AeroDms::ouvrirUnFichierDeDemandeDeSubvention);

                //Boucle While pour ajouter l'accès à chaque fichier
                int idFichier = 0;
                while (idFichier < listeFichiers.size())
                {
                    QAction* action = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FICHIER), 
                        listeFichiers.at(idFichier), 
                        this);
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
    if (idFactureDetectee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        idFactureDetectee = AeroDmsTypes::K_INIT_INT_INVALIDE;
        //On rince les données de vol
        dureeDuVol->setTime(QTime::QTime(0, 0));
        prixDuVol->setValue(0);
        remarqueVol->clear();
        immat->clear();
        //La mise à jour de ces données provoque la réélaboration de l'état des boutons de validation => a faire
        //impérativement après le rinçage de idFactureDetectee car cette donnée ne redéclenche pas ce traitement

        vueVolsDetectes->clearSelection();
        supprimerLeVolSelectionne->setEnabled(false);

        statusBar()->clearMessage();
    } 
}

const bool AeroDms::lePiloteEstAJourDeCotisation()
{
    const int anneeRenseignee = dateDuVol->date().year();
    const QString idPilote = choixPilote->currentData().toString();

    //On vérifie si le pilote est a jour de sa cotisation pour l'année du vol
    if (!db->piloteEstAJourDeCotisation(idPilote, anneeRenseignee))
    {
        QMessageBox::critical(this, QApplication::applicationName() + " - " + tr("Pilote non à jour de cotisation"),
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
    selecteurFiltreInformationsSupplementaires->setVisible(false);
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

    if (mainTabWidget->currentWidget() == vuePilotes)
    {
        selecteurFiltreInformationsSupplementaires->setVisible(true);
    }
    else if (mainTabWidget->currentWidget() == vueVols)
    {
        actionListeDeroulanteElementsSoumis->setVisible(true);
        actionListeDeroulanteType->setVisible(true);
    }
    else if (mainTabWidget->currentWidget() == vueSubventions)
    {
        ajusterTableSubventionsDemandeesAuContenu();
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

    //On gère aussi le bouton d'édition pilote. De base on désactive, et dans 
    //les onglets qui permettent l'édition des pilotes, on appelle la méthode
    //qui gère le bouton associé
    boutonEditerLePiloteSelectionne->setEnabled(false);
    if (mainTabWidget->currentWidget() == vuePilotes
        || mainTabWidget->currentWidget() == vueVols
        || mainTabWidget->currentWidget() == vueSubventions
        || mainTabWidget->currentWidget() == vueFactures
        || mainTabWidget->currentWidget() == vueRecettes
        || mainTabWidget->currentWidget() == widgetAjoutVol )
    {
        gererBoutonEditionPilote();
    }
}

void AeroDms::imprimerLaDerniereDemande()
{
    fichierAImprimer = rechercherDerniereDemande();
    imprimerLaDemande();
}

void AeroDms::imprimerLaDemande()
{
    PdfPrinter impression;
    const AeroDmsTypes::EtatImpression etatImpression = impression.imprimerFichier(fichierAImprimer, parametresSysteme.parametresImpression);
    afficherEtatImpression(etatImpression);

    detruireFenetreProgressionGenerationPdf();
}

void AeroDms::imprimerLaDerniereDemandeAgrafage()
{
    dossierSortieGeneration = QFileInfo(rechercherDerniereDemande()).absolutePath();
    imprimerLaDemandeAgrafage();
}

void AeroDms::imprimerLaDemandeAgrafage()
{
    PdfPrinter impression;

    const AeroDmsTypes::EtatImpression etatImpression = impression.imprimerDossier(dossierSortieGeneration, parametresSysteme.parametresImpression);
    afficherEtatImpression(etatImpression);

    detruireFenetreProgressionGenerationPdf();
}

void AeroDms::afficherEtatImpression(const AeroDmsTypes::EtatImpression p_etatImpression)
{
    switch (p_etatImpression)
    {
        case AeroDmsTypes::EtatImpression_TERMINEE:
        {
            statusBar()->showMessage(tr("Impression terminée avec succès"));
        }
        break;
        case AeroDmsTypes::EtatImpression_ANNULEE_PAR_UTILISATEUR:
        default:
        {
            statusBar()->showMessage(tr("Impression annulée par l'utilisateur"));
        }
        break;
    }
}

const int AeroDms::calculerValeurGraphAGenererPdf()
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
    if (boutonGraphRecapAnnuelEurosParPilote->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_EUROS_PAR_PILOTE;
    }
    if (boutonGraphRecapAnnuelHeuresParTypeDeVol->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL;
    }
    if (boutonGraphRecapAnnuelEurosParTypeDeVol->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_EUROS_PAR_TYPE_DE_VOL;
    }
    if (boutonGraphRecapAnnuelHeuresParActivite->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE;
    }
    if (boutonGraphRecapAnnuelEurosParActivite->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Statistiques_EUROS_PAR_ACTIVITE;
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

    if (boutonGraphRatio16x9->isChecked())
    {
        valeur = valeur + AeroDmsTypes::Resolution_RATIO_16_9;
    }
    //Sinon par défaut le ratio d'une page A4.

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

void AeroDms::gererChargementDonneesSitesExternes(const AeroDmsTypes::EtatRecuperationDonneesFactures p_etatRecuperation)
{
    switch(p_etatRecuperation)
    {
        case AeroDmsTypes::EtatRecuperationDonneesFactures_CONNEXION_EN_COURS:
        {
            statusBar()->showMessage(tr("Connexion en cours au site du DACA"));
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonneesFactures_CONNECTE:
        {

        }
        break;
        case AeroDmsTypes::EtatRecuperationDonnnesFactures_ECHEC_CONNEXION:
        {
            statusBar()->showMessage(tr("Connexion impossible au site du DACA"));
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonnnesFactures_ECHEC_CONNEXION_SSL_TLS_INDISPONIBLE:
        {
            statusBar()->showMessage(tr("Connexion impossible au site du DACA. La connexion SSL/TLS n'est pas disponible avec votre version de Qt."));
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonneesFactures_RECUPERATION_DONNEES_EN_COURS:
        {
            statusBar()->showMessage(tr("Récupération de la liste des factures disponibles sur le site du DACA en cours"));
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonneesFactures_DONNEES_RECUPEREES:
        {
            AeroDmsTypes::DonneesFacturesDaca donneesDaca = pdfdl->recupererDonneesDaca();
            if (donneesDaca.listeMoisAnnees.size() > 0
                && (donneesDaca.listePilotes.size() > 0
                    || donneesDaca.listePilotesNonConnus.size() > 0))
            {
                boutonChargerFacturesDaca->setVisible(false);
                statusBar()->showMessage(tr("Liste des factures disponibles récupérée sur le site du DACA. Factures téléchargeables via le menu \"Outils/") 
                    + texteTitreQMenuFacturesDaca
                    + "\". Facture la plus récente disponible : "
                    + QLocale::system().toString(donneesDaca.listeMoisAnnees.at(0), "MMMM yyyy")
                    + ".");

                for (QDate mois : donneesDaca.listeMoisAnnees)
                {
                    if (mois.isValid())
                    {
                        QString date = QLocale::system().toString(mois, "MMMM yyyy");
                        date[0] = date[0].toUpper();
                        QMenu* menu = facturesDaca->addMenu(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TELECHARGER_DOSSIER),
                            date);

                        ajouterPilotesDansMenuFacturesDaca(menu, 
                            donneesDaca.listePilotes,
                            mois);

                        menu->addSeparator();

                        ajouterPilotesDansMenuFacturesDaca(menu, 
                            donneesDaca.listePilotesNonConnus, 
                            mois);
                    }
                }

                QSettings settingsDaca(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(), "DACA");
                const QDate derniereVerificationDaca = settingsDaca.value("DACA/dateDerniereVerification", QDate::currentDate().toString("yyyy-MM-dd")).toDate();
                const bool estEnVerificationAutomatiqueDeNouvelleFacture = derniereVerificationDaca.daysTo(QDate::currentDate()) >= parametresSysteme.periodiciteVerificationPresenceFactures;
                const QDate dateDernierFactureDaca = settingsDaca.value("DACA/valeurDerniereVerification", QDate::currentDate().toString("yyyy-MM-dd")).toDate();

                if (estEnVerificationAutomatiqueDeNouvelleFacture
                    && donneesDaca.listeMoisAnnees.at(0) > dateDernierFactureDaca)
                {
                    QString article = "de ";
                    if (QLocale::system().toString(donneesDaca.listeMoisAnnees.at(0), "MMMM yyyy").at(0) == "a"
                        || QLocale::system().toString(donneesDaca.listeMoisAnnees.at(0), "MMMM yyyy").at(0) == "o")
                    {
                        article = "d'";
                    }
                    QMessageBox info(QMessageBox::Information, "", "", QMessageBox::Ok);
                    info.setWindowTitle(QApplication::applicationName() + " - " + tr("Nouvelles factures DACA disponibles"));
                    info.setText(tr("De nouvelles factures pour le DACA sont disponibles pour le mois ")
                        + article
                        + QLocale::system().toString(donneesDaca.listeMoisAnnees.at(0), "MMMM yyyy")
                        + ".\n\n"
                        + tr("Vous pouvez les télécharger via le menu \"Outils/")
                        + texteTitreQMenuFacturesDaca
                        + "\".\n\n"
                        + tr("Cette notification ne sera plus affichée tant que de nouvelles factures ne seront pas disponibles."));
                    info.setStandardButtons(QMessageBox::NoButton);
                    QAbstractButton *chargerFacture = info.addButton(" Ouvrir la première facture " + article + QLocale::system().toString(donneesDaca.listeMoisAnnees.at(0), "MMMM yyyy "), QMessageBox::YesRole);
                    QAbstractButton* neRienFaire = info.addButton(" Ne rien faire pour le moment ", QMessageBox::NoRole);
                    connect(chargerFacture, SIGNAL(pressed()), this, SLOT(demanderTelechargementPremiereFactureDaca()));
                    info.exec();
                }
                else if (estEnVerificationAutomatiqueDeNouvelleFacture)
                {
                    statusBar()->showMessage(tr("Vérification automatique de nouvelles factures DACA effectuée. Pas de nouvelle facture. Factures téléchargeables via le menu \"Outils/")
                        + texteTitreQMenuFacturesDaca
                        + "\". Facture la plus récente disponible : "
                        + QLocale::system().toString(donneesDaca.listeMoisAnnees.at(0), "MMMM yyyy")
                        + ".");
                }

                //Systématiquement, on met à jour les données de dernière vérification
                settingsDaca.beginGroup("DACA");
                settingsDaca.setValue("dateDerniereVerification", QDate::currentDate().toString("yyyy-MM-dd"));
                settingsDaca.setValue("valeurDerniereVerification", donneesDaca.listeMoisAnnees.at(0).toString("yyyy-MM-dd"));
                settingsDaca.endGroup();
            }
            else
            {
                statusBar()->showMessage(tr("La liste des pilotes ou la liste des dates récupérées sur le site du DACA est vide. Impossible d'afficher la liste des factures disponibles."));
            }
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_DONNEES:
        {
            statusBar()->showMessage(tr("Impossible de récupérer les données depuis le site du DACA"));
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonneesFactures_RECUPERATION_FACTURE_EN_COURS:
        {
            statusBar()->showMessage(tr("Téléchargement en cours de la facture"));
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonneesFactures_FACTURE_RECUPEREE :
        {
            chargerUneFactureAvecScan(pdfdl->recupererCheminDernierFichierTelecharge(), true);

            choixPilote->setCurrentIndex(choixPilote->findData(pdfdl->recupererIdentifiantFactureTelechargee().idPilote));
            mettreAJourBoutonsFichierSuivantPrecedent();
            statusBar()->showMessage(tr("Facture téléchargée avec succès. ") + texteSubventionRestante());
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_FACTURE:
        {
            statusBar()->showMessage(tr("Impossible de récupérer la facture"));
        }
        break;
        case AeroDmsTypes::EtatRecuperationDonneesFactures_AUCUN: 
        default:
        {

        }
        break;
    }     
}

void AeroDms::ajouterPilotesDansMenuFacturesDaca(QMenu *p_menu, 
    const AeroDmsTypes::ListeCleStringValeur p_listePilote, 
    const QDate p_mois)
{
    for (AeroDmsTypes::CleStringValeur pilote : p_listePilote)
    {
        QAction* action = new QAction(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_TELECHARGER_FICHIER),
            pilote.texte,
            p_menu);
        AeroDmsTypes::IdentifiantFacture identifiant;
        identifiant.moisAnnee = p_mois;
        identifiant.pilote = pilote.cle;
        identifiant.idPilote = pilote.idPilote;
        identifiant.nomPrenomPilote = pilote.texte;
        action->setData(QVariant::fromValue(identifiant));
        action->setCheckable(true);
        p_menu->addAction(action);

        connect(action, SIGNAL(triggered()), this, SLOT(demanderTelechargementFactureDaca()));
    }
}

void AeroDms::demanderTelechargementPremiereFactureDaca()
{
    fichierPrecedent->setDisabled(true);
    fichierSuivant->setDisabled(true);

    actionFactureDacaEnCours = facturesDaca->actions().at(1)->menu()->actions().at(0);
    actionFactureDacaEnCours->setChecked(true);
    actionFactureDacaEnCours->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FICHIER_TELECHARGE));
    AeroDmsTypes::IdentifiantFacture identifiant = actionFactureDacaEnCours->data().value<AeroDmsTypes::IdentifiantFacture>();

    pdfdl->telechargerFactureDaca(parametresSysteme.loginSiteDaca, parametresSysteme.motDePasseSiteDaca, identifiant);
}

void AeroDms::demanderTelechargementFactureDaca()
{
    fichierPrecedent->setDisabled(true);
    fichierSuivant->setDisabled(true);

    QAction* action = static_cast<QAction*>(sender());
    action->setChecked(true);
    action->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FICHIER_TELECHARGE));

    if (action->data().canConvert<AeroDmsTypes::IdentifiantFacture>()) 
    {
        actionFactureDacaEnCours = action;
        AeroDmsTypes::IdentifiantFacture identifiant = action->data().value<AeroDmsTypes::IdentifiantFacture>();

        pdfdl->telechargerFactureDaca(parametresSysteme.loginSiteDaca, parametresSysteme.motDePasseSiteDaca, identifiant);
    }
    else 
    {
        statusBar()->showMessage(tr("Identifiant invalide. Impossible de télécharger la facture"));
    }

    mettreAJourBoutonsFichierSuivantPrecedent();
}

void AeroDms::demanderTelechagementFactureSuivanteOuPrecedente()
{
    //On desactive les boutons pendant le téléchargement
    fichierPrecedent->setDisabled(true);
    fichierSuivant->setDisabled(true);

    QList<QAction*> actions = static_cast<QMenu*>((actionFactureDacaEnCours->parent()))->actions();
    int actionCourante = 0;
    bool actionTrouvee = false;
    
    while(actionCourante < actions.size() 
        && !actionTrouvee)
    {
        if (actions.at(actionCourante) != actionFactureDacaEnCours)
        {
            actionCourante = actionCourante + 1;
        }
        else
        {
            actionTrouvee = true;
        }
    }

    int actionACharger = AeroDmsTypes::K_INIT_INT_INVALIDE;
    if (sender() == fichierSuivant)
    {
        if (actions.at(actionCourante+1)->data().canConvert<AeroDmsTypes::IdentifiantFacture>())
        {
            actionACharger = actionCourante + 1;
        }
        else if ((actionCourante + 2) < actions.size()-1) // On est sur un séparateur, on fait + 2, si c'est possible
        {
            actionACharger = actionCourante + 2;
        }
        //Sinon -1, valeur d'init
    }
    else //sender == fichierPrecedent
    {
        if (actions.at(actionCourante - 1)->data().canConvert<AeroDmsTypes::IdentifiantFacture>())
        {
            actionACharger = actionCourante - 1;
        }
        else if ((actionCourante - 2) > 0) // On est sur un séparateur, on fait - 2, si c'est possible
        {
            actionACharger = actionCourante - 2;
        }
        //Sinon -1, valeur d'init
    }
    
    if (actionACharger != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        actionFactureDacaEnCours = actions.at(actionACharger);
        AeroDmsTypes::IdentifiantFacture identifiant = actions.at(actionACharger)->data().value<AeroDmsTypes::IdentifiantFacture>();
        actions.at(actionACharger)->setChecked(true);
        actions.at(actionACharger)->setIcon(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_FICHIER_TELECHARGE));

        pdfdl->telechargerFactureDaca(parametresSysteme.loginSiteDaca, parametresSysteme.motDePasseSiteDaca, identifiant);
    }
}

void AeroDms::chargerListeFacturesDaca()
{
    pdfdl->telechargerDonneesDaca(parametresSysteme.loginSiteDaca, parametresSysteme.motDePasseSiteDaca);
}

void AeroDms::mettreAJourBoutonsFichierSuivantPrecedent()
{
    QList<QAction*> actions = static_cast<QMenu*>((actionFactureDacaEnCours->parent()))->actions();

    fichierPrecedent->setVisible(true);
    fichierSuivant->setVisible(true);
    fichierPrecedent->setDisabled(false);
    fichierSuivant->setDisabled(false);
    if (actionFactureDacaEnCours == actions.at(0))
    {
        fichierPrecedent->setDisabled(true);
    }
    else if (actionFactureDacaEnCours == actions.at((actions.size() - 1)))
    {
        fichierSuivant->setDisabled(true);
    }

    int actionCourante = 0;
    bool actionTrouvee = false;

    while (actionCourante < actions.size()
        && !actionTrouvee)
    {
        if (actions.at(actionCourante) != actionFactureDacaEnCours)
        {
            actionCourante = actionCourante + 1;
        }
        else
        {
            actionTrouvee = true;
        }
    }

    const QString factureDe = tr("Facture de ");
    const QString pasDeFacturePrec = tr("Pas de facture précédente");
    const QString pasDeFactureSuiv = tr("Pas de facture suivante");
    if (actionCourante + 1 < actions.size() - 1)
    {
        if (!actions.at(actionCourante + 1)->isSeparator())
        {
            fichierSuivant->setToolTip(factureDe
                + actions.at(actionCourante + 1)->data().value<AeroDmsTypes::IdentifiantFacture>().nomPrenomPilote);
        }
        else if (actionCourante + 2 < actions.size() - 1)
        {
            fichierSuivant->setToolTip(factureDe
                + actions.at(actionCourante + 2)->data().value<AeroDmsTypes::IdentifiantFacture>().nomPrenomPilote);
        }
        else
        {
            fichierSuivant->setToolTip(pasDeFactureSuiv);
        }     
    }
    else
    {
        fichierSuivant->setToolTip(pasDeFactureSuiv);
    }

    if (actionCourante - 1 >= 0)
    {
        if (!actions.at(actionCourante - 1)->isSeparator())
        {
            fichierPrecedent->setToolTip(factureDe
                + actions.at(actionCourante - 1)->data().value<AeroDmsTypes::IdentifiantFacture>().nomPrenomPilote);
        }
        else if (actionCourante - 2 >= 0)
        {
            fichierPrecedent->setToolTip(factureDe
                + actions.at(actionCourante - 2)->data().value<AeroDmsTypes::IdentifiantFacture>().nomPrenomPilote);
        }
        else
        {
            fichierPrecedent->setToolTip(pasDeFacturePrec);
        }     
    }
    else
    {
        fichierPrecedent->setToolTip(pasDeFacturePrec);
    }
}

void AeroDms::verifierDispoIdentifiantsDaca()
{
    if (facturesDaca != nullptr)
    {
        if (parametresSysteme.loginSiteDaca == ""
            || parametresSysteme.motDePasseSiteDaca == "")
        {
            facturesDaca->setEnabled(false);
            facturesDaca->setStatusTip(tr("Fonction désactivée : identifiants et/ou mot de passe non fournis"));
        }
        else
        {
            facturesDaca->setEnabled(true);
            facturesDaca->setStatusTip(tr("Chargement des factures du DACA"));
        }
    }
}

void AeroDms::demanderTelechargementMiseAJourLogiciel()
{
    statusBar()->showMessage(tr("Une mise à jour du logiciel est nécessaire. Téléchargement en cours..."));

    const QString url = "https://github.com/cvermot/AeroDMS/releases/download/v"
        + db->recupererVersionLogicielleMinimale().toString()
        + "/"
        + db->recupererNomFichierMiseAJour();
    gestionnaireDonneesEnLigne->telechargerMiseAJour(url);
}

void AeroDms::verifierVersionBddSuiteChargement()
{
    passerLeLogicielEnLectureSeule(false, false);
    statusBar()->showMessage(tr("Base de données locale à jour"), 10000);

    if (QVersionNumber::fromString(QApplication::applicationVersion()) < db->recupererVersionLogicielleMinimale())
    {
        passerLeLogicielEnLectureSeule(true, false, true);
        if (parametresSysteme.modeFonctionnementLogiciel == AeroDmsTypes::ModeFonctionnementLogiciel_EXERNE_AUTORISE_MODE_EXTERNE)
        {
            demanderTelechargementMiseAJourLogiciel();
        }
        else
        {
            QMessageBox dialogueMiseAJour;
            dialogueMiseAJour.setText(tr("La version du logiciel en cours d'utilisation (")
                + QApplication::applicationVersion()
                + tr(") est inférieure à la version minimale autorisée (")
                + db->recupererVersionLogicielleMinimale().toString()
                + ").\n\n"
                + tr("L'application va passer en mode lecture seule pour éviter tout problème.\n\n")
                + tr("Consultez le développeur / responsable de l'application pour plus d'informations."));
            dialogueMiseAJour.setWindowTitle(QApplication::applicationName() + " - " + tr("Erreur de version du logiciel"));
            dialogueMiseAJour.setIcon(QMessageBox::Critical);
            dialogueMiseAJour.setStandardButtons(QMessageBox::Close);
            dialogueMiseAJour.exec();
        }
    }
    else if (!db->laBddEstALaVersionAttendue())
    {
        QMessageBox dialogueMiseAJour;
        dialogueMiseAJour.setText(tr("La version de la base de données ne correspond pas à la version attendue par le logiciel.\n\n\
L'application va passer en mode lecture seule pour éviter tout risque d'endommagement de la BDD.\n\n\
Consultez le développeur / responsable de l'application pour plus d'informations."));
        dialogueMiseAJour.setWindowTitle(QApplication::applicationName() + " - " + tr("Erreur de version de base de données"));
        dialogueMiseAJour.setIcon(QMessageBox::Critical);
        dialogueMiseAJour.setStandardButtons(QMessageBox::Close);
        dialogueMiseAJour.exec();

        passerLeLogicielEnLectureSeule(true, false);
    }
}

void AeroDms::afficherStatusDebutTelechargementBdd()
{
    statusBar()->showMessage(tr("Base de données locale non à jour. Téléchargement de la BDD... Patientez"));
}

void AeroDms::afficherEtapesChargementBdd(const AeroDmsTypes::EtapeChargementBdd p_etape)
{
    if (barreDeProgressionEstAMettreAJourPourBddInitialeOuFinale)
    {
        barreDeProgressionStatusBar->setValue(p_etape);
        barreDeProgressionStatusBar->setToolTip(AeroDmsTypes::recupererChaineEtapeChargementBdd(p_etape));

        if (p_etape == AeroDmsTypes::EtapeChargementBdd_TERMINE)
        {
            QTimer::singleShot(5000, this, &AeroDms::masquerBarreDeProgressionDeLaStatusBar);
            barreDeProgressionEstAMettreAJourPourBddInitialeOuFinale = false;
            //On vérifie la présence de factures du DACA :
            verifierPresenceFacturesDaca();
        }
    }
    
}

void AeroDms::signalerBaseDeDonneesBloqueeParUnAutreUtilisateur(const QString p_nomVerrou,
    const QDateTime p_heureVerrouInitial,
    const QDateTime p_heureDerniereVerrou)
{
    passerLeLogicielEnLectureSeule(true, false);
    barreDeProgressionStatusBar->setValue(AeroDmsTypes::EtapeChargementBdd_TERMINE);
    QTimer::singleShot(5000, this, &AeroDms::masquerBarreDeProgressionDeLaStatusBar);

    QMessageBox dialogueErreurVersionBdd;
    dialogueErreurVersionBdd.setText(tr("La base de données est verrouillée par ")
        + p_nomVerrou
        + tr(" depuis le ")
        + QLocale::system().toString(p_heureDerniereVerrou, "d MMMM yyyy 'à' hh'h'mm")
        + ".<br /><br />Le logiciel est passé en lecture seule."
        + "<br />Attendez que l'utilisateur libère la base de données puis relancez le logiciel.");
    dialogueErreurVersionBdd.setWindowTitle(QApplication::applicationName() + " - " + tr("Base de données verrouillée"));
    dialogueErreurVersionBdd.setIcon(QMessageBox::Critical);
    dialogueErreurVersionBdd.setStandardButtons(QMessageBox::Close);
    dialogueErreurVersionBdd.exec();
}

void AeroDms::mettreAJourAerodromes()
{
    QMessageBox::information(this,
        QApplication::applicationName() + " - " + tr("Mise à jour de la liste des aérodromes"),
        tr("Cette fonction permet de mettre à jour la liste des aérodromes à partir d'un fichier AIXM 4.5.<br><br>Le fichier AIXM est téléchargeable gratuitement depuis la boutique du <a href=\"https://www.sia.aviation-civile.gouv.fr/produits-numeriques-en-libre-disposition/les-bases-de-donnees-sia.html\">SIA</a>."));

    QString fichier = QFileDialog::getOpenFileName(
        this,
        QApplication::applicationName() + " - " + "Ouvrir un fichier AIXM 4.5",
        "",
        tr("Fichier AIXM (*.xml)"));

    if (!fichier.isNull())
    {
        barreDeProgressionStatusBar->setMaximum(1);
        barreDeProgressionStatusBar->setValue(0);
        barreDeProgressionStatusBar->show();
        statusBar()->showMessage(tr("Lecture du fichier AIXM en cours"));
        qApp->processEvents();

        AixmParser aixm(db);
        connect(&aixm, &AixmParser::signalerMiseAJourAerodrome, this, &AeroDms::afficherProgressionMiseAJourAerodromes);

        aixm.mettreAJourAerodromes(fichier);

        peuplerMenuMailPilotesDUnAerodrome();
        dialogueGestionAeroclub->peuplerListeAerodrome();

        db->demanderEnvoiBdd();
    } 
}

void AeroDms::afficherProgressionMiseAJourAerodromes(int nombreTotal, 
    int nombreTraite, 
    int nombreCree, 
    int nombreMisAJour)
{
	statusBar()->showMessage(tr("Mise à jour des aérodromes en cours : ")
		+ QString::number(nombreTraite)
		+ tr(" traités")
        +"/"
		+ QString::number(nombreTotal)
        + tr(" lus")
        + " ("
        + tr("Créés : ")
        + QString::number(nombreCree)
        + tr(" / Mis à jour : ")
        + QString::number(nombreMisAJour)
        + ")");
    barreDeProgressionStatusBar->setMaximum(nombreTotal);
    barreDeProgressionStatusBar->setValue(nombreTraite);

    if ((barreDeProgressionStatusBar->maximum() - barreDeProgressionStatusBar->value()) == 0)
    {
        statusBar()->showMessage(tr("Mise à jour des aérodromes terminée")
            + " ("
            + tr("Créés : ")
            + QString::number(nombreCree)
            + tr(" / Mis à jour : ")
            + QString::number(nombreMisAJour)
            + ")");
        QTimer::singleShot(5000, this, &AeroDms::masquerBarreDeProgressionDeLaStatusBar);
    }
}

void AeroDms::masquerBarreDeProgressionDeLaStatusBar()
{
    barreDeProgressionStatusBar->hide();
    barreDeProgressionStatusBar->setToolTip("");
}

void AeroDms::closeEvent(QCloseEvent* event)
{   
    //tant  qu'on est dans l'état EtapeFermeture_FERMETURE_BDD, on ignore d'éventuelles autres demandes de fermeture
    if (etapeFermetureEnCours == EtapeFermeture_FERMETURE_BDD)
    {
        event->ignore();
    }

    //Si on est dans l'état BDD fermée, on ne doit pas intercepter le signal de fermeture
    if (etapeFermetureEnCours != EtapeFermeture_BDD_FERMEE)
    {
        //On ne renvoie la BDD en ligne que si le gestionnaire de données en ligne est actif
        //et 
        //     si le logiciel n'est pas en lecture seule
        //   ou
        //     si le retour en mode lecture ecriture a été interdit (dans certaines situations
        //        dans ce cas, on a pu prendre le verrou)
        if (gestionnaireDonneesEnLigne->estActif() 
            && (!logicielEnModeLectureSeule
                  || retourEnModeLectureEcritureEstInterdit))
        {
            event->ignore();

            barreDeProgressionStatusBar->show();
            barreDeProgressionStatusBar->setMaximum(AeroDmsTypes::EtapeChargementBdd_TERMINE);
            barreDeProgressionStatusBar->setValue(AeroDmsTypes::EtapeChargementBdd_PRISE_VERROU);
            barreDeProgressionEstAMettreAJourPourBddInitialeOuFinale = true;

            const EtapeFermeture etapeFermeturePrecedente = etapeFermetureEnCours;
            etapeFermetureEnCours = EtapeFermeture_FERMETURE_BDD;

            statusBar()->showMessage(tr("Libération base de données et envoi BDD en ligne en cours... Patientez, le logiciel fermera automatiquement une fois cette étape effectuée..."));
            passerLeLogicielEnLectureSeule(true, true);

            //Si on est va demander la fermeture de la BDD pour envoi, donc qu'on est dans le premier appel de cette méthode
            //on demande la libération du verrou de BDD
            if (etapeFermeturePrecedente == EtapeFermeture_NON_DEMANDE)
            {
                db->libererVerrouBdd();
            }
        }
    }
    
    if (factureRecupereeEnLigneEstNonTraitee == true)
    {
        delete pdfDocument;
        factureRecupereeEnLigneEstNonTraitee = false;
        QFile::remove(cheminDeLaFactureCourante);
    }
}