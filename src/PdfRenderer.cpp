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
#include "PdfRenderer.h"
#include "StatistiqueWidget.h"
#include "StatistiqueDiagrammeCirculaireWidget.h"
#include "StatistiqueDonutCombineWidget.h"
#include "StatistiqueDonuts.h"
#include "StatistiqueHistogrammeEmpile.h"

#include <podofo/podofo.h>

#include <QFile>
#include <QPrinter>
#include <QChart>
#include <QSvgGenerator>

PdfRenderer::PdfRenderer()
{
    db = new ManageDb();
    view = new QWebEnginePage(this);
    nombreEtapesEffectuees = 0;
    indiceFichier = 0;
    laDemandeEstPourUnDocumentUnique = false;
}

PdfRenderer::PdfRenderer( ManageDb *p_db, 
                          QString p_cheminTemplatesHtml, 
                          QWidget* parent)
{
    db = p_db;
	view = new QWebEnginePage(this);
    nombreEtapesEffectuees = 0;
    indiceFichier = 0;
    laDemandeEstPourUnDocumentUnique = false;

    ressourcesHtml = QUrl(QString("file:///%1/").arg(p_cheminTemplatesHtml));

    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(chargementTermine(bool)));
    connect(view, SIGNAL(pdfPrintingFinished(const QString&, bool)), this, SLOT(impressionTerminee(const QString&, bool)));
}

QString PdfRenderer::mergerPdf()
{
    PoDoFo::PdfMemDocument document;

    const QString nomFichier = cheminSortieFichiersGeneres
        + demandeEnCours.nomFichier;
    document.Load(nomFichier.toStdString());
    
    for (int i = 0; i < demandeEnCours.listeFactures.size(); i++)
    {
        const QString nomFacture = repertoireDesFactures
            + demandeEnCours.listeFactures.at(i);
        PoDoFo::PdfMemDocument facture;
        facture.Load(nomFacture.toStdString());
        document.GetPages().AppendDocumentPages(facture);
    }

    const QString appVersion = "AeroDMS v" + QApplication::applicationVersion();
    document.GetMetadata().SetCreator(PoDoFo::PdfString(appVersion.toStdString()));
    document.GetMetadata().SetAuthor(PoDoFo::PdfString(demandeEnCours.nomTresorier.toStdString()));
    document.GetMetadata().SetTitle(PoDoFo::PdfString(demandeEnCours.nomFichier.toStdString()));
    document.GetMetadata().SetSubject(PoDoFo::PdfString("Formulaire de demande de subvention"));

    document.SaveUpdate(nomFichier.toStdString());

    return nomFichier;
}

QString PdfRenderer::numeroFichierSur3Digits()
{
    QString numero = QString::number(indiceFichier);
    if (numero.size() == 1)
    {
        numero = QString("00").append(numero);
    }
    else if (numero.size() == 2)
    {
        numero = QString("0").append(numero);
    }
    indiceFichier++;

    return numero;
}

void PdfRenderer::chargementTermine(bool retour)
{
    demandeEnCours.nomFichier = numeroFichierSur3Digits()
        +demandeEnCours.nomFichier
        +(".pdf");
    QString nomFichier = cheminSortieFichiersGeneres
        + demandeEnCours.nomFichier;

    //De base le format est portrait
    QPageLayout pageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF()));
    if (demandeEnCours.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_RECAP_ANNUEL)
    {
        pageLayout.setOrientation(QPageLayout::Landscape);
    }

    view->printToPdf(nomFichier, pageLayout);
    listeDesFichiers.append(demandeEnCours.nomFichier);
    nombreEtapesEffectuees++;
    emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
}

void PdfRenderer::impressionTerminee( const QString& filePath, 
                                      bool success)
{
    const QString fichier = mergerPdf();

    nombreEtapesEffectuees++;
    emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);

    db->ajouterDemandeCeEnBdd(demandeEnCours);

    //Si la demande ne concerne pas un document unique, on fait les demandes suivantes
    if (!laDemandeEstPourUnDocumentUnique)
    {
        imprimerLaProchaineDemandeDeSubvention();
    }
    //Sinon on emet directement la fin d'impression
    else
    {
        emit generationTerminee(cheminSortieFichiersGeneres, fichier);
    }
   
}

void PdfRenderer::imprimerLeRecapitulatifDesHeuresDeVol( const int p_annee,
                                                         const QString p_cheminSortieFichiersGeneres,
                                                         const QString p_cheminStockageFactures,
                                                         const AeroDmsTypes::Signature p_signature,
                                                         const int p_graphAGenerer)
{
    nombreEtapesEffectuees = 0;
    indiceFichier = 0;
    laDemandeEstPourUnDocumentUnique = true;

    demandeEnCours.typeDeSignatureDemandee = p_signature;
    demandeEnCours.mergerTousLesPdf = true;

    demandeEnCours.recapHdVAvecBaladesEtSorties = true;
    demandeEnCours.recapHdVAvecRecettes = true;

    demandeEnCours.recapHdvGraphAGenerer = p_graphAGenerer;

    cheminSortieFichiersGeneres = QString(p_cheminSortieFichiersGeneres).append(QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmm"));
    repertoireDesFactures = p_cheminStockageFactures;
    QDir().mkdir(cheminSortieFichiersGeneres);
    if (QDir(cheminSortieFichiersGeneres).exists())
    {
        emit mettreAJourNombreFacturesATraiter(3+ calculerNbEtapesGenerationRecapHdV(p_graphAGenerer));
        emit mettreAJourNombreFacturesTraitees(0);

        cheminSortieFichiersGeneres.append("/");
        listeDesFichiers.clear();

        const AeroDmsTypes::ListeSubventionsParPilotes listePilotesDeCetteAnnee = db->recupererSubventionsPilotes( p_annee,
                                                                                                                   "*",
                                                                                                                   false);

        const AeroDmsTypes::SubventionsParPilote totaux = db->recupererTotauxAnnuel(p_annee, false);
        const AeroDmsTypes::EtatGeneration generationRecapAnnuel =  imprimerLeFichierPdfDeRecapAnnuel( p_annee, 
                                                                                                       listePilotesDeCetteAnnee, 
                                                                                                       totaux);
        nombreEtapesEffectuees++;
        emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);

        if (generationRecapAnnuel != AeroDmsTypes::EtatGeneration_OK)
        {
            emit echecGeneration();
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Création de répertoire impossible"), tr("Impossible de créer le repertoire de sortie sous :\n") +
            p_cheminSortieFichiersGeneres +
            tr("\nImpossible de générer les demandes de subventions.\n\n Arrêt"));

        emit echecGeneration();
    }
}

