// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef NESTEDDONUTSWIDGET_H
#define NESTEDDONUTSWIDGET_H

#include "StatistiqueWidget.h"
#include "ManageDb.h"

QT_FORWARD_DECLARE_CLASS(QPieSeries)

class StatistiqueDonuts : public StatistiqueWidget
{
    Q_OBJECT
public:
    StatistiqueDonuts( ManageDb* p_db, 
                       const AeroDmsTypes::Statistiques p_statistique, 
                       QWidget* parent = nullptr,
                       int p_annee = -1,
                       const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres = AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES);

public slots:
    void explodeSlice(bool exploded);

private:
    QList<QPieSeries*> m_donuts;

    void afficherStatsPilotes(ManageDb* p_db, const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres);
    void afficherStatsAeronefs(ManageDb* p_db, int p_annee, const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres);
};

#endif
