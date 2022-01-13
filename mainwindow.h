#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void traversalSource(QString strSource);
    void init();
    void copyToDest();
    void clearDir(QString path); // 清空目录
private slots:
    void on_btn_choice_load_clicked();

    void on_btn_go_clicked();

    void on_btn_choice_out_clicked();

private:
    Ui::MainWindow *ui;
    QStringListModel mItemModel;
    QStringList mFolderList;
    QStringList mFileList;
    QStringList mItemList;


};
#endif // MAINWINDOW_H
