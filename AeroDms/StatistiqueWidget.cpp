// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueWidget.h"

#include <QFont>
#include <QLabel>
#include <QMetaObject>
#include <QMetaMethod>
#include <QPropertyAnimation>
#include <QQuickItem>
#include <QQuickWidget>
#include <QResizeEvent>
#include <QUrl>
#include <QVariantMap>
#include <QVBoxLayout>

#include <QtGraphs/QBarSeries>
#include <QtGraphs/QBarSet>
#include <QtGraphs/QAbstractAxis>
#include <QtGraphs/QPieSeries>
#include <QtGraphs/QPieSlice>

#include <cmath>

namespace {

bool invokeSeriesMethod(QObject* target, const char* signature, QObject* series)
{
    if (!target || !series)
        return false;

    const QMetaObject* metaObject = target->metaObject();
    const int methodIndex = metaObject->indexOfMethod(signature);
    if (methodIndex < 0)
        return false;

    return metaObject->method(methodIndex).invoke(target, Q_ARG(QObject*, series));
}

QColor interpolateColor(const QColor& start, const QColor& end, qreal ratio)
{
    return QColor::fromRgbF(start.redF() + (end.redF() - start.redF()) * ratio,
                            start.greenF() + (end.greenF() - start.greenF()) * ratio,
                            start.blueF() + (end.blueF() - start.blueF()) * ratio,
                            start.alphaF() + (end.alphaF() - start.alphaF()) * ratio);
}

QList<QColor> createBluePiePalette(int count)
{
    const QList<QColor> anchors = {
        QColor("#0f3b5b"),
        QColor("#145277"),
        QColor("#1b6994"),
        QColor("#227fb1"),
        QColor("#2f95c9"),
        QColor("#46a8d9"),
        QColor("#67b9e1"),
        QColor("#8ccae9")
    };

    QList<QColor> palette;
    if (count <= 0)
        return palette;

    if (count == 1) {
        palette.append(anchors.at(3));
        return palette;
    }

    for (int i = 0; i < count; ++i) {
        const qreal t = static_cast<qreal>(i) / static_cast<qreal>(count - 1);
        const qreal scaled = t * static_cast<qreal>(anchors.size() - 1);
        const int lowerIndex = static_cast<int>(std::floor(scaled));
        const int upperIndex = qMin(lowerIndex + 1, anchors.size() - 1);
        const qreal ratio = scaled - static_cast<qreal>(lowerIndex);
        palette.append(interpolateColor(anchors.at(lowerIndex), anchors.at(upperIndex), ratio));
    }

    return palette;
}

QList<QColor> createBarPalette()
{
    return {
        QColor("#2f89ca"),
        QColor("#f28e2b"),
        QColor("#59a14f")
    };
}

QColor preferredLabelColor(const QColor& color)
{
    const double darkness = 0.2126 * color.redF() + 0.7152 * color.greenF() + 0.0722 * color.blueF();
    return darkness > 0.6 ? Qt::black : Qt::white;
}

}

StatistiqueWidget::StatistiqueWidget(QWidget* parent)
    : QWidget(parent)
{
}

void StatistiqueWidget::load()
{
    if (m_loaded)
        return;

    if (!m_loadError.isEmpty() || !doLoad()) {
        auto errorLabel = new QLabel(this);
        auto errorLayout = new QVBoxLayout(this);
        errorLabel->setText(tr("Erreur de chargement :\n%1").arg(m_loadError));
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
    if (m_quickWidget)
        m_quickWidget->resize(size());
}

// Most examples are simple and need only basic chart view widget, so provide it in this base class
// to avoid duplicating code
void StatistiqueWidget::createDefaultChartView(const QString& p_titre,
                                               bool p_legendeVisible,
                                               Qt::Alignment p_alignementLegende,
                                               int p_tailleTitre)
{
    if (m_quickWidget)
        return;

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setClearColor(Qt::white);
    m_quickWidget->setSource(QUrl("qrc:/AeroDms/ressources/GraphsViewContainer.qml"));

    m_rootObject = m_quickWidget->rootObject();
    if (!m_rootObject) {
        m_loadError = tr("Impossible de charger la vue Qt Graphs");
        delete m_quickWidget;
        m_quickWidget = nullptr;
        return;
    }

    m_defaultChartView = m_rootObject->findChild<QObject*>("graphView");
    if (!m_defaultChartView) {
        m_loadError = tr("Impossible d'initialiser la vue de graphique");
        delete m_quickWidget;
        m_quickWidget = nullptr;
        m_rootObject = nullptr;
        return;
    }

    m_rootObject->setProperty("chartTitle", p_titre);
    m_rootObject->setProperty("legendVisible", p_legendeVisible);
    m_rootObject->setProperty("titlePointSize", p_tailleTitre);
    m_rootObject->setProperty("legendPosition", p_alignementLegende == Qt::AlignBottom ? "bottom" : "right");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_quickWidget);
    setLayout(layout);
}

