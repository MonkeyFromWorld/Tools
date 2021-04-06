/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "common.h"
#include "modbusServer.h"
#include "serverSettingsdialog.h"
#include "ui_modbusServer.h"

#include <QModbusRtuSerialSlave>
#include <QModbusTcpServer>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStatusBar>
#include <QUrl>
#include <QSpinBox>
#include <QDebug>

enum ModbusConnection {
    Serial,
    Tcp
};

ModbusServer::ModbusServer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ModbusServer)
{
    ui->setupUi(this);
    setupWidgetContainers();

#if QT_CONFIG(modbus_serialport)
    ui->connectType->setCurrentIndex(0);
    onCurrentConnectTypeChanged(0);
#else
    // lock out the serial port option
    ui->connectType->setCurrentIndex(1);
    onCurrentConnectTypeChanged(1);
    ui->connectType->setEnabled(false);
#endif

    m_settingsDialog = new ServerSettingsDialog(this);
    initActions();
}

ModbusServer::~ModbusServer()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;

    delete ui;
}

void ModbusServer::initActions()
{
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionOptions->setEnabled(true);

    connect(ui->spinBox_coilStartAddress, QOverload<int>::of(&QSpinBox::valueChanged), this, &ModbusServer::onCoilStartAddressChange);
    connect(ui->spinBox_DI_startAddress, QOverload<int>::of(&QSpinBox::valueChanged), this, &ModbusServer::onDIStartAddressChange);
    connect(ui->spinBox_IR_startAddress, QOverload<int>::of(&QSpinBox::valueChanged), this, &ModbusServer::onIRStartAddressChange);
    connect(ui->spinBox_HR_startAddress, QOverload<int>::of(&QSpinBox::valueChanged), this, &ModbusServer::onHRStartAddressChange);

    connect(ui->connectButton, &QPushButton::clicked, this, &ModbusServer::onConnectButtonClicked);
    connect(ui->actionConnect, &QAction::triggered, this, &ModbusServer::onConnectButtonClicked);
    connect(ui->actionDisconnect, &QAction::triggered, this, &ModbusServer::onConnectButtonClicked);
    connect(ui->connectType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::onCurrentConnectTypeChanged);
    connect(ui->actionOptions, &QAction::triggered, m_settingsDialog, &QDialog::show);
    connect(ui->actionNavigation, &QAction::triggered, this, &ModbusServer::on_actionNavigation_triggered);

}

void ModbusServer::on_actionNavigation_triggered()
{
    m_settingsDialog->hide();
    emit display(NAVIGATION_PAGE);
}

void ModbusServer::onCoilStartAddressChange(int startAddress)
{
    qDebug() << "CoilStartAddressChange:" << startAddress << endl;
    QString text;
    for (int i = 0; i < 10; ++i)
    {
        coilButtons.button(i)->setText(text.setNum(startAddress+i, 10));
        //coilCheckBox.value(QStringLiteral("coils_%1").arg(i))->setText(text.setNum(startAddress+i, 16));
    }
}

void ModbusServer::onDIStartAddressChange(int startAddress)
{
    qDebug() << "DIStartAddressChange:" << startAddress<< endl;
}

void ModbusServer::onIRStartAddressChange(int startAddress)
{
    qDebug() << "IRStartAddressChange：" << startAddress<< endl;
}

void ModbusServer::onHRStartAddressChange(int startAddress)
{
    qDebug() << "HRStartAddressChange：" << startAddress<< endl;
}

