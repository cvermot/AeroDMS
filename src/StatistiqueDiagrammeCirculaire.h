// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDIAGRAMMECIRCULAIRE_H
#define STATISTIQUEDIAGRAMMECIRCULAIRE_H

#include <QChart>

QT_FORWARD_DECLARE_CLASS(QAbstractSeries);
QT_FORWARD_DECLARE_CLASS(QPieSlice);

class StatistiqueDiagrammeCirculaire : public QChart
{
    Q_OBJECT
public:
    explicit StatistiqueDiagrammeCirculaire(QGraphicsItem* parent = nullptr, Qt::WindowFlags wFlags = {});
    void changeSeries(QAbstractSeries* series);

public slots:
    void handleSliceClicked(QPieSlice* slice);

private:
    QAbstractSeries* m_currentSeries = nullptr;
};

#endif //STATISTIQUEDIAGRAMMECIRCULAIRE_H
