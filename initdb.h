/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef INITDB_H
#define INITDB_H

#include <QtSql>

QVariant addOrder(QSqlQuery &q, const QString &name, int year, const QVariant &supplierId,
             const QVariant &productId, int rating)
{
    q.addBindValue(name);
    q.addBindValue(year);
    q.addBindValue(supplierId);
    q.addBindValue(productId);
    q.addBindValue(rating);
    q.exec();
    return q.lastInsertId();
}

QVariant addProduct(QSqlQuery &q, const QString &name, double price)
{
    q.addBindValue(name);
    q.addBindValue(price);
    q.exec();
    return q.lastInsertId();
}

QVariant addSupplier(QSqlQuery &q, const QString &name, const QDate &created)
{
    q.addBindValue(name);
    q.addBindValue(created);
    q.exec();
    return q.lastInsertId();
}

void addOrderItem(QSqlQuery &q, const QVariant &productId, const QVariant &orderId, int quantity)
{
    q.addBindValue(productId);
    q.addBindValue(orderId);
    q.addBindValue(quantity);
    q.exec();
}

QSqlError initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("tarod");
    db.setUserName("tarod");
    db.setPassword("tarod");

    if (!db.open())
        return db.lastError();

    /* TESTING */
    // TODO: remove after testing    
    QSqlQuery qTesting;
    if (!qTesting.exec(QLatin1String("DROP TABLE IF EXISTS orders, suppliers, products, order_items")))
        return qTesting.lastError();


    QStringList tables = db.tables();
    if (tables.contains("orders", Qt::CaseInsensitive)
        && tables.contains("suppliers", Qt::CaseInsensitive))
        return QSqlError();

    QSqlQuery q;
    if (!q.exec(QLatin1String("CREATE TABLE orders(id SERIAL PRIMARY KEY, name varchar, supplier integer, product integer, year integer, rating integer)")))
        return q.lastError();
    if (!q.exec(QLatin1String("CREATE TABLE suppliers(id SERIAL PRIMARY KEY, name varchar, created date)")))
        return q.lastError();
    if (!q.exec(QLatin1String("CREATE TABLE products(id SERIAL PRIMARY KEY, name varchar, price numeric)")))
        return q.lastError();
    // now, the many-to-many relationships between tables orders and products
    if (!q.exec(QLatin1String("CREATE TABLE order_items("
                              "product_id integer REFERENCES products,"
                              "order_id integer REFERENCES orders,"
                              "quantity integer,"
                              "PRIMARY KEY (product_id, order_id)"
                              ")")))
        return q.lastError();

    if (!q.prepare(QLatin1String("insert into suppliers(name, created) values(?, ?)")))
        return q.lastError();
    QVariant supplier1Id = addSupplier(q, QLatin1String("Supplier #1"), QDate(2016, 12, 1));
    QVariant supplier2Id = addSupplier(q, QLatin1String("Supplier #2"), QDate(2016, 12, 1));
    QVariant supplier3Id = addSupplier(q, QLatin1String("Supplier #3"), QDate(2016, 12, 1));

    if (!q.prepare(QLatin1String("insert into products(name, price) values(?, ?)")))
        return q.lastError();
    QVariant product1 = addProduct(q, QLatin1String("Product #1"), 100.1);
    QVariant product2 = addProduct(q, QLatin1String("Product #2"), 200.2);
    QVariant product3 = addProduct(q, QLatin1String("Product #3"), 300.3);

    if (!q.prepare(QLatin1String("insert into orders(name, year, supplier, product, rating) values(?, ?, ?, ?, ?)")))
        return q.lastError();
    QVariant order1 = addOrder(q, QLatin1String("Foundation"), 2012, supplier1Id, product1, 3);
    QVariant order2 = addOrder(q, QLatin1String("Foundation and Empire"), 2012, supplier1Id, product1, 4);
    QVariant order3 = addOrder(q, QLatin1String("Second Foundation"), 2012, supplier1Id, product1, 3);
    QVariant order4 = addOrder(q, QLatin1String("Foundation's Edge"), 2012, supplier1Id, product1, 3);
    QVariant order5 = addOrder(q, QLatin1String("Foundation and Earth"), 2012, supplier1Id, product1, 4);
    QVariant order6 = addOrder(q, QLatin1String("Prelude to Foundation"), 2012, supplier1Id, product1, 3);
    QVariant order7 = addOrder(q, QLatin1String("Forward the Foundation"), 2012, supplier1Id, product1, 3);
    QVariant order8 = addOrder(q, QLatin1String("The Power and the Glory"), 2012, supplier2Id, product2, 4);
    QVariant order9 = addOrder(q, QLatin1String("The Third Man"), 2012, supplier2Id, product2, 5);
    QVariant order10 = addOrder(q, QLatin1String("Our Man in Havana"), 2012, supplier2Id, product2, 4);
    QVariant order11 = addOrder(q, QLatin1String("Guards! Guards!"), 2013, supplier3Id, product3, 3);
    QVariant order12 = addOrder(q, QLatin1String("Night Watch"), 2014, supplier3Id, product3, 3);
    QVariant order13 = addOrder(q, QLatin1String("Going Postal"), 2015, supplier3Id, product3, 3);

    if (!q.prepare(QLatin1String("insert into order_items(product_id, order_id, quantity) values(?, ?, ?)")))
        return q.lastError();
    addOrderItem(q, product1, order1, 1);
    addOrderItem(q, product2, order2, 2);
    addOrderItem(q, product3, order3, 3);
    addOrderItem(q, product1, order4, 4);
    addOrderItem(q, product2, order4, 5);
    addOrderItem(q, product3, order4, 6);

    // Notifications
    if (!q.exec(QLatin1String("DROP RULE IF EXISTS notifications ON orders; CREATE RULE notifications AS ON UPDATE TO orders DO NOTIFY dbupdated")))
        return q.lastError();

    return QSqlError();
}

#endif