void ModbusServer::onCurrentConnectTypeChanged(int index)
{
    if (modbusDevice) {
        modbusDevice->disconnect();
        delete modbusDevice;
        modbusDevice = nullptr;
    }

    auto type = static_cast<ModbusConnection>(index);
    if (type == Serial) {
#if QT_CONFIG(modbus_serialport)
        modbusDevice = new QModbusRtuSerialSlave(this);
#endif
    } else if (type == Tcp) {
        modbusDevice = new QModbusTcpServer(this);
        if (ui->portEdit->text().isEmpty())
            ui->portEdit->setText(QLatin1String("127.0.0.1:502"));
    }
    ui->listenOnlyBox->setEnabled(type == Serial);

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
        if (type == Serial)
            statusBar()->showMessage(tr("Could not create Modbus slave."), 5000);
        else
            statusBar()->showMessage(tr("Could not create Modbus server."), 5000);
    } else {
        QModbusDataUnitMap reg;
        reg.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, 10 });
        reg.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, 10 });
        reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, 10 });
        reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, 10 });

        modbusDevice->setMap(reg);

        connect(modbusDevice, &QModbusServer::dataWritten,
                this, &ModbusServer::updateWidgets);
        connect(modbusDevice, &QModbusServer::stateChanged,
                this, &ModbusServer::onStateChanged);
        connect(modbusDevice, &QModbusServer::errorOccurred,
                this, &ModbusServer::handleDeviceError);

        connect(ui->listenOnlyBox, &QCheckBox::toggled, this, [this](bool toggled) {
            if (modbusDevice)
                modbusDevice->setValue(QModbusServer::ListenOnlyMode, toggled);
        });
        emit ui->listenOnlyBox->toggled(ui->listenOnlyBox->isChecked());
        connect(ui->setBusyBox, &QCheckBox::toggled, this, [this](bool toggled) {
            if (modbusDevice)
                modbusDevice->setValue(QModbusServer::DeviceBusy, toggled ? 0xffff : 0x0000);
        });
        emit ui->setBusyBox->toggled(ui->setBusyBox->isChecked());

        setupDeviceData();
    }
}

void ModbusServer::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError == QModbusDevice::NoError || !modbusDevice)
        return;

    statusBar()->showMessage(modbusDevice->errorString(), 5000);
}

void ModbusServer::onConnectButtonClicked()
{
    bool intendToConnect = (modbusDevice->state() == QModbusDevice::UnconnectedState);

    statusBar()->clearMessage();

    if (intendToConnect) {
        if (static_cast<ModbusConnection>(ui->connectType->currentIndex()) == Serial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                ui->portEdit->text());
#if QT_CONFIG(modbus_serialport)
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                m_settingsDialog->settings().parity);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                m_settingsDialog->settings().baud);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                m_settingsDialog->settings().dataBits);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                m_settingsDialog->settings().stopBits);
#endif
        } else {
            const QUrl url = QUrl::fromUserInput(ui->portEdit->text());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
        }
        modbusDevice->setServerAddress(ui->serverEdit->text().toInt());
        if (!modbusDevice->connectDevice()) {
            statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        } else {
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
        }
    } else {
        modbusDevice->disconnectDevice();
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
    }
}

