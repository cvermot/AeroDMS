/******************************************************************************\
<AeroDmsLauncher : lanceur pour AeroDms>
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

#include <QApplication>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <QSplashScreen>
#include <QThread>
#include <QCommandLineParser>
#include <QScreen>

#include "resource.h"

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE* fp_out = NULL;
        FILE* fp_err = NULL;
        freopen_s(&fp_out, "CONOUT$", "w", stdout);
        freopen_s(&fp_err, "CONOUT$", "w", stderr);
    }
#endif

    QApplication app(argc, argv);

    QPixmap pixmap(":/AeroDmsLauncher/ressources/splash.webp");
    QSize tailleSplashScreen = app.primaryScreen()->size() * 0.85;
    pixmap = pixmap.scaled(tailleSplashScreen, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QSplashScreen* splash = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
    splash->showMessage("Chargement en cours...", Qt::AlignLeft | Qt::AlignBottom, Qt::black);
    splash->show();

    QApplication::setApplicationVersion(VER_PRODUCTVERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("Lanceur AeroDMS"));
    QCommandLineOption helpOption(QStringList() << "h" << "help",
        QApplication::tr("Affiche l'aide du logiciel"));
    parser.addOption(helpOption);
    QCommandLineOption versionOption(QStringList() << "v" << "version",
        QApplication::tr("Affiche la version du logiciel"));
    parser.addOption(versionOption);
    parser.process(app);

    if (parser.isSet(versionOption))
    {
        QDate date = QDate::fromString(__DATE__, "MMM dd yyyy");
        if (!date.isValid())
        {
            date = QDate::fromString(__DATE__, "MMM  d yyyy");
        }
        const QTime heure = QTime::fromString(__TIME__);

        QTextStream(stdout) << "\n";
        QTextStream(stdout) << "AeroDmsLauncher v" 
            << QApplication::applicationVersion() 
            << "\n";
        QTextStream(stdout) 
            << "Version compilée le " 
            << date.toString("dd/MM/yyyy") 
            << " à " 
            << heure.toString("hh'h'mm") 
            << "\n";
    }
    if (parser.isSet(helpOption))
    {
        QTextStream(stdout) << "\n";
        parser.showHelp();
        QTextStream(stdout) << "\n";
    }

    // Chemin vers le répertoire contenant "legacy.dll"
    const QString opensslModulesPath = QDir::currentPath(); // Répertoire actuel de l'exécutable

    // Définir la variable d'environnement OPENSSL_MODULES
    if (!qputenv("OPENSSL_MODULES", opensslModulesPath.toUtf8()))
    {
        splash->close();
        QMessageBox::critical(nullptr,
            QApplication::applicationName() + " - Erreur",
            "Impossible de définir la variable d'environnement OPENSSL_MODULES comme : "
            + opensslModulesPath
            + ". <br /><br />"
            + "Cette variable d'environnement est indispensable au démarrage d'AeroDms.<br />Impossible d'exécuter AeroDms.");
        return -1;
    }

    QString appName = "AeroDms.exe";
    //Si le nom du lanceur vaut AeroDms.exe, c'est que le binaire à lancer vaut Aero.exe
    //Sinon, on vérifie ce qui existe : si Aero.exe existe, on lancera Aero.exe
    if ( QCoreApplication::applicationFilePath().split("/").last() == appName
         || QFile::exists(QDir::currentPath() + QDir::separator() + "Aero.exe"))
    {
        appName = "Aero.exe";
    }
    //Sinon, on lancer AeroDms.exe

    // Chemin vers l'application principale
    const QString program = QDir::currentPath() + QDir::separator() + appName;
    // Chemin vers DLL legacy
    const QString legacyDll = QDir::currentPath() + QDir::separator() + "legacy.dll";

    splash->showMessage("AeroDmsLaucher v"+QApplication::applicationVersion() + "\nVérification de la présence de legacy.dll", Qt::AlignLeft | Qt::AlignBottom, Qt::black);
    // Vérifie si la DLL critique existe avant de le lancer
    if (!QFile::exists(legacyDll)) 
    {
        splash->close();
        QMessageBox::critical(nullptr, 
            QApplication::applicationName() + " - Erreur", 
            "La DLL legacy.dll est introuvable dans : "
            + program
            + ". <br /><br />"
            + "Cette DLL est indispensable au fonctionnement d'AeroDms.<br />Impossible d'exécuter AeroDms.");
        return -1;
    }

    // Vérifie si le fichier existe avant de le lancer
    if (!QFile::exists(program)) 
    {
        splash->close();
        QMessageBox::critical(nullptr, 
            QApplication::applicationName() + " - Erreur",
            "L'exécutable principal "
            + appName 
            +" est introuvable dans : "
            + program);
        return -1;
    }

    splash->showMessage("AeroDmsLaucher v" + QApplication::applicationVersion() + "\nLancement de " + appName, Qt::AlignLeft | Qt::AlignBottom, Qt::black);
    // Préparer le lancement d'AeroDms.exe
    QProcess process;
    process.setProgram(program);

    // Arguments supplémentaires : on passe la taille du splashscreen, ce qui évitera une désynchro entre les 2 tailles
    // et évite de voir la transition entre le launcher et l'application principale
    QStringList arguments; 
    arguments << "--splashWidth" << QString::number(tailleSplashScreen.width())
        << "--splashHeight" << QString::number(tailleSplashScreen.height());
    process.setArguments(arguments);

    // Lancer l'application principale
    if (!process.startDetached()) 
    {
        splash->close();
        QMessageBox::critical(nullptr, "Erreur", "Erreur lors du lancement de l'application principale : " + process.errorString());
        return -1;
    }

    splash->showMessage("AeroDmsLaucher v" + QApplication::applicationVersion() + "\nOuverture de " + appName, Qt::AlignRight | Qt::AlignBottom, Qt::black);
    //On donne un peu de temps au processus AeroDms d'afficher son propre bootscreen
    QThread::sleep(2);

    return 0; // Le launcher se termine ici
}