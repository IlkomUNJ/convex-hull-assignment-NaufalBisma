#include "mainwindow.h"

// Konstruktor
DrawingCanvas::DrawingCanvas(QWidget *parent)
    : QWidget(parent), slowIterations(0), fastIterations(0), isSlowResult(true)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(false);
}

// Fungsi bantu orientasi (Cross Product Test)
int DrawingCanvas::orientation(const QPoint& p, const QPoint& q, const QPoint& r) const
{
    qint64 val = (q.y() - p.y()) * (r.x() - q.x()) -
                 (q.x() - p.x()) * (r.y() - q.y());

    if (val == 0) return 0;
    return (val > 0) ? 1 : 2;
}

// Fungsi bantu jarak kuadrat
qint64 DrawingCanvas::distSq(const QPoint& p1, const QPoint& p2) const {
    return (qint64)(p1.x() - p2.x()) * (p1.x() - p2.x()) +
           (qint64)(p1.y() - p2.y()) * (p1.y() - p2.y());
}

// Fungsi bantu perbandingan sudut polar untuk Graham Scan
bool DrawingCanvas::comparePolarAngle(const QPoint& a, const QPoint& b)
{
    int o = orientation(p0, a, b);
    if (o == 0)
        return (distSq(p0, a) < distSq(p0, b));
    return (o == 2);
}

// Menangani klik mouse untuk menambahkan titik
void DrawingCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        points.append(event->pos());
        slowHull.clear();
        fastHull.clear();
        slowIterations = 0;
        fastIterations = 0;
        update();
    }
}

// Menggambar titik, hull, dan hasil iterasi
void DrawingCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Latar belakang putih
    painter.fillRect(rect(), QBrush(Qt::white));

    // Menggambar Titik
    painter.setBrush(QBrush(Qt::blue));
    for (const QPoint& p : points) {
        painter.drawEllipse(p, 4, 4);
    }

    // Menggambar Convex Hull (hasil yang aktif)
    const QVector<QPoint>& currentHull = isSlowResult ? slowHull : fastHull;
    const QString algorithmName = isSlowResult ? "Brute Force" : "Graham Scan";
    const int iterations = isSlowResult ? slowIterations : fastIterations;

    if (!currentHull.isEmpty() && currentHull.size() > 1) {
        painter.setPen(QPen(Qt::red, 3));
        for (int i = 0; i < currentHull.size(); ++i) {
            painter.drawLine(currentHull[i], currentHull[(i + 1) % currentHull.size()]);
        }
    }

    // Menuliskan hasil Iterasi
    painter.setPen(QPen(Qt::black));
    painter.setFont(QFont("Arial", 12));

    QString iterationText = "Titik: " + QString::number(points.size());
    if (points.size() >= 3) {
        iterationText += "\nAlgoritma: " + algorithmName;
        iterationText += "\nIterasi (Orientasi Test): " + QString::number(iterations);
    } else {
        iterationText += "\nTambahkan minimal 3 titik untuk menjalankan algoritma.";
    }

    // Teks di sudut kanan bawah
    painter.drawText(this->width() - 300, this->height() - 70, 290, 60, Qt::AlignRight | Qt::AlignBottom, iterationText);
}

// Menghapus kanvas dan mereset data
void DrawingCanvas::clearCanvas()
{
    points.clear();
    slowHull.clear();
    fastHull.clear();
    slowIterations = 0;
    fastIterations = 0;
    update();
}

// Algoritma: Slow Convex Hull (Brute Force O(N^3))
void DrawingCanvas::runSlowConvexHull()
{
    if (points.size() < 3) return;

    slowHull.clear();
    slowIterations = 0;
    isSlowResult = true;

    int n = points.size();
    QVector<QPoint> hullPoints;
    QVector<QPoint> potentialEdges;

    // 1. Cek setiap pasangan titik (pi, pj)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;

            bool isEdge = true;
            int side = 0;

            // 2. Cek apakah semua titik lain (pk) berada pada sisi yang sama
            for (int k = 0; k < n; ++k) {
                if (k == i || k == j) continue;

                slowIterations++;

                int o = orientation(points[i], points[j], points[k]);

                if (o != 0) {
                    if (side == 0) {
                        side = o;
                    } else if (side != o) {
                        isEdge = false;
                        break;
                    }
                }
            }

            // 3. Jika (pi, pj) adalah sisi hull, tambahkan ke hasil (pastikan urutan CCW)
            if (isEdge) {
                potentialEdges.append(points[i]);
            }
        }
    }

    // Membersihkan dan menyortir potentialEdges menjadi urutan Convex Hull yang benar
    if (!potentialEdges.isEmpty()) {
        std::sort(potentialEdges.begin(), potentialEdges.end(), [](const QPoint& a, const QPoint& b){
            if (a.x() != b.x()) return a.x() < b.x();
            return a.y() < b.y();
        });
        potentialEdges.erase(std::unique(potentialEdges.begin(), potentialEdges.end()), potentialEdges.end());

        // 1. Temukan p0 (titik dengan y terkecil)
        int minY = potentialEdges[0].y(), minX = potentialEdges[0].x(), minIdx = 0;
        for (int i = 1; i < potentialEdges.size(); i++) {
            int y = potentialEdges[i].y();
            if ((y < minY) || (minY == y && potentialEdges[i].x() < minX)) {
                minY = potentialEdges[i].y();
                minX = potentialEdges[i].x();
                minIdx = i;
            }
        }
        std::swap(potentialEdges[0], potentialEdges[minIdx]);
        p0 = potentialEdges[0];

        // 2. Sort points by polar angle with respect to p0 (using a lambda that captures `this`)
        std::sort(potentialEdges.begin() + 1, potentialEdges.end(), [this](const QPoint& a, const QPoint& b){
            return this->comparePolarAngle(a, b);
        });

        // 3. Gunakan Stack untuk hasil akhir
        if (potentialEdges.size() < 3) {
            slowHull = potentialEdges;
            update();
            return;
        }

        QVector<QPoint> S;
        S.append(potentialEdges[0]);
        S.append(potentialEdges[1]);
        S.append(potentialEdges[2]);

        for (int i = 3; i < potentialEdges.size(); i++) {
            // Cek belokan
            while (S.size() > 1 && orientation(S[S.size()-2], S.last(), potentialEdges[i]) != 2) {
                S.pop_back();
            }
            S.append(potentialEdges[i]);
        }
        slowHull = S;
    }

    update();
}


