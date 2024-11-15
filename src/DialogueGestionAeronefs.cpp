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

    setWindowTitle(QApplication::applicationName() + " - " + tr("Gestion des aéronefs"));

    vueAeronefs = new QTableWidget(0, AeroDmsTypes::AeronefTableElement_NB_COLONNES, this);
    vueAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTableElement_IMMAT, new QTableWidgetItem("Immatriculation"));
    vueAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTableElement_TYPE, new QTableWidgetItem("Type"));
    vueAeronefs->setEditTriggers(QAbstractItemView::DoubleClicked);

    connect(vueAeronefs, &QTableWidget::cellChanged, this, &DialogueGestionAeronefs::sauvegarderDonneesSaisies);

    mainLayout->addWidget(vueAeronefs, 0, 0);
}

void DialogueGestionAeronefs::peuplerListeAeronefs()
{
    vueAeronefs->clearContents();

    const AeroDmsTypes::ListeAeronefs listeAeronefs = database->recupererListeAeronefs();

    vueAeronefs->blockSignals(true);

    vueAeronefs->setRowCount(listeAeronefs.size());
    for (int i = 0; i < listeAeronefs.size(); i++)
    {
        const AeroDmsTypes::Aeronef aeronef = listeAeronefs.at(i);
        QTableWidgetItem* itemImmat = new QTableWidgetItem(aeronef.immatriculation);
        itemImmat->setFlags(itemImmat->flags() & ~Qt::ItemIsEditable);
        vueAeronefs->setItem(i, AeroDmsTypes::AeronefTableElement_IMMAT, itemImmat);
        vueAeronefs->setItem(i, AeroDmsTypes::AeronefTableElement_TYPE, new QTableWidgetItem(aeronef.type));
    }
    vueAeronefs->resizeColumnsToContents();

    vueAeronefs->blockSignals(false);
}

void DialogueGestionAeronefs::sauvegarderDonneesSaisies(int p_ligne, int p_colonne)
{
    AeroDmsTypes::AeronefTableElement elementEdite = AeroDmsTypes::AeronefTableElement_TYPE;

    if (p_colonne == AeroDmsTypes::AeronefTableElement_TYPE)
    {
        elementEdite = AeroDmsTypes::AeronefTableElement_TYPE;
    }

    database->mettreAJourDonneesAeronefs( vueAeronefs->item(p_ligne, AeroDmsTypes::AeronefTableElement_IMMAT)->data(Qt::DisplayRole).toString(),
                                          vueAeronefs->item(p_ligne, p_colonne)->data(Qt::DisplayRole).toString(),
                                          elementEdite );
}