#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    QMainWindow window;
    window.setWindowTitle("Agenda Qt");
    window.resize(900, 600);

    QLabel *label = new QLabel("Agenda Qt - project initialized");
    label->setAlignment(Qt::AlignCenter);

    window.setCentralWidget(label);
    window.show();

    return app.exec();
}