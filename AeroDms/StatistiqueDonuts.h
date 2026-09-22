// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDONUTS_H
#define STATISTIQUEDONUTS_H

#include "StatistiqueWidget.h"
#include "ManageDb.h"

#include <QtGraphs/QPieSeries>

class StatistiqueDonuts : public StatistiqueWidget
{
    Q_OBJECT
public:
    StatistiqueDonuts( ManageDb* p_db, 
                       const AeroDmsTypes::Statistiques p_statistique, 
                       QWidget* parent = nullptr,
                       int p_annee = AeroDmsTypes::K_INIT_INT_INVALIDE,
                       const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres = AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES,
                       const AeroDmsTypes::OptionsDonneesStatistiques p_options = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS,
                       const bool p_legende = false);

public slots:
    void explodeSlice(bool exploded);

private:
    QList<QPieSeries*> m_donuts;

    void afficherStatsPilotes(ManageDb* p_db, 
        const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres, 
        bool p_animation,
        bool p_legende);
    void afficherStatsAeronefs(ManageDb* p_db, 
        int p_annee, 
        const AeroDmsTypes::OptionsDonneesStatistiques p_options,
        const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres, 
        bool p_animation,
        bool p_legende);
};

#endif //STATISTIQUEDONUTS_H
