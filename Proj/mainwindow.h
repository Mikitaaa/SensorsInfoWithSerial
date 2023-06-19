#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QSerialPort>
#include <QMessageBox>
#include <QString>
#include <QListWidgetItem>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void Read_Data();
    void on_toggle_Button_clicked();
    void on_action_triggered();
    void handleParameterSelection();
    void InsertDataToDatabase(const QByteArray& data);

    void on_action_4_hovered();

private:
    Ui::MainWindow *ui;
    QSerialPort *COMPORT;
    QString Serial_Data = "";
    bool Is_Data_Received = false;
    QListWidget *parameterList;
    QDialog* parameterDialog; // Объявление переменной parameterDialog
    QMenu* subMenu;

    int lineNumber = 1; // переменная номера строки
    QSqlDatabase database;
    QString Green ="background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(80, 255, 80, 255), stop:1 rgba(50, 230, 50, 255));"
                    "border: none;"
                    "border-radius: 10px;"
                    "width: 20px;"
                    "height: 20px;";
    QString Red ="background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 80, 80, 255), stop:1 rgba(230, 50, 50, 255));"
                    "border: none;"
                    "border-radius: 10px;"
                    "width: 20px;"
                    "height: 20px;";
    QString Grey ="background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(220, 220, 220, 255), stop:1 rgba(180, 180, 180, 255));"
                    "border: none;"
                    "border-radius: 10px;"
                    "width: 20px;"
                    "height: 20px;";
};
#endif // MAINWINDOW_H
