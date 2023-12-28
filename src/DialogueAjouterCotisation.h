#ifndef DIALOGUEAJOUTERCOTISATION_H
#define DIALOGUEAJOUTERCOTISATION_H
#include <QDialog>
#include "ManageDb.h"

class DialogueAjouterCotisation : public QDialog
{
    Q_OBJECT

public:
    DialogueAjouterCotisation();
    DialogueAjouterCotisation(ManageDb* db, QWidget* parent = nullptr);

    void mettreAJourLeContenuDeLaFenetre();
    AeroDmsTypes::CotisationAnnuelle recupererInfosCotisationAAjouter();

private:
    QComboBox* listePilote;
    QComboBox* annee;
    QDoubleSpinBox* montant;
    QDoubleSpinBox* montantSubventionAnnuelle;

    ManageDb* database;

    QDialogButtonBox* buttonBox;
    QPushButton* cancelButton;
    QPushButton* okButton;
    QWidget* extension;

    void peuplerListePilote();
};

#endif // DIALOGUEAJOUTERCOTISATION_H

