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

#include <QtWidgets>

DialogueLicence::DialogueLicence(QWidget* parent) : QDialog(parent)
{
    QPushButton *cancelButton = new QPushButton(tr("&Fermer"), this);
    cancelButton->setDefault(false);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    QGridLayout* mainLayout = new QGridLayout(this);
    QTabWidget* tabLicences = new QTabWidget(this);

    QTextEdit* texteLicenceGnuGpl = new QTextEdit(this);
    texteLicenceGnuGpl->setReadOnly(true);
    tabLicences->addTab(texteLicenceGnuGpl, tr("GNU GPL v3"));
    QFile lienceGnuGpl(":/AeroDms/ressources/licences/GPL-3.0-or-later.txt");
    if (lienceGnuGpl.open(QIODevice::ReadOnly)) 
    {
        QTextStream licence(&lienceGnuGpl);
        QString texteLicense = licence.readAll();
        texteLicenceGnuGpl->setText(texteLicense);
    }

    QTextEdit* texteLicenceBsd3Clause = new QTextEdit(this);
    texteLicenceBsd3Clause->setReadOnly(true);
    tabLicences->addTab(texteLicenceBsd3Clause, tr("3-Clause BSD"));
    QFile lienceBsd3Clause(":/AeroDms/ressources/licences/BSD-3-Clause.txt");
    if (lienceBsd3Clause.open(QIODevice::ReadOnly)) 
    {
        QTextStream licence(&lienceBsd3Clause);
        QString texteLicense = licence.readAll();
        texteLicenceBsd3Clause->setText(texteLicense);
    }

    mainLayout->addWidget(tabLicences);

    mainLayout->addWidget(buttonBox, 7, 0, 1, 2);

    setLayout(mainLayout);
    setWindowTitle(QApplication::applicationName() + " - " + tr("Licences"));
    resize(700, 500);
}
