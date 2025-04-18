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

#include "DialogueProgressionImpression.h"
#include <QtWidgets>

DialogueProgressionImpression::DialogueProgressionImpression(QWidget* parent) : QDialog(parent)
{
	boutonFermer = new QPushButton(tr("&Fermer"), this);
	boutonFermer->setToolTip(tr("Fermer cette fenêtre"));
	boutonFermer->setDefault(true);
	connect(boutonFermer, SIGNAL(clicked()), this, SLOT(accept()));

	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(boutonFermer, QDialogButtonBox::ActionRole);

	label = new QLabel(tr("Impression en cours...\n"), this);
	label->setAlignment(Qt::AlignCenter);

	labelFichier = new QLabel(tr("Initialisation en cours..."), this);
	labelFichier->setAlignment(Qt::AlignCenter);

	barreDeProgressionFichier = new QProgressBar(this);
	barreDeProgressionFichier->setMaximum(1);
	barreDeProgressionFichier->setValue(0);

	labelPage = new QLabel(tr("Initialisation en cours..."), this);
	labelPage->setAlignment(Qt::AlignCenter);

	barreDeProgressionPage = new QProgressBar(this);
	barreDeProgressionPage->setMaximum(1);
	barreDeProgressionPage->setValue(0);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(label, Qt::AlignCenter);
	mainLayout->addWidget(labelFichier, Qt::AlignCenter);
	mainLayout->addWidget(barreDeProgressionFichier);
	mainLayout->addWidget(labelPage, Qt::AlignCenter);
	mainLayout->addWidget(barreDeProgressionPage);
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
	setWindowTitle(QApplication::applicationName() + " - " + tr("Impression"));
	setWindowModality(Qt::ApplicationModal);
	setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
	resize(450, size().height());
}

void DialogueProgressionImpression::setMaximumFichier(const int p_nbFichier)
{
	labelFichier->setText(tr("Fichier 1/") + QString::number(p_nbFichier));
	barreDeProgressionFichier->reset();
	barreDeProgressionFichier->setMaximum(p_nbFichier+1);
	barreDeProgressionFichier->setValue(0);

	boutonFermer->setEnabled(false);
}

void DialogueProgressionImpression::traitementFichierSuivant()
{
	barreDeProgressionFichier->setValue(barreDeProgressionFichier->value()+1);

    if (barreDeProgressionPage->value() >= barreDeProgressionPage->maximum()
		&& barreDeProgressionFichier->value() >= barreDeProgressionFichier->maximum())
	{
		boutonFermer->setEnabled(true);
		label->setText(tr("Impression terminée"));
	}
	else
	{
		labelPage->setText(tr("Initialisation..."));
		labelFichier->setText(tr("Fichier ") + QString::number(barreDeProgressionFichier->value()) + "/" + QString::number(barreDeProgressionFichier->maximum() - 1));
		barreDeProgressionPage->setValue(0);
	}
}

void DialogueProgressionImpression::traitementPageSuivante(int currentPage, int totalPages)
{
	barreDeProgressionPage->setMaximum(totalPages);
	barreDeProgressionPage->setValue(currentPage);

	labelPage->setText(tr("Page ") + QString::number(barreDeProgressionPage->value()) + "/" + QString::number(barreDeProgressionPage->maximum()));

	if (barreDeProgressionPage->value() >= barreDeProgressionPage->maximum()
		&& barreDeProgressionFichier->value() == barreDeProgressionFichier->maximum()-1)
	{
		label->setText(tr("Finalisation en cours, veuillez patientier..."));
	}

	else if (barreDeProgressionPage->value() >= barreDeProgressionPage->maximum()
		&& barreDeProgressionFichier->value() >= barreDeProgressionFichier->maximum())
	{
		boutonFermer->setEnabled(true);
		label->setText(tr("Impression terminée"));
	}
}
