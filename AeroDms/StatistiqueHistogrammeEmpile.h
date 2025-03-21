// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEHISTOGRAMMEEMPILE_H
#define STATISTIQUEHISTOGRAMMEEMPILE_H

#include "StatistiqueWidget.h"
#include "ManageDb.h"
#include <QChart>

class StatistiqueHistogrammeEmpile : public StatistiqueWidget
{
    Q_OBJECT
public:
    explicit StatistiqueHistogrammeEmpile( ManageDb* p_db, 
        const int p_annee,
        QWidget* parent = nullptr,
        const int p_options = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS,
        const QChart::AnimationOption p_animation = QChart::AllAnimations,
        const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres = AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES);
};

#endif //STATISTIQUEHISTOGRAMMEEMPILE_H
