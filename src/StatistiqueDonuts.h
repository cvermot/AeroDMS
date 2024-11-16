// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDONUTS_H
#define STATISTIQUEDONUTS_H

#include "StatistiqueWidget.h"
#include "ManageDb.h"
#include <QChart>

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

    void afficherStatsPilotes(ManageDb* p_db, 
        const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres, 
        QChart::AnimationOption p_animation);
    void afficherStatsAeronefs(ManageDb* p_db, 
        int p_annee, 
        const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres, 
        QChart::AnimationOption p_animation);
};

#endif //STATISTIQUEDONUTS_H
