#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QListWidget>
#include <QVBoxLayout>
#include <QTime>
#include <QSerialPortInfo>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , parameterDialog(nullptr) // Инициализируем указатель parameterDialog значением nullptr
{
    ui->setupUi(this);
    subMenu = nullptr;

    // Установка соединения с базой данных
    QSqlDatabase database = QSqlDatabase::addDatabase("QMYSQL");
            database.setHostName("127.0.0.1");
            database.setDatabaseName("SensorData");
            database.setUserName("root");
            database.setPassword("1111");

        if (!database.open()) {
            QMessageBox::warning(this, "Error", "Failed to open database!");
        }

    // Инициализация порта
    COMPORT = new QSerialPort();
    COMPORT->setPortName("/dev/tty.usbmodem14201");
    COMPORT->setBaudRate(QSerialPort::BaudRate::Baud9600);
    COMPORT->setParity(QSerialPort::Parity::NoParity);
    COMPORT->setDataBits(QSerialPort::DataBits::Data8);
    COMPORT->setStopBits(QSerialPort::StopBits::OneStop);
    COMPORT->setFlowControl(QSerialPort::FlowControl::NoFlowControl);

    // Чистка файла при его открытии
    QFile file("/Users/nikitapodterob/Desktop/Proj/serial_data.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.close();
    } else {
        // Отображение окна предупреждения при неудачной чистке файла
        QMessageBox::warning(this, "Error", "Failed to clear file!");
    }

    COMPORT->open(QIODevice::ReadWrite);
    if (COMPORT->isOpen()) {
        ui->toggle_Button->setText("Disconnect");
        statusBar()->showMessage("Connected to serial port", 3000); // Вывод сообщения в статусбар
    } else {
        statusBar()->showMessage("Failed to connect to serial port", 3000); // Вывод сообщения в статусбар
    }

    connect(COMPORT, SIGNAL(readyRead()), this, SLOT(Read_Data()));
    COMPORT->flush();

    ui->serialData->insertPlainText("\n");
    ui->edit->append("№\tvalue\t\tDate");

}

MainWindow::~MainWindow()
{
    delete ui;
    delete parameterDialog; // Удаляем объект QDialog, чтобы избежать утечки памяти
    // Закрытие соединения с базой данных
        QSqlDatabase::database().close();
}

// считывание информации сериал порта
void MainWindow::Read_Data()
{
    while (COMPORT->canReadLine()) {
        QTime curr_time = QTime::currentTime(); // текущее время
        QString time = curr_time.toString("hh:mm:ss");

        QByteArray data = COMPORT->readLine(); // читаем порт
        QString lineNumberString = QString::number(lineNumber);

        QString output = QString("%1\t%2\t\t%3\n").arg(lineNumberString, QString(data).trimmed(), time);
        ui->serialData->append(output);

        lineNumber++;
        ui->statusIndicator_2->setStyleSheet(Green);

        // Запись строки в файл
        QFile file("/Users/nikitapodterob/Desktop/Proj/serial_data.txt");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << data;
            file.close();
        } else {
            // Отображение окна предупреждения при неудачном открытии файла
            QMessageBox::warning(this, "Error", "Failed to open file for writing!");
        }

        // Добавление данных в базу данных
        if (ui->toggle_Button->text() == "Disconnect") {
                    InsertDataToDatabase(data);
        }
    }
}

// вставка данных в БД
void MainWindow::InsertDataToDatabase(const QByteArray& data)
{
    if (ui->toggle_Button->text() == "Disconnect") {
        if (parameterDialog) {
            QListWidget* parameterList = parameterDialog->findChild<QListWidget*>();
            if (parameterList) {
                QListWidgetItem* selectedItem = parameterList->currentItem();
                if (selectedItem) {
                    QString selectedParameter = selectedItem->text();

                    // Определение имени таблицы и имени поля в зависимости от выбранного параметра
                    QString fieldName;
                    int id = 1;

                    if (selectedParameter == "TemperatureSensor") {
                        fieldName = "temperature";
                        id = 1;
                    } else if (selectedParameter == "PressureSensor") {
                        fieldName = "pressure";
                        id = 2;
                    } else if (selectedParameter == "LightSensor") {
                        fieldName = "light_intensity";
                        id = 3;
                    } else if (selectedParameter == "ProximitySensor") {
                        fieldName = "proximity";
                        id = 5;
                    } else if (selectedParameter == "SoundSensor") {
                        fieldName = "sound_level";
                        id = 6;
                    } else {
                        // Параметр не соответствует ни одному из случаев
                        return;
                    }

                    // Формирование SQL-запроса
                    QString queryStr = "INSERT INTO " + selectedParameter + " (" + fieldName + ", sensor_id, sensor_type) VALUES (?, ?, ?)";

                    // Выполнение SQL-запроса
                    QSqlQuery query;
                    query.prepare(queryStr);
                    query.addBindValue(data.toFloat()); // Значение для первого поля
                    query.addBindValue(id); // Значение для поля sensor_id
                    query.addBindValue(selectedParameter); // Значение для поля sensor_type

                    if (query.exec()) {
                        ui->statusIndicator->setStyleSheet(Green);
                    } else {
                        qDebug() << query.lastError().text();
                        ui->statusIndicator->setStyleSheet(Red);
                    }
                }
            }
        } else {
            // Объект parameterDialog не существует, выполните нужные действия
            ui->statusIndicator->setStyleSheet(Red);
        }
    }
}


