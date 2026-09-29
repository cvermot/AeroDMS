/******************************************************************************\
<QUas : a Free Software logbook for UAS operators>
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

// Plan (pseudocode détaillé) :
// - Ajouter une sous-classe de QComboBox nommée NoWheelComboBox.
// - Redéfinir la méthode wheelEvent pour ignorer l'événement (event->ignore()),
//   afin d'empêcher le défilement par la roulette de la souris de changer la valeur.
// - Remplacer les créations de QComboBox dans ce fichier par NoWheelComboBox,
//   de sorte que toutes les instances utilisées dans cette fenêtre ignorent la roulette.
// - Laisser les pointeurs et vecteurs de type QComboBox* pour compatibilité,
//   car NoWheelComboBox est un QComboBox dérivé.
// - Aucun changement de logique métier, uniquement comportement UX pour la roulette.

#include "DialogueGestionTypesAeronefs.h"
#include "AeroDmsServices.h"

#include <QtWidgets>

DialogueGestionTypesAeronefs::DialogueGestionTypesAeronefs(ManageDb* db,
    QWidget* parent) : QDialog(parent)
{
    database = db;

    QGridLayout* mainLayout = new QGridLayout(this);
    resize(625, 450);

    setLayout(mainLayout);

    setWindowTitle(QApplication::applicationName() + " - " + tr("Gestion des types d'aéronefs"));

    listeConsommation = new QVector<QDoubleSpinBox*>();
    listeUniteTypeConso = new QVector<QComboBox*>();
    listeTypeDecompte = new QVector<QComboBox*>();

    vueTypesAeronefs = new QTableWidget(0, AeroDmsTypes::AeronefTypeTableElement_NB_COLONNES, this);
    vueTypesAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTypeTableElement_TYPE_AERONEF, new QTableWidgetItem(tr("Type")));
    vueTypesAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTypeTableElement_MARQUE_AERONEF, new QTableWidgetItem(tr("Marque")));
    vueTypesAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTypeTableElement_CONSOMMATION, new QTableWidgetItem(tr("Consommation")));
    vueTypesAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTypeTableElement_UNITE_TYPE_CONSOMMATION, new QTableWidgetItem(tr("Unité")));
    vueTypesAeronefs->setHorizontalHeaderItem(AeroDmsTypes::AeronefTypeTableElement_TYPE_DECOMPTE, new QTableWidgetItem(tr("Récurrence")));
    vueTypesAeronefs->setEditTriggers(QAbstractItemView::DoubleClicked);

    connect(vueTypesAeronefs, &QTableWidget::cellChanged, this, &DialogueGestionTypesAeronefs::changementDonneesTexte);

    mainLayout->addWidget(vueTypesAeronefs, 0, 0);
}

void DialogueGestionTypesAeronefs::peuplerListeTypesAeronefs()
{
    vueTypesAeronefs->clearContents();
    QVectorIterator<QDoubleSpinBox*> itC(*listeConsommation);
    while (itC.hasNext())
    {
        QDoubleSpinBox* sb = itC.next();
        delete sb;
    }
    listeConsommation->clear();
    QVectorIterator<QComboBox*> itUtc(*listeUniteTypeConso);
    while(itUtc.hasNext())
    {
        QComboBox* cb = itUtc.next();
        delete cb;
	}
    listeUniteTypeConso->clear();
    QVectorIterator<QComboBox*> itTd(*listeTypeDecompte);
    while (itTd.hasNext())
    {
        QComboBox* cb = itTd.next();
        delete cb;
    }
    listeTypeDecompte->clear();
    

    const AeroDmsTypes::ListeTypesAeronefs listeTypesAeronefs = database->recupererListeTypesAeronefs();

    vueTypesAeronefs->blockSignals(true);

    vueTypesAeronefs->setRowCount(listeTypesAeronefs.size());
    for (int i = 0; i < listeTypesAeronefs.size(); i++)
    {
        const AeroDmsTypes::TypeAeronef aeronefType = listeTypesAeronefs.at(i);
        QTableWidgetItem* itemType = new QTableWidgetItem(aeronefType.type);
        itemType->setFlags(itemType->flags() & ~Qt::ItemIsEditable);
        vueTypesAeronefs->setItem(i, AeroDmsTypes::AeronefTypeTableElement_TYPE_AERONEF, itemType);

        vueTypesAeronefs->setItem(i, AeroDmsTypes::AeronefTypeTableElement_MARQUE_AERONEF, new QTableWidgetItem(aeronefType.marque));

        NoWheelDoubleSpinBox* consommation = new NoWheelDoubleSpinBox();
		consommation->setMinimum(0);
		consommation->setValue(aeronefType.consommation);
        consommation->setSingleStep(1.0);
        consommation->setProperty("type", aeronefType.type);
        connect(consommation, &QDoubleSpinBox::valueChanged, this, &DialogueGestionTypesAeronefs::changementConsommation);
        listeConsommation->append(consommation);
        vueTypesAeronefs->setCellWidget(i, AeroDmsTypes::AeronefTypeTableElement_CONSOMMATION, 
            listeConsommation->last());

        NoWheelComboBox* uniteTypeConso = new NoWheelComboBox();
        uniteTypeConso->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_INCONNUE),
            tr("Indeterminé"),
            AeroDmsTypes::UniteTypeConsommation_INDEFINI);
		uniteTypeConso->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_ESSENCE),
            tr("Litres (essence)"), 
            AeroDmsTypes::UniteTypeConsommation_LITRES_ESSENCE);
        uniteTypeConso->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_GASOIL_KEROSENE),
            tr("Litres (gasoil/kérosène)"),
            AeroDmsTypes::UniteTypeConsommation_LITRES_GASOIL_KEROSENE);
        uniteTypeConso->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_AVION_ELECTRIQUE),
            tr("Kilowatt-heures"), 
            AeroDmsTypes::UniteTypeConsommation_KILOWATTHEURES);
        uniteTypeConso->setCurrentIndex(uniteTypeConso->findData(aeronefType.uniteTypeConsommation));
        uniteTypeConso->setToolTip(tr("Unité utilsée pour la consommation de cet avion.\nLa sélection essence ou gasoil/kérosène modifie la quantité\nde CO₂ émises pour chaque litre consommé."));
        uniteTypeConso->setProperty("type", aeronefType.type);
        connect(uniteTypeConso, &QComboBox::currentIndexChanged, this, &DialogueGestionTypesAeronefs::changementUnite);
		listeUniteTypeConso->append(uniteTypeConso);
		vueTypesAeronefs->setCellWidget(i, AeroDmsTypes::AeronefTypeTableElement_UNITE_TYPE_CONSOMMATION, 
            listeUniteTypeConso->last());

        NoWheelComboBox* typeDecompte = new NoWheelComboBox();
        typeDecompte->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_INCONNUE),
            tr("Indéterminé"),
            AeroDmsTypes::TypeDecompte_INDEFINI);
        typeDecompte->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_HORAIRE),
            tr("Horaire"),
            AeroDmsTypes::TypeDecompte_HORAIRE);
        typeDecompte->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_UNITAIRE),
            tr("Unitaire"),
            AeroDmsTypes::TypeDecompte_UNITAIRE);
        typeDecompte->setToolTip(tr("Permet d'indiquer si la consommation indiquée depend de la durée du vol ou si elle est unitaire pour chaque vol.\nLe cas unitaire est utilisé par exemple pour les vols planeurs, ou les émission de CO₂\nliées à la mise en l'air sont \"forfataire\" pour chaque vol et ne dépend pas de la durée du vol."));
        typeDecompte->setCurrentIndex(typeDecompte->findData(aeronefType.typeDecompte));
        typeDecompte->setProperty("type", aeronefType.type);
        connect(typeDecompte, &QComboBox::currentIndexChanged, this, &DialogueGestionTypesAeronefs::changementRecurrence);
        listeTypeDecompte->append(typeDecompte);
        vueTypesAeronefs->setCellWidget(i, AeroDmsTypes::AeronefTypeTableElement_TYPE_DECOMPTE, 
            listeTypeDecompte->last());
    }
    vueTypesAeronefs->resizeColumnsToContents();

    vueTypesAeronefs->blockSignals(false);
}

void DialogueGestionTypesAeronefs::changementConsommation()
{
    QDoubleSpinBox* sb = static_cast<QDoubleSpinBox*>(sender());

    database->mettreAJourConsommationTypeAeronef(sb->property("type").toString(),
        sb->value());
}

void DialogueGestionTypesAeronefs::changementUnite()
{
    QComboBox* cb = static_cast<QComboBox*>(sender());
    database->mettreAJourUniteTypeAeronef(cb->property("type").toString(),
        static_cast<AeroDmsTypes::UniteTypeConsommation>(cb->currentData().toInt()));
}

void DialogueGestionTypesAeronefs::changementRecurrence()
{
    QComboBox* cb = static_cast<QComboBox*>(sender());
    database->mettreAJourDecompteTypeAeronef(cb->property("type").toString(),
        static_cast<AeroDmsTypes::TypeDecompte>(cb->currentData().toInt()));
}

void DialogueGestionTypesAeronefs::changementDonneesTexte(const int p_ligne,
    const int p_colonne)
{
    database->mettreAJourMarqueAeronef(vueTypesAeronefs->item(p_ligne, AeroDmsTypes::AeronefTypeTableElement_TYPE_AERONEF)->data(Qt::DisplayRole).toString(),
        vueTypesAeronefs->item(p_ligne, AeroDmsTypes::AeronefTypeTableElement_MARQUE_AERONEF)->data(Qt::DisplayRole).toString());
}