void PdfRenderer::imprimerLesDemandesDeSubvention( const QString p_nomTresorier,
                                                   const QString p_cheminSortieFichiersGeneres,
                                                   const QString p_cheminStockageFactures,
                                                   const AeroDmsTypes::TypeGenerationPdf p_typeGenerationPdf,
                                                   const AeroDmsTypes::Signature p_signature,
                                                   const bool p_mergerTousLesPdf,
                                                   const bool p_recapHdVAvecRecettes,
                                                   const bool p_recapHdvAvecBaladesEtSorties,
                                                   const int p_valeurGraphAGenerer )
{
    demandeEnCours.listeFactures = QStringList();
    demandeEnCours.nomTresorier = p_nomTresorier;
    demandeEnCours.typeDeGenerationDemandee = p_typeGenerationPdf;
    demandeEnCours.typeDeSignatureDemandee = p_signature;
    demandeEnCours.mergerTousLesPdf = p_mergerTousLesPdf;
    demandeEnCours.recapHdVAvecBaladesEtSorties = p_recapHdvAvecBaladesEtSorties;
    demandeEnCours.recapHdVAvecRecettes = p_recapHdVAvecRecettes;
    demandeEnCours.recapHdvGraphAGenerer = p_valeurGraphAGenerer;

    nombreEtapesEffectuees = 0 ;
    indiceFichier = 0;
    laDemandeEstPourUnDocumentUnique = false;

    cheminSortieFichiersGeneres = QString(p_cheminSortieFichiersGeneres).append(QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmm"));
    repertoireDesFactures = p_cheminStockageFactures;

    QDir().mkdir(cheminSortieFichiersGeneres);
    if(QDir(cheminSortieFichiersGeneres).exists())
    { 
        cheminSortieFichiersGeneres.append("/");
        listeDesFichiers.clear();
        int nombreEtapesAEffectuer = 0 ;

        if ( demandeEnCours.typeDeGenerationDemandee == AeroDmsTypes::TypeGenerationPdf_TOUTES
             || demandeEnCours.typeDeGenerationDemandee == AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT)
        {
            nombreEtapesAEffectuer = db->recupererLesCotisationsAEmettre().size() +
                                     db->recupererLesRecettesBaladesEtSortiesAEmettre().size();
        }
        if ( demandeEnCours.typeDeGenerationDemandee == AeroDmsTypes::TypeGenerationPdf_TOUTES
             || demandeEnCours.typeDeGenerationDemandee == AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT)
        {
            listeAnnees = db->recupererAnneesAvecVolNonSoumis();
            nombreEtapesAEffectuer = nombreEtapesAEffectuer +
                                     db->recupererLesSubventionsAEmettre().size() +
                                     db->recupererLesDemandesDeRembousementAEmettre().size() +
                                     listeAnnees.size();
        }

        nombreEtapesAEffectuer = 3 * nombreEtapesAEffectuer + calculerNbEtapesGenerationRecapHdV(p_valeurGraphAGenerer);

        emit mettreAJourNombreFacturesATraiter(nombreEtapesAEffectuer);
        emit mettreAJourNombreFacturesTraitees(0);

        db->sauvegarderLaBdd(cheminSortieFichiersGeneres);

        if (imprimerLaProchaineDemandeDeSubvention() != AeroDmsTypes::EtatGeneration_OK)
        {
            emit echecGeneration();
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Création de répertoire impossible"), tr("Impossible de créer le repertoire de sortie sous :\n") +
            p_cheminSortieFichiersGeneres +
            tr("\nImpossible de générer les demandes de subventions.\n\n Arrêt"));
        
        emit echecGeneration();
    }
}

AeroDmsTypes::EtatGeneration PdfRenderer::imprimerLaProchaineDemandeDeSubvention()
{
    AeroDmsTypes::EtatGeneration etatGenerationARetourner = AeroDmsTypes::EtatGeneration_OK;

    //On ouvre le template et on met à jour les informations communes à toutes les demandes
    QFile f(QString(ressourcesHtml.toLocalFile()).append("COMPTA_2023.htm"));
    QString templateCeTmp = "";
    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);
        templateCeTmp = in.readAll();
    }
    else
    {
        QMessageBox::critical(this, tr("Fichier template introuvable"), tr("Le fichier \"COMPTA_2023.htm\" attendu dans\n") +
            ressourcesHtml.toString() +
            tr("\nest introuvable. Impossible de générer les demandes de subventions.\n\n Arrêt"));

        return AeroDmsTypes::EtatGeneration_FICHIER_ABSENT;
    }
    //QString templateCeTmp = templateCe;
    //Date de la demande
    templateCeTmp.replace("xxDate", QDate::currentDate().toString("dd/MM/yyyy"));
    //Investissement => toujours décoché, pour le moment non géré par le logiciel
    templateCeTmp.replace("xxI", "");
    //Signataire => toujours celui qui exécute le logiciel
    templateCeTmp.replace("xxSignataire", demandeEnCours.nomTresorier);

    //On genere un fichier de recap de l'état des subventions déjà allouées avant les demandes que l'on va générer ensuite
    if (listeAnnees.size() > 0)
    {
        const int annee = listeAnnees.takeFirst();

        const AeroDmsTypes::ListeSubventionsParPilotes listePilotesDeCetteAnnee = db->recupererLesSubventionesDejaAllouees(annee);
        const AeroDmsTypes::SubventionsParPilote totaux = db->recupererTotauxAnnuel( annee, 
                                                                                     true );
        etatGenerationARetourner =  imprimerLeFichierPdfDeRecapAnnuel( annee,
                                                                       listePilotesDeCetteAnnee, 
                                                                       totaux);
    }
    else if (db->recupererLesSubventionsAEmettre().size() > 0
              && demandeEnCours.typeDeGenerationDemandee != AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT)
    {    
        const AeroDmsTypes::DemandeRemboursement demande = db->recupererLesSubventionsAEmettre().at(0);
        //Depense
        templateCeTmp.replace("xxD", "X");
        //Recette
        templateCeTmp.replace("xxR", "");

        //Montant
        remplirLeChampMontant(templateCeTmp, demande.montantARembourser);
        //Signature
        remplirLeChampSignature(templateCeTmp);

        //Cheque a retirer au CE par le demandeur => a cocher
        templateCeTmp.replace("zzC", "X");

        //Bénéficaire
        //L'aéroclub du pilote :
        templateCeTmp.replace("xxBeneficiaire", db->recupererAeroclub(demande.piloteId));

        //Observation
        QString observation = demande.typeDeVol;
        observation.append(" / ");
        observation.append(db->recupererNomPrenomPilote(demande.piloteId));
        templateCeTmp.replace("xxObservation", observation);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(demande.typeDeVol)) ;
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(demande.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        //On envoie le HTML en génération
        view->setHtml( templateCeTmp, 
                       ressourcesHtml);

        QStringList facturesAssociees = db->recupererListeFacturesAssocieeASubvention(demande);
        listeDesFichiers.append(facturesAssociees);

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_HEURE_DE_VOL;
        demandeEnCours.idPilote = demande.piloteId;
        demandeEnCours.annee = demande.annee;
        demandeEnCours.typeDeVol = demande.typeDeVol;
        demandeEnCours.nomBeneficiaire = db->recupererAeroclub(demande.piloteId);
        demandeEnCours.montant = demande.montantARembourser;
        demandeEnCours.nomFichier = ".HdV_"
            + (demande.typeDeVol) 
            + "_" 
            + (demande.piloteId) 
            + ("_") 
            + (QString::number(demande.annee));
        demandeEnCours.listeFactures = facturesAssociees;

    }
    //Recettes "Cotisations"
    else if (db->recupererLesCotisationsAEmettre().size() > 0
              && demandeEnCours.typeDeGenerationDemandee != AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT )
    {
        const AeroDmsTypes::Recette recette = db->recupererLesCotisationsAEmettre().at(0);
        //Depense
        templateCeTmp.replace("xxD", "");
        //Recette
        templateCeTmp.replace("xxR", "X");
        //Cheque a retirer au CE par le demandeur => non coché
        templateCeTmp.replace("zzC", "");
        //Bénéficaire : le CSE
        templateCeTmp.replace("xxBeneficiaire", "CSE Thales");
        //Montant
        remplirLeChampMontant(templateCeTmp, recette.montant);
        //Signature
        remplirLeChampSignature(templateCeTmp);
        //Observation
        templateCeTmp.replace("xxObservation", recette.intitule);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta("Cotisation"));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(recette.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml( templateCeTmp,
                       ressourcesHtml);

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_COTISATION;
        demandeEnCours.annee = recette.annee;
        demandeEnCours.typeDeVol = "Cotisation";
        demandeEnCours.nomBeneficiaire = "CSE Thales";
        demandeEnCours.montant = recette.montant;
        demandeEnCours.nomFichier = ".Cotisations_"
            +(QString::number(demandeEnCours.annee));
        demandeEnCours.listeFactures = QStringList();
    }
    //Recettes "passagers" des sorties et balades
    else if (db->recupererLesRecettesBaladesEtSortiesAEmettre().size() > 0
              && demandeEnCours.typeDeGenerationDemandee != AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT )
    {
        const AeroDmsTypes::Recette recette = db->recupererLesRecettesBaladesEtSortiesAEmettre().at(0);
        //Depense
        templateCeTmp.replace("xxD", "");
        //Recette
        templateCeTmp.replace("xxR", "X");
        //Cheque a retirer au CE par le demandeur => non coché
        templateCeTmp.replace("zzC", "");
        //Bénéficaire : le CSE
        templateCeTmp.replace("xxBeneficiaire", "CSE Thales");

        //Montant
        remplirLeChampMontant(templateCeTmp, recette.montant);
        //Signature
        remplirLeChampSignature(templateCeTmp);
        //Observation
        const QList<QString> listeRecettes = db->recupererListeRecettesNonSoumisesCse(recette.annee, recette.typeDeSortie);

        QString observation = QString("Participations ").append(recette.intitule).append(" <font size='1'>(");

        //55 catactères sur la première ligne
        int nbCaracteresRestants = 55;
        for (int i = 0; i < listeRecettes.size(); i++)
        {
            if (listeRecettes.at(i).size() > nbCaracteresRestants - 2)
            {
                observation.append("<br/>");
                //Sur les lignes suivantes on autorise 90 caractères
                nbCaracteresRestants = 90;
            }
            //if (listeRecettes.at(i).size() < nbCaracteresRestants-2)
            {
                //On ajoute la recette à la ligne
                observation.append(listeRecettes.at(i));
                //Si on est pas sur le dernier élément, on ajoute " | "
                if (i != listeRecettes.size() - 1)
                {
                    observation.append(" | ");
                }
                nbCaracteresRestants = nbCaracteresRestants - (listeRecettes.at(i).size() + 2);
            }
        }

        observation.append(")</font>");

        templateCeTmp.replace("xxObservation", observation);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(recette.typeDeSortie));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(recette.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml( templateCeTmp,
                       ressourcesHtml);

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_PAIEMENT_SORTIE_OU_BALADE;
        demandeEnCours.annee = recette.annee;
        demandeEnCours.typeDeVol = recette.intitule;
        demandeEnCours.nomBeneficiaire = "CSE Thales";
        demandeEnCours.montant = recette.montant;
        demandeEnCours.nomFichier = ".RecetteBalades_"
            +(QString::number(demandeEnCours.annee));
        demandeEnCours.listeFactures = QStringList();
    }
    //Factures payées par les pilotes à rembourser
    else if ( db->recupererLesDemandesDeRembousementAEmettre().size() > 0
              && demandeEnCours.typeDeGenerationDemandee != AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT )
    {
        const AeroDmsTypes::DemandeRemboursementFacture demandeRembousement = db->recupererLesDemandesDeRembousementAEmettre().at(0);
        //Depense
        templateCeTmp.replace("xxD", "X");
        //Recette
        templateCeTmp.replace("xxR", "");
        //Cheque a retirer au CE par le demandeur => coché
        templateCeTmp.replace("zzC", "X");
        //Bénéficaire : le CSE
        templateCeTmp.replace("xxBeneficiaire", demandeRembousement.payeur);

        //Montant
        remplirLeChampMontant(templateCeTmp, demandeRembousement.montant);
        //Signature
        remplirLeChampSignature(templateCeTmp);
        //Observation
        templateCeTmp.replace("xxObservation", demandeRembousement.intitule);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(demandeRembousement.typeDeSortie));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(demandeRembousement.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml( templateCeTmp,
                       ressourcesHtml);
        //recopierFacture(demandeRembousement.nomFacture);
        listeDesFichiers.append(QString(repertoireDesFactures).append(demandeRembousement.nomFacture));

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_FACTURE;
        //L'annee n'est pas utile pour la facture => on fourni directement l'ID facture
        demandeEnCours.annee = demandeRembousement.id;
        demandeEnCours.typeDeVol = demandeRembousement.intitule;
        demandeEnCours.nomBeneficiaire = demandeRembousement.payeur;
        demandeEnCours.montant = demandeRembousement.montant;
        demandeEnCours.nomFichier = QString(".DemandeRemboursementFacturePayeeParPilote");
        demandeEnCours.listeFactures = QStringList(demandeRembousement.nomFacture);
    }
    else
    {
        QString fichierMerge = "";
        if (demandeEnCours.mergerTousLesPdf)
        {
            fichierMerge = produireFichierPdfGlobal();
        }
        
        emit generationTerminee(cheminSortieFichiersGeneres, fichierMerge);
    }
    nombreEtapesEffectuees++;

    return etatGenerationARetourner;
}

