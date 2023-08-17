#include "mvc_frame.h"

MVC_Frame::MVC_Frame(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    QHBoxLayout* lay = new QHBoxLayout(this);
    QPushButton* sortBtn = new QPushButton(tr("排序"));
    QPushButton* addBtn = new QPushButton(tr("添加"));

    //创建视图
    QTableView* tableView = new QTableView(this);
    tableView->setMinimumSize(400, 400);

    m_pModel = new FileChangeModel(this);
    QList<FileChange> exampleChanges;
    FileChange change1;
    change1.m_fileName = "file1.txt";
    change1.m_date = "2023-08-15";
    change1.m_time = "12:30 PM";
    change1.m_changeType = "Modified";
    change1.m_size = "256 KB";
    change1.m_describe = "Updated content";
    exampleChanges.append(change1);

    FileChange change2;
    change2.m_fileName = "file2.png";
    change2.m_date = "2023-08-14";
    change2.m_time = "03:45 PM";
    change2.m_changeType = "Added";
    change2.m_size = "1.2 MB";
    change2.m_describe = "New image added";
    exampleChanges.append(change2);

    m_pModel->setFileChanges(exampleChanges);

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_pModel);

    tableView->setModel(m_pModel);

    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(0);

    tableView->show();
    lay->addWidget(tableView);
    lay->addWidget(sortBtn);
    lay->addWidget(addBtn);

    connect(sortBtn, &QPushButton::clicked, [&]() {
        proxyModel->setSortRole(Qt::DisplayRole);
        proxyModel->sort(4, Qt::AscendingOrder);
    });

    connect(m_pModel, &FileChangeModel::dataChanged, tableView, &QTableView::reset);

    connect(addBtn, &QPushButton::clicked, [&]() {
        FileChange change3;
        change3.m_fileName = "file3.png";
        change3.m_date = "2023-08-15";
        change3.m_time = "03:30 PM";
        change3.m_changeType = "Delete";
        change3.m_size = "2.5 MB";
        change3.m_describe = "New image added";
        qDebug() << "FileChangeModel rowCount before addFileChange: " << m_pModel->rowCount();
        m_pModel->addFileChange(change3);

        qDebug() << "FileChangeModel rowCount after addFileChange: " << m_pModel->rowCount();

        //proxyModel->invalidate();
        });

    setLayout(lay);
}

MVC_Frame::~MVC_Frame()
{}
