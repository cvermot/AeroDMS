// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueWidget.h"

#include <QChart>
#include <QChartView>
#include <QFont>
#include <QLabel>
#include <QVBoxLayout>

StatistiqueWidget::StatistiqueWidget(QWidget* parent)
    : QWidget(parent)
{
}

void StatistiqueWidget::load()
{
    if (m_loaded || layout())
        return;

    if (!doLoad()) {
        auto errorLabel = new QLabel(this);
        auto errorLayout = new QVBoxLayout(this);
        errorLabel->setText(tr("Error loading the example:\n%1").arg(m_loadError));
        QFont font = errorLabel->font();
        font.setPointSize(20);
        errorLabel->setFont(font);
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLayout->addWidget(errorLabel);
        setLayout(errorLayout);
    }

    m_loaded = true;
}

bool StatistiqueWidget::doLoad()
{
    // Most examples do their initialization in constructor.
    // Only those that can fail and show error message need to reimplement this method.
    return true;
}

void StatistiqueWidget::resizeEvent(QResizeEvent*)
{
    if (m_defaultChartView)
        m_defaultChartView->resize(size());
}

// Most examples are simple and need only basic chart view widget, so provide it in this base class
// to avoid duplicating code
void StatistiqueWidget::createDefaultChartView(QChart* chart)
{
    m_defaultChartView = new QChartView(chart, this);
    m_defaultChartView->setRenderHint(QPainter::Antialiasing);
}

void StatistiqueWidget::setDefaultChartView(QChartView* view)
{
    m_defaultChartView = view;
    m_defaultChartView->setRenderHint(QPainter::Antialiasing);
}
