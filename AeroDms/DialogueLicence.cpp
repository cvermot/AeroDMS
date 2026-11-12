/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2026 Clément VERMOT-DESROCHES (clement@vermot.net)

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
#include "DialogueLicence.h"

DialogueLicence::DialogueLicence(QWidget* parent) : QDialog(parent)
{
    QPushButton *cancelButton = new QPushButton(tr("&Fermer"), this);
    cancelButton->setDefault(false);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    QGridLayout* mainLayout = new QGridLayout(this);
    QTabWidget* tabLicences = new QTabWidget(this);

    ajouterLicenceADialogue(tabLicences,
        tr("GNU GPL v3"),
        tr("Licence de l'application\nLicence de Qt Graphs"),
        "GPL-3.0-or-later.txt");

    ajouterLicenceADialogue(tabLicences,
        tr("LGPL v3"),
        tr("Licence de Qt Core"),
        "LGPL-3.0-only.txt");

    ajouterLicenceADialogue(tabLicences,
        tr("3-Clause BSD"),
        tr("Code intégré issu d'exemples Qt"),
        "BSD-3-Clause.txt");

    ajouterLicenceADialogue(tabLicences,
        tr("MPL 2.0"),
        tr("Licence de Bit7z"),
        "MPL-2.0.txt");

   

    /*QWidget* ongletGnuGpl = new QWidget(this);
    QVBoxLayout* layoutOngletGnuGpl = new QVBoxLayout(ongletGnuGpl);
    QLabel* labelGnuGpl = new QLabel(tr("Licence de l'application\nLicence de Qt"), ongletGnuGpl);
    layoutOngletGnuGpl->addWidget(labelGnuGpl);
    QTextEdit* texteLicenceGnuGpl = new QTextEdit(ongletGnuGpl);
    texteLicenceGnuGpl->setReadOnly(true);
    layoutOngletGnuGpl->addWidget(texteLicenceGnuGpl);
    QFile licenceGnuGpl(":/AeroDms/ressources/licences/GPL-3.0-or-later.txt");
    if (licenceGnuGpl.open(QIODevice::ReadOnly))
    {
        QTextStream licence(&licenceGnuGpl);
        QString texteLicense = licence.readAll();
        texteLicenceGnuGpl->setText(texteLicense);
    }
    ongletGnuGpl->setLayout(layoutOngletGnuGpl);
    tabLicences->addTab(ongletGnuGpl, tr("GNU GPL v3"));

    QTextEdit* texteLicenceBsd3Clause = new QTextEdit(this);
    texteLicenceBsd3Clause->setReadOnly(true);
    tabLicences->addTab(texteLicenceBsd3Clause, tr("3-Clause BSD"));
    QFile licenceBsd3Clause(":/AeroDms/ressources/licences/BSD-3-Clause.txt");
    if (licenceBsd3Clause.open(QIODevice::ReadOnly)) 
    {
        QTextStream licence(&licenceBsd3Clause);
        QString texteLicense = licence.readAll();
        texteLicenceBsd3Clause->setText(texteLicense);
    }

    QTextEdit* texteLicenceMpl2 = new QTextEdit(this);
    texteLicenceMpl2->setReadOnly(true);
    tabLicences->addTab(texteLicenceMpl2, tr("MPL 2.0"));
    QFile licenceMpl2(":/AeroDms/ressources/licences/MPL-2.0.txt");
    if (licenceMpl2.open(QIODevice::ReadOnly))
    {
        QTextStream licence(&licenceMpl2);
        QString texteLicense = licence.readAll();
        texteLicenceMpl2->setText(texteLicense);
    }*/

    mainLayout->addWidget(tabLicences);

    mainLayout->addWidget(buttonBox, 7, 0, 1, 2);

    setLayout(mainLayout);
    setWindowTitle(QApplication::applicationName() + " - " + tr("Licences"));
    resize(700, 500);
}

void DialogueLicence::ajouterLicenceADialogue(QTabWidget* tabLicences, 
    QString p_nomLicence,
    QString p_nomBibSousLicence, 
    QString p_nomFichierLicence)
{
    QWidget* onglet = new QWidget(this);
    QVBoxLayout* layoutOnglet = new QVBoxLayout(onglet);
    QLabel* label = new QLabel(p_nomBibSousLicence, onglet);
    layoutOnglet->addWidget(label);
    QTextEdit* texteLicence = new QTextEdit(onglet);
    texteLicence->setReadOnly(true);
    layoutOnglet->addWidget(texteLicence);
    QFile licence(":/AeroDms/ressources/licences/" + p_nomFichierLicence);
    if (licence.open(QIODevice::ReadOnly))
    {
        QTextStream licenceStream(&licence);
        QString texteLicense = licenceStream.readAll();
        texteLicence->setText(texteLicense);
    }
    onglet->setLayout(layoutOnglet);
    tabLicences->addTab(onglet, p_nomLicence);
}
