// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDIAGRAMMECIRCULAIRE_H
#define STATISTIQUEDIAGRAMMECIRCULAIRE_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QAbstractSeries);
QT_FORWARD_DECLARE_CLASS(QPieSlice);

class StatistiqueDiagrammeCirculaire : public QObject
{
    Q_OBJECT
public:
    explicit StatistiqueDiagrammeCirculaire(QObject* parent = nullptr);
    void changeSeries(QAbstractSeries* series);

signals:
    void seriesChanged(QAbstractSeries* series);

public slots:
    void handleSliceClicked(QPieSlice* slice);

private:
    QAbstractSeries* m_currentSeries = nullptr;
};

#endif //STATISTIQUEDIAGRAMMECIRCULAIRE_H
