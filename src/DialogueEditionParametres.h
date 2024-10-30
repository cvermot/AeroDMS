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
    DialogueEditionParametres();
    DialogueEditionParametres(const AeroDmsTypes::ParametresMetier p_parametresMetiers,
        const AeroDmsTypes::ParametresSysteme p_parametresSysteme,
        QWidget* parent = nullptr);

private:
    

public slots:

signals:
  
};

#endif // DIALOGUEEDITIONPARAMETRES_H