AeroDmsTypes::EtatGeneration PdfRenderer::imprimerLeFichierPdfDeRecapAnnuel( const int p_annee,
                                                                             const AeroDmsTypes::ListeSubventionsParPilotes p_listePilotesDeCetteAnnee,
                                                                             const AeroDmsTypes::SubventionsParPilote p_totaux)
{
    AeroDmsTypes::EtatGeneration etatGenerationARetourner = AeroDmsTypes::EtatGeneration_OK;

    QFile table(QString(ressourcesHtml.toLocalFile()).append("TableauRecap.html"));
    QFile tableItem(QString(ressourcesHtml.toLocalFile()).append("TableauRecapItem.html"));
    QFile tableRecettes(QString(ressourcesHtml.toLocalFile()).append("TableauRecapRecettes.html"));
    QString templateTable = "";
    QString templateTableItem = "";
    QString templateTableRecettes = "";
    if (table.open(QFile::ReadOnly | QFile::Text) 
        && tableItem.open(QFile::ReadOnly | QFile::Text)
        && tableRecettes.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream inTable(&table);
        QTextStream inTableItem(&tableItem);
        QTextStream inTableRecettes(&tableRecettes);
        templateTable = inTable.readAll();
        templateTableItem = inTableItem.readAll();
        templateTableRecettes = inTableRecettes.readAll();
    }
    else
    {
        QMessageBox::critical(this, tr("Fichier template introuvable"), tr("Un ou plusieurs fichiers parmi :\n")
            + "     -\"TableauRecap.html\"\n"
            + "     -\"TableauRecapItem.html\"\n"
            + "     -\"TableauRecapRecettes.html\"\n"
            + tr("attendus dans\n") 
            + ressourcesHtml.toString()
            + tr("\nsont introuvables. Impossible de générer les récapitulatifs annuels.\n\n Arrêt"));

        return AeroDmsTypes::EtatGeneration_FICHIER_ABSENT;
    }
    templateTable.replace("__date__", QDate::currentDate().toString("dd/MM/yyyy"));
    templateTable.replace("__exercice__", QString::number(p_annee));

    for (int i = 0; i < p_listePilotesDeCetteAnnee.size(); i++)
    {
        QString item = templateTableItem;
        item.replace("__pilote__", QString(p_listePilotesDeCetteAnnee.at(i).nom).append(" ").append(p_listePilotesDeCetteAnnee.at(i).prenom));
        item.replace("__HdvEnt__", p_listePilotesDeCetteAnnee.at(i).entrainement.heuresDeVol);
        item.replace("__CouEnt__", QString::number(p_listePilotesDeCetteAnnee.at(i).entrainement.coutTotal, 'f', 2));
        item.replace("__SubEnt__", QString::number(p_listePilotesDeCetteAnnee.at(i).entrainement.montantRembourse, 'f', 2));
        item.replace("__HdVBal__", p_listePilotesDeCetteAnnee.at(i).balade.heuresDeVol);
        item.replace("__CouBal__", QString::number(p_listePilotesDeCetteAnnee.at(i).balade.coutTotal, 'f', 2));
        item.replace("__SubBal__", QString::number(p_listePilotesDeCetteAnnee.at(i).balade.montantRembourse, 'f', 2));
        item.replace("__HdVSor__", p_listePilotesDeCetteAnnee.at(i).sortie.heuresDeVol);
        item.replace("__CouSor__", QString::number(p_listePilotesDeCetteAnnee.at(i).sortie.coutTotal, 'f', 2));
        item.replace("__SubSor__", QString::number(p_listePilotesDeCetteAnnee.at(i).sortie.montantRembourse, 'f', 2));
        item.replace("__HdvTot__", p_listePilotesDeCetteAnnee.at(i).totaux.heuresDeVol);
        item.replace("__CouTot__", QString::number(p_listePilotesDeCetteAnnee.at(i).totaux.coutTotal, 'f', 2));
        item.replace("__SubTot__", QString::number(p_listePilotesDeCetteAnnee.at(i).totaux.montantRembourse, 'f', 2));

        templateTable.replace("<!--Accroche-->", item);
    }

    templateTable.replace("__TotHdvEnt__", p_totaux.entrainement.heuresDeVol);
    templateTable.replace("__TotCouEnt__", QString::number(p_totaux.entrainement.coutTotal, 'f', 2));
    templateTable.replace("__TotSubEnt__", QString::number(p_totaux.entrainement.montantRembourse, 'f', 2));
    templateTable.replace("__TotHdVBal__", p_totaux.balade.heuresDeVol);
    templateTable.replace("__TotCouBal__", QString::number(p_totaux.balade.coutTotal, 'f', 2));
    templateTable.replace("__TotSubBal__", QString::number(p_totaux.balade.montantRembourse, 'f', 2));
    templateTable.replace("__TotHdVSor__", p_totaux.sortie.heuresDeVol);
    templateTable.replace("__TotCouSor__", QString::number(p_totaux.sortie.coutTotal, 'f', 2));
    templateTable.replace("__TotSubSor__", QString::number(p_totaux.sortie.montantRembourse, 'f', 2));
    templateTable.replace("__TotHdvTot__", p_totaux.totaux.heuresDeVol);
    templateTable.replace("__TotCouTot__", QString::number(p_totaux.totaux.coutTotal, 'f', 2));
    templateTable.replace("__TotSubTot__", QString::number(p_totaux.totaux.montantRembourse, 'f', 2));

    if (demandeEnCours.recapHdVAvecRecettes)
    {
        templateTable.replace("<!--AccrocheRecette-->", templateTableRecettes);

        const AeroDmsTypes::TotauxRecettes totaux = db->recupererTotauxRecettes(p_annee);
        templateTable.replace("_RecetteCotisation_", QString::number(totaux.cotisations, 'f', 2) + " €");
        templateTable.replace("_RecetteBalade_", QString::number(totaux.balades, 'f', 2) + " €");
        templateTable.replace("_RecetteSortie_", QString::number(totaux.sorties, 'f', 2) + " €");
    }

    if (demandeEnCours.recapHdVAvecBaladesEtSorties)
    {
        const QString htmlRecapBaladesSorties = genererHtmlRecapBaladesSorties(p_annee, etatGenerationARetourner);
        templateTable.replace("<!--AccrocheRecapBaladesSorties-->", htmlRecapBaladesSorties);
    }

    const QString images = genererImagesStatistiques(p_annee);
    templateTable.replace("<!--AccrocheGraphiques-->", images);

    if ( demandeEnCours.mergerTousLesPdf
         && demandeEnCours.typeDeSignatureDemandee == AeroDmsTypes::Signature_NUMERIQUE_LEX_COMMUNITY)
    {
        templateTable.replace("<!--Signature-->", "<p>[SignatureField#1]</p>");
    }

    if (etatGenerationARetourner == AeroDmsTypes::EtatGeneration_OK)
    {
        view->setHtml(templateTable,
            ressourcesHtml);
    }  

    demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_RECAP_ANNUEL;
    demandeEnCours.nomFichier = ".Recap_pilote_"
        +(QString::number(p_annee));

    return etatGenerationARetourner;
}

