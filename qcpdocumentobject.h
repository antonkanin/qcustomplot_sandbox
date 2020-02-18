#ifndef QCPDOCUMENTOBJECT_H
#define QCPDOCUMENTOBJECT_H

#include <QPainter>
#include <QPicture>
#include <QTextObjectInterface>

class QCPDocumentObject : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    enum
    {
        PlotTextFormat = QTextFormat::UserObject
    };

    enum
    {
        PicturePropertyId = 1
    };

    explicit QCPDocumentObject(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    QSizeF intrinsicSize(QTextDocument* doc, int posInDocument, const QTextFormat& format) override;

    void drawObject(QPainter* painter, const QRectF& rect, QTextDocument* doc, int posInDocument,
        const QTextFormat& format) override;
};

Q_DECLARE_METATYPE(QPicture)

#endif // QCPDOCUMENTOBJECT_H
