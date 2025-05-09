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

#ifndef DIALOGUEEDITIONPARAMETRES_H
#define DIALOGUEEDITIONPARAMETRES_H
#include <QDialog>
#include <QtGui>
#include <QtWidgets>

#include "AeroDmsTypes.h"

class DialogueEditionParametres : public QDialog
{
    Q_OBJECT

public:
    DialogueEditionParametres(const AeroDmsTypes::ParametresMetier p_parametresMetiers,
        const AeroDmsTypes::ParametresSysteme p_parametresSysteme,
        const bool p_editionParametresCritiques,
        QWidget* parent = nullptr);

private:
    QTabWidget* onglets = nullptr;
    QLineEdit* imprimante = nullptr;
    QLineEdit* objetChequeDispo = nullptr;
    QTextEdit* texteChequeDispo = nullptr;
    QLineEdit* objetVirementEffectue = nullptr;
    QTextEdit* texteVirementEffectue = nullptr;
    QLineEdit* objetSubventionRestantes = nullptr;
    QTextEdit* texteSubventionRestantes = nullptr;
    QLineEdit* objetAutresMailings = nullptr;
    QLineEdit* nomTresorier = nullptr;
    QCheckBox* autoriserVirement = nullptr;
    QSpinBox* delaisGardeBdd = nullptr;
    QDoubleSpinBox* proportionRemboursementBalade = nullptr;
    QDoubleSpinBox* proportionParticipationBalade = nullptr;
    QDoubleSpinBox* proportionRemboursementEntrainement = nullptr;
    QDoubleSpinBox* montantCotisationPilote = nullptr;
    QDoubleSpinBox* plafondHoraireRemboursementEntrainement = nullptr;
    QDoubleSpinBox* montantSubventionEntrainement = nullptr;

    QLineEdit* cheminBdd = nullptr;
    QLineEdit* facturesSaisies = nullptr;
    QLineEdit* factureATraiter = nullptr;
    QLineEdit* sortieFichiersGeneres = nullptr;
    QComboBox* modeFonctionnementLogiciel = nullptr;
    QLineEdit* adresseServeurModeExterne = nullptr;
    QLineEdit* loginServeurModeExterne = nullptr;
    QLineEdit* motDePasseServeurModeExterne = nullptr;
    QComboBox* utiliserRessourcesHtmlInternes = nullptr;
    QLineEdit* loginDaca = nullptr;
    QLineEdit* motDePasseDaca = nullptr;
    QSpinBox* periodiciteVerificationNouvellesFactures = nullptr;

    QComboBox* impressionCouleur = nullptr;
    QComboBox* resolutionImpression = nullptr;
    QCheckBox* forcageImpressionRectoSimple = nullptr;
    QSpinBox* margesHautBas = nullptr;
    QSpinBox* margesGaucheDroite = nullptr;

    QPushButton* boutonSelectionFactureATraiter = nullptr;
    QPushButton* boutonSelectionFacturesSaisies = nullptr;
    QPushButton* boutonSelectionSortieFichiersGeneres = nullptr;
    
    void peuplerResolutionImpression(const int p_resolutionMax);

public slots:
    void selectionnerImprimante();
    void selectionnerBdd();
    void selectionnerRepertoire();
    void enregistrerParametres();
    void afficherMotDePasse();
    void masquerMotDePasse();
    void afficherMotDePasseModeExterne();
    void masquerMotDePasseModeExterne();

signals:
    void envoyerParametres( const AeroDmsTypes::ParametresMetier parametresMetiers, 
                            const AeroDmsTypes::ParametresSysteme parametresSysteme);
  
};

#endif // DIALOGUEEDITIONPARAMETRES_H
