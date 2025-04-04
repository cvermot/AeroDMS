// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDONUTCOMBINE_H
#define STATISTIQUEDONUTCOMBINE_H

#include <QChart>

QT_FORWARD_DECLARE_CLASS(QPieSeries)

class StatistiqueDonutCombine : public QChart
{
public:
    StatistiqueDonutCombine(QGraphicsItem* parent = nullptr, Qt::WindowFlags wFlags = {});
    void addBreakdownSeries(QPieSeries* series, QColor color, int tailleDePolice);

private:
    void recalculateAngles();
    void updateLegendMarkers(QFont p_font);

private:
    QPieSeries* m_mainSeries = nullptr;
};

#endif //STATISTIQUEDONUTCOMBINE_H
