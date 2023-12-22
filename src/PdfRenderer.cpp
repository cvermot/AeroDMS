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

    QFile f("C:/Users/cleme/OneDrive/Projets/HTML/COMPTA_2023.htm");
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
    templateCeTmp.replace("xxDate", "12/12/2012");

    view->setHtml(templateCeTmp);

    //QUrl url = QUrl::fromLocalFile("C:/Users/cleme/OneDrive/Projets/HTML/COMPTA_2023.htm");
    //view->load(url);
}

void PdfRenderer::wevLoadFinished(bool retour)
{
    qDebug() << "Chargement termine QWEV" << view->url() << retour;
    //view->save("C:/Users/cleme/source/AeroDms/AeroDms/testWV.html", QWebEngineDownloadRequest::SingleHtmlSaveFormat);
    view->printToPdf("C:/Users/cleme/source/AeroDms/AeroDms/testWV_2.pdf");
}

void PdfRenderer::wevPdfPrintingFinished(const QString& filePath, bool success)
{
    qDebug() << "Impression termine QWEV" << filePath << success;
}

void PdfRenderer::wevLoadTerminationStatus(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
{
    qDebug() << "QWEV erreur" << terminationStatus << exitCode;
}

void PdfRenderer::imprimerLesDemandesDeSubvention()
{
    AeroDmsTypes::ListeDemandeRemboursement listeDesRemboursements = db->recupererLesSubventionsAEmettre();

}
