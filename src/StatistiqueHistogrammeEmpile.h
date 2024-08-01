// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BARPERCENTWIDGET_H
#define BARPERCENTWIDGET_H

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
                                           const QChart::AnimationOption p_animation = QChart::AllAnimations);
};

#endif
