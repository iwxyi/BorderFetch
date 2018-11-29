#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QFile>
#include <QString>
#include <QPixmap>
#include <QDebug>
#include <QPoint>
#include <QMouseEvent>
#include <QImage>
#include <QColor>
#include <QRgb>
#include <QQueue>
#include <QList>
#include <QSet>

#define COLOR_DIFF 10

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

protected:
    QImage *toGrayImage(QImage* origin);
    QPoint winToImg(QPoint point);

    bool isSame(QColor a, QColor b);
    bool isSame(QColor a);
    bool isSelected(int x, int y);
    void setSelected(int x, int y);
    void movePointsToColors();
    void startChoose();
    void startUnChoose();
    bool chooseNext(int x, int y, int d, int dx, int dy);

private:
    int asb(int a, int b);

private:
    QImage* originImg;
    QImage* selectImg;
    QImage* resultImg;
    QImage* grayImg;

    int img_w, img_h;
    bool isDraging;
    bool isChoosing;
    QList<QPoint>movePoints;
    QList<QColor>moveColors;

    QQueue<QPoint> q;
    QImage* vis;
};

#endif // MAINWINDOW_H
