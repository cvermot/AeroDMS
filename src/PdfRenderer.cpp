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

    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(wevLoadFinished(bool)));
    connect(view, SIGNAL(renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus, bool)), this, SLOT(wevLoadTerminationStatus(QWebEnginePage::RenderProcessTerminationStatus, bool)));
    connect(view, SIGNAL(pdfPrintingFinished(const QString&, bool)), this, SLOT(wevPdfPrintingFinished(const QString&, bool)));

    //QUrl url = QUrl::fromLocalFile("C:/Users/cleme/OneDrive/Projets/HTML/COMPTA_2023.htm");
    //view->load(url);
}

void PdfRenderer::wevLoadFinished(bool retour)
{
    qDebug() << "Chargement termine QWEV" << view->url() << retour;
    //view->save("C:/Users/cleme/source/AeroDms/AeroDms/testWV.html", QWebEngineDownloadRequest::SingleHtmlSaveFormat);
    view->printToPdf(QString("C:/Users/cleme/source/AeroDms/AeroDms/testWV_").append(QString::number(nombreFacturesTraitees)).append(".pdf"));
}

void PdfRenderer::wevPdfPrintingFinished(const QString& filePath, bool success)
{
    qDebug() << "Impression termine QWEV" << filePath << success;

    nombreFacturesTraitees++;
    emit mettreAJourNombreFactureTraitees(nombreFacturesATraiter, nombreFacturesTraitees);
    imprimerLaProchaineDemandeDeSubvention();
}

void PdfRenderer::wevLoadTerminationStatus(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
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
    AeroDmsTypes::ListeDemandeRemboursement listeDesRemboursements = db->recupererLesSubventionsAEmettre();
    if (nombreFacturesTraitees < listeDesRemboursements.size())
    {

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
        templateCeTmp.replace("xxDate", QDate::currentDate().toString("dd/MM/yyyy"));
        //Depense
        templateCeTmp.replace("xxD", "X");
        //Recette
        templateCeTmp.replace("xxR", "");
        //Investissement
        templateCeTmp.replace("xxI", "");

        //Montant
        QString montantARembourser = QString::number(listeDesRemboursements.at(nombreFacturesTraitees).montantARembourser);
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
            templateCeTmp.replace("yyM", QString(partieEntiere.at(3)).append("&nbsp;&nbsp;"));
        }
        else
        {
            templateCeTmp.replace("yyM", "");
        }
        if (partieEntiere.size() >= 3)
        {
            templateCeTmp.replace("yyC", QString(partieEntiere.at(2)).append("&nbsp;&nbsp;"));
        }
        else
        {
            templateCeTmp.replace("yyC", "");
        }
        if (partieEntiere.size() >= 2)
        {
            templateCeTmp.replace("yyD", QString(partieEntiere.at(1)).append("&nbsp;&nbsp;"));
        }
        else
        {
            templateCeTmp.replace("yyD", "");
        }
        if (partieEntiere.size() >= 1)
        {
            templateCeTmp.replace("yyU", QString(partieEntiere.at(0)).append("&nbsp;&nbsp;"));
        }
        else
        {
            templateCeTmp.replace("yyU", "0");
        }
        if (partieDecimale.size() >= 1)
        {
            templateCeTmp.replace("yyd", QString("&nbsp;&nbsp;").append(partieDecimale.at(0)));
        }
        else
        {
            templateCeTmp.replace("yyd", "0");
        }
        if (partieDecimale.size() >= 2)
        {
            templateCeTmp.replace("yyc", QString("&nbsp;&nbsp;").append(partieDecimale.at(1)));
        }
        else
        {
            templateCeTmp.replace("yyc", "0");
        }

        //Cheque a retirer au CE par le demandeur => a cocher
        templateCeTmp.replace("zzC", "X");

        //Bénéficaire
        //L'aéroclub du pilote :
        templateCeTmp.replace("xxBeneficiaire", db->recupererAeroclub(listeDesRemboursements.at(nombreFacturesTraitees).piloteId));

        //Signataire
        templateCeTmp.replace("xxSignataire", "Cl&eacute;ment VERMOT-DESROCHES");

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
