/*
 * Copyright (C) 2013 Jolla Mobile <andrew.den.exter@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#ifndef CONFIGURATIONGROUP_H
#define CONFIGURATIONGROUP_H

#if QT_VERSION_5
# include <QQmlParserStatus>
# include <QQmlListProperty>
# define QDeclarativeParserStatus QQmlParserStatus
# define QDeclarativeListProperty QQmlListProperty
#else
# include <qdeclarative.h>
# include <QDeclarativeParserStatus>
# include <QDeclarativeListProperty>
#endif

#include <QVector>

#ifndef GCONF_DISABLED
#include <gconf/gconf-value.h>
#include <gconf/gconf-client.h>
#endif

class ConfigurationGroup : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QDeclarativeListProperty<QObject> data READ data CONSTANT)
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_CLASSINFO("DefaultProperty", "data")
public:
    ConfigurationGroup(QObject *parent = 0);
    ~ConfigurationGroup();

    void classBegin();
    void componentComplete();

    QString path() const;
    void setPath(const QString &path);

    QDeclarativeListProperty<QObject> data();

signals:
    void pathChanged();

private slots:
    void propertyChanged();

private:
    static void data_append(QDeclarativeListProperty<QObject> *property, QObject *value);
    static QObject *data_at(QDeclarativeListProperty<QObject> *property, int index);
    static int data_count(QDeclarativeListProperty<QObject> *property);
    static void data_clear(QDeclarativeListProperty<QObject> *property);

    void resolveProperties(const QByteArray &parentPath);

    QByteArray m_absolutePath;
    QString m_path;
    QList<QObject *> m_data;
    QList<ConfigurationGroup *> m_children;
    ConfigurationGroup *m_parent;

#ifndef GCONF_DISABLED
    void cancelNotifications();
    void readValue(const QMetaProperty &property, GConfValue *value);

    static void notify(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data);

    GConfClient *m_client;
    guint m_notifyId;
#endif

    int m_readPropertyIndex;
};

#endif
