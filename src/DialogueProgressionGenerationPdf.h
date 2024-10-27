#ifndef DIALOGUEPROGRESSIONGENERATIONPDF_H
#define DIALOGUEPROGRESSIONGENERATIONPDF_H
#include <QDialog>
#include <QtGui>
#include <QtWidgets>

class DialogueProgressionGenerationPdf : public QDialog
{
    Q_OBJECT

public:
    DialogueProgressionGenerationPdf();
    DialogueProgressionGenerationPdf(QWidget* parent = nullptr);

    void setLabelText(const QString p_texte);
    void setMaximum(const int p_maximum);
    void setValue(const int p_valeur);

private:
    QPushButton *boutonFermer = nullptr;
    QPushButton *boutonOuvrirPdf = nullptr;
    QPushButton *boutonImprimer = nullptr;
    QProgressBar* barreDeProgression = nullptr;
    QLabel* label = nullptr;

public slots:
    void demanderImpression();

signals:
    void imprimer();
};

#endif // DIALOGUEPROGRESSIONGENERATIONPDF_H

