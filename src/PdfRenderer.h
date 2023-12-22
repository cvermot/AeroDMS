#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include <QWidget>
#include <QWebEnginePage>
#include "ManageDb.h"

class PdfRenderer : public QWidget {
	Q_OBJECT

public:
	PdfRenderer();
	PdfRenderer(ManageDb *p_db, QWidget* parent = nullptr);

	void imprimerLesDemandesDeSubvention();

private :
	QWebEnginePage* view;
	ManageDb *db;

	QString templateCe;

public slots:
    void wevLoadFinished(bool retour);
    void wevPdfPrintingFinished(const QString& filePath, bool success);
    void wevLoadTerminationStatus(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode);

};

#endif // PDFRENDERER_H