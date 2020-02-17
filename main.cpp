#include "qcpdocumentobject.h"
#include "qcustomplot.h"
#include <QApplication>
#include <QMainWindow>
#include <QSvgGenerator>
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

    auto saveSvgButton = new QPushButton("Save as SVG");
    layout->addWidget(saveSvgButton);

    auto savePngButton = new QPushButton("Save as PNG");
    layout->addWidget(savePngButton);

    auto savePdfButton = new QPushButton("Save as PDF");
    layout->addWidget(savePdfButton);

    // получаем данные для графиков
    const auto [x, y] = plotData();

    // "верхний" график
    {
        auto topAxisRect = new QCPAxisRect(cp);
        topAxisRect->axis(QCPAxis::atBottom)->setLabel("X");
        topAxisRect->axis(QCPAxis::atLeft)->setLabel("Y");

        cp->plotLayout()->addElement(0, 0, topAxisRect);
        auto topGraph =
            cp->addGraph(topAxisRect->axis(QCPAxis::atBottom), topAxisRect->axis(QCPAxis::atLeft));

        topGraph->setData(x, y);
        topGraph->rescaleAxes();
    }

    // "нижний" график
    {
        auto bottomAxisRect = new QCPAxisRect(cp);
        bottomAxisRect->axis(QCPAxis::atBottom)->setLabel("X");
        bottomAxisRect->axis(QCPAxis::atLeft)->setLabel("Y");

        cp->plotLayout()->addElement(1, 0, bottomAxisRect);
        auto bottomGraph = cp->addGraph(
            bottomAxisRect->axis(QCPAxis::atBottom), bottomAxisRect->axis(QCPAxis::atLeft));

        bottomGraph->setData(x, y);
        bottomGraph->rescaleAxes();
    }

    QCPItemStraightLine* startLine{};
    QCPItemStraightLine* endLine{};

    QPointF mouseStartPosition{};

    QObject::connect(cp, &QCustomPlot::mousePress, [&](QMouseEvent* event) {
        if (Qt::LeftButton == event->button())
        {
            mouseStartPosition = event->pos();
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
        if (Qt::RightButton == event->button() && startLine)
        {
            const auto horRangeBegin = startLine->point1->coords().x();
            const auto horRangeEnd   = endLine->point1->coords().x();

            for (auto axisRect : cp->axisRects())
            {
                axisRect->axis(QCPAxis::atBottom)->setRange(horRangeBegin, horRangeEnd);
            }

            cp->removeItem(startLine);
            startLine = nullptr;

            cp->removeItem(endLine);
            endLine = nullptr;

            cp->replot();
        }
    });

    QObject::connect(cp, &QCustomPlot::mouseMove, [&](QMouseEvent* event) {
        if (Qt::LeftButton & event->buttons())
        {
            auto currentAxis = cp->axisRectAt(event->pos());

            for (auto axisRect : cp->axisRects())
            {
                {
                    auto horAxis = axisRect->axis(QCPAxis::atBottom);

                    const auto diff = horAxis->pixelToCoord(mouseStartPosition.x()) -
                                      horAxis->pixelToCoord(event->pos().x());

                    horAxis->moveRange(diff);
                }

                if (currentAxis == axisRect)
                {
                    auto vertAxis = axisRect->axis(QCPAxis::atLeft);

                    const auto diff = vertAxis->pixelToCoord(mouseStartPosition.y()) -
                                      vertAxis->pixelToCoord(event->pos().y());

                    vertAxis->moveRange(diff);
                }
            }
            cp->replot();

            mouseStartPosition = event->pos();
        }

        if ((Qt::RightButton & event->buttons()) && endLine)
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

            // НЕ делаем масштабирование по горизонтали если нажат Ctrl
            if (false == (QApplication::keyboardModifiers() & Qt::ControlModifier))
            {
                axisRect->axis(QCPAxis::atBottom)->scaleRange(factor);
            }

            if (axisRect == currentAxisRect)
                axisRect->axis(QCPAxis::atLeft)->scaleRange(factor);

            cp->replot();
        }
    });

    QObject::connect(saveSvgButton, &QPushButton::pressed, [&]() {
        qDebug() << "Saving to SVG";

        QSvgGenerator svgGenerator;
        svgGenerator.setFileName("image.svg");

        QCPPainter painter;
        painter.begin(&svgGenerator);
        cp->toPainter(&painter, 800, 600);
        painter.end();
    });

    QObject::connect(savePngButton, &QPushButton::pressed, [&]() {
        qDebug() << "Saving to PNG";
        cp->savePng("image.png", 800, 600);
    });

    QObject::connect(savePdfButton, &QPushButton::pressed, [&]() {
        qDebug() << "Saving to PDF";
        QTextDocument document;

        const int PlotTextFormat = QTextFormat::UserObject + 3902;

        document.documentLayout()->registerHandler(PlotTextFormat, new QCPDocumentObject(&window));

        {
            QTextCursor cursor(&document);

            {
                QString html = "<h1>Document Title1</h1>"
                               "Some simple text to test the reporting</br>"
                               "Second line of this awesome report";

                cursor.beginEditBlock();
                cursor.insertHtml(html);
                cursor.endEditBlock();
            }

            {
                QPicture   picture;
                QCPPainter painter;
                painter.begin(&picture);
                cp->toPainter(&painter, 400, 400);
                painter.end();

                QTextCharFormat format;
                format.setObjectType(PlotTextFormat);
                format.setProperty(1, QVariant::fromValue(picture));

                cursor.beginEditBlock();
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
                cursor.endEditBlock();
            }

            {
                cursor.beginEditBlock();
                cursor.insertText("\nJust a text at the end of the report");
                cursor.endEditBlock();
            }

            {
                QPrinter printer(QPrinter::PrinterResolution);
                printer.setOutputFormat(QPrinter::PdfFormat);
                printer.setOutputFileName("image.pdf");
                document.print(&printer);
            }
        }
    });

    window.show();
    return a.exec();
}
