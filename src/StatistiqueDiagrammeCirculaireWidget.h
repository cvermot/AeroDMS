// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PIEDRILLDOWNWIDGET_H
#define PIEDRILLDOWNWIDGET_H

#include "StatistiqueWidget.h"
#include "ManageDb.h"

class StatistiqueDiagrammeCirculaireWidget : public StatistiqueWidget
{
    Q_OBJECT
private:
    AeroDmsTypes::ListeSubventionsParPilotes recupererSubventionsPilotes(ManageDb* p_db, const int p_annee);
public:
    explicit StatistiqueDiagrammeCirculaireWidget(ManageDb* p_db, const int p_annee, const AeroDmsTypes::Statistiques p_statistique, QWidget* parent = nullptr);
};

#endif
