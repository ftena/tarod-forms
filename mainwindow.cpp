#include "mainwindow.h"
#include "addorderwindow.h"
#include "bookdelegate.h"
#include "initdb.h"
#include "tools.h"

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
    model_ = std::shared_ptr<QSqlRelationalTableModel>(new QSqlRelationalTableModel(ui.orderTable));
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
    ui.orderTable->setModel(model_.get());
    ui.orderTable->setItemDelegate(new BookDelegate(ui.orderTable));
    ui.orderTable->setColumnHidden(model_->fieldIndex("id"), true);
    ui.orderTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Initialize the supplier combo box with the model
    ui.supplierEdit->setModel(model_->relationModel(supplierIdx_));
    ui.supplierEdit->setModelColumn(model_->relationModel(supplierIdx_)->fieldIndex("name"));

    // Initialize the product combo box with the model
    ui.productEdit->setModel(model_->relationModel(productIdx_));
    ui.productEdit->setModelColumn(model_->relationModel(productIdx_)->fieldIndex("name"));

    QDataWidgetMapper *mapper = new QDataWidgetMapper(this);
    mapper->setModel(model_.get());
    mapper->setItemDelegate(new BookDelegate(this));
    mapper->addMapping(ui.orderEdit, model_->fieldIndex("name"));
    mapper->addMapping(ui.yearEdit, model_->fieldIndex("year"));
    mapper->addMapping(ui.supplierEdit, supplierIdx_);
    mapper->addMapping(ui.productEdit, productIdx_);
    mapper->addMapping(ui.ratingEdit, model_->fieldIndex("rating"));

    connect(ui.orderTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            mapper, SLOT(setCurrentModelIndex(QModelIndex)));

    ui.orderTable->setCurrentIndex(model_->index(0, 0));

    // Init the Add Order Window with the model and the view
    addOrderWindow_->init(model_, ui.orderTable);

    connect(ui.addOrderButton, &QPushButton::clicked,
            this, &MainWindow::addOrder);

    createMenuBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Forms"),
            tr("<p>Support - fran.tena@outlook.com</p>"
               "<p>Developed by <b>www.tarod.net</b></p>"
               "<p>0.0.1</p>"));
}

void MainWindow::addOrder()
{
    addOrderWindow_->show();
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
    QAction *productsAction = new QAction(tr("&Products..."), this);
    QAction *suppliersAction = new QAction(tr("&Suppliers..."), this);
    QAction *quitAction = new QAction(tr("&Exit"), this);
    QAction *aboutAction = new QAction(tr("&About"), this);

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
