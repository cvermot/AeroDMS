// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEWIDGET_H
#define STATISTIQUEWIDGET_H

#include <QWidget>

#include "ManageDb.h"

QT_FORWARD_DECLARE_CLASS(QChart)
QT_FORWARD_DECLARE_CLASS(QChartView)

class StatistiqueWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StatistiqueWidget(QWidget* parent = nullptr);

    void load();

protected:
    virtual bool doLoad();
    void resizeEvent(QResizeEvent*) override;
    void createDefaultChartView(QChart* chart);
    QChartView* defaultChartView() const { return m_defaultChartView; }
    void setDefaultChartView(QChartView* view);

    QString m_loadError;

private:
    QChartView* m_defaultChartView = nullptr;
    bool m_loaded = false;
    ManageDb* db;
};

#endif //STATISTIQUEWIDGET_H
