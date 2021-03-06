#include "ZoomGraphicsViewLite.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <qmath.h>
#include <QDebug>

const qreal SCALE_FACTOR = 1.25;
#define VIEW_CENTER viewport() ->rect().center()
#define VIEW_WIDTH  viewport() ->rect().width()
#define VIEW_HEIGHT viewport() ->rect().height()


QImage ZoomGraphicsViewLite::cvMat2QImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

//void ZoomGraphicsViewLite::translateImage(QPointF point)
//{
//    point *= scale_;
//    point *= translateSpeed_;
//    // view 根据鼠标下的点作为锚点来定位 scene
//    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
//    QPoint newCenter(VIEW_WIDTH / 2 -point.x(), VIEW_HEIGHT/ 2 -point.y());
//    centerOn(mapToScene(newCenter));
//    // scene 在 view 的中心点作为锚点
//    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
//}


ZoomGraphicsViewLite::ZoomGraphicsViewLite(QWidget *parent) :
    QGraphicsView(parent),
    scene_(new PaintScene(this)),
    pixmapitem_(new QGraphicsPixmapItem())

{
    scene_->addItem(pixmapitem_);
    setScene(scene_);
    setDragMode (QGraphicsView::ScrollHandDrag);
}

ZoomGraphicsViewLite::~ZoomGraphicsViewLite()
{
    if(scene_ != NULL)
    {
        delete scene_;
        scene_ = NULL;
    }
}

void ZoomGraphicsViewLite::displayImage(cv::Mat image)
{
    // convert mat to pixmap
    QPixmap pixmap = QPixmap::fromImage(cvMat2QImage(image));

    // set pixmap
    pixmapitem_->setPixmap(pixmap);
    pixmapitem_ ->setFlags(QGraphicsItem::ItemIsMovable);

    // view fit the pixmap boundary and init the draw mode
    setSceneRect(pixmapitem_->boundingRect());

    fitInView(pixmapitem_, Qt::KeepAspectRatio);
}

void ZoomGraphicsViewLite::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(SCALE_FACTOR, event->delta() / 150.0);
    scaleImage(factor);
    event->accept();
}

void ZoomGraphicsViewLite::scaleImage(const qreal factor)
{
    qreal currentScale = transform().m11();
    qreal autualscale = currentScale * factor;
    qreal actualFactor = factor;

    if (autualscale > 1000)
    {
        actualFactor = 1000./currentScale;
    }
    else if (autualscale < 0.001)
    {
        actualFactor = 0.001/currentScale;
    }
    scale(actualFactor, actualFactor);
}
