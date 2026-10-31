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

#include "AeroDms.h"
#include <QtWidgets/QApplication>

//Debug
QtMessageHandler originalHandler = nullptr;

void logToFile(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QString message = qFormatLogMessage(type, context, msg);
    static FILE* f = fopen("log.txt", "a");
    fprintf(f, "%s\n", qPrintable(message));
    fflush(f);

    if (originalHandler)
        originalHandler(type, context, msg);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //originalHandler = qInstallMessageHandler(logToFile);

    QTranslator* translator = new QTranslator();
    if (translator->load("qt_fr", QCoreApplication::applicationDirPath()+"/translations")) 
    {
        a.installTranslator(translator);
    }

    QCommandLineParser parser;
    QCommandLineOption splashWidthOpt("splashWidth", "Largeur du splashscreen", "0");
    QCommandLineOption splashHeightOpt("splashHeight", "Hauteur du splashscreen", "0");
    parser.addOption(splashWidthOpt);
    parser.addOption(splashHeightOpt);
    parser.process(a);

    QSize tailleSplashScreen(parser.value(splashWidthOpt).toInt(), 
        parser.value(splashHeightOpt).toInt());

    AeroDms aeroDms(tailleSplashScreen);
    aeroDms.show();
    
    return a.exec();
}
