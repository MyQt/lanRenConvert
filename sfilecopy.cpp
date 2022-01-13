#include "sfilecopy.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>


SFileCopy::SFileCopy(QObject *parent) : QObject(parent)
{
    m_createfile = new QDir();
}


SFileCopy::~SFileCopy()
{
    if(m_createfile) {
        m_createfile = Q_NULLPTR;
        delete m_createfile;
    }
}


bool SFileCopy::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
{
    toDir.replace("\\","/");
    if (sourceDir == toDir){
        return true;
    }
    if (!QFile::exists(sourceDir)){
        return false;
    }
    bool exist = m_createfile->exists(toDir);
    if (exist){
        if(coverFileIfExist){
            m_createfile->remove(toDir);
        }
    }


    if(!QFile::copy(sourceDir, toDir)) {
        return false;
    }
    return true;
}


bool SFileCopy::copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    qDebug() << "copyDirectoryFiles:" << fromDir << toDir;
    if(!targetDir.exists()){    /**< 如果目标目录不存在，则进行创建 */
        if(!targetDir.mkdir(targetDir.absolutePath())) {
            return false;
        }
    }
    QFileInfoList fileInfoList = sourceDir.entryInfoList();

    foreach(QFileInfo fileInfo, fileInfoList){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == ".." || fileInfo.fileName().contains(".cache") || fileInfo.fileName().contains(".user")) {
            continue;
        }
        if(fileInfo.isDir()){    /**< 当为目录时，递归的进行copy */
            if(!copyDirectoryFiles(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverFileIfExist)) {
                return false;
            }
        } else{            /**< 当允许覆盖操作时，将旧文件进行删除操作 */
            if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
                targetDir.remove(fileInfo.fileName());
            }
            /// 进行文件copy
            /// 新目录
            QStringList oldPath = fileInfo.filePath().split("/");
            QString folder = oldPath[oldPath.length()-2];
            QString dirTo = toDir+"/"+folder;
            QDir dirDest;
            if (!dirDest.exists(dirTo)) {
                dirDest.mkdir(dirTo);
            }
            if(!QFile::copy(fileInfo.filePath(), targetDir.filePath(dirTo+"/"+fileInfo.fileName()))){
                return false;
            }
        }
    }
    return true;
}
