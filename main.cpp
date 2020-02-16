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

    // включаем зум по средней кнопке мыши
    cp->setInteractions(QCP::iRangeZoom);

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
                qDebug() << "line";
                const auto pos = event->pos();

                startLine = new QCPItemStraightLine(cp);
                startLine->point1->setPixelPosition(pos);
                startLine->point2->setPixelPosition(pos + upVector);

                endLine = new QCPItemStraightLine(cp);
                endLine->point1->setPixelPosition(pos);
                endLine->point2->setPixelPosition(pos + upVector);

                cp->replot();
            }
        }

        const auto axisRect = cp->axisRectAt(event->pos());
        if (axisRect)
        {
            // понятно, что может упасть, но у нас же есть как минимум один график =)
            auto graph = *(axisRect->graphs().begin());
            qDebug() << graph->name();
        }
    });

    QObject::connect(cp, &QCustomPlot::mouseRelease, [&](QMouseEvent* event) {
        // отлючаем зум, включаем перетаскивания графика
        if (Qt::LeftButton == event->button())
        {
            cp->setInteractions(QCP::iRangeZoom);
            cp->setSelectionRectMode(QCP::srmZoom);
        }

        if (Qt::RightButton == event->button())
        {
            const auto axisRect = cp->axisRectAt(event->pos());
            if (axisRect)
            {
                axisRect->zoom(
                    { startLine->point1->pixelPosition(), endLine->point1->pixelPosition() },
                    { axisRect->axis(QCPAxis::atBottom) });
            }

            if (startLine)
            {
                cp->removeItem(startLine);
                startLine = nullptr;

                cp->removeItem(endLine);
                endLine = nullptr;

                cp->replot();
            }
        }
    });

    QObject::connect(cp, &QCustomPlot::mouseMove, [&](QMouseEvent* event) {
        if (endLine)
        {
            endLine->point1->setPixelPosition(event->pos());
            endLine->point2->setPixelPosition(event->pos() + upVector);

            cp->replot();
        }
    });

    window.show();
    return a.exec();
}
