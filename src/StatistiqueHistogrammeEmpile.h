// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BARPERCENTWIDGET_H
#define BARPERCENTWIDGET_H

#include "StatistiqueWidget.h"
#include "ManageDb.h"

class StatistiqueHistogrammeEmpile : public StatistiqueWidget
{
    Q_OBJECT
public:
    explicit StatistiqueHistogrammeEmpile(ManageDb* p_db, QWidget* parent = nullptr);
};

#endif
