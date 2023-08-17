#pragma once

#include <QtWidgets/QWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QSortFilterProxyModel>
#include "ui_mvc_frame.h"

#ifdef WIN32  
#pragma execution_character_set("utf-8")  
#endif

class FileChange
{
public:
    FileChange() : m_fileName(""), m_date(""), m_time(""), m_changeType(""), m_size(""), m_describe("") {}

    QString m_fileName;     // 文件名
    QString m_date;         // 文件改变日期
    QString m_time;         // 文件改变时间
    QString m_changeType;   // 文件改变类型
    QString m_size;         // 文件大小
    QString m_describe;     // 文件改变描述
};

class FileChangeModel : public QStandardItemModel
{
    Q_OBJECT
public:
    FileChangeModel(QObject* parent = nullptr) : QStandardItemModel(parent)
    {
        setColumnCount(6);
        setHorizontalHeaderLabels({ tr("文件名"), tr("变化日期"), tr("变化时间"),
                                    tr("变化类型"), tr("文件大小"), tr("变化描述") });
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::DisplayRole) {
            const FileChange& fileChange = m_fileChanges[index.row()];
            switch (index.column()) {
            case 0: return fileChange.m_fileName;
            case 1: return fileChange.m_date;
            case 2: return fileChange.m_time;
            case 3: return fileChange.m_changeType;
            case 4: return fileChange.m_size;
            case 5: return fileChange.m_describe;
            default: break;
            }
        }
        return QVariant();
    }

    void setFileChanges(const QList<FileChange>& fileChanges)
    {
        m_fileChanges = fileChanges;
        setRowCount(fileChanges.size());
    }

    void addFileChange(const FileChange& fileChange)
    {
        // 在末尾添加新的文件变化
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        qDebug() << "FileChangeModel rowCount before addFileChange: " << m_fileChanges.size();
        m_fileChanges.append(fileChange);
        qDebug() << "文件变化信息: "
            << fileChange.m_fileName << " "
            << fileChange.m_date << " "
            << fileChange.m_time << " "
            << fileChange.m_changeType << " "
            << fileChange.m_size << " "
            << fileChange.m_describe;
        qDebug() << "FileChangeModel rowCount after addFileChange: " << m_fileChanges.size();
        endInsertRows();
    }

private:
    QList<FileChange> m_fileChanges;
};

class MVC_Frame : public QWidget
{
    Q_OBJECT

public:
    MVC_Frame(QWidget *parent = nullptr);
    ~MVC_Frame();

private:
    Ui::MVC_FrameClass ui;

    FileChangeModel* m_pModel;
    QSortFilterProxyModel* proxyModel;
};
