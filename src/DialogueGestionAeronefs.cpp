/******************************************************************************\
<QUas : a Free Software logbook for UAS operators>
Copyright (C) 2023 Clément VERMOT-DESROCHES (clement@vermot.net)

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
#include "DialogueGestionAeronefs.h"
#include "AeroDmsServices.h"

#include <QtWidgets>

DialogueGestionAeronefs::DialogueGestionAeronefs()
{

}

DialogueGestionAeronefs::DialogueGestionAeronefs(ManageDb* db, QWidget* parent) : QDialog(parent)
{
    database = db;

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    setLayout(mainLayout);

    setWindowTitle(tr("Gestion des aéronefs"));

    vueAeronefs = new QTableWidget(0, AeroDmsTypes::AeronefTableElement_NB_COLONNES, this);
    vueAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTableElement_IMMAT, new QTableWidgetItem("Immatriculation"));
    vueAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTableElement_TYPE, new QTableWidgetItem("Type"));

    mainLayout->addWidget(vueAeronefs, 0, 0);
}

void DialogueGestionAeronefs::peuplerListeAeronefs()
{
    vueAeronefs->clear();

    const AeroDmsTypes::ListeAeronefs listeAeronefs = database->recupererListeAeronefs();

    vueAeronefs->setRowCount(listeAeronefs.size());
    for (int i = 0; i < listeAeronefs.size(); i++)
    {
        const AeroDmsTypes::Aeronef aeronef = listeAeronefs.at(i);
        vueAeronefs->setItem(i, AeroDmsTypes::AeronefTableElement_IMMAT, new QTableWidgetItem(aeronef.immatriculation));
        vueAeronefs->setItem(i, AeroDmsTypes::AeronefTableElement_TYPE, new QTableWidgetItem(aeronef.type));
    }
    vueAeronefs->resizeColumnsToContents();
}