#include "mainwindow.h"
#include "addorderwindow.h"
#include "bookdelegate.h"
#include "initdb.h"

#include <QtSql>

MainWindow::MainWindow(): addOrderWindow_(new AddOrderWindow(this))
{
    ui.setupUi(this);

    /*
     * If we don't set the flag Qt::Window but we set the MainWindow
     * as parent of AddOrderWindow (see below), AddOrderWindow becomes a child
     * of the main window and would be added inside the main window
     * instead of a separate window.
     */
    addOrderWindow_->setWindowFlags(Qt::Window);

    if (!QSqlDatabase::drivers().contains("QPSQL"))
        QMessageBox::critical(this, "Unable to load database", "QPSQL driver not found");

    // initialize the database
    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
        showError(err);
        return;
    }

    // Subscribe to the notification "dbupdated" (created in postgresql)
    QSqlDatabase::database().driver()->subscribeToNotification("dbupdated");

    connect(QSqlDatabase::database().driver(), SIGNAL(notification(const QString&)),
            this, SLOT(notificationHandler(const QString&)));

    // Create the data model
    model_ = std::shared_ptr<QSqlRelationalTableModel>(new QSqlRelationalTableModel(ui.bookTable));
    //TODO: confirm! model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model_->setEditStrategy(QSqlTableModel::OnFieldChange);
    model_->setTable("orders");

    // Remember the indexes of the columns
    supplierIdx_ = model_->fieldIndex("supplier");
    productIdx_ = model_->fieldIndex("product");

    // Set the relations to the other database tables
    /*
     * The QSqlRelation class stores information about an
     * SQL foreign key.
     */
    model_->setRelation(supplierIdx_, QSqlRelation("suppliers", "id", "name"));
    model_->setRelation(productIdx_, QSqlRelation("products", "id", "name"));

    // Set the localized header captions
    model_->setHeaderData(supplierIdx_, Qt::Horizontal, tr("Supplier"));
    model_->setHeaderData(productIdx_, Qt::Horizontal, tr("Product"));
    model_->setHeaderData(model_->fieldIndex("name"), Qt::Horizontal, tr("Name"));
    model_->setHeaderData(model_->fieldIndex("year"), Qt::Horizontal, tr("Year"));
    model_->setHeaderData(model_->fieldIndex("rating"), Qt::Horizontal, tr("Rating"));

    // Populate the model
    if (!model_->select()) {
        showError(model_->lastError());
        return;
    }

    // Set the model and hide the ID column
    ui.bookTable->setModel(model_.get());
    ui.bookTable->setItemDelegate(new BookDelegate(ui.bookTable));
    ui.bookTable->setColumnHidden(model_->fieldIndex("id"), true);
    ui.bookTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Initialize the supplier combo box with the model
    ui.supplierEdit->setModel(model_->relationModel(supplierIdx_));
    ui.supplierEdit->setModelColumn(model_->relationModel(supplierIdx_)->fieldIndex("name"));

    // Initialize the product combo box with the model
    ui.productEdit->setModel(model_->relationModel(productIdx_));
    ui.productEdit->setModelColumn(model_->relationModel(productIdx_)->fieldIndex("name"));

    QDataWidgetMapper *mapper = new QDataWidgetMapper(this);
    mapper->setModel(model_.get());
    mapper->setItemDelegate(new BookDelegate(this));
    mapper->addMapping(ui.titleEdit, model_->fieldIndex("name"));
    mapper->addMapping(ui.yearEdit, model_->fieldIndex("year"));
    mapper->addMapping(ui.supplierEdit, supplierIdx_);
    mapper->addMapping(ui.productEdit, productIdx_);
    mapper->addMapping(ui.ratingEdit, model_->fieldIndex("rating"));

    connect(ui.bookTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            mapper, SLOT(setCurrentModelIndex(QModelIndex)));

    ui.bookTable->setCurrentIndex(model_->index(0, 0));

    // Init the Add Order Window with the model
    addOrderWindow_->init(model_);

    connect(ui.addOrderButton, &QPushButton::clicked,
            this, &MainWindow::addOrder);

    createMenuBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::about()
{
    QMessageBox::about(this, tr("Acerca de Forms"),
            tr("<p>Soporte - fran.tena@outlook.com</p>"
               "<p>Creado por <b>www.tarod.net</b></p>"
               "<p>Versión 0.0.1</p>"));
}

void MainWindow::addOrder()
{
    addOrderWindow_->show();

    QSqlRecord record = model_->record();

    /*
     * id serial, name varchar, supplier integer, product integer, year integer, rating integer
     */
    QSqlField f0("id", QVariant::Int);
    QSqlField f1("name", QVariant::String);
    QSqlField f2("supplier", QVariant::Int);
    QSqlField f3("product", QVariant::Int);
    QSqlField f4("year", QVariant::Int);
    QSqlField f5("rating", QVariant::Int);

    // get next value for the id sequence
    QSqlQuery q;
    if(!q.exec("SELECT nextval(pg_get_serial_sequence('orders', 'id'))"))
    {
        showError(q.lastError());
        return;
    } else {
        if (q.next()) {
            int lastId = q.value(0).toInt();
            f0.setValue(lastId);
        }
        else {
            showError(q.lastError());
            return;
        }
    }

    f1.setValue(QVariant("Test1"));
    f2.setValue(QVariant(3));
    f3.setValue(QVariant(3));
    f4.setValue(QVariant(2001));
    f5.setValue(QVariant(5));

    record.append(f0);
    record.append(f1);
    record.append(f2);
    record.append(f3);
    record.append(f4);
    record.append(f5);

    if (model_->insertRecord(-1, record))
    {
        qDebug() << Q_FUNC_INFO << " OK ";
    } else {
        qDebug() << Q_FUNC_INFO << " NO OK " << model_->lastError().text();
    }

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

    emit model_->dataChanged(QModelIndex(), QModelIndex());
}

void MainWindow::createMenuBar()
{
    QAction *productsAction = new QAction(tr("&Productos..."), this);
    QAction *suppliersAction = new QAction(tr("&Proveedores..."), this);
    QAction *quitAction = new QAction(tr("&Salir"), this);
    QAction *aboutAction = new QAction(tr("&Acerca de"), this);

    productsAction->setShortcut(tr("Ctrl+A"));
    suppliersAction->setShortcut(tr("Ctrl+D"));
    quitAction->setShortcuts(QKeySequence::Quit);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(productsAction);
    fileMenu->addAction(suppliersAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Ayuda"));
    helpMenu->addAction(aboutAction);    

    connect(productsAction, SIGNAL(triggered(bool)), this, SLOT(addAlbum()));
    connect(suppliersAction, SIGNAL(triggered(bool)), this, SLOT(deleteAlbum()));
    connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(about()));
}

void MainWindow::showError(const QSqlError &err)
{
    QMessageBox::critical(this, "Unable to initialize Database",
                "Error initializing database: " + err.text());
}

