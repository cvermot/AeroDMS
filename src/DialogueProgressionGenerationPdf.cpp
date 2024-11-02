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

#include "DialogueProgressionGenerationPdf.h"
#include <QtWidgets>

DialogueProgressionGenerationPdf::DialogueProgressionGenerationPdf()
{

}

DialogueProgressionGenerationPdf::DialogueProgressionGenerationPdf(QWidget* parent) : QDialog(parent)
{
	boutonFermer = new QPushButton(tr("&Fermer"), this);
	boutonFermer->setToolTip(tr("Fermer cette fenêtre sans action supplémentaire"));
	boutonFermer->setDefault(false);
	connect(boutonFermer, SIGNAL(clicked()), this, SLOT(reject()));

	boutonOuvrirPdf = new QPushButton(tr("&Ouvrir PDF"), this);
	boutonOuvrirPdf->setToolTip(tr("Ouvrir le fichier PDF généré dans le lecteur PDF par défaut"));
	boutonOuvrirPdf->setDefault(false);
	connect(boutonOuvrirPdf, SIGNAL(clicked()), this, SLOT(accept()));

	boutonOuvrirDossier = new QPushButton(tr("Ouvrir le &dossier"), this);
	boutonOuvrirDossier->setToolTip(tr("Ouvrir le dossier contenant les fichiers générés"));
	boutonOuvrirDossier->setDefault(false);
	connect(boutonOuvrirDossier, SIGNAL(clicked()), this, SLOT(demanderOuvrirLeDossier()));

	boutonImprimer = new QPushButton(tr("&Imprimer"), this);
	boutonImprimer->setToolTip(tr("Imprimer directement les fichiers générés"));
	boutonImprimer->setDefault(true);
	connect(boutonImprimer, SIGNAL(clicked()), this, SLOT(demanderImpression()));

	boutonImprimerAgrafage = new QPushButton(tr("Imprimer avec &agrafage"), this);
	boutonImprimerAgrafage->setToolTip(tr("Imprimer directement les fichiers générés"));
	boutonImprimerAgrafage->setDefault(false);
	connect(boutonImprimerAgrafage, SIGNAL(clicked()), this, SLOT(demanderImpressionAgrafage()));

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(boutonFermer, QDialogButtonBox::ActionRole);
	buttonBox->addButton(boutonOuvrirPdf, QDialogButtonBox::ActionRole);
	buttonBox->addButton(boutonOuvrirDossier, QDialogButtonBox::ActionRole);
	buttonBox->addButton(boutonImprimer, QDialogButtonBox::ActionRole);
	buttonBox->addButton(boutonImprimerAgrafage, QDialogButtonBox::ActionRole);

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
	setWindowModality(Qt::WindowModal);
}

void DialogueProgressionGenerationPdf::setMaximum(const int p_maximum)
{
	label->setText(tr("Génération PDF en cours"));

	barreDeProgression->setMaximum(p_maximum);

	boutonFermer->setEnabled(false);
	boutonOuvrirPdf->setEnabled(false);
	boutonImprimer->setEnabled(false);
	boutonOuvrirDossier->setEnabled(false);

	repaint();
	show();
}

void DialogueProgressionGenerationPdf::setValue(const int p_valeur)
{
	barreDeProgression->setValue(p_valeur);
	label->setText(tr("Génération PDF en cours"));

	if (p_valeur >= barreDeProgression->maximum())
	{
		boutonFermer->setEnabled(true);
		boutonOuvrirPdf->setEnabled(true);
		boutonImprimer->setEnabled(true);
		boutonOuvrirDossier->setEnabled(true);
		boutonOuvrirPdf->setToolTip(tr("Ouvrir le fichier PDF généré dans le lecteur PDF par défaut"));
		boutonImprimer->setToolTip(tr("Imprimer directement les fichiers générés"));
	}
}

void DialogueProgressionGenerationPdf::generationEstTerminee(const bool p_fichierMergeEstDisponible)
{
	label->setText(tr("Génération PDF terminée"));
	boutonImprimer->setEnabled(p_fichierMergeEstDisponible);
	boutonOuvrirPdf->setEnabled(p_fichierMergeEstDisponible);

	if (!p_fichierMergeEstDisponible)
	{
		boutonOuvrirPdf->setToolTip(tr("Option indisponible lorsque l'option de fusion est désactivée"));
		boutonImprimer->setToolTip(tr("Option indisponible lorsque l'option de fusion est désactivée"));
	}
}

void DialogueProgressionGenerationPdf::demanderImpression()
{
	hide();
	emit imprimer();
}

void DialogueProgressionGenerationPdf::demanderImpressionAgrafage()
{
	hide();
	emit imprimerAgrafage();
}

void DialogueProgressionGenerationPdf::demanderOuvrirLeDossier()
{
	hide();
	emit ouvrirLeDossier();
}


