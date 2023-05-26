/**
* @filename main.cpp
* @brief    ResourceMonitor主程序入口，包括命令行参数解析处理
* @author   wanghp
* @date     2023-05-12
*/

#include "resource_monitor.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFontDatabase>
#include <QMessageBox>

void initApplication()
{
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);
    QCoreApplication::setOrganizationName("winring");
    QCoreApplication::setOrganizationDomain("winring.com.cn");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //! 设置样式表
    QFile qssFile(":/pure.qss");
    if( qssFile.open(QFile::ReadOnly))
    {
        QString style = QString(qssFile.readAll());
        a.setStyleSheet(style);
        qssFile.close();
    }

    //! 设置默认字体
    QFont defaultFont = a.font();
    defaultFont.setFamily(QString::fromLocal8Bit("微软雅黑"));
    defaultFont.setPixelSize(12);
    defaultFont.setBold(false);
    a.setFont(defaultFont);

    qRegisterMetaType<ICDInfo>("ICDInfo");
    initApplication();
    //! [start]---------initCommandLine-----------
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription(QStringLiteral("命令行帮助说明"));
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();

    QCommandLineOption fullscreenOption(QStringList() << "m" << "Maximized", "showMaximized");
    cmdParser.addOption(fullscreenOption);

    cmdParser.addPositionalArgument(QStringLiteral("1. iwrp文件路径"), QStringLiteral("请填写iwrp2文件所在完整路径"));
    cmdParser.addPositionalArgument(QStringLiteral("2. ICD文件路径"),  QStringLiteral("请填写ICD文件所在完整路径"));
    cmdParser.addPositionalArgument(QStringLiteral("3. XCD文件路径"),  QStringLiteral("请填写XCD文件所在完整路径"));

    cmdParser.process(a);

    QStringList argvs;
#ifndef _DEBUG
    if (cmdParser.positionalArguments().size() != 3)
    {
        QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("传入参数不正确！"));
        return 0;
    }
#endif
    argvs = cmdParser.positionalArguments();
#ifdef _DEBUG
    if(argvs.size() == 0)
    {
        argvs.append(QStringLiteral("C:/Users/Administrator/Desktop/testA1/testA.iwrp2"));
    //    argvs.append(QStringLiteral("D:/123/wrp_pro/PS-0512项目校验.iwrp2"));
        argvs.append("D:/123/wrp_pro/WS/metaData");
        argvs.append("D:/123/wrp_pro/xcd");
    }
#endif
    ResourceMonitor w(argvs.at(0), argvs.at(1), argvs.at(2));
    if(cmdParser.isSet("m")) {
        w.showMaximized();
    } else {
        w.show();
    }
    //! [end]---------initCommandLine-----------

    return a.exec();
}
