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

	int imprimerLesDemandesDeSubvention();

private :
	QWebEnginePage* view;
	ManageDb *db;

	QString templateCe;

	int nombreFacturesTraitees;
	int nombreFacturesATraiter;

	void imprimerLaProchaineDemandeDeSubvention();
	void remplirLeChampMontant(QString& p_html, const float p_montant);

signals:
	void mettreAJourNombreFactureTraitees(int nombreFacturesATraiter, int nombreFactureTraitees);

public slots:
    void chargementTermine(bool retour);
    void impressionTerminee(const QString& filePath, bool success);
    void statusDeChargementAVarie(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode);

};

#endif // PDFRENDERER_H