const QString PdfRenderer::produireFichierPdfGlobal()
{
    PoDoFo::PdfMemDocument document;

    const QString nomFichier = cheminSortieFichiersGeneres
        + numeroFichierSur3Digits()
        + ".FichiersAssembles.pdf";

    QDir dir(cheminSortieFichiersGeneres);
    const QStringList fichiers = dir.entryList(QStringList("*.pdf"), QDir::Files);
   
    for (int i = 0; i < fichiers.size(); i++)
    {
        const QString nomFacture = cheminSortieFichiersGeneres
            + fichiers.at(i);
        PoDoFo::PdfMemDocument facture;
        facture.Load(nomFacture.toStdString());
        document.GetPages().AppendDocumentPages(facture);
    }

    const QString appVersion = "AeroDMS v" + QApplication::applicationVersion();
    document.GetMetadata().SetCreator(PoDoFo::PdfString(appVersion.toStdString()));
    document.GetMetadata().SetAuthor(PoDoFo::PdfString(demandeEnCours.nomTresorier.toStdString()));
    document.GetMetadata().SetTitle(PoDoFo::PdfString(demandeEnCours.nomFichier.toStdString()));
    document.GetMetadata().SetSubject(PoDoFo::PdfString("Formulaire de demande de subvention"));
    document.Save(nomFichier.toStdString());

    return nomFichier;
}

