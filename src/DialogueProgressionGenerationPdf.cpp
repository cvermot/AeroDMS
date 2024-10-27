#include "DialogueProgressionGenerationPdf.h"
#include <QtWidgets>

DialogueProgressionGenerationPdf::DialogueProgressionGenerationPdf()
{

}

DialogueProgressionGenerationPdf::DialogueProgressionGenerationPdf(QWidget* parent) : QDialog(parent)
{
	boutonFermer = new QPushButton(tr("&Fermer"), this);
	boutonFermer->setDefault(false);
	connect(boutonFermer, SIGNAL(clicked()), this, SLOT(reject()));

	boutonOuvrirPdf = new QPushButton(tr("&Ouvrir PDF"), this);
	boutonOuvrirPdf->setDefault(true);
	connect(boutonOuvrirPdf, SIGNAL(clicked()), this, SLOT(accept()));

	boutonImprimer = new QPushButton(tr("&Imprimer"), this);
	boutonImprimer->setDefault(true);
	connect(boutonImprimer, SIGNAL(clicked()), this, SLOT(demanderImpression()));

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(boutonFermer, QDialogButtonBox::ActionRole);
	buttonBox->addButton(boutonOuvrirPdf, QDialogButtonBox::ActionRole);
	buttonBox->addButton(boutonImprimer, QDialogButtonBox::ActionRole);

	label = new QLabel(tr("Génération PDF en cours"), this);

	barreDeProgression = new QProgressBar(this);
	barreDeProgression->setMaximum(1);
	barreDeProgression->setValue(0);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(label, Qt::AlignCenter);
	mainLayout->addWidget(barreDeProgression);
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
	setWindowTitle(tr("Génération PDF"));
}

void DialogueProgressionGenerationPdf::setLabelText(const QString p_texte)
{
	label->setText(p_texte);
}

void DialogueProgressionGenerationPdf::setMaximum(const int p_maximum)
{
	barreDeProgression->setMaximum(p_maximum);

	boutonFermer->setEnabled(false);
	boutonOuvrirPdf->setEnabled(false);
	boutonImprimer->setEnabled(false);
}

void DialogueProgressionGenerationPdf::setValue(const int p_valeur)
{
	barreDeProgression->setValue(p_valeur);

	if (p_valeur >= barreDeProgression->maximum())
	{
		boutonFermer->setEnabled(true);
		boutonOuvrirPdf->setEnabled(true);
		boutonImprimer->setEnabled(true);
	}
}

void DialogueProgressionGenerationPdf::demanderImpression()
{
	hide();
	emit imprimer();
}


