#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <QMouseEvent>
#include <QPainter>
#include "ciede2000.h"
#define ROCK_TEXTURE 0
#define SAND_TEXTURE 1
#define TREE_TEXTURE 2
#define WATER_TEXTURE 3
#define NO_TEXTURE_GRAY 0
#define ROCK_TEXTURE_GRAY 0
#define SAND_TEXTURE_GRAY 86
#define TREE_TEXTURE_GRAY 171
#define WATER_TEXTURE_GRAY 255
#define ROCK_TEXTURE_B 0
#define ROCK_TEXTURE_G 0
#define ROCK_TEXTURE_R 255
#define SAND_TEXTURE_B 0
#define SAND_TEXTURE_G 255
#define SAND_TEXTURE_R 255
#define TREE_TEXTURE_B 0
#define TREE_TEXTURE_G 255
#define TREE_TEXTURE_R 0
#define WATER_TEXTURE_B 255
#define WATER_TEXTURE_G 0
#define WATER_TEXTURE_R 0
using namespace std;
using namespace cv;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    startDrawBound(false),
    startPaint(false),
    mScale(1.0)
{
    ui->setupUi(this);
    QObject::connect(ui->actionOpenImage, SIGNAL(triggered(bool)), this, SLOT(open_image()));
    labelPic = new QLabel();
    //QObject::connect(labelPic, SIGNAL(triggered(bool)), this, SLOT(show_pointer_loc()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open_image()
{
    filename = QFileDialog::getOpenFileName
            (this,
             tr("选择图像"),
             "",
             tr("Images (*.png *.bmp *.jpg *.tif *.GIF )"));
    if(filename.isEmpty())
    {
        return;
    }
    QPixmap* img=new QPixmap;
    if(! ( img->load(filename) ) ) //加载图像
    {
        QMessageBox::information(this,
                                 tr("打开图像失败"),
                                 tr("打开图像失败!"));
        delete img;
        return;
    }

    srcPic = imread(filename.toStdString());
    cvtColor(srcPic, labPic, CV_BGR2Lab);
    texFile = Mat::zeros(srcPic.rows, srcPic.cols, CV_8U);
    cout << texFile.channels() << endl;

    labelPic->setPixmap(*img);
    labelPic->resize(img->size());
    ui->scrollArea->setMaximumHeight(labelPic->height()+2);
    ui->scrollArea->setMaximumWidth(labelPic->width()+2);
    ui->scrollArea->setWidget(labelPic);
}

//void MainWindow::wheelEvent(QWheelEvent *event)
//{

//    Mat src;
//    cvtColor(srcPic, src, COLOR_BGR2RGB);
//    imgSelectArea = QImage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
//    QPixmap pixmap = QPixmap::fromImage(imgSelectArea);
//    pixmap.scaled(QSize(50,50), Qt::KeepAspectRatioByExpanding);
//    labelPic->setPixmap(pixmap);
//    return;
//}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    QPoint pt;
    pt = e->globalPos();
    pt = labelPic->mapFromGlobal(pt);
    if(pt.x()-1 >= 0 && pt.x()-1 <= labelPic->width() &&
            pt.y()-2 >= 0 && pt.y()-2 <= labelPic->height()
            && ui->btnColorChoose->isChecked()
            && ui->materialList->currentRow() != -1
            && !ui->btnRectangle->isChecked())
    {
        Vec3b color;
        cout << pt.x()-1 << "," << pt.y()-2 << endl;
        color = srcPic.at<Vec3b>(pt.y()-2, pt.x()-1);
        //        cout << (int)color[0] << ","
        //             << (int)color[1] << ","
        //             << (int)color[2] << endl;
        QString strStyle;
        strStyle = QString("QLabel{background-color:rgb(%1,%2,%3)}")
                .arg((int)color[2]).arg((int)color[1])
                .arg((int)color[0]);
        //        cout << strStyle.toStdString() << endl;
        ui->lblColor->setStyleSheet(strStyle);

        int row= labPic.rows; // number of lines
        int col= labPic.cols;
        for (int i = 0; i < row; i++)
        {
            //uchar* src_data = src.ptr<uchar>(i);
            //uchar* dst_data = dst.ptr<uchar>(i);
            for (int j = 0; j < col; j++)
            {
                if (color_similar(pt.x()-1, pt.y()-2, j, i, 3.0))
                {
                    switch (ui->materialList->currentRow()) {
                    case ROCK_TEXTURE:
                        srcPic.at<Vec3b>(i, j)[0] = ROCK_TEXTURE_B;
                        srcPic.at<Vec3b>(i, j)[1] = ROCK_TEXTURE_G;
                        srcPic.at<Vec3b>(i, j)[2] = ROCK_TEXTURE_R;
                        texFile.at<uchar>(i, j) = ROCK_TEXTURE_GRAY;
                        break;
                    case SAND_TEXTURE:
                        srcPic.at<Vec3b>(i, j)[0] = SAND_TEXTURE_B;
                        srcPic.at<Vec3b>(i, j)[1] = SAND_TEXTURE_G;
                        srcPic.at<Vec3b>(i, j)[2] = SAND_TEXTURE_R;
                        texFile.at<uchar>(i, j) = SAND_TEXTURE_GRAY;
                        break;
                    case TREE_TEXTURE:
                        srcPic.at<Vec3b>(i, j)[0] = TREE_TEXTURE_B;
                        srcPic.at<Vec3b>(i, j)[1] = TREE_TEXTURE_G;
                        srcPic.at<Vec3b>(i, j)[2] = TREE_TEXTURE_R;
                        texFile.at<uchar>(i, j) = TREE_TEXTURE_GRAY;
                        break;
                    case WATER_TEXTURE:
                        srcPic.at<Vec3b>(i, j)[0] = WATER_TEXTURE_B;
                        srcPic.at<Vec3b>(i, j)[1] = WATER_TEXTURE_G;
                        srcPic.at<Vec3b>(i, j)[2] = WATER_TEXTURE_R;
                        texFile.at<uchar>(i, j) = WATER_TEXTURE_GRAY;
                        break;
                    default:
                        break;
                    }

                }
            }
        }
        refreshSrcImg();
    }else if(ui->btnSelect->isChecked())
    {
        //QRegion region();
        poly.clear();
        startDrawBound = true;
        QPoint pt;
        pt = e->globalPos();
        pt = labelPic->mapFromGlobal(pt);
        cout << "start:";
        cout << "(" << pt.x() << "," << pt.y() << ")" << endl;
        pt.setX(pt.x()-1);
        pt.setY(pt.y()-2);
        poly << pt;
    }else if(!ui->btnColorChoose->isChecked()
             && ui->btnRectangle->isChecked())
    {
        recStart = e->globalPos();
        recStart = labelPic->mapFromGlobal(recStart);
        recStart.setX(recStart.x()-1);
        recStart.setY(recStart.y()-2);
        cout << "start:";
        cout << "(" << recStart.x() << "," << recStart.y() << ")" << endl;
    }else if(pt.x()-1 >= 0 && pt.x()-1 <= labelPic->width() &&
             pt.y()-2 >= 0 && pt.y()-2 <= labelPic->height()
             && ui->btnColorChoose->isChecked()
             && ui->materialList->currentRow() != -1
             && ui->btnRectangle->isChecked())
    {
        int row = fabs(recStart.y()-recEnd.y());
        int col = fabs(recStart.x()-recEnd.x());
        int startX = min(recStart.x(), recEnd.x());
        int startY = min(recStart.y(), recEnd.y());
        startX = max(startX, 0);
        startY = max(startY, 0);
        int endX = min(col+startX, srcPic.cols);
        int endY = min(row+startY, srcPic.rows);

        Vec3b color;
        cout << pt.x()-1 << "," << pt.y()-2 << endl;
        color = srcPic.at<Vec3b>(pt.y()-2, pt.x()-1);
        //        cout << (int)color[0] << ","
        //             << (int)color[1] << ","
        //             << (int)color[2] << endl;
        QString strStyle;
        strStyle = QString("QLabel{background-color:rgb(%1,%2,%3)}")
                .arg((int)color[2]).arg((int)color[1])
                .arg((int)color[0]);
        //        cout << strStyle.toStdString() << endl;
        ui->lblColor->setStyleSheet(strStyle);

        for (int i = startY; i < endY; i++)
        {
            for (int j = startX; j < endX; j++)
            {
                if (color_similar(pt.x()-1, pt.y()-2, j, i, 3.0))
                {
                    switch (ui->materialList->currentRow()) {
                    case ROCK_TEXTURE:
                        srcPic.at<Vec3b>(i, j)[0] = ROCK_TEXTURE_B;
                        srcPic.at<Vec3b>(i, j)[1] = ROCK_TEXTURE_G;
                        srcPic.at<Vec3b>(i, j)[2] = ROCK_TEXTURE_R;
                        texFile.at<uchar>(i, j) = ROCK_TEXTURE_GRAY;
                        break;
                    case SAND_TEXTURE:
                        srcPic.at<Vec3b>(i, j)[0] = SAND_TEXTURE_B;
                        srcPic.at<Vec3b>(i, j)[1] = SAND_TEXTURE_G;
                        srcPic.at<Vec3b>(i, j)[2] = SAND_TEXTURE_R;
                        texFile.at<uchar>(i, j) = SAND_TEXTURE_GRAY;
                        break;
                    case TREE_TEXTURE:
                        srcPic.at<Vec3b>(i, j)[0] = TREE_TEXTURE_B;
                        srcPic.at<Vec3b>(i, j)[1] = TREE_TEXTURE_G;
                        srcPic.at<Vec3b>(i, j)[2] = TREE_TEXTURE_R;
                        texFile.at<uchar>(i, j) = TREE_TEXTURE_GRAY;
                        break;
                    case WATER_TEXTURE:
                        srcPic.at<Vec3b>(i, j)[0] = WATER_TEXTURE_B;
                        srcPic.at<Vec3b>(i, j)[1] = WATER_TEXTURE_G;
                        srcPic.at<Vec3b>(i, j)[2] = WATER_TEXTURE_R;
                        texFile.at<uchar>(i, j) = WATER_TEXTURE_GRAY;
                        break;
                    default:
                        break;
                    }

                }
            }
        }
        QPainter painter;
        Mat src;
        int width = fabs(recStart.x() - recEnd.x());
        int height = fabs(recStart.y() - recEnd.y());
        cvtColor(srcPic, src, COLOR_BGR2RGB);
        imgSelectArea = QImage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
        painter.begin(&imgSelectArea);
        painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
        painter.drawRect(startX, startY, width, height);
        labelPic->setPixmap(QPixmap::fromImage(imgSelectArea));
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if(startDrawBound)
    {
        QPoint pt;
        pt = e->globalPos();
        pt = labelPic->mapFromGlobal(pt);
        pt.setX(pt.x()-1);
        pt.setY(pt.y()-2);
        cout << "add:(" << pt.x() << "," << pt.y() << ")" << endl;
        poly << pt;
    }else if(ui->btnRectangle->isChecked() && !ui->btnColorChoose->isChecked())
    {
        recEnd = e->globalPos();
        recEnd = labelPic->mapFromGlobal(recEnd);
        recEnd.setX(recEnd.x()-1);
        recEnd.setY(recEnd.y()-2);
        int startX = min(recStart.x(), recEnd.x());
        int startY = min(recStart.y(), recEnd.y());
        int width = fabs(recStart.x() - recEnd.x());
        int height = fabs(recStart.y() - recEnd.y());
        QPainter painter;
        Mat src;
        cvtColor(srcPic, src, COLOR_BGR2RGB);
        imgSelectArea = QImage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
        painter.begin(&imgSelectArea);
        painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
        painter.drawRect(startX, startY, width, height);
        labelPic->setPixmap(QPixmap::fromImage(imgSelectArea));
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    if(startDrawBound)
    {
        startDrawBound = false;
        startPaint = true;
        QPainter painter;
        Mat src;
        cvtColor(srcPic, src, COLOR_BGR2RGB);
        imgSelectArea = QImage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
        painter.begin(&imgSelectArea);
        painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
        painter.drawPolygon(poly);
        //        painter.drawPolygon(points, 4);
        //        painter.drawRect(50, 50, 160, 100);
        labelPic->setPixmap(QPixmap::fromImage(imgSelectArea));
        ui->btnSelect->setChecked(false);
    }else if(ui->btnRectangle->isChecked() && !ui->btnColorChoose->isChecked())
    {
        recEnd = e->globalPos();
        recEnd = labelPic->mapFromGlobal(recEnd);
        recEnd.setX(recEnd.x()-1);
        recEnd.setY(recEnd.y()-2);
        int startX = min(recStart.x(), recEnd.x());
        int startY = min(recStart.y(), recEnd.y());
        int width = fabs(recStart.x() - recEnd.x());
        int height = fabs(recStart.y() - recEnd.y());
        QPainter painter;
        Mat src;
        cvtColor(srcPic, src, COLOR_BGR2RGB);
        imgSelectArea = QImage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
        painter.begin(&imgSelectArea);
        painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
        painter.drawRect(startX, startY, width, height);
        labelPic->setPixmap(QPixmap::fromImage(imgSelectArea));
//        ui->btnRectangle->setChecked(false);
        cout << "end:";
        cout << "(" << recEnd.x() << "," << recEnd.y() << ")" << endl;
    }
}


//void MainWindow::on_btnCanny_clicked()
//{
//    if (filename.isEmpty())
//        return;
//    Mat src, src_gray, dst;
//    GaussianBlur(srcPic, src, Size(3,3),0);
//    cvtColor(src,src_gray,COLOR_BGR2GRAY);
//    Canny(src_gray,dst,50,200);
//    //    labelPic->setPixmap(QPixmap::fromImage(QImage(dst.data, dst.cols, dst.rows, dst.step, QImage::Format_Grayscale8)));

//    //    int src_row= src.rows; // number of lines
//    //    int src_col= src.cols * src.channels();
//    int dst_row= dst.rows; // number of lines
//    int dst_col= dst.cols * dst.channels();

//    for (int i = 0; i < dst_row; i++)
//    {
//        //uchar* src_data = src.ptr<uchar>(i);
//        //uchar* dst_data = dst.ptr<uchar>(i);
//        for (int j = 0; j < dst_col; j++)
//        {
//            int pixel = dst.at<uchar>(i, j);
//            if (pixel == 255)
//            {
//                src.at<Vec3b>(i, j)[0]= 0;//b
//                src.at<Vec3b>(i, j)[1]= 0;//g
//                src.at<Vec3b>(i, j)[2]= 0;//r
//            }
//        }
//    }
//    cvtColor(src,src,COLOR_BGR2RGB);
//    labelPic->setPixmap(QPixmap::fromImage(QImage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888)));
//}
Mat MainWindow::get_contour()
{
    Mat src, gray, dst;
    GaussianBlur(srcPic, src, Size(3,3),0);
    cvtColor(src,gray,CV_BGR2GRAY);
    //    threshold(gray,gray,145,255,THRESH_BINARY_INV);
    Canny(gray,gray,50,200);

    Mat element(3,3,CV_8U,Scalar(255));
    dilate(gray,gray,element);

    vector<vector<Point>> contours;
    //    CV_RETR_EXTERNAL：只搜索最外层的等高线
    //    CV_RETR_LIST：搜索所有的等高线，而且不建立层级关系
    //    CV_RETR_CCOMP：所有所有的等高线，并把它们组织成两个层级，第一层级包好的是组件的外层边缘，第二层及包含的是洞的边缘
    //    CV_RETR_TREE：搜索所有的轮廓线，并且组织成多个层级
    //    method：估计等高线的方法
    //    CV_CHAIN_APPROX_NONE：保存等高线的所有点
    //    CV_CHAIN_APPROX_SIMPLE：压缩水平、竖直和对角线方向上的段，只保留两端点
    findContours(gray,
                 contours, // a vector of contours
                 CV_RETR_LIST ,
                 CV_CHAIN_APPROX_NONE);
    Mat result(src.size(),CV_8U,Scalar(255));
    drawContours(result,contours,
                 -1, // draw all contours
                 Scalar(0), // in black
                 0.5); // with a thickness of 2
    return result;
}

void MainWindow::on_btnContour_clicked()
{
    Mat img = get_contour();
    labelPic->setPixmap(QPixmap::fromImage(QImage(img.data, img.cols, img.rows, img.step, QImage::Format_Grayscale8)));
}

bool MainWindow::color_similar(int ax, int ay, int bx, int by, double threshold)
{
    Vec3b avec, bvec;
    avec = labPic.at<Vec3b>(ay, ax);
    bvec = labPic.at<Vec3b>(by, bx);
    CIEDE2000::LAB lab1, lab2;
    lab1.l = (double)avec[0];
    lab1.a = (double)avec[1];
    lab1.b = (double)avec[2];
    lab2.l = (double)bvec[0];
    lab2.a = (double)bvec[1];
    lab2.b = (double)bvec[2];
    double myResult = CIEDE2000::CIEDE2000(lab1, lab2);
    return (myResult < threshold);
}

void MainWindow::on_btnClear_clicked()
{
    if (poly.isEmpty() && !ui->btnRectangle->isChecked())
    {
        switch (ui->comboClear->currentIndex()) {
        case ROCK_TEXTURE:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == ROCK_TEXTURE_GRAY)
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case SAND_TEXTURE:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == SAND_TEXTURE_GRAY)
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case TREE_TEXTURE:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == TREE_TEXTURE_GRAY)
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case WATER_TEXTURE:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == WATER_TEXTURE_GRAY)
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        default:
            srcPic = imread(filename.toStdString());
            texFile = Mat::zeros(srcPic.rows, srcPic.cols, CV_8U);
            break;
        }
    }else if (!poly.isEmpty())
    {
        switch (ui->comboClear->currentIndex()) {
        case ROCK_TEXTURE:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == ROCK_TEXTURE_GRAY
                            && poly.containsPoint(QPoint(j, i), Qt::OddEvenFill))
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case SAND_TEXTURE:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == SAND_TEXTURE_GRAY
                            && poly.containsPoint(QPoint(j, i), Qt::OddEvenFill))
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case TREE_TEXTURE:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == TREE_TEXTURE_GRAY
                            && poly.containsPoint(QPoint(j, i), Qt::OddEvenFill))
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case WATER_TEXTURE:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == WATER_TEXTURE_GRAY
                            && poly.containsPoint(QPoint(j, i), Qt::OddEvenFill))
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        default:
        {
            int row = srcPic.rows;
            int col = srcPic.cols;
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    if (poly.containsPoint(QPoint(j, i), Qt::OddEvenFill))
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        }
        poly.clear();
    }else if (ui->btnRectangle->isChecked())
    {
        int row = fabs(recStart.y()-recEnd.y());
        int col = fabs(recStart.x()-recEnd.x());
        int startX = min(recStart.x(), recEnd.x());
        int startY = min(recStart.y(), recEnd.y());
        startX = max(startX, 0);
        startY = max(startY, 0);
        int endX = min(col+startX, srcPic.cols);
        int endY = min(row+startY, srcPic.rows);
        switch (ui->comboClear->currentIndex()) {
        case ROCK_TEXTURE:
        {
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = startY; i < endY; i++)
            {
                for (int j = startX; j < endX; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == ROCK_TEXTURE_GRAY)
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case SAND_TEXTURE:
        {
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = startY; i < endY; i++)
            {
                for (int j = startX; j < endX; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == SAND_TEXTURE_GRAY)
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case TREE_TEXTURE:
        {
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = startY; i < endY; i++)
            {
                for (int j = startX; j < endX; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == TREE_TEXTURE_GRAY)
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        case WATER_TEXTURE:
        {
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = startY; i < endY; i++)
            {
                for (int j = startX; j < endX; j++)
                {
                    if ((int)texFile.at<uchar>(i, j) == WATER_TEXTURE_GRAY)
                    {
                        texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                        srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                    }
                }
            }
            break;
        }
        default:
        {
            Mat origin;
            origin = imread(filename.toStdString());
            for (int i = startY; i < endY; i++)
            {
                for (int j = startX; j < endX; j++)
                {

                    texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
                    srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
                }
            }
            break;
        }
        }
    }
    refreshSrcImg();
}

void MainWindow::on_btnSave_clicked()
{
    labelPic->setPixmap(QPixmap::fromImage(QImage(texFile.data, texFile.cols, texFile.rows, texFile.step, QImage::Format_Grayscale8)));
    imwrite("IRTexture.bmp",texFile);
    //    Mat tmp;
    //    cvtColor(srcPic,tmp,COLOR_BGR2RGB);
    //    labelPic->setPixmap(QPixmap::fromImage(QImage(tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888)));
}

void MainWindow::on_btnConvert_clicked()
{
    int row = srcPic.rows; // number of lines
    int col = srcPic.cols;
    int fromIdx, toIdx;
    int fromColor, toColor;
    int toColorBGR[3];
    fromIdx = ui->comboFrom->currentIndex();
    toIdx = ui->comboTo->currentIndex();
    switch (fromIdx) {
    case ROCK_TEXTURE:
        fromColor = ROCK_TEXTURE_GRAY;
        break;
    case SAND_TEXTURE:
        fromColor = SAND_TEXTURE_GRAY;
        break;
    case TREE_TEXTURE:
        fromColor = TREE_TEXTURE_GRAY;
        break;
    case WATER_TEXTURE:
        fromColor = WATER_TEXTURE_GRAY;
        break;
    default:
        break;
    }
    switch (toIdx) {
    case ROCK_TEXTURE:
        toColor = ROCK_TEXTURE_GRAY;
        toColorBGR[0] = ROCK_TEXTURE_B;
        toColorBGR[1] = ROCK_TEXTURE_G;
        toColorBGR[2] = ROCK_TEXTURE_R;
        break;
    case SAND_TEXTURE:
        toColor = SAND_TEXTURE_GRAY;
        toColorBGR[0] = SAND_TEXTURE_B;
        toColorBGR[1] = SAND_TEXTURE_G;
        toColorBGR[2] = SAND_TEXTURE_R;
        break;
    case TREE_TEXTURE:
        toColor = TREE_TEXTURE_GRAY;
        toColorBGR[0] = TREE_TEXTURE_B;
        toColorBGR[1] = TREE_TEXTURE_G;
        toColorBGR[2] = TREE_TEXTURE_R;
        break;
    case WATER_TEXTURE:
        toColor = WATER_TEXTURE_GRAY;
        toColorBGR[0] = WATER_TEXTURE_B;
        toColorBGR[1] = WATER_TEXTURE_G;
        toColorBGR[2] = WATER_TEXTURE_R;
        break;
    default:
        break;
    }
    if (!poly.isEmpty())
    {
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                if (poly.containsPoint(QPoint(j, i), Qt::OddEvenFill)
                        && (int)texFile.at<uchar>(i, j) == fromColor)
                {
                    srcPic.at<Vec3b>(i, j)[0] = toColorBGR[0];
                    srcPic.at<Vec3b>(i, j)[1] = toColorBGR[1];
                    srcPic.at<Vec3b>(i, j)[2] = toColorBGR[2];
                    texFile.at<uchar>(i, j) = toColor;
                }
            }
        }
    }else if(ui->btnRectangle->isChecked())
    {
        int row = fabs(recStart.y()-recEnd.y());
        int col = fabs(recStart.x()-recEnd.x());
        int startX = min(recStart.x(), recEnd.x());
        int startY = min(recStart.y(), recEnd.y());
        startX = max(startX, 0);
        startY = max(startY, 0);
        int endX = min(col+startX, srcPic.cols);
        int endY = min(row+startY, srcPic.rows);
        for (int i = startY; i < endY; i++)
        {
            for (int j = startX; j < endX; j++)
            {
                if ((int)texFile.at<uchar>(i, j) == fromColor)
                {
                    srcPic.at<Vec3b>(i, j)[0] = toColorBGR[0];
                    srcPic.at<Vec3b>(i, j)[1] = toColorBGR[1];
                    srcPic.at<Vec3b>(i, j)[2] = toColorBGR[2];
                    texFile.at<uchar>(i, j) = toColor;
                }
            }
        }
    }

    refreshSrcImg();
}

