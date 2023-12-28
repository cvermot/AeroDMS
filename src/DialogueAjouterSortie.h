#ifndef DIALOGUEAJOUTERSORTIE_H
#define DIALOGUEAJOUTERSORTIE_H
#include <QDialog>
#include "ManageDb.h"

class DialogueAjouterSortie : public QDialog
{
    Q_OBJECT

public:
    DialogueAjouterSortie();
    DialogueAjouterSortie(QWidget* parent = nullptr);

    void mettreAJourLeContenuDeLaFenetre();
    AeroDmsTypes::Sortie recupererInfosSortieAAjouter();

private:
    QLineEdit* nomSortie;
    QDateEdit* date;

    QDialogButtonBox* buttonBox;
    QPushButton* cancelButton;
    QPushButton* okButton;
};

#endif // DIALOGUEAJOUTERSORTIE_H

