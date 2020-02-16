#include "qcustomplot.h"
#include <QApplication>
#include <QMainWindow>
#include <tuple>

using AxisData = QVector<double>;

const QPointF upVector{ 0.0, 1.0 };

std::tuple<AxisData, AxisData> plotData()
{
    int chartSize = 1000;

    AxisData x(chartSize), y(chartSize);

    for (int i = 0; i < chartSize; ++i)
    {
        x[i] = i / (static_cast<double>(chartSize) / 2.0) - 1;
        y[i] = x[i] * x[i]; // let's plot a quadratic function
    }

    return { x, y };
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QMainWindow  window;
    window.resize(800, 600);

    auto centralWidget = new QWidget(&window);
    window.setCentralWidget(centralWidget);

    auto layout = new QVBoxLayout(centralWidget);

    auto cp = new QCustomPlot(&window);
    layout->addWidget(cp);
    cp->plotLayout()->clear();

    // получаем данные для графиков
    const auto [x, y] = plotData();

    // "верхний" график
    {
        auto topAxisRect = new QCPAxisRect(cp);

        cp->plotLayout()->addElement(0, 0, topAxisRect);
        auto topGraph =
            cp->addGraph(topAxisRect->axis(QCPAxis::atBottom), topAxisRect->axis(QCPAxis::atLeft));
        topGraph->setData(x, y);
        topGraph->rescaleAxes();
    }

    // "нижний" график
    {
        auto bottomAxisRect = new QCPAxisRect(cp);
        cp->plotLayout()->addElement(1, 0, bottomAxisRect);

        auto bottomGraph = cp->addGraph(
            bottomAxisRect->axis(QCPAxis::atBottom), bottomAxisRect->axis(QCPAxis::atLeft));
        bottomGraph->setData(x, y);
        bottomGraph->rescaleAxes();
    }

    QCPItemStraightLine* startLine{};
    QCPItemStraightLine* endLine{};

    QObject::connect(cp, &QCustomPlot::mousePress, [&](QMouseEvent* event) {
        // включаем зум по прямоугольной области
        if (Qt::LeftButton == event->button())
        {
            cp->setInteractions(QCP::iRangeDrag);
            cp->setSelectionRectMode(QCP::srmNone);
        }

        if (Qt::RightButton == event->button())
        {
            if (!startLine)
            {
                const auto pos = event->pos();

                const auto axisRect = cp->axisRectAt(pos);

                startLine = new QCPItemStraightLine(cp);
                startLine->setClipAxisRect(axisRect);
                startLine->point1->setPixelPosition(pos);
                startLine->point2->setPixelPosition(pos + upVector);

                endLine = new QCPItemStraightLine(cp);
                endLine->setClipAxisRect(axisRect);
                endLine->point1->setPixelPosition(pos);
                endLine->point2->setPixelPosition(pos + upVector);

                cp->replot();
            }
        }
    });

    QObject::connect(cp, &QCustomPlot::mouseRelease, [&](QMouseEvent* event) {
        // отлючаем зум, включаем перетаскивания графика
        if (Qt::LeftButton == event->button())
        {
            cp->setSelectionRectMode(QCP::srmZoom);
        }

        if (Qt::RightButton == event->button() && startLine)
        {
            const auto horRangeBegin = startLine->point1->coords().x();
            const auto horRangeEnd   = endLine->point1->coords().x();

            for (auto axisRect : cp->axisRects())
                axisRect->axis(QCPAxis::atBottom)->setRange(horRangeBegin, horRangeEnd);

            cp->removeItem(startLine);
            startLine = nullptr;

            cp->removeItem(endLine);
            endLine = nullptr;

            cp->replot();
        }
    });

    QObject::connect(cp, &QCustomPlot::mouseMove, [&](QMouseEvent* event) {
        if (Qt::LeftButton == event->button())
        {
            const auto currenAxisRect = cp->axisRectAt(event->pos());

            for (auto axisRect : cp->axisRects())
            {
                if (axisRect != currenAxisRect)
                {
                    // work in progress....
                }
            }
        }

        if (endLine)
        {
            endLine->point1->setPixelPosition(event->pos());
            endLine->point2->setPixelPosition(event->pos() + upVector);

            cp->replot();
        }
    });

    QObject::connect(cp, &QCustomPlot::mouseWheel, [&](QWheelEvent* event) {
        auto currentAxisRect = cp->axisRectAt(event->pos());

        for (auto axisRect : cp->axisRects())
        {
            // математику подсмотрел в void QCPAxisRect::wheelEvent(QWheelEvent *event)
            const auto wheelSteps = event->delta() / 120.0;
            const auto factor     = qPow(axisRect->rangeZoomFactor(Qt::Horizontal), wheelSteps);

            axisRect->axis(QCPAxis::atBottom)->scaleRange(factor);

            if (axisRect == currentAxisRect)
                axisRect->axis(QCPAxis::atLeft)->scaleRange(factor);

            cp->replot();
        }
    });

    window.show();
    return a.exec();
}