// Algoritma: Fast Convex Hull (Graham Scan O(N log N))
void DrawingCanvas::runFastConvexHull()
{
    if (points.size() < 3) return;
    fastHull.clear();
    fastIterations = 0;
    isSlowResult = false;
    QVector<QPoint> sortedPoints = points;

    // 1. Temukan p0 (titik dengan y terkecil, dan jika sama, x terkecil)
    int minY = sortedPoints[0].y(), minX = sortedPoints[0].x(), minIdx = 0;
    for (int i = 1; i < sortedPoints.size(); i++) {
        int y = sortedPoints[i].y();
        if ((y < minY) || (minY == y && sortedPoints[i].x() < minX)) {
            minY = sortedPoints[i].y();
            minX = sortedPoints[i].x();
            minIdx = i;
        }
    }
    std::swap(sortedPoints[0], sortedPoints[minIdx]);
    p0 = sortedPoints[0];

    // 2. Urutkan titik berdasarkan sudut polar terhadap p0 (Dominasi O(N log N))
    // Note: Iterasi di sini tidak dihitung sebagai 'orientasi test' untuk metrik yang diminta.
    std::sort(sortedPoints.begin() + 1, sortedPoints.end(), [this](const QPoint& a, const QPoint& b){
        return this->comparePolarAngle(a, b);
    });

    // 3. Bangun Convex Hull menggunakan Stack
    // Handle kasus titik-titik yang segaris setelah p0
    int m = 1;
    for (int i=1; i<sortedPoints.size(); i++) {
        while (i < sortedPoints.size()-1 && orientation(p0, sortedPoints[i], sortedPoints[i+1]) == 0)
            i++;
        sortedPoints[m] = sortedPoints[i];
        m++;
    }

    if (m < 3) {
        fastHull = sortedPoints.mid(0, m);
        update();
        return;
    }

    QVector<QPoint> S;
    S.append(sortedPoints[0]);
    S.append(sortedPoints[1]);
    S.append(sortedPoints[2]);

    for (int i = 3; i < m; i++) {
        while (S.size() > 1 && orientation(S[S.size()-2], S.last(), sortedPoints[i]) != 2) {
            fastIterations++;
            S.pop_back();
        }
        fastIterations++;
        S.append(sortedPoints[i]);
    }

    fastHull = S;
    update();
}

// Implementasi MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Aplikasi Convex Hull (Qt Widgets)");

    // 1. Central Widget & Main Layout (QVBoxLayout)
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 2. Drawing Widget (Canvas)
    canvas = new DrawingCanvas(this);
    canvas->setMinimumSize(400, 400);
    mainLayout->addWidget(canvas, 1);

    // 3. Button Layout (QHBoxLayout)
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 4. Buttons
    runSlowButton = new QPushButton("Run Slow CH (Brute Force)");
    runFastButton = new QPushButton("Run Fast CH (Graham Scan)");
    clearButton = new QPushButton("Clear Canvas");

    // Styling minimal untuk tombol
    QString buttonStyle = "QPushButton { padding: 10px; border-radius: 8px; font-weight: bold; }";
    runSlowButton->setStyleSheet(buttonStyle + " QPushButton { background-color: #F8BBD0; color: #880E4F; }");
    runFastButton->setStyleSheet(buttonStyle + " QPushButton { background-color: #BBDEFB; color: #0D47A1; }");
    clearButton->setStyleSheet(buttonStyle + " QPushButton { background-color: #CFD8DC; color: #37474F; }");

    // Menambahkan tombol ke layout horizontal
    buttonLayout->addWidget(runSlowButton);
    buttonLayout->addWidget(runFastButton);
    buttonLayout->addWidget(clearButton);

    // Menambahkan layout tombol ke layout utama
    mainLayout->addLayout(buttonLayout);

    // Menghubungkan Sinyal ke Slot
    connect(runSlowButton, &QPushButton::clicked, canvas, &DrawingCanvas::runSlowConvexHull);
    connect(runFastButton, &QPushButton::clicked, canvas, &DrawingCanvas::runFastConvexHull);
    connect(clearButton, &QPushButton::clicked, canvas, &DrawingCanvas::clearCanvas);

    // Atur ukuran jendela awal
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    // Destruktor tidak perlu melakukan apa-apa karena Qt menangani penghapusan child objects
}
