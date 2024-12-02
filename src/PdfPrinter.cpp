/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2023-2024 Clément VERMOT-DESROCHES (clement@vermot.net)

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

#include "PdfPrinter.h"
#include "PdfPrinterWorker.h"

#include <QtPdf>

PdfPrinter::PdfPrinter()
{
    progressionImpression = new DialogueProgressionImpression(this);
}

AeroDmsTypes::EtatImpression PdfPrinter::imprimerDossier(const QString p_dossier,
    const AeroDmsTypes::ParametresImpression p_parametresImpression)
{
    dossierAImprimer = p_dossier;
    parametresImpression = p_parametresImpression;
    imprimerLaDemandeAgrafage();

    if (demandeImpressionEstConfirmee)
    {
        if (progressionImpression->exec() == QDialog::Rejected)
        {
            return AeroDmsTypes::EtatImpression_TERMINEE;
        }
    }
    return AeroDmsTypes::EtatImpression_ANNULEE_PAR_UTILISATEUR;
}

AeroDmsTypes::EtatImpression PdfPrinter::imprimerFichier(const QString p_fichier,
    const AeroDmsTypes::ParametresImpression p_parametresImpression)
{
    fichierAImprimer = p_fichier;
    parametresImpression = p_parametresImpression;
    imprimerLaDemande();

    if (demandeImpressionEstConfirmee)
    {
        if (progressionImpression->exec() == QDialog::Rejected)
        {
            return AeroDmsTypes::EtatImpression_TERMINEE;
        }
    }
    return AeroDmsTypes::EtatImpression_ANNULEE_PAR_UTILISATEUR;
}

void PdfPrinter::imprimerLaDemande()
{
    liste.clear();
    if (selectionnerImprimante(imprimante, parametresImpression))
    {
        //On demande l'affichage de la fenêtre de génération
        ouvrirFenetreProgressionImpression(1);

        imprimer(imprimante, parametresImpression.forcageImpressionRecto);

        progressionImpression->traitementFichierSuivant();
    }
}

void PdfPrinter::imprimerLaDemandeAgrafage()
{
    liste.clear();
    if (selectionnerImprimante(imprimante, parametresImpression))
    {
        //On compte les fichiers
        QDir repertoire(dossierAImprimer, "*.pdf", QDir::QDir::Name, QDir::Files);

        liste = repertoire.entryInfoList();
        //Le fichier assemblé est forcément le dernier de la liste car tous les fichiers sont suffixés
        //par un chiffre dans l'ordre de génération. Si la fichier assemblé est présent, on le supprime 
        //de la liste
        if (liste.last().filePath().contains("FichiersAssembles.pdf"))
        {
            liste.removeLast();
        }

        //On demande l'affichage de la fenêtre de génération
        ouvrirFenetreProgressionImpression(liste.size());
        imprimerFichierSuivant();
    }
}

void PdfPrinter::imprimerFichierSuivant()
{
    if (liste.size() > 0)
    {
        QFileInfo fichier = liste.takeFirst();
        fichierAImprimer = fichier.filePath();
        imprimer(imprimante, parametresImpression.forcageImpressionRecto);        
    }
    QThread::usleep(250);
    progressionImpression->traitementFichierSuivant();
    QThread::usleep(250);
}

bool PdfPrinter::selectionnerImprimante(QPrinter& p_printer, 
    const AeroDmsTypes::ParametresImpression p_parametresImpression)
{
    p_printer.setPrinterName(p_parametresImpression.imprimante);
    p_printer.setColorMode(p_parametresImpression.modeCouleurImpression);
    p_printer.setResolution(p_parametresImpression.resolutionImpression);

    QPrintDialog dialog(&p_printer, this);
    dialog.setOption(QAbstractPrintDialog::PrintSelection, false);
    dialog.setOption(QAbstractPrintDialog::PrintPageRange, false);
    dialog.setOption(QAbstractPrintDialog::PrintCollateCopies, false);
    dialog.setOption(QAbstractPrintDialog::PrintToFile, false);
    dialog.setOption(QAbstractPrintDialog::PrintShowPageSize, false);
    dialog.setWindowTitle(QApplication::applicationName() + " - " + tr("Imprimer la demande de subvention"));

    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    p_printer.setResolution(p_parametresImpression.resolutionImpression);
    //En mode forçage recto on force le mode à recto-verso : on insèrera ensuite
    //un page blanche entre chaque page qui fera que le rendu sera un recto simple
    if (p_parametresImpression.forcageImpressionRecto)
    {
        p_printer.setDuplex(QPrinter::DuplexLongSide);
    }

    demandeImpressionEstConfirmee = true;
    return true;
}

void PdfPrinter::imprimer(QPrinter &printer, const bool forcerImpressionRecto)
{
    auto* worker = new PdfPrinterWorker(fichierAImprimer, &printer, forcerImpressionRecto, this);

    connect(worker, &PdfPrinterWorker::progress, this, [this](int currentPage, int totalPages) {
        progressionImpression->traitementPageSuivante(currentPage, totalPages);
        });

    connect(worker, &PdfPrinterWorker::finished, this, [this, worker]() {
        imprimerFichierSuivant();
        worker->deleteLater();
        });

    connect(worker, &PdfPrinterWorker::error, this, [this](const QString& message) {
        QMessageBox::critical(nullptr, tr("Erreur d'impression"), message);
        });

    // Lance le thread
    worker->start();
}

void PdfPrinter::ouvrirFenetreProgressionImpression(const int p_nombreDeFichiersAImprimer)
{
    progressionImpression->setMaximumFichier(p_nombreDeFichiersAImprimer);

    connect(progressionImpression, SIGNAL(accepted()), this, SLOT(detruireFenetreProgressionImpression()));

    progressionImpression->update();
}

void PdfPrinter::detruireFenetreProgressionImpression()
{
    delete progressionImpression;
    progressionImpression = nullptr;
}
