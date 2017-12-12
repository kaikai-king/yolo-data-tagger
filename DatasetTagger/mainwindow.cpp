#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfoList>
#include <QDir>
#include <QColor>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mLabelModel(NULL),
    mImgListModel(NULL),
    mScene(NULL)
{
    ui->setupUi(this);

    mAutoLabNameCount = 0;

    // >>>>> Image Rendering
    mScene = new QGraphicsScene();
    ui->graphicsView_image->setScene(mScene);
    mImgItem = new QGraphicsPixmapItem();

    mScene->addItem( mImgItem );
    // <<<<< Image Rendering

    // >>>>> Label table
    mLabelModel = new QStandardItemModel( 0, 3 );
    mLabelModel->setHorizontalHeaderItem(0, new QStandardItem(QString("Idx")));
    mLabelModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Name")));
    mLabelModel->setHorizontalHeaderItem(2, new QStandardItem(QString("Color")));

    ui->tableView_labels->setModel(mLabelModel);
    // <<<<< Label table
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onImageListCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    QString imageName = mImgListModel->data( current ).toString();

    QString currImagePath = mDataSet[imageName]->getFullPath();
    if( !currImagePath.endsWith( "/") )
        currImagePath += "/";
    currImagePath += imageName;

    QImage image(currImagePath);

    if( image.isNull() )
        return;

    mImgItem->setPixmap( QPixmap::fromImage(image) );
    mImgItem->setPos( 0,0 );
    mImgItem->setZValue( -10.0 );

    ui->graphicsView_image->setSceneRect( 0,0, image.width(), image.height() );


    ui->graphicsView_image->fitInView(QRectF(0,0, image.width(), image.height()),
                                      Qt::KeepAspectRatio );

    ui->graphicsView_image->update();
}

void MainWindow::on_pushButton_base_folder_clicked()
{
    mBaseFolder = QFileDialog::getExistingDirectory( this, tr("Base folder for relative paths"),
                  QDir::homePath() );

    if( mBaseFolder.isEmpty() )
    {
        ui->pushButton_img_folder->setEnabled(false);
        return;
    }

    ui->lineEdit_base_folder_path->setText(mBaseFolder);

    ui->pushButton_img_folder->setEnabled(true);
}

void MainWindow::on_pushButton_img_folder_clicked()
{
    mImgFolder = QFileDialog::getExistingDirectory( this, tr("Image folder"), mBaseFolder );

    if( mImgFolder.isEmpty() )
    {
        return;
    }

    if( mImgListModel )
    {
        QItemSelectionModel *imgSelModel = ui->listView_images->selectionModel();
        disconnect(imgSelModel,&QItemSelectionModel::currentChanged,
                   this, &MainWindow::onImageListCurrentChanged );

        delete mImgListModel;
        mImgListModel = NULL;
    }

    ui->listView_images->setViewMode( QListView::ListMode );

    ui->lineEdit_img_folder_path->setText( mImgFolder );

    QDir imgDir( mImgFolder );

    QStringList fileTypes;
    fileTypes << "*.jpg" << "*.jpeg" << "*.png";
    imgDir.setNameFilters(fileTypes);

    QStringList imgList = imgDir.entryList( fileTypes, QDir::Files|QDir::NoSymLinks, QDir::Name|QDir::Name );

    mImgListModel = new QStringListModel(imgList);
    ui->listView_images->setModel( mImgListModel );

    QItemSelectionModel *imgSelModel = ui->listView_images->selectionModel();
    connect(imgSelModel,&QItemSelectionModel::currentChanged,
            this, &MainWindow::onImageListCurrentChanged );

    ui->listView_images->setCurrentIndex( QModelIndex());

    // >>>>> Dataset initialization
    mDataSet.clear();

    foreach(QString imgFile, imgList)
    {
        QTrainSetExample* ts = new QTrainSetExample(imgFile);

        ts->setRelFolderPath( mImgFolder, mBaseFolder );

        mDataSet[imgFile] = ts;
    }
    // <<<<< Dataset initialization
}


void MainWindow::on_pushButton_add_label_clicked()
{
    int idx = mLabelModel->rowCount();

    QStandardItem* idxItem = new QStandardItem();
    idxItem->setText( tr("%1").arg(idx,3,10,QChar('0')) );
    idxItem->setEditable( false );
    idxItem->setTextAlignment( Qt::AlignHCenter|Qt::AlignVCenter );

    QStandardItem* labelItem = new QStandardItem();
    QString label = ui->lineEdit_label->text();
    if( label.isEmpty() )
    {
        label = tr("Label_%1").arg(mAutoLabNameCount++,3,10,QChar('0'));
    }
    labelItem->setText( label );
    labelItem->setEditable( true );

    QStandardItem* colorItem = new QStandardItem();

    quint8 r = (idx+9)*30;
    quint8 g = (idx+6)*60;
    quint8 b = (idx+3)*90;

    colorItem->setText(  tr("%1,%2,%3").arg(r,3,10,QChar('0')).arg(g,3,10,QChar('0')).arg(b,3,10,QChar('0')) );
    colorItem->setEditable( false );
    colorItem->setBackground( QBrush( QColor(r,g,b)) );
    colorItem->setForeground( QBrush( QColor(255-r,255-g,255-b)) );
    colorItem->setSelectable(false);

    QList<QStandardItem*> row;
    row << idxItem << labelItem << colorItem;
    mLabelModel->appendRow( row );
}

void MainWindow::on_pushButton_remove_label_clicked()
{
    int idx = ui->tableView_labels->currentIndex().row();

    mLabelModel->removeRow( idx  );

    for( int i=idx; i<mLabelModel->rowCount(); i++ )
    {
        QStandardItem* idxItem = mLabelModel->item( i, 0 );
        idxItem->setText( tr("%1").arg(i,3,10,QChar('0')) );
    }

}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{

}