// стоп | пуск  данных из сериал порта
void MainWindow::on_toggle_Button_clicked()
{
    if (ui->toggle_Button->text() == "Connect") {
        COMPORT->close();
        COMPORT->open(QIODevice::ReadWrite);
        if (COMPORT->isOpen()) {
            ui->toggle_Button->setText("Disconnect");
            statusBar()->showMessage("Connected to serial port", 3000); // Вывод сообщения в статусбар
        } else {
            statusBar()->showMessage("Failed to connect to serial port", 3000); // Вывод сообщения в статусбар
        }
    } else {
        ui->statusIndicator->setStyleSheet(Grey);
        ui->statusIndicator_2->setStyleSheet(Grey);

        COMPORT->close();
        ui->toggle_Button->setText("Connect");
        statusBar()->showMessage("Disconnected from serial port", 3000); // Вывод сообщения в статусбар
    }
}

// при нажатии на кнопку в меню для появления списка параметров
void MainWindow::on_action_triggered()
{
    if (!parameterDialog) {
        // Создаем диалоговое окно только при первом вызове функции
        parameterDialog = new QDialog(this);
        parameterDialog->setWindowTitle("Выберите Датчик");
        parameterDialog->setModal(true);

        // Создаем список параметров
        QListWidget* parameterList = new QListWidget(parameterDialog);
        parameterList->addItem("GasSensor");
        parameterList->addItem("TemperatureSensor");
        parameterList->addItem("PressureSensor");
        parameterList->addItem("LightSensor");
        parameterList->addItem("HumiditySensor");
        parameterList->addItem("ProximitySensor");
        parameterList->addItem("SoundSensor");

        // Устанавливаем вертикальное размещение для диалогового окна
        QVBoxLayout* layout = new QVBoxLayout(parameterDialog);
        layout->addWidget(parameterList);
        parameterDialog->setLayout(layout);

        // Подключаем сигнал выбора параметра к слоту
        connect(parameterList, &QListWidget::itemSelectionChanged, this, &MainWindow::handleParameterSelection);
    }

    // Показываем диалоговое окно для выбора параметра
    parameterDialog->show();
}

// для выбора параметров в списке в меню
void MainWindow::handleParameterSelection()
{
    QListWidget* parameterList = qobject_cast<QListWidget*>(sender());
    if (parameterList) {
        QListWidgetItem* selectedItem = parameterList->currentItem();
        if (selectedItem) {
            QString selectedParameter = selectedItem->text();
            // Выполнение действия на основе выбранного параметра
            QMessageBox::information(this, "Success", selectedParameter + " selected");
            selectedParameter.remove("Sensor");
            ui->edit->setPlainText("№\t" + selectedParameter + "\t\tDate");
            ui->serialData->clear();
            lineNumber = 0;
        }
    }
}

void MainWindow::on_action_4_hovered()
{
    if (!subMenu) {
        subMenu = new QMenu(this);
        ui->action_4->setMenu(subMenu);
    } else {
        subMenu->clear();
    }

    QList<QSerialPortInfo> usbDevices = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& usbDevice : usbDevices) {
        QString portName = usbDevice.portName();
        if (portName.startsWith("tty.usbmodem")) {
            QString portLocation = usbDevice.systemLocation();
            QAction* action = subMenu->addAction(portName + "  ("+usbDevice.description()+")");
            connect(action, &QAction::triggered, this, [this, portName, portLocation]() {
                // Обработка нажатия на пункт меню
                COMPORT->setPortName(portLocation);
            });
            subMenu->addAction(action);
        }
    }
}
