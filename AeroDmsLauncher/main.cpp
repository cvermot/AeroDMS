/******************************************************************************\
<AeroDmsLauncher : lanceur pour AeroDms>
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

#include <QApplication>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <QDebug>

int main(int argc, char* argv[])
{
    // Crée un QCoreApplication pour votre logique et un QApplication pour l'interface graphique
    QApplication app(argc, argv);

    // Chemin vers le répertoire contenant "legacy.dll"
    QString opensslModulesPath = QDir::currentPath(); // Répertoire actuel de l'exécutable

    // Définir la variable d'environnement OPENSSL_MODULES
    qputenv("OPENSSL_MODULES", opensslModulesPath.toUtf8());
    qDebug() << "OPENSSL_MODULES set to:" << opensslModulesPath;

    // Chemin vers l'application principale
    QString program = QDir::currentPath() + QDir::separator() + "AeroDms.exe";

    // Arguments supplémentaires (si besoin)
    QStringList arguments; // Exemple : arguments << "--debug" << "--config=config.ini";

    // Vérifie si le fichier existe avant de le lancer
    if (!QFile::exists(program)) {
        QMessageBox::critical(nullptr, "Erreur", "L'exécutable principal AeroDms.exe est introuvable dans : " + program);
        return -1;
    }

    // Lancer AeroDms.exe
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);

    // Lancer l'application principale et attendre sa fin
    if (!process.startDetached()) {
        QMessageBox::critical(nullptr, "Erreur", "Erreur lors du lancement de l'application principale : " + process.errorString());
        return -1;
    }

    //QMessageBox::information(nullptr, "Succès", "L'application principale a été lancée avec succès.");
    return 0; // Le launcher se termine ici
}