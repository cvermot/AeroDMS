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

#include <QtPdf>

PdfPrinter::PdfPrinter()
{
    progressionImpression = new DialogueProgressionImpression(this);
}

AeroDmsTypes::EtatImpression PdfPrinter::imprimerDossier(const QString p_dossier,
    const AeroDmsTypes::ParametresImpression p_parametresImpression)
{
    dossierAImprimer = p_dossier;
    imprimerLaDemandeAgrafage(p_parametresImpression);

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
    imprimerLaDemande(p_parametresImpression);

    if (demandeImpressionEstConfirmee)
    {
        if (progressionImpression->exec() == QDialog::Rejected)
        {
            return AeroDmsTypes::EtatImpression_TERMINEE;
        }
    }
    return AeroDmsTypes::EtatImpression_ANNULEE_PAR_UTILISATEUR;
}

void PdfPrinter::imprimerLaDemande(const AeroDmsTypes::ParametresImpression p_parametresImpression)
{
    QPrinter imprimante;
    if (selectionnerImprimante(imprimante, p_parametresImpression))
    {
        //On demande l'affichage de la fenêtre de génération
        ouvrirFenetreProgressionImpression(1);

        QThread::usleep(500);

        imprimer(imprimante, p_parametresImpression.forcageImpressionRecto);

        progressionImpression->traitementFichierSuivant();
    }
}

void PdfPrinter::imprimerLaDemandeAgrafage(const AeroDmsTypes::ParametresImpression p_parametresImpression)
{
    QPrinter imprimante;
    if (selectionnerImprimante(imprimante, p_parametresImpression))
    {
        //On compte les fichiers
        QDir repertoire(dossierAImprimer, "*.pdf", QDir::QDir::Name, QDir::Files);

        QFileInfoList liste = repertoire.entryInfoList();
        //Le fichier assemblé est forcément le dernier de la liste car tous les fichiers sont suffixés
        //par un chiffre dans l'ordre de génération. Si la fichier assemblé est présent, on le supprime 
        //de la liste
        if (liste.last().filePath().contains("FichiersAssembles.pdf"))
        {
            liste.removeLast();
        }

        //On demande l'affichage de la fenêtre de génération
        ouvrirFenetreProgressionImpression(liste.size());

        QThread::usleep(500);

        for (QFileInfo fichier : liste)
        {
            //On imprime tout 
            fichierAImprimer = fichier.filePath();
            imprimer(imprimante, p_parametresImpression.forcageImpressionRecto);
        }
        progressionImpression->traitementFichierSuivant();
    }
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
    //En mode forcage recto on force le mode à recto-verso : on inserera ensuite
    //un page blanche entre chaque page qui fera que le rendu sera un recto simple
    if (p_parametresImpression.forcageImpressionRecto)
    {
        p_printer.setDuplex(QPrinter::DuplexLongSide);
    }

    demandeImpressionEstConfirmee = true;
    return true;
}

void PdfPrinter::imprimer(QPrinter& p_printer, const bool p_forcerImpressionRecto)
{
    progressionImpression->traitementFichierSuivant();

    QPdfDocument* doc = new QPdfDocument(this);
    doc->load(fichierAImprimer);

    int attenteChargementFichier = 0;
    while (doc->status() != QPdfDocument::Status::Ready
        || attenteChargementFichier > 500)
    {
        attenteChargementFichier++;
        QThread::usleep(10);
    }

    if (doc->status() == QPdfDocument::Status::Ready)
    {
        progressionImpression->setMaximumPage(doc->pageCount());

        QPainter painter;
        painter.begin(&p_printer);

        for (int i = 0; i < doc->pageCount(); i++)
        {
            progressionImpression->traitementPageSuivante();
            QSizeF size = doc->pagePointSize(i);
            QImage image = doc->render(i,
                QSize(size.width() * p_printer.resolution() / AeroDmsTypes::K_DPI_PAR_DEFAUT,
                    size.height() * p_printer.resolution() / AeroDmsTypes::K_DPI_PAR_DEFAUT));

            //Si la page du PDF est en paysage, on retourne l'image pour la place en portrait
            //pour l'impression
            if (size.width() > size.height())
            {
                QTransform transformation;
                transformation.rotate(270);
                image = image.transformed(transformation);
                painter.drawImage(0, 0, image);
            }
            else
            {
                painter.drawImage(0, 0, image);
            }

            //S'il reste des pages derrière... on démarre une nouvelle page
            if (i + 1 < doc->pageCount())
            {
                p_printer.newPage();

                //Et si on est en mode forcage Recto, on ajoute une page blanche
                if (p_forcerImpressionRecto)
                {
                    p_printer.newPage();
                }
            }
        }
        progressionImpression->traitementPageSuivante();

        painter.end();
    }
    delete doc;
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