//void MainWindow::clearAt(Mat origin, int i, int j)
//{
//    texFile.at<uchar>(i, j) = NO_TEXTURE_GRAY;
//    srcPic.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j);
//}

void MainWindow::on_btnPaint_clicked()
{
    if (ui->materialList->currentRow() == -1)
        return;
    if (!poly.isEmpty())
    {
        int row = srcPic.rows; // number of lines
        int col = srcPic.cols;
        int toColor;
        int toColorBGR[3];
        switch (ui->materialList->currentRow()) {
        case ROCK_TEXTURE:
            toColor = ROCK_TEXTURE_GRAY;
            toColorBGR[0] = ROCK_TEXTURE_B;
            toColorBGR[1] = ROCK_TEXTURE_G;
            toColorBGR[2] = ROCK_TEXTURE_R;
            break;
        case SAND_TEXTURE:
            toColor = SAND_TEXTURE_GRAY;
            toColorBGR[0] = SAND_TEXTURE_B;
            toColorBGR[1] = SAND_TEXTURE_G;
            toColorBGR[2] = SAND_TEXTURE_R;
            break;
        case TREE_TEXTURE:
            toColor = TREE_TEXTURE_GRAY;
            toColorBGR[0] = TREE_TEXTURE_B;
            toColorBGR[1] = TREE_TEXTURE_G;
            toColorBGR[2] = TREE_TEXTURE_R;
            break;
        case WATER_TEXTURE:
            toColor = WATER_TEXTURE_GRAY;
            toColorBGR[0] = WATER_TEXTURE_B;
            toColorBGR[1] = WATER_TEXTURE_G;
            toColorBGR[2] = WATER_TEXTURE_R;
            break;
        default:
            break;
        }
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                if(poly.containsPoint(QPoint(j, i), Qt::OddEvenFill))
                {
                    srcPic.at<Vec3b>(i, j)[0] = toColorBGR[0];
                    srcPic.at<Vec3b>(i, j)[1] = toColorBGR[1];
                    srcPic.at<Vec3b>(i, j)[2] = toColorBGR[2];
                    texFile.at<uchar>(i, j) = toColor;
                }
            }
        }
        refreshSrcImg();
    }else if (ui->btnRectangle->isChecked())
    {
        int row = fabs(recStart.y()-recEnd.y());
        int col = fabs(recStart.x()-recEnd.x());
        int startX = min(recStart.x(), recEnd.x());
        int startY = min(recStart.y(), recEnd.y());
        startX = max(startX, 0);
        startY = max(startY, 0);
        int endX = min(col+startX, srcPic.cols);
        int endY = min(row+startY, srcPic.rows);
        int toColor;
        int toColorBGR[3];
        switch (ui->materialList->currentRow()) {
        case ROCK_TEXTURE:
            toColor = ROCK_TEXTURE_GRAY;
            toColorBGR[0] = ROCK_TEXTURE_B;
            toColorBGR[1] = ROCK_TEXTURE_G;
            toColorBGR[2] = ROCK_TEXTURE_R;
            break;
        case SAND_TEXTURE:
            toColor = SAND_TEXTURE_GRAY;
            toColorBGR[0] = SAND_TEXTURE_B;
            toColorBGR[1] = SAND_TEXTURE_G;
            toColorBGR[2] = SAND_TEXTURE_R;
            break;
        case TREE_TEXTURE:
            toColor = TREE_TEXTURE_GRAY;
            toColorBGR[0] = TREE_TEXTURE_B;
            toColorBGR[1] = TREE_TEXTURE_G;
            toColorBGR[2] = TREE_TEXTURE_R;
            break;
        case WATER_TEXTURE:
            toColor = WATER_TEXTURE_GRAY;
            toColorBGR[0] = WATER_TEXTURE_B;
            toColorBGR[1] = WATER_TEXTURE_G;
            toColorBGR[2] = WATER_TEXTURE_R;
            break;
        default:
            break;
        }
        for (int i = startY; i < endY; i++)
        {
            for (int j = startX; j < endX; j++)
            {
                srcPic.at<Vec3b>(i, j)[0] = toColorBGR[0];
                srcPic.at<Vec3b>(i, j)[1] = toColorBGR[1];
                srcPic.at<Vec3b>(i, j)[2] = toColorBGR[2];
                texFile.at<uchar>(i, j) = toColor;
            }
        }
        refreshSrcImg();
    }

}

void MainWindow::refreshSrcImg()
{
    Mat tmp;
    cvtColor(srcPic,tmp,COLOR_BGR2RGB);
    labelPic->setPixmap(QPixmap::fromImage(QImage(tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888)));
}

void MainWindow::on_btnRectangle_clicked()
{
    poly.clear();
}