void PdfRenderer::remplirLeChampMontant(QString &p_html, const float p_montant)
{
    QString montantARembourser = QString::number(p_montant, 'f', 2);
    QString partieEntiere;
    QString partieDecimale;
    if (montantARembourser.contains("."))
    {
        QStringList entier = montantARembourser.split(".");
        partieEntiere = entier.at(0);
        std::reverse(partieEntiere.begin(), partieEntiere.end());
        partieDecimale = entier.at(1);
        if (partieDecimale.size() == 1)
        {
            partieDecimale.append("0");
        }
    }
    else
    {
        partieEntiere = montantARembourser;
        std::reverse(partieEntiere.begin(), partieEntiere.end());
        partieDecimale = "00";
    }
    if (partieEntiere.size() >= 4)
    {
        p_html.replace("yyM", QString(partieEntiere.at(3)).append("&nbsp;&nbsp;"));
    }
    else
    {
        p_html.replace("yyM", "");
    }
    if (partieEntiere.size() >= 3)
    {
        p_html.replace("yyC", QString(partieEntiere.at(2)).append("&nbsp;&nbsp;"));
    }
    else
    {
        p_html.replace("yyC", "");
    }
    if (partieEntiere.size() >= 2)
    {
        p_html.replace("yyD", QString(partieEntiere.at(1)).append("&nbsp;&nbsp;"));
    }
    else
    {
        p_html.replace("yyD", "");
    }
    if (partieEntiere.size() >= 1)
    {
        p_html.replace("yyU", QString(partieEntiere.at(0)).append("&nbsp;&nbsp;"));
    }
    else
    {
        p_html.replace("yyU", "0");
    }
    if (partieDecimale.size() >= 1)
    {
        p_html.replace("yyd", QString("&nbsp;&nbsp;").append(partieDecimale.at(0)));
    }
    else
    {
        p_html.replace("yyd", "0");
    }
    if (partieDecimale.size() >= 2)
    {
        p_html.replace("yyc", QString("&nbsp;&nbsp;").append(partieDecimale.at(1)));
    }
    else
    {
        p_html.replace("yyc", "0");
    }
}

