/******************************************************************************\
<QUas : a Free Software logbook for UAS operators>
Copyright (C) 2023-2026 Clément VERMOT-DESROCHES (clement@vermot.net)

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


DialogueGestionAeronefs::DialogueGestionAeronefs(ManageDb* db, 
    QWidget* parent) : QDialog(parent)
{
    database = db;

    QGridLayout* mainLayout = new QGridLayout(this);
    resize(400, 450);

    setLayout(mainLayout);

    setWindowTitle(QApplication::applicationName() + " - " + tr("Gestion des aéronefs"));

    listeCompensationCarbone = new QVector<QCheckBox*>();

    vueAeronefs = new QTableWidget(0, AeroDmsTypes::AeronefTableElement_NB_COLONNES, this);
    vueAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTableElement_IMMAT, new QTableWidgetItem("Immatriculation"));
    vueAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTableElement_TYPE, new QTableWidgetItem("Type"));
    vueAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTableElement_COMPENSATION_CARBONE, new QTableWidgetItem(tr("Compensation CO₂")));
    vueAeronefs->setEditTriggers(QAbstractItemView::DoubleClicked);

    connect(vueAeronefs, &QTableWidget::cellChanged, this, &DialogueGestionAeronefs::sauvegarderDonneesSaisies);

    mainLayout->addWidget(vueAeronefs, 0, 0);
}

void DialogueGestionAeronefs::peuplerListeAeronefs()
{
    QVectorIterator<QCheckBox*> itCc(*listeCompensationCarbone);
    while (itCc.hasNext())
    {
        QCheckBox* cb = itCc.next();
        delete cb;
    }
    listeCompensationCarbone->clear();
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

        QCheckBox* compensationCarbone = new QCheckBox();
        compensationCarbone->setChecked(aeronef.emissionsSontCompensees);
        compensationCarbone->setToolTip(tr("Permet d'indiquer si l'exploitant compense les émissions du CO2 pour cet avion"));
        compensationCarbone->setProperty("immat", aeronef.immatriculation);
        connect(compensationCarbone, &QCheckBox::checkStateChanged, this, &DialogueGestionAeronefs::changementCompensationCarbone);
        listeCompensationCarbone->append(compensationCarbone);
        vueAeronefs->setCellWidget(i, AeroDmsTypes::AeronefTableElement_COMPENSATION_CARBONE,
            listeCompensationCarbone->last());
    }
    vueAeronefs->resizeColumnsToContents();

    vueAeronefs->blockSignals(false);
}

void DialogueGestionAeronefs::sauvegarderDonneesSaisies(const int p_ligne, 
    const int p_colonne)
{
    database->mettreAJourTypeAeronef( vueAeronefs->item(p_ligne, AeroDmsTypes::AeronefTableElement_IMMAT)->data(Qt::DisplayRole).toString(),
                                      vueAeronefs->item(p_ligne, AeroDmsTypes::AeronefTableElement_TYPE)->data(Qt::DisplayRole).toString());
}

void DialogueGestionAeronefs::changementCompensationCarbone()
{
    QCheckBox* cb = static_cast<QCheckBox*>(sender());

    database->mettreAJourCompensationCarboneAeronef(cb->property("immat").toString(),
        cb->isChecked());
}
