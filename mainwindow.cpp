#include "mainwindow.h"
#include "bookdelegate.h"
#include "initdb.h"

#include <QtSql>

MainWindow::MainWindow()
{
    ui.setupUi(this);

    if (!QSqlDatabase::drivers().contains("QPSQL"))
        QMessageBox::critical(this, "Unable to load database", "QPSQL driver not found");

    // initialize the database
    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
        showError(err);
        return;
    }

    // Subscribe to the notification "dbupdated"
    QSqlDatabase::database().driver()->subscribeToNotification("dbupdated");

    connect(QSqlDatabase::database().driver(), SIGNAL(notification(const QString&)),
            this, SLOT(notificationHandler(const QString&)));

    // Create the data model
    model = new QSqlRelationalTableModel(ui.bookTable);
    //TODO: confirm! model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->setTable("orders");

    // Remember the indexes of the columns
    supplierIdx = model->fieldIndex("supplier");
    productIdx = model->fieldIndex("product");

    // Set the relations to the other database tables
    /*
     * The QSqlRelation class stores information about an
     * SQL foreign key.
     */
    model->setRelation(supplierIdx, QSqlRelation("suppliers", "id", "name"));
    model->setRelation(productIdx, QSqlRelation("products", "id", "name"));

    // Set the localized header captions
    model->setHeaderData(supplierIdx, Qt::Horizontal, tr("Supplier"));
    model->setHeaderData(productIdx, Qt::Horizontal, tr("Product"));
    model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Name"));
    model->setHeaderData(model->fieldIndex("year"), Qt::Horizontal, tr("Year"));
    model->setHeaderData(model->fieldIndex("rating"), Qt::Horizontal, tr("Rating"));

    // Populate the model
    if (!model->select()) {
        showError(model->lastError());
        return;
    }

    // Set the model and hide the ID column
    ui.bookTable->setModel(model);
    ui.bookTable->setItemDelegate(new BookDelegate(ui.bookTable));
    ui.bookTable->setColumnHidden(model->fieldIndex("id"), true);
    ui.bookTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Initialize the Author combo box
    ui.supplierEdit->setModel(model->relationModel(supplierIdx));
    ui.supplierEdit->setModelColumn(model->relationModel(supplierIdx)->fieldIndex("name"));

    ui.productEdit->setModel(model->relationModel(productIdx));
    ui.productEdit->setModelColumn(model->relationModel(productIdx)->fieldIndex("name"));

    QDataWidgetMapper *mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
    mapper->setItemDelegate(new BookDelegate(this));
    mapper->addMapping(ui.titleEdit, model->fieldIndex("name"));
    mapper->addMapping(ui.yearEdit, model->fieldIndex("year"));
    mapper->addMapping(ui.supplierEdit, supplierIdx);
    mapper->addMapping(ui.productEdit, productIdx);
    mapper->addMapping(ui.ratingEdit, model->fieldIndex("rating"));

    connect(ui.bookTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            mapper, SLOT(setCurrentModelIndex(QModelIndex)));

    ui.bookTable->setCurrentIndex(model->index(0, 0));

    createMenuBar();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("Acerca de Forms"),
            tr("<p>Soporte - fran.tena@outlook.com</p>"
               "<p>Creado por <b>www.tarod.net</b></p>"
               "<p>Versión 0.0.1</p>"));
}

void MainWindow::addAlbum()
{
    /* TODO
    Dialog *dialog = new Dialog(model, albumData, file, this);
    int accepted = dialog->exec();

    if (accepted == 1) {
        int lastRow = model->rowCount() - 1;
        albumView->selectRow(lastRow);
        albumView->scrollToBottom();
        showAlbumDetails(model->index(lastRow, 0));
    }
    */
}

void MainWindow::notificationHandler(const QString &name)
{    
    qDebug() << Q_FUNC_INFO;

    // Populates the model with data from the table that was set via setTable().
    // TODO: valid way? emit model->select();

    emit model->dataChanged(QModelIndex(), QModelIndex());
}

void MainWindow::createMenuBar()
{
    QAction *addAction = new QAction(tr("&Productos..."), this);
    QAction *deleteAction = new QAction(tr("&Proveedores..."), this);
    QAction *quitAction = new QAction(tr("&Salir"), this);
    QAction *aboutAction = new QAction(tr("&Acerca de"), this);

    addAction->setShortcut(tr("Ctrl+A"));
    deleteAction->setShortcut(tr("Ctrl+D"));
    quitAction->setShortcuts(QKeySequence::Quit);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(addAction);
    fileMenu->addAction(deleteAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Ayuda"));
    helpMenu->addAction(aboutAction);    

    connect(addAction, SIGNAL(triggered(bool)), this, SLOT(addAlbum()));
    connect(deleteAction, SIGNAL(triggered(bool)), this, SLOT(deleteAlbum()));
    connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(about()));
}

void MainWindow::showError(const QSqlError &err)
{
    QMessageBox::critical(this, "Unable to initialize Database",
                "Error initializing database: " + err.text());
}