void PdfRenderer::remplirLeChampSignature(QString& p_html)
{
    //Signature => 
    switch (demandeEnCours.typeDeSignatureDemandee)
    {
        case AeroDmsTypes::Signature_MANUSCRITE_IMAGE:
        {
            //Le chemin vers le fichier
            QString cheminSignature = QCoreApplication::applicationDirPath() + "/ressources/signature.jpg";
            p_html.replace("xxSignature", "<img src=\""+ cheminSignature +"\" width=\"140\" />");
        }
        break;

        case AeroDmsTypes::Signature_NUMERIQUE_LEX_COMMUNITY:
        {
            if (demandeEnCours.mergerTousLesPdf)
            {
                //Si on est sur un PDF mergé, la signature "visible" réalisée par Lex Community
                //se trouvera sur la première page   
                if (QFile("./ressources/signature.jpg").exists())
                {
                    QString cheminSignature = QCoreApplication::applicationDirPath() + "/ressources/signature.jpg";
                    p_html.replace("xxSignature", "<font size=\"1\">Cachet de signature numérique<br/>en première page</font><br /><img src=\"" + cheminSignature + "\" width=\"140\" />");
                }
                else
                {
                    p_html.replace("xxSignature", "<font size=\"1\">Cachet de signature numérique<br/>en première page</font>");

                }
                
            }
            else
            {
                //Le tag réservé pour Lex Community
                p_html.replace("xxSignature", "[SignatureField#1]");
            }
            
        }
        break;

        case AeroDmsTypes::Signature_SANS:
        default:
        {
            //Vide si on signe à la main
            p_html.replace("xxSignature", "");
        }
        break;
    }
    
}

