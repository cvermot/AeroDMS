// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDONUTCOMBINE_H
#define STATISTIQUEDONUTCOMBINE_H

#include <QColor>
#include <QList>

#include <QtGraphs/QPieSeries>

class StatistiqueDonutCombine
{
public:
    explicit StatistiqueDonutCombine(QObject* parent = nullptr);
    void addBreakdownSeries(QPieSeries* series, QColor color, int tailleDePolice);
    QList<QPieSeries*> series() const;

private:
    void recalculateAngles();

private:
    QPieSeries* m_mainSeries = nullptr;
    QList<QPieSeries*> m_series;
};

#endif //STATISTIQUEDONUTCOMBINE_H
