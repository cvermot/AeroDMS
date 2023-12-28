#include "PdfRenderer.h"

#include <QFile>

PdfRenderer::PdfRenderer()
{
    db = new ManageDb();
    view = new QWebEnginePage(this);
}

PdfRenderer::PdfRenderer(ManageDb *p_db, QWidget* parent)
{
    db = p_db;
	view = new QWebEnginePage(this);

    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(chargementTermine(bool)));
    connect(view, SIGNAL(renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus, bool)), this, SLOT(statusDeChargementAVarie(QWebEnginePage::RenderProcessTerminationStatus, bool)));
    connect(view, SIGNAL(pdfPrintingFinished(const QString&, bool)), this, SLOT(impressionTerminee(const QString&, bool)));

    //QUrl url = QUrl::fromLocalFile("C:/Users/cleme/OneDrive/Projets/HTML/COMPTA_2023.htm");
    //view->load(url);
}

void PdfRenderer::chargementTermine(bool retour)
{
    qDebug() << "Chargement termine QWEV" << view->url() << retour;
    //view->save("C:/Users/cleme/source/AeroDms/AeroDms/testWV.html", QWebEngineDownloadRequest::SingleHtmlSaveFormat);
    view->printToPdf(QString("C:/Users/cleme/source/AeroDms/AeroDms/fomulaire_").append(QString::number(nombreFacturesTraitees)).append(".pdf"));
}

void PdfRenderer::impressionTerminee(const QString& filePath, bool success)
{
    qDebug() << "Impression termine QWEV" << filePath << success;

    nombreFacturesTraitees++;
    emit mettreAJourNombreFactureTraitees(nombreFacturesATraiter, nombreFacturesTraitees);
    imprimerLaProchaineDemandeDeSubvention();
}

void PdfRenderer::statusDeChargementAVarie(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
{
    qDebug() << "QWEV erreur" << terminationStatus << exitCode;
}

int PdfRenderer::imprimerLesDemandesDeSubvention()
{
    AeroDmsTypes::ListeDemandeRemboursement listeDesRemboursements = db->recupererLesSubventionsAEmettre();
    nombreFacturesATraiter = listeDesRemboursements.size();
    nombreFacturesTraitees = 0 ;
    emit mettreAJourNombreFactureTraitees(nombreFacturesATraiter, nombreFacturesTraitees);

    imprimerLaProchaineDemandeDeSubvention();
    return listeDesRemboursements.size();
}

void PdfRenderer::imprimerLaProchaineDemandeDeSubvention()
{
    //On ouvre le template et on met à jour les informations communes à toutes les demandes
    QFile f("./ressources/HTML/COMPTA_2023.htm");
    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);
        templateCe = in.readAll();
    }
    else
    {
        qDebug() << "Erreur ouverture fichier";
    }
    QString templateCeTmp = templateCe;
    //Date de la demande
    templateCeTmp.replace("xxDate", QDate::currentDate().toString("dd/MM/yyyy"));
    //Investissement => toujours décoché, pour le moment non géré par le logiciel
    templateCeTmp.replace("xxI", "");
    //Signataire => toujours celui qui exécute le logiciel
    templateCeTmp.replace("xxSignataire", "Cl&eacute;ment VERMOT-DESROCHES");

    AeroDmsTypes::ListeDemandeRemboursement listeDesRemboursements = db->recupererLesSubventionsAEmettre();
    if (nombreFacturesTraitees < listeDesRemboursements.size())
    {    
        //Depense
        templateCeTmp.replace("xxD", "X");
        //Recette
        templateCeTmp.replace("xxR", "");

        //Montant
        remplirLeChampMontant(templateCeTmp, listeDesRemboursements.at(nombreFacturesTraitees).montantARembourser);

        //Cheque a retirer au CE par le demandeur => a cocher
        templateCeTmp.replace("zzC", "X");

        //Bénéficaire
        //L'aéroclub du pilote :
        templateCeTmp.replace("xxBeneficiaire", db->recupererAeroclub(listeDesRemboursements.at(nombreFacturesTraitees).piloteId));

        //Observation
        QString observation = listeDesRemboursements.at(nombreFacturesTraitees).typeDeVol;
        observation.append(" / ");
        observation.append(db->recupererNomPrenomPilote(listeDesRemboursements.at(nombreFacturesTraitees).piloteId));
        templateCeTmp.replace("xxObservation", observation);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(listeDesRemboursements.at(nombreFacturesTraitees).typeDeVol)) ;
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(listeDesRemboursements.at(nombreFacturesTraitees).annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        //On envoie le HTML en génération
        view->setHtml(templateCeTmp);
    }

}

void PdfRenderer::remplirLeChampMontant(QString &p_html, const float p_montant)
{
    QString montantARembourser = QString::number(p_montant);
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
    //qDebug() << "Montant a rembouser " << montantARembourser << partieEntiere << partieDecimale;
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