void ModbusServer::onStateChanged(int state)
{
    bool connected = (state != QModbusDevice::UnconnectedState);
    ui->actionConnect->setEnabled(!connected);
    ui->actionDisconnect->setEnabled(connected);

    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

void ModbusServer::coilChanged(int id)
{
    QAbstractButton *button = coilButtons.button(id);
    bitChanged(id, QModbusDataUnit::Coils, button->isChecked());
}

void ModbusServer::discreteInputChanged(int id)
{
    QAbstractButton *button = discreteButtons.button(id);
    bitChanged(id, QModbusDataUnit::DiscreteInputs, button->isChecked());
}

void ModbusServer::bitChanged(int id, QModbusDataUnit::RegisterType table, bool value)
{
    if (!modbusDevice)
        return;

    if (!modbusDevice->setData(table, quint16(id), value))
        statusBar()->showMessage(tr("Could not set data: ") + modbusDevice->errorString(), 5000);
}

void ModbusServer::setRegister(const QString &value)
{
    if (!modbusDevice)
        return;

    const QString objectName = QObject::sender()->objectName();
    if (registers.contains(objectName)) {
        bool ok = true;
        const quint16 id = quint16(QObject::sender()->property("ID").toUInt());
        if (objectName.startsWith(QStringLiteral("inReg")))
            ok = modbusDevice->setData(QModbusDataUnit::InputRegisters, id, value.toUShort(&ok, 16));
        else if (objectName.startsWith(QStringLiteral("holdReg")))
            ok = modbusDevice->setData(QModbusDataUnit::HoldingRegisters, id, value.toUShort(&ok, 16));

        if (!ok)
            statusBar()->showMessage(tr("Could not set register: ") + modbusDevice->errorString(),
                                     5000);
    }
}

void ModbusServer::updateWidgets(QModbusDataUnit::RegisterType table, int address, int size)
{
    for (int i = 0; i < size; ++i) {
        quint16 value;
        QString text;
        switch (table) {
        case QModbusDataUnit::Coils:
            modbusDevice->data(QModbusDataUnit::Coils, quint16(address + i), &value);
            coilButtons.button(address + i)->setChecked(value);
            break;
        case QModbusDataUnit::HoldingRegisters:
            modbusDevice->data(QModbusDataUnit::HoldingRegisters, quint16(address + i), &value);
            registers.value(QStringLiteral("holdReg_%1").arg(address + i))->setText(text
                .setNum(value, 16));
            break;
        default:
            break;
        }
    }
}

// -- private

void ModbusServer::setupDeviceData()
{
    if (!modbusDevice)
        return;

    for (quint16 i = 0; i < coilButtons.buttons().count(); ++i)
        modbusDevice->setData(QModbusDataUnit::Coils, i, coilButtons.button(i)->isChecked());

    for (quint16 i = 0; i < discreteButtons.buttons().count(); ++i) {
        modbusDevice->setData(QModbusDataUnit::DiscreteInputs, i,
            discreteButtons.button(i)->isChecked());
    }

    bool ok;
    for (QLineEdit *widget : qAsConst(registers)) {
        if (widget->objectName().startsWith(QStringLiteral("inReg"))) {
            modbusDevice->setData(QModbusDataUnit::InputRegisters, quint16(widget->property("ID").toUInt()),
                widget->text().toUShort(&ok, 16));
        } else if (widget->objectName().startsWith(QStringLiteral("holdReg"))) {
            modbusDevice->setData(QModbusDataUnit::HoldingRegisters, quint16(widget->property("ID").toUInt()),
                widget->text().toUShort(&ok, 16));
        }
    }
}

void ModbusServer::setupWidgetContainers()
{
    coilButtons.setExclusive(false);
    discreteButtons.setExclusive(false);

    QRegularExpression regexp(QStringLiteral("coils_(?<ID>\\d+)"));
    const QList<QCheckBox *> coils = findChildren<QCheckBox *>(regexp);
    for (QCheckBox *cbx : coils)
        coilButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    connect(&coilButtons, SIGNAL(buttonClicked(int)), this, SLOT(coilChanged(int)));

    regexp.setPattern(QStringLiteral("disc_(?<ID>\\d+)"));
    const QList<QCheckBox *> discs = findChildren<QCheckBox *>(regexp);
    for (QCheckBox *cbx : discs)
        discreteButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    connect(&discreteButtons, SIGNAL(buttonClicked(int)), this, SLOT(discreteInputChanged(int)));

    regexp.setPattern(QLatin1String("(in|hold)Reg_(?<ID>\\d+)"));
    const QList<QLineEdit *> qle = findChildren<QLineEdit *>(regexp);
    for (QLineEdit *lineEdit : qle) {
        registers.insert(lineEdit->objectName(), lineEdit);
        lineEdit->setProperty("ID", regexp.match(lineEdit->objectName()).captured("ID").toInt());
        lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9a-f]{0,4}"),
            QRegularExpression::CaseInsensitiveOption), this));
        connect(lineEdit, &QLineEdit::textChanged, this, &ModbusServer::setRegister);
    }
}