void StatistiqueWidget::setGraphTitle(const QString& p_titre)
{
    if (m_rootObject)
        m_rootObject->setProperty("chartTitle", p_titre);
}

void StatistiqueWidget::addSeriesToGraph(QAbstractSeries* p_series)
{
    if (!m_defaultChartView || !p_series)
        return;

    if (!invokeSeriesMethod(m_defaultChartView, "addSeries(QObject*)", p_series))
        return;

    p_series->setOpacity(0.0);
    m_series.append(p_series);
    refreshLegend();

    auto* animation = new QPropertyAnimation(p_series, "opacity", this);
    animation->setDuration(250);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void StatistiqueWidget::showGraphSeries(QAbstractSeries* p_series)
{
    if (!m_defaultChartView || !p_series)
        return;

    if (m_series.contains(p_series)) {
        if (m_activeSeries == p_series)
            return;

        const QList<QAbstractSeries*> trackedSeries = m_series;
        for (QAbstractSeries* serie : trackedSeries)
            serie->setVisible(serie == p_series);

        p_series->setOpacity(0.0);
        auto* animation = new QPropertyAnimation(p_series, "opacity", this);
        animation->setDuration(250);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);

        m_activeSeries = p_series;
        refreshLegend();
        return;
    }

    clearGraphSeries();
    m_activeSeries = p_series;
    addSeriesToGraph(p_series);
}

void StatistiqueWidget::clearGraphSeries()
{
    if (!m_defaultChartView)
        return;

    const QList<QAbstractSeries*> series = m_series;
    for (QAbstractSeries* serie : series) {
        invokeSeriesMethod(m_defaultChartView, "removeSeries(QObject*)", serie);
    }
    m_series.clear();
    m_activeSeries = nullptr;
    refreshLegend();
}

void StatistiqueWidget::setGraphAxisX(QAbstractAxis* p_axis)
{
    if (m_rootObject)
        m_rootObject->setProperty("axisX", QVariant::fromValue(p_axis));
}

void StatistiqueWidget::setGraphAxisY(QAbstractAxis* p_axis)
{
    if (m_rootObject)
        m_rootObject->setProperty("axisY", QVariant::fromValue(p_axis));
}

void StatistiqueWidget::refreshLegend()
{
    if (!m_rootObject)
        return;

    const QList<QColor> barPalette = createBarPalette();
    int totalAutoPieSlices = 0;
    for (QAbstractSeries* serie : m_series) {
        if (!serie || !serie->isVisible() || serie->property("excludeFromLegend").toBool())
            continue;

        if (auto* pieSeries = qobject_cast<QPieSeries*>(serie)) {
            for (QPieSlice* slice : pieSeries->slices()) {
                if (!slice->property("customColor").toBool())
                    totalAutoPieSlices++;
            }
        }
    }

    QVariantList legendEntries;
    const QList<QColor> piePalette = createBluePiePalette(totalAutoPieSlices);
    int pieColorIndex = 0;
    for (QAbstractSeries* serie : m_series) {
        if (!serie || !serie->isVisible() || serie->property("excludeFromLegend").toBool())
            continue;

        if (auto* pieSeries = qobject_cast<QPieSeries*>(serie)) {
            const auto slices = pieSeries->slices();
            for (int i = 0; i < slices.size(); ++i) {
                QPieSlice* slice = slices.at(i);
                QColor color = slice->color();
                if (!slice->property("customColor").toBool()) {
                    color = piePalette.at(pieColorIndex % piePalette.size());
                    slice->setColor(color);
                    pieColorIndex++;
                }
                slice->setBorderColor(Qt::white);
                slice->setLabelColor(preferredLabelColor(color));

                QVariantMap entry;
                entry.insert("legendColor", color);
                entry.insert("legendBorderColor", slice->borderColor());
                entry.insert("legendLabel", slice->label());
                legendEntries.append(entry);
            }
            continue;
        }

        if (auto* barSeries = qobject_cast<QBarSeries*>(serie)) {
            const auto barSets = barSeries->barSets();

            for (int i = 0; i < barSets.size(); ++i) {
                QBarSet* barSet = barSets.at(i);
                QColor color = barSet->color();
                if (!color.isValid()) {
                    color = barPalette.at(i % barPalette.size());
                    barSet->setColor(color);
                }
                QColor borderColor = barSet->borderColor();
                if (!borderColor.isValid()) {
                    borderColor = color.darker(125);
                    barSet->setBorderColor(borderColor);
                }

                QVariantMap entry;
                entry.insert("legendColor", color);
                entry.insert("legendBorderColor", borderColor);
                entry.insert("legendLabel", barSet->label());
                legendEntries.append(entry);
            }
        }
    }

    m_rootObject->setProperty("legendEntries", legendEntries);
    QMetaObject::invokeMethod(m_rootObject, "rebuildLegend");
}
