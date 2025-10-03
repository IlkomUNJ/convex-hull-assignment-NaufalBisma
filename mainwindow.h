#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QPoint>
#include <QVector>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <cmath>
#include <QDebug>

// ====================================================================
// Custom Widget: DrawingCanvas
// Widget ini berfungsi sebagai kanvas gambar dan implementasi algoritma
// ====================================================================
class DrawingCanvas : public QWidget
{
    Q_OBJECT

private:
    QVector<QPoint> points; // Kumpulan titik yang dimasukkan pengguna
    QVector<QPoint> slowHull; // Hasil Convex Hull Lambat
    QVector<QPoint> fastHull; // Hasil Convex Hull Cepat
    int slowIterations;       // Jumlah iterasi Convex Hull Lambat
    int fastIterations;       // Jumlah iterasi Convex Hull Cepat
    bool isSlowResult;        // Bendera untuk menentukan hasil mana yang akan digambar

    // Fungsi bantu untuk menentukan orientasi tiga titik (p, q, r)
    // 0: Collinear (Segaris)
    // 1: Clockwise (Searah jarum jam / Belok Kanan)
    // 2: Counterclockwise (Berlawanan arah jarum jam / Belok Kiri)
    int orientation(const QPoint& p, const QPoint& q, const QPoint& r) const;

    // Fungsi bantu untuk menghitung jarak kuadrat antara dua titik
    qint64 distSq(const QPoint& p1, const QPoint& p2) const;

    // Fungsi bantu untuk membandingkan sudut polar (diperlukan untuk Graham Scan)
    QPoint p0; // Titik referensi untuk Graham Scan
    bool comparePolarAngle(const QPoint& a, const QPoint& b);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public:
    explicit DrawingCanvas(QWidget *parent = nullptr);

public slots:
    void clearCanvas();
    void runSlowConvexHull(); // Brute Force (O(N^3))
    void runFastConvexHull(); // Graham Scan (O(N log N))

signals:
    void canvasUpdated(); // Sinyal untuk memberitahu MainWindow saat ada perubahan
};

// ====================================================================
// MainWindow
// Mengatur tata letak utama dan menghubungkan tombol ke slot canvas
// ====================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    DrawingCanvas *canvas;
    QPushButton *runSlowButton;
    QPushButton *runFastButton;
    QPushButton *clearButton;
};

#endif // MAINWINDOW_H
