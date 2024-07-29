// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DONUTBREAKDOWNWIDGET_H
#define DONUTBREAKDOWNWIDGET_H

#include <QChart>

#include "StatistiqueWidget.h"

class StatistiqueDonutCombineWidget : public StatistiqueWidget
{
    Q_OBJECT
public:
    explicit StatistiqueDonutCombineWidget( ManageDb* p_db,
                                            const AeroDmsTypes::Statistiques p_statistique,
                                            QWidget* parent = nullptr,
                                            const int p_annee = -1,
                                            const QChart::AnimationOption p_animation = QChart::AllAnimations,
                                            const bool p_legende = true);

private:
    const Qt::GlobalColor recupererNouvelleCouleur();

    int indiceCouleurEnCours = 0;
};

#endif
