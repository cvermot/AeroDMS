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
#ifndef AIXMPARSER_H
#define AIXMPARSER_H

#include <QWidget>

#include "ManageDb.h"

class AixmParser : public QWidget {
	Q_OBJECT

public:
	AixmParser(ManageDb *p_db,
		QWidget* p_parent = nullptr);

	void mettreAJourAerodromes(const QString p_fichierAixm);

private:
	ManageDb* db;

signals:
	void signalerMiseAJourAerodrome(int nombreTotal, int nombreTraite, int nombreCree, int nombreMisAJour);
	
};

#endif // AIXMPARSER_H
