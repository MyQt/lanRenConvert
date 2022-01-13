#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sfilecopy.h"
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QByteArray>
#include <QModelIndex>
#include <QStandardItemModel>
#include  <QDebug>
#include <QToolTip>
#include <QPoint>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    mFolderList.clear();
    mFileList.clear();
    mItemList.clear();
}

void MainWindow::traversalSource(QString strSource)
{
    if (strSource.endsWith(".") ||  strSource.contains(".cache"))
        return;
    if (strSource.split("/").last().startsWith(".")) {
        mFolderList.push_front(strSource);
    }
    QDir dir(strSource);
    QFileInfoList fileList = dir.entryInfoList();
    foreach(QFileInfo fileInfo, fileList) {
        QString name = fileInfo.fileName();
        QString path = fileInfo.filePath();
        QString suffix = fileInfo.suffix();
        if (name == ".cache" || name== "..") { // 忽略无效目录或文件
            continue;
        }
        else {
            if (fileInfo.isDir()) { // 遍历子目录
                if (name[0] != '.') { // 忽略掉非音频目录
                    continue;
                }
                traversalSource(path); // 递归调用遍历
            } else { //文件忽略掉非音频文件
                if (suffix == "db") {

                    continue;
                }
                mFileList.append(path);
            }
        }
    }
}

void MainWindow::on_btn_choice_load_clicked()
{
    QString filePath = QFileDialog::getExistingDirectory(this,"选择源音频目录");
    if (filePath.isEmpty()) {
        QMessageBox::information(this, "警告", "请选择合法源音频目录");
        return;
    }
    init();
    ui->label_dir_load->setText(filePath);
    traversalSource(filePath);

    mItemList = mFileList;
    mItemList.append(mFolderList);

    mItemModel.setStringList(mItemList);
    ui->list_file->setModel(&mItemModel);
    if (mItemList.length() > 0) {
        ui->btn_go->setDisabled(false);
        ui->btn_go->setText("点击转换");

        ui->progress_convert->setMinimum(0);
        ui->progress_convert->setValue(0);
        ui->progress_convert->setMaximum(mItemList.length());

    } else {
        ui->btn_go->setDisabled(true);
        ui->btn_go->setText("没有转换文件");
    }

}

void MainWindow::on_btn_go_clicked()
{
    QDir dir;
    if(!dir.exists(ui->label_dir_load->text()) || !dir.exists(ui->label_dir_out->text())) {
        QMessageBox::information(this, "警告","请先选择加载和输出目录");
        return;
    }
    if (mItemList.length() <= 0) {
        QString strToolInfo = "没有可以转换的文件";
        QToolTip::showText(QPoint(this->pos().x()-this->fontMetrics().horizontalAdvance(strToolInfo)/2+this->width()/2, this->pos().y()+this->height()/2-this->fontMetrics().height()/2), strToolInfo);
    }
    int index = 0;
    foreach(QString item, mItemList) {
        QStringList itemSplitList = item.split("/");
        QString strSplitName = itemSplitList.last();
        //涉及网络url传输,其中的+和/会被转义成_和-
        QString beforeName = strSplitName;
        QByteArray byteName = beforeName.toUtf8();
        byteName = QByteArray::fromBase64(byteName, QByteArray::Base64UrlEncoding); // url路径要用到这个参数
        QString strAfterName = QString(byteName).toUtf8(); // 转换完成

        QString strAfterItem = item.left(item.length()-strSplitName.length()) + strAfterName;
        // 重命名
        QFileInfo fileInfo(item);
        if (fileInfo.isDir()) {
            QDir dir;
            bool bSuccess = dir.rename(item, strAfterItem);
            qDebug()<<QString(bSuccess);
        } else if(fileInfo.isFile()) {
            bool bSuccess = QFile::rename(item, strAfterItem+".mp3");
            qDebug()<<QString(bSuccess);
        }
        ui->list_file->setCurrentIndex(QModelIndex(mItemModel.index(index, 0)));
        // 更新UI
        ui->progress_convert->setValue(ui->progress_convert->value()+1);
        QModelIndex modelIndex = ui->list_file->currentIndex();
        mItemModel.setData(modelIndex, strAfterItem, Qt::EditRole);
        index++;
        QApplication::processEvents();
    }
    // 转换完成，复制文件

    copyToDest();
    // 删除源目录
    QModelIndex modelIndex = ui->list_file->currentIndex();
    QString strRoot = mItemModel.data(modelIndex).toString();
    clearDir(strRoot);
}

void MainWindow::on_btn_choice_out_clicked()
{
    QString filePath = QFileDialog::getExistingDirectory(this,"选择音频输出目录");
    if (filePath.isEmpty()) {
        QMessageBox::information(this, "警告", "请选择合法音频输出目录");
        return;
    }
    ui->label_dir_out->setText(filePath);
}

void MainWindow::copyToDest()
{
    QModelIndex modelIndex = ui->list_file->currentIndex();
    QString strLast = mItemModel.data(modelIndex).toString();
    SFileCopy copy;
    copy.copyDirectoryFiles(strLast, ui->label_dir_out->text(), true);
}

void MainWindow::clearDir(QString path)
{
    if(path.isEmpty())
    {
        return;
    }
    QDir dir(path);

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    foreach(QFileInfo fileInfo, dir.entryInfoList())
    {
        if(fileInfo.isFile())
        {
            if(!fileInfo.isWritable())
            {
                QFile file(fileInfo.absoluteFilePath());
                file.setPermissions(QFile::WriteOwner);
            }

            fileInfo.dir().remove(fileInfo.fileName());

        }
        else if(fileInfo.isDir())
        {
            clearDir(fileInfo.absoluteFilePath());
        }
    }
    dir.rmpath(dir.absolutePath());
}
