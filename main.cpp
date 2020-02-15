#include "qcustomplot.h"
#include <QMainWindow>

#include <QApplication>
#include <tuple>

using AxisData = QVector<double>;

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

    QObject::connect(cp, &QCustomPlot::mousePress, [&](QMouseEvent* event) {
        // включаем зум по прямоугольной области
        if (Qt::LeftButton == event->button())
        {
            cp->setInteractions(QCP::iRangeDrag);
            cp->setSelectionRectMode(QCP::srmNone);
        }
    });

    QObject::connect(cp, &QCustomPlot::mouseRelease, [&](QMouseEvent* event) {
        // отлючаем зум, включаем перетаскивания графика
        if (Qt::LeftButton == event->button())
        {
            cp->setInteractions(QCP::iRangeZoom);
            cp->setSelectionRectMode(QCP::srmZoom);
        }
    });

    window.show();
    return a.exec();
}
