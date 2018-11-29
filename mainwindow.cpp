#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QString path("D:/Qt/BorderFetch/Run/debug/pic.jpg");
    QFile file(path);

    if (file.exists()){ // 文件存在
        originImg = new QImage(path); // 原图二维数组

        resultImg = new QImage(path); // 结果二维数组
        selectImg = new QImage(originImg->size(), QImage::Format_ARGB32); // 选中情况Bool
        vis = new QImage(originImg->size(), QImage::Format_ARGB32); // 广搜的时候用到

        img_w = originImg->width(); // 图片宽度
        img_h = originImg->height(); // 图片高度

        this->setFixedSize(500, 500);
        //this->setFixedSize(img_w, img_h);

        for (int x = 0; x < selectImg->width(); x++)
            for (int y = 0; y < selectImg->height(); y++)
                selectImg->setPixel(QPoint(x, y), qRgb(0, 0, 0));
    }
    else {
        qDebug() << "file not exists";
    }
}

/**
 * 界面重绘事件，就是画个图片
 */
void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, this->width(), this->height(), QPixmap::fromImage(*resultImg));
    return QWidget::paintEvent(event);
}

/**
 * 鼠标按下
 */
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    isDraging = true;
    isChoosing = true;
    movePoints.clear();
    moveColors.clear();

    QPoint point = winToImg(event->pos());
    int x = point.x(); // 图片像素的位置
    int y = point.y();
    movePoints.append(QPoint(x, y)); // 先添加按下的点
}

/**
 * 鼠标松开，把选择的点变成红色
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    isDraging = false;

    if (isChoosing) {
        startChoose(); // 选中
    }
    else {
        startUnChoose(); // 取消选中
    }
}

/**
 * 鼠标拖拽划过，选中/取消选中点
 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isDraging) {
        QPoint point = winToImg(event->pos());
        int x = point.x(); // 图片像素的位置
        int y = point.y();
        movePoints.append(QPoint(x, y)); // 鼠标位置的点放到选择的序列里面
    }
}

/**
 * 图像变为灰度图
 * @param  origin 原图
 * @return        灰度图
 */
QImage* MainWindow::toGrayImage(QImage *origin)
{
    QImage* newImg = new QImage(origin->width(), origin->height(), QImage::Format_ARGB32);
    QColor oldColor;
    int average; // 平均灰度值

    for (int x = 0; x < newImg->width(); x++) {
        for (int y = 0; y < newImg->height(); y++) {
            oldColor = QColor(origin->pixel(x, y));
            average = (oldColor.red()+oldColor.green()+oldColor.blue())/3;
            newImg->setPixel(QPoint(x, y), qRgb(average, average, average));
        }
    }

    return newImg;
}

/**
 * 窗口坐标变成图片坐标
 * @param  point 窗口坐标
 * @return       图片坐标
 */
QPoint MainWindow::winToImg(QPoint point)
{
    int win_w = this->width();
    int win_h = this->height();

    return QPoint(point.x()*img_w/win_w, point.y()*img_h/win_h);
}

/**
 * 某个点是不是选中
 * @param  x 横坐标
 * @param  y 纵坐标
 * @return   是否选中
 */
bool MainWindow::isSelected(int x, int y)
{
    return QColor(selectImg->pixel(x, y)).red() > 0;
}

/**
 * 设置某个点选中状态
 * @param x 横坐标
 * @param y 纵坐标
 */
void MainWindow::setSelected(int x, int y)
{
    selectImg->setPixel(x, y, qRgb(1,1,1));
}

/**
 * 将移动的点转换到颜色List，并且减少点的数量（大幅度提高效率）
 */
void MainWindow::movePointsToColors()
{
    int half = COLOR_DIFF >> 2;
    for (int i = 0; i < movePoints.size(); i++) {
        QPoint point(movePoints.at(i));
        QColor color(originImg->pixel(point.x(), point.y()));

        int flag = false;
        int sum = color.red()+color.blue()+color.green(); // 点平均值
        for (int j = 0; j < moveColors.size(); j++) {
            QColor tc(moveColors.at(j)); // list中已选中的点
            // 如果和list中已有的点只差一点点，就放弃掉这个点（放弃一些准确性，但大大提高了效率）
            if (abs(tc.red()+tc.green()+tc.blue()-sum) < half) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            moveColors.append(color);
        }
    }

    //qDebug() << "movePoints:" << movePoints.size() << "  usedColors:" << moveColors.size();
}

/**
 * 开始把拖拽过（筛选后）的点选中相似的
 * 算法思路是广度优先搜索
 */
