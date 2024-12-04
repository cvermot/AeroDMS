// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDIAGRAMMECIRCULAIREWIDGET_H
#define STATISTIQUEDIAGRAMMECIRCULAIREWIDGET_H

#include "StatistiqueWidget.h"
#include "ManageDb.h"
#include <QChart>

class StatistiqueDiagrammeCirculaireWidget : public StatistiqueWidget
{
    Q_OBJECT
private:
    AeroDmsTypes::ListeSubventionsParPilotes recupererSubventionsPilotes(ManageDb* p_db, const int p_annee);
public:
    explicit StatistiqueDiagrammeCirculaireWidget( ManageDb* p_db, 
                                                   const int p_annee, 
                                                   const AeroDmsTypes::Statistiques p_statistique, 
                                                   QWidget* parent = nullptr,
                                                   const QChart::AnimationOption p_animation = QChart::AllAnimations,
                                                   const bool p_legende = true,
                                                   const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres = AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES) ;
};

#endif //STATISTIQUEDIAGRAMMECIRCULAIREWIDGET_H
