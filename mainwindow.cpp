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

    // Subscribe to the notification "dbupdated" (created in postgresql -- see initdb.h)
    QSqlDatabase::database().driver()->subscribeToNotification("dbupdated");

    connect(QSqlDatabase::database().driver(), SIGNAL(notification(const QString&)),
            this, SLOT(notificationHandler(const QString&)));

    // Create the data model for orders table
    orderModel_ = std::shared_ptr<QSqlRelationalTableModel>(new QSqlRelationalTableModel(ui.orderTable));
    //TODO: confirm! orderModel_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    orderModel_->setEditStrategy(QSqlTableModel::OnFieldChange);
    orderModel_->setTable("orders");

    // Remember the indexes of the columns
    orderIdx_ = orderModel_->fieldIndex("id");
    supplierIdx_ = orderModel_->fieldIndex("supplier");
    productIdx_ = orderModel_->fieldIndex("product");

    // Set the relations to the other database tables
    /*
     * The QSqlRelation class stores information about an
     * SQL foreign key.
     */
    orderModel_->setRelation(supplierIdx_, QSqlRelation("suppliers", "id", "name"));
    orderModel_->setRelation(productIdx_, QSqlRelation("products", "id", "name"));

    // Set the localized header captions
    orderModel_->setHeaderData(supplierIdx_, Qt::Horizontal, tr("Supplier"));
    orderModel_->setHeaderData(productIdx_, Qt::Horizontal, tr("Product"));
    orderModel_->setHeaderData(orderModel_->fieldIndex("name"), Qt::Horizontal, tr("Name"));
    orderModel_->setHeaderData(orderModel_->fieldIndex("year"), Qt::Horizontal, tr("Year"));
    orderModel_->setHeaderData(orderModel_->fieldIndex("rating"), Qt::Horizontal, tr("Rating"));

    // Populate the model
    if (!orderModel_->select()) {
        showError(orderModel_->lastError());
        return;
    }

    // Set the model and hide the ID column
    ui.orderTable->setModel(orderModel_.get());
    ui.orderTable->setItemDelegate(new BookDelegate(ui.orderTable));
    ui.orderTable->setColumnHidden(orderModel_->fieldIndex("id"), true);
    ui.orderTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Initialize the supplier combo box with the model
    ui.supplierEdit->setModel(orderModel_->relationModel(supplierIdx_));
    ui.supplierEdit->setModelColumn(orderModel_->relationModel(supplierIdx_)->fieldIndex("name"));

    // Initialize the product combo box with the model
    ui.productEdit->setModel(orderModel_->relationModel(productIdx_));
    ui.productEdit->setModelColumn(orderModel_->relationModel(productIdx_)->fieldIndex("name"));

    QDataWidgetMapper *mapper = new QDataWidgetMapper(this);
    mapper->setModel(orderModel_.get());
    mapper->setItemDelegate(new BookDelegate(this));
    mapper->addMapping(ui.orderEdit, orderModel_->fieldIndex("name"));
    mapper->addMapping(ui.yearEdit, orderModel_->fieldIndex("year"));
    mapper->addMapping(ui.supplierEdit, supplierIdx_);
    mapper->addMapping(ui.productEdit, productIdx_);
    mapper->addMapping(ui.ratingEdit, orderModel_->fieldIndex("rating"));

    connect(ui.orderTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            mapper, SLOT(setCurrentModelIndex(QModelIndex)));

    ui.orderTable->setCurrentIndex(orderModel_->index(0, 0));

    // Initialize the products view with the model
    initProductsView();

    // Init the Add Order Window with the model and the view
    addOrderWindow_->init(orderModel_, ui.orderTable);

    connect(ui.orderTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(showOrderItemsDetails(QModelIndex)));

    connect(ui.addOrderButton, &QPushButton::clicked,
            this, &MainWindow::addOrder);

    createMenuBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initProductsView()
{
    // Create the data model for order_items table
    orderItemsModel_ = std::shared_ptr<QSqlRelationalTableModel>(new QSqlRelationalTableModel(ui.productsView));
    orderItemsModel_->setTable("order_items");

    // Remember the indexes of the columns
    ordersIdx_ = orderItemsModel_->fieldIndex("order_id");
    productsIdx_ = orderItemsModel_->fieldIndex("product_id");

    // Set the relations to the other database tables.
    // The next colums are foreign keys and the view
    // should present the name fields instead of the ids
    // (order_id and product_id)
    /*
     * The QSqlRelation class stores information about an
     * SQL foreign key.
     */
    orderItemsModel_->setRelation(ordersIdx_, QSqlRelation("orders", "id", "name"));
    orderItemsModel_->setRelation(productsIdx_, QSqlRelation("products", "id", "name"));

    // Populate the model
    if (!orderItemsModel_->select()) {
        showError(orderItemsModel_->lastError());
        return;
    }

    // Set the model
    ui.productsView->setModel(orderItemsModel_.get());

    // Filter the model to show only the order id selected in orders table
    int row = ui.orderTable->currentIndex().row();
    QModelIndex index = ui.orderTable->model()->index(row,
                                                      0); // 0 = the id in orders table
    orderItemsModel_->setFilter("order_id = '" + index.data().toString() + '\'') ;
}

void MainWindow::showOrderItemsDetails(const QModelIndex &index)
{
    // Filter the model to show only the order id selected in orders table
    int row = index.row();
    QModelIndex orderIndex = ui.orderTable->model()->index(row,
                                                             0); // 0 = the id in orders table
    orderItemsModel_->setFilter("order_id = '" + orderIndex.data().toString() + '\'') ;
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
    // TODO: valid way the next line of code?

    // emit model->select();

    emit orderModel_->dataChanged(QModelIndex(), QModelIndex());
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