void MainWindow::startChoose()
{
    movePointsToColors(); // 拖拽过的点转化成颜色+筛选点

    // 初始化 vis 数组
    //QImage vis(img_w, img_h, QImage::Format_ARGB32);
    for (int i = 0; i < img_w; i++)
        for (int j = 0; j < img_h; j++)
            vis->setPixel(i, j, qRgb(0,0,0));

    QPoint p;
    int x, y, count = 0;
    QColor cv;
    q.clear();
    // 初始化把鼠标拖拽过的点放到队列里
    for (int i =0; i < movePoints.size(); i++) {
        q.push_back(movePoints.at(i));
    }
    while (!q.empty()) {
        p = q.front();
        x = p.x();
        y = p.y();
        q.pop_front();
        if (vis->pixelColor(x,y).red()!=0) continue;
        vis->setPixel(x, y, qRgb(1,1,1));
        cv = QColor(originImg->pixel(x, y));
        count++; // 遍历过的点数量

        resultImg->setPixel(x, y, qRgb(100,0,0));
        selectImg->setPixel(x, y, qRgb(1,1,1));

        chooseNext(x-1, y, 0, -1, 0);
        chooseNext(x+1, y, 0,  1, 0);
        chooseNext(x, y-1, 0, 0, -1);
        chooseNext(x, y+1, 0, 0,  1);
        chooseNext(x-1, y-1, 0, -1, -1);
        chooseNext(x-1, y+1, 0, -1, +1);
        chooseNext(x+1, y-1, 0, +1, -1);
        chooseNext(x+1, y+1, 0, +1, +1);
    }

    update();
    qDebug() << "choose finished, change count = " << count;

    /*int hx, hy;
    bool find = false;
    for (int i = 0; i < img_h; i++)
    {
        for (int j = 0; j < img_w; j++)
            if (isSelected(j, i)){
                hx = i;
                hy = j;
                find = true;
                break;
            }
        if (find)
            break;
    }
    qDebug() << "toppest point : hx=" << hx << ", hy=" << hy;*/
}

/**
 * 选择下一个点。
 * @param p 坐标
 * @param q 队列
 */
bool MainWindow::chooseNext(int x, int y, int d, int dx, int dy)
{
    if (d > 5) return false;
    // 超过图片边界
    if (x < 0 || x >= img_w || y < 0 || y >= img_h)
        return false;
    if (isSelected(x, y) || vis->pixelColor(x,y).red()!=0)
        return false;
    if (isSame(QColor(originImg->pixel(x, y))))
    {
        q.push_back(QPoint(x,y));
        return true;
    }
    else if (chooseNext(x, y, (d+3), dx*(d+3), dy*(d+3)) || chooseNext(x, y, ++d, dx*d, dy*d) || chooseNext(x, y, ++d, dx*d, dy*d)
        || chooseNext(x, y, ++d, dx*d, dy*d) || chooseNext(x, y, ++d, dx*d, dy*d)) // 向外扩张5个点
    {
        q.push_back(QPoint(x,y));
    }
    return false;
}

/**
 * 取消选择
 */
void MainWindow::startUnChoose()
{

}

/**
 * 取绝对值
 */
int MainWindow::asb(int a, int b)
{
    return a-b>=0 ? a-b : b-a;
}

/**
 * 容差判断两个点颜色（灰度值）是否相似
 */
bool MainWindow::isSame(QColor a, QColor b)
{
    int ar = a.red(), ag = a.green(), ab = a.blue();
    int br = b.red(), bg = b.green(), bb = b.blue();
    int cr = abs(ar-br), cg = abs(ag-bg), cb = abs(ab-bb);
    return cr<=COLOR_DIFF && cg<=COLOR_DIFF && cb<=COLOR_DIFF;
}

/**
 * 是否与要选择的点list中有相似的
 * @param  a 要判断的点
 * @return   是否相似
 */
bool MainWindow::isSame(QColor a)
{
    if (a.red()>250 && a.green()>250 && a.blue()>250)
        return false; // 颜色偏白，可以认为是天空，跳过
    for (int i = 0; i < moveColors.size(); i++) {
        QColor b(moveColors.at(i));
        int ar = a.red(), ag = a.green(), ab = a.blue();
        int br = b.red(), bg = b.green(), bb = b.blue();
        int cr = abs(ar-br), cg = abs(ag-bg), cb = abs(ab-bb);
        if (cr<=COLOR_DIFF && cg<=COLOR_DIFF && cb<=COLOR_DIFF) {
            return true;
        }
    }

    return false;

}