QString PdfRenderer::genererHtmlRecapBaladesSorties( const int p_annee,
                                                     AeroDmsTypes::EtatGeneration & p_etatGenerationARetourner)
{
    QString html = "";

    const AeroDmsTypes::ListeDetailsBaladesEtSorties listeDetails = db->recupererListeDetailsBaladesEtSorties(p_annee);

    AeroDmsTypes::ListeBaladesEtSortiesParIdSortie listesParIdSortie;

    //On ajoute le premier item, si la liste retournée par la requete n'est pas vide...
    if (listeDetails.size() > 0)
    {
        AeroDmsTypes::BaladesEtSortiesParId baladeParId;
        baladeParId.nombreDeLignes = 0;
        baladeParId.idSortie = listeDetails.at(0).idSortie;
        AeroDmsTypes::GroupeBaladesEtSortiesAssociees baladesEtSortiesAssociees;
        baladesEtSortiesAssociees.recettesUniques.append(listeDetails.at(0));
        baladesEtSortiesAssociees.volsUniques.append(listeDetails.at(0));
        baladeParId.baladesEtSortiesAssociees.append(baladesEtSortiesAssociees);
        listesParIdSortie.push_back(baladeParId);

        QVector<int> idsVolsDejaAjoute;
        QVector<int> idsRecettesDejaAjoute;
        idsVolsDejaAjoute.clear();
        idsRecettesDejaAjoute.clear();
        idsVolsDejaAjoute.append(listeDetails.at(0).volId);
        idsRecettesDejaAjoute.append(listeDetails.at(0).idRecette);

        //1) on parcours la liste des listeDetails pour élaborer listesParIdSortie
        for (int i = 0; i < listeDetails.size(); i++)
        {
            AeroDmsTypes::DetailsBaladesEtSorties details = listeDetails.at(i);

            //Si le nouvel idSortie est différent de celui en cours de traitement, on ajoute un nouvel item dans listesParIdSortie
            if (details.idSortie != listesParIdSortie.last().idSortie)
            {
                //On finalise le groupe N-1
                const int nombreItemVol = listesParIdSortie.last().baladesEtSortiesAssociees.last().volsUniques.size();
                const int nombreItemRecette = std::max(int(listesParIdSortie.last().baladesEtSortiesAssociees.last().recettesUniques.size()), 1);
                listesParIdSortie.last().nombreDeLignes = listesParIdSortie.last().nombreDeLignes + nombreItemVol * nombreItemRecette;

                AeroDmsTypes::BaladesEtSortiesParId balade;
                AeroDmsTypes::GroupeBaladesEtSortiesAssociees baladesEtSortiesAssociees;
                baladesEtSortiesAssociees.recettesUniques.append(details);
                baladesEtSortiesAssociees.volsUniques.append(details);
                balade.idSortie = details.idSortie;
                balade.nombreDeLignes = 0;
                balade.baladesEtSortiesAssociees.append(baladesEtSortiesAssociees);
                listesParIdSortie.push_back(balade);

                idsVolsDejaAjoute.clear();
                idsRecettesDejaAjoute.clear();

                idsVolsDejaAjoute.append(details.volId);
                idsRecettesDejaAjoute.append(details.idRecette);
            }

            //Dans tous les cas on effectue les traitements
            if (idsVolsDejaAjoute.contains(details.volId)
                || idsRecettesDejaAjoute.contains(details.idRecette))
            {
                //Sî l'une des 2 listes contient déjà l'identifiant associé, alors on ajoute l'identifiant à l'autre liste, s'il ne s'y trouve pas déjà
                if (!idsVolsDejaAjoute.contains(details.volId))
                {
                    idsVolsDejaAjoute.append(details.volId);
                    listesParIdSortie.last().baladesEtSortiesAssociees.last().volsUniques.append(details);
                }
                if (!idsRecettesDejaAjoute.contains(details.idRecette))
                {
                    idsRecettesDejaAjoute.append(details.idRecette);
                    listesParIdSortie.last().baladesEtSortiesAssociees.last().recettesUniques.append(details);
                }
            }
            else
            {
                const int nombreItemVol = listesParIdSortie.last().baladesEtSortiesAssociees.last().volsUniques.size();
                const int nombreItemRecette = std::max(int(listesParIdSortie.last().baladesEtSortiesAssociees.last().recettesUniques.size()), 1);
                listesParIdSortie.last().nombreDeLignes = listesParIdSortie.last().nombreDeLignes + nombreItemVol * nombreItemRecette;

                AeroDmsTypes::GroupeBaladesEtSortiesAssociees baladesEtSortiesAssociees;
                listesParIdSortie.last().baladesEtSortiesAssociees.append(baladesEtSortiesAssociees);

                idsVolsDejaAjoute.clear();
                idsRecettesDejaAjoute.clear();

                idsVolsDejaAjoute.append(details.volId);
                idsRecettesDejaAjoute.append(details.idRecette);
                listesParIdSortie.last().baladesEtSortiesAssociees.last().volsUniques.append(details);
                listesParIdSortie.last().baladesEtSortiesAssociees.last().recettesUniques.append(details);
            }
        }

        //On finalise le groupe N-1
        const int nombreItemVol = listesParIdSortie.last().baladesEtSortiesAssociees.last().volsUniques.size();
        const int nombreItemRecette = std::max(int(listesParIdSortie.last().baladesEtSortiesAssociees.last().recettesUniques.size()), 1);
        listesParIdSortie.last().nombreDeLignes = listesParIdSortie.last().nombreDeLignes + nombreItemVol * nombreItemRecette;

        //Phase de génération HTML

        QFile tableBaladesSorties(QString(ressourcesHtml.toLocalFile()).append("TableauRecapBaladesSorties.html"));
        if (tableBaladesSorties.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream inTableBaladesSorties(&tableBaladesSorties);
            html = inTableBaladesSorties.readAll();
        }
        else
        {
            QMessageBox::critical(this, tr("Fichier template introuvable"), tr("Le fichier \"TableauRecapBaladesSorties.html\" attendu dans\n") +
                ressourcesHtml.toString() +
                tr("\nest introuvable. Impossible de générer le tableau des recap balades et sorties.\n\n Arrêt"));

            p_etatGenerationARetourner = AeroDmsTypes::EtatGeneration_FICHIER_ABSENT;
            return "";
        }

        int nbItem = 0;

        for (int i = 0; i < listesParIdSortie.size(); i++)
        {
            html = html + "  <tr>\n";
            html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(listesParIdSortie.at(i).nombreDeLignes) + "'>" + listesParIdSortie.at(i).baladesEtSortiesAssociees.at(0).volsUniques.at(0).nomSortie + "</td>\n";

            for (int j = 0; j < listesParIdSortie.at(i).baladesEtSortiesAssociees.size(); j++)
            {
                nbItem++;
                const int nombreLignes = listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).volsUniques.size() * std::max(1, int(listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).recettesUniques.size()));

                if (j != 0)
                    html = html + "  <tr>\n";
                html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(nombreLignes) + "'>" + "#" + QString::number(nbItem) + " " + "</td >\n";

                int compteurLignesPourVol = 0;
                int compteurLignesPourRecette = 0;

                int compteurVol = 0;
                int compteurRecettes = 0;

                for (int k = 0; k < nombreLignes; k++)
                {

                    if (k != 0)
                        html = html + "  <tr>\n";
                    if (compteurLignesPourVol == 0)
                    {
                        compteurLignesPourVol = std::max(1, int(listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).recettesUniques.size()));

                        html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(compteurLignesPourVol) + "'>" + listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).volsUniques.at(compteurVol).dateVol.toString("dd/MM/yyyy") + "</td >\n";
                        html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(compteurLignesPourVol) + "'>" + listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).volsUniques.at(compteurVol).nomPassagers + "</td>\n";
                        html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(compteurLignesPourVol) + "'>" + AeroDmsServices::convertirMinutesEnHeuresMinutes(listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).volsUniques.at(compteurVol).dureeVol) + "</td>\n";
                        html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(compteurLignesPourVol) + "'>" + QString::number(listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).volsUniques.at(compteurVol).coutVol) + " €</td>\n";
                        html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(compteurLignesPourVol) + "'>" + QString::number(listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).volsUniques.at(compteurVol).montantRembouse) + " €</td>\n";
                        compteurVol++;
                    }
                    if (compteurLignesPourRecette == 0)
                    {
                        compteurLignesPourRecette = std::max(1, int(listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).volsUniques.size()));

                        if (listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).recettesUniques.at(compteurRecettes).montantRecette != 0)
                        {
                            html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(compteurLignesPourRecette) + "'>" + listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).recettesUniques.at(compteurRecettes).intituleRecette + "</td >\n";
                            html = html + "    <td class = 'tg-lboi' rowspan = '" + QString::number(compteurLignesPourRecette) + "'>" + QString::number(listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j).recettesUniques.at(compteurRecettes).montantRecette) + " €</td>\n";
                        }
                        else
                        {
                            html = html + "    <td class = 'tg-1zu3' colspan='2' rowspan = '" + QString::number(compteurLignesPourRecette) + "'>Paiement non encore reçu</td >\n";
                        }

                        compteurRecettes++;
                    }

                    compteurLignesPourVol--;
                    compteurLignesPourRecette--;

                    html = html + "  </tr>\n";
                }

                listesParIdSortie.at(i).baladesEtSortiesAssociees.at(j);
            }
            html = html + "  </tr>\n";
        }
        html = html + "</tbody>\n</table>";
    }
    else
    {
        html = "<p>Pas de balades ou sorties saisie pour l'exercice en cours (pour le moment...).</p>";
    }

    return html;
}

