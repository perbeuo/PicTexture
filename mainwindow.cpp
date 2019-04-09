#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <QMouseEvent>
#include <QPainter>
#include "ciede2000.h"
using namespace std;
using namespace cv;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    startDrawBound(false),
    startPaint(false)
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

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    QPoint pt;
    pt = e->globalPos();
    pt = labelPic->mapFromGlobal(pt);
    if(pt.x()-1 >= 0 && pt.x()-1 <= labelPic->width() &&
            pt.y()-2 >= 0 && pt.y()-2 <= labelPic->height()
            && ui->btnColorChoose->isChecked()
            && ui->materialList->currentRow() != -1)
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
                    case 0:
                        srcPic.at<Vec3b>(i, j)[0] = 0;
                        srcPic.at<Vec3b>(i, j)[1] = 255;
                        srcPic.at<Vec3b>(i, j)[2] = 255;
                        texFile.at<uchar>(i, j) = 0;
                        break;
                    case 1:
                        srcPic.at<Vec3b>(i, j)[0] = 0;
                        srcPic.at<Vec3b>(i, j)[1] = 255;
                        srcPic.at<Vec3b>(i, j)[2] = 0;
                        texFile.at<uchar>(i, j) = 128;
                        break;
                    case 2:
                        srcPic.at<Vec3b>(i, j)[0] = 255;
                        srcPic.at<Vec3b>(i, j)[1] = 0;
                        srcPic.at<Vec3b>(i, j)[2] = 0;
                        texFile.at<uchar>(i, j) = 255;
                        break;
                    default:
                        break;
                    }

                }
            }
        }
        Mat tmp;
        cvtColor(srcPic,tmp,COLOR_BGR2RGB);
        labelPic->setPixmap(QPixmap::fromImage(QImage(tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888)));
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
    }else if(ui->btnContain->isChecked())
    {
        QPoint pt;
        pt = e->globalPos();
        pt = labelPic->mapFromGlobal(pt);
        pt.setX(pt.x()-1);
        pt.setY(pt.y()-2);
        cout << "start:";
        cout << "(" << pt.x() << "," << pt.y() << ")" << endl;
        cout << poly.containsPoint(pt, Qt::OddEvenFill) << endl;

//        for (int i = 0; i < srcPic.rows; i++)
//        {
//            for (int j = 0; j < srcPic.cols; j++)
//            {
//                if (poly.containsPoint(QPoint(j, i), Qt::OddEvenFill))
//                {
//                    cout << "(" << j << "," << i << ")" << endl;
//                }

//            }
//        }
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
        painter.setBrush(Qt::red);
        painter.drawPolygon(poly);
//        painter.drawPolygon(points, 4);
//        painter.drawRect(50, 50, 160, 100);
        labelPic->setPixmap(QPixmap::fromImage(imgSelectArea));
    }
}

//if a point is in this polygon?
//1. use some algorithm to find out
//2. draw a new black&white image as the same size of the original one,
//   find out that point's color


void MainWindow::on_btnCanny_clicked()
{
    if (filename.isEmpty())
        return;
    Mat src, src_gray, dst;
    GaussianBlur(srcPic, src, Size(3,3),0);
    cvtColor(src,src_gray,COLOR_BGR2GRAY);
    Canny(src_gray,dst,50,200);
    //    labelPic->setPixmap(QPixmap::fromImage(QImage(dst.data, dst.cols, dst.rows, dst.step, QImage::Format_Grayscale8)));

    //    int src_row= src.rows; // number of lines
    //    int src_col= src.cols * src.channels();
    int dst_row= dst.rows; // number of lines
    int dst_col= dst.cols * dst.channels();

    for (int i = 0; i < dst_row; i++)
    {
        //uchar* src_data = src.ptr<uchar>(i);
        //uchar* dst_data = dst.ptr<uchar>(i);
        for (int j = 0; j < dst_col; j++)
        {
            int pixel = dst.at<uchar>(i, j);
            if (pixel == 255)
            {
                src.at<Vec3b>(i, j)[0]= 0;//b
                src.at<Vec3b>(i, j)[1]= 0;//g
                src.at<Vec3b>(i, j)[2]= 0;//r
            }
        }
    }
    cvtColor(src,src,COLOR_BGR2RGB);
    labelPic->setPixmap(QPixmap::fromImage(QImage(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888)));
}
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
    Mat tmp;
    srcPic = imread(filename.toStdString());
    cvtColor(srcPic,tmp,COLOR_BGR2RGB);
    labelPic->setPixmap(QPixmap::fromImage(QImage(tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888)));
    texFile = Mat::zeros(srcPic.rows, srcPic.cols, CV_8U);
}

void MainWindow::on_btnSave_clicked()
{
    labelPic->setPixmap(QPixmap::fromImage(QImage(texFile.data, texFile.cols, texFile.rows, texFile.step, QImage::Format_Grayscale8)));
}
