#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel *labelPic;
    QString filename;
    cv::Mat srcPic;
    cv::Mat labPic;
    cv::Mat texFile;
    QPolygon poly;
    QImage imgSelectArea;
    QPoint recStart;
    QPoint recEnd;
    bool startDrawBound;
    bool startPaint;
    double mScale;
    cv::Mat get_contour();
    void mousePressEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
//    void wheelEvent(QWheelEvent *event);
//    void paintEvent ( QPaintEvent * );
    bool color_similar(int ax, int ay, int bx, int by, double threshold);
//    void clearAt(cv::Mat origin, int i, int j);
    void refreshSrcImg();

private slots:
    void open_image();
//    void on_btnCanny_clicked();
    void on_btnContour_clicked();
    void on_btnClear_clicked();
    void on_btnSave_clicked();
    void on_btnConvert_clicked();
    void on_btnPaint_clicked();
    void on_btnContain_clicked();
    void on_btnRectangle_clicked();
    void on_btnColorChoose_clicked();
};

#endif // MAINWINDOW_H