QString PdfRenderer::genererImagesStatistiques(const int p_annee)
{
    QString cheminSortie = cheminSortieFichiersGeneres + "img";
    QDir().mkdir(cheminSortie);
    cheminSortie = cheminSortie + "/";

    QString html;

    QWidget* m_contentArea = nullptr;

    const AeroDmsTypes::ResolutionEtParametresStatistiques tailleImage = convertirResolution(demandeEnCours.recapHdvGraphAGenerer);

    if ((demandeEnCours.recapHdvGraphAGenerer & AeroDmsTypes::Statistiques_HEURES_ANNUELLES) == AeroDmsTypes::Statistiques_HEURES_ANNUELLES)
    {
        StatistiqueWidget* stats = new StatistiqueHistogrammeEmpile( db,
                                                                     p_annee,
                                                                     m_contentArea,
                                                                     QChart::NoAnimation, 
                                                                     tailleImage );

        const QString urlImage = cheminSortie + "heuresAnnuelles";

        enregistrerImage(stats,
            urlImage,
            tr("Heures annuelles ") + QString::number(p_annee));

        html = html + "<center><img width=\"1120\" src=\"" + urlImage + ".svg" + "\"></center><br/>\n";

        nombreEtapesEffectuees++;
        emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
    }

    if ((demandeEnCours.recapHdvGraphAGenerer & AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE) == AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE)
    {
        StatistiqueWidget* stats = new StatistiqueDiagrammeCirculaireWidget(db,
            p_annee,
            AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE,
            m_contentArea,
            QChart::NoAnimation,
            false, 
            tailleImage);
        const QString urlImage = cheminSortie + "pilote";

        enregistrerImage(stats,
            urlImage,
            tr("Heures par pilote ") + QString::number(p_annee));

        html = html + "<center><img width=\"1120\" src=\"" + urlImage + ".svg" + "\"></center><br/>\n";

        nombreEtapesEffectuees++;
        emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
    }

    if ((demandeEnCours.recapHdvGraphAGenerer & AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL) == AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL)
    {
        StatistiqueWidget* stats = new StatistiqueDiagrammeCirculaireWidget(db,
            p_annee,
            AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL,
            m_contentArea,
            QChart::NoAnimation,
            false, 
            tailleImage);

        const QString urlImage = cheminSortie + "typeVol";

        enregistrerImage(stats,
            urlImage,
            tr("Type de vol ") + QString::number(p_annee));

        html = html + "<center><img width=\"1120\" src=\"" + urlImage + ".svg" + "\"></center><br/>\n";

        nombreEtapesEffectuees++;
        emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
    }

    if ((demandeEnCours.recapHdvGraphAGenerer & AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE) == AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE)
    {
        StatistiqueWidget* stats = new StatistiqueDiagrammeCirculaireWidget(db,
            p_annee,
            AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE,
            m_contentArea,
            QChart::NoAnimation,
            false,
            tailleImage);

        const QString urlImage = cheminSortie + "activite";

        enregistrerImage(stats,
            urlImage,
            tr("Activités ") + QString::number(p_annee));

        html = html + "<center><img width=\"1120\" src=\"" + urlImage + ".svg" + "\"></center><br/>\n";

        nombreEtapesEffectuees++;
        emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
    }

    if ((demandeEnCours.recapHdvGraphAGenerer & AeroDmsTypes::Statistiques_AERONEFS) == AeroDmsTypes::Statistiques_AERONEFS)
    {
        StatistiqueDonutCombineWidget* stats = new StatistiqueDonutCombineWidget(db,
            AeroDmsTypes::Statistiques_AERONEFS,
            m_contentArea,
            p_annee,
            QChart::NoAnimation,
            false,
            tailleImage);

        const QString urlImage = cheminSortie + "aeronef";

        enregistrerImage( stats,
                          urlImage,
                          tr("Aéronefs ") + QString::number(p_annee));

        html = html + "<center><img width=\"1120\" src=\"" + urlImage + ".svg" + "\"></center><br/>\n";

        nombreEtapesEffectuees++;
        emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
    }

    if ((demandeEnCours.recapHdvGraphAGenerer & AeroDmsTypes::Statistiques_STATUTS_PILOTES) == AeroDmsTypes::Statistiques_STATUTS_PILOTES)
    {
        StatistiqueDonuts* stats = new StatistiqueDonuts(db,
            AeroDmsTypes::Statistiques_STATUTS_PILOTES,
            m_contentArea,
            0,
            tailleImage);

        const QString urlImage = cheminSortie + "statutsPilotes";

        enregistrerImage(stats,
            urlImage,
            tr("Aéronefs ") + QString::number(p_annee));

        html = html + "<center><img width=\"1120\" src=\"" + urlImage + ".svg" + "\"></center><br/>\n";

        nombreEtapesEffectuees++;
        emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
    }
    
    delete m_contentArea;

    return html;
}

void PdfRenderer::enregistrerImage( QWidget *p_widget,
                                    const QString p_urlImage,
                                    const QString p_titre)
{
    p_widget->grab().save(p_urlImage + ".png");

    QSvgGenerator generator;
    generator.setFileName(p_urlImage + ".svg");
    generator.setSize(p_widget->size());
    generator.setViewBox(p_widget->rect());
    generator.setTitle(p_titre);
    generator.setDescription(tr("Image générée avec AeroDms v") + QApplication::applicationVersion());
    p_widget->render(&generator);

    delete p_widget;
}

AeroDmsTypes::ResolutionEtParametresStatistiques PdfRenderer::convertirResolution(const int p_resolution)
{
    QSize size(1920,1080);
    AeroDmsTypes::ResolutionEtParametresStatistiques resolutionEtParametres = AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES;

    const AeroDmsTypes::Resolution resolution = static_cast<AeroDmsTypes::Resolution>(p_resolution & AeroDmsTypes::Resolution_MASQUE);

    if (resolution == AeroDmsTypes::Resolution_Full_HD)
    {
        resolutionEtParametres.tailleMiniImage = QSize(1920, 1080);
        resolutionEtParametres.tailleDePolice = 10;
    }
    else if (resolution == AeroDmsTypes::Resolution_QHD)
    {
        resolutionEtParametres.tailleMiniImage = QSize(2560, 1440);
        resolutionEtParametres.tailleDePolice = 22;
    }
    else if (resolution == AeroDmsTypes::Resolution_4K)
    {
        resolutionEtParametres.tailleMiniImage = QSize(3840, 2160);
        resolutionEtParametres.tailleDePolice = 30;
    }

    return resolutionEtParametres;
}

int PdfRenderer::calculerNbEtapesGenerationRecapHdV(const int p_graphAGenerer)
{
    int nbGraph =  0;
    if ((p_graphAGenerer & AeroDmsTypes::Statistiques_HEURES_ANNUELLES) == AeroDmsTypes::Statistiques_HEURES_ANNUELLES)
    {
        nbGraph++;
    }
    if ((p_graphAGenerer & AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE) == AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE)
    {
        nbGraph++;
    }
    if ((p_graphAGenerer & AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL) == AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL)
    {
        nbGraph++;
    }
    if ((p_graphAGenerer & AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE) == AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE)
    {
        nbGraph++;
    }
    if ((p_graphAGenerer & AeroDmsTypes::Statistiques_AERONEFS) == AeroDmsTypes::Statistiques_AERONEFS)
    {
        nbGraph++;
    }

    return nbGraph;
}

