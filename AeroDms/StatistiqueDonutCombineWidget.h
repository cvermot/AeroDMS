// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDONUTCOMBINEWIDGET_H
#define STATISTIQUEDONUTCOMBINEWIDGET_H

#include <QChart>

#include "StatistiqueWidget.h"

class StatistiqueDonutCombineWidget : public StatistiqueWidget
{
    Q_OBJECT
public:
    explicit StatistiqueDonutCombineWidget( ManageDb* p_db,
                                            const AeroDmsTypes::Statistiques p_statistique,
                                            QWidget* parent = nullptr,
                                            const int p_annee = AeroDmsTypes::K_INIT_INT_INVALIDE,
                                            const int p_options = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS,
                                            const QChart::AnimationOption p_animation = QChart::AllAnimations,
                                            const bool p_legende = true,
                                            const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres = AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES);

private:
    const Qt::GlobalColor recupererNouvelleCouleur();

    int indiceCouleurEnCours = 0;
};

#endif //STATISTIQUEDONUTCOMBINEWIDGET_H
