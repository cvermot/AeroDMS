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
    StatistiqueDonuts(ManageDb* p_db, QWidget* parent = nullptr);

public slots:
    void explodeSlice(bool exploded);

private:
    QList<QPieSeries*> m_donuts;
};

#endif
