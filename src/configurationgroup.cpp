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

#include "configurationgroup.h"
#include "configuration.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QQmlInfo>
#else
# include <QDeclarativeInfo>
#endif
#include <QMetaProperty>

ConfigurationGroup::ConfigurationGroup(QObject *parent)
    : QObject(parent)
    , m_parent(0)
#ifndef GCONF_DISABLED
    , m_client(0)
    , m_notifyId(0)
#endif
    , m_readPropertyIndex(-1)
{
}

ConfigurationGroup::~ConfigurationGroup()
{
#ifndef GCONF_DISABLED
    if (m_client) {
        if (!m_parent && !m_absolutePath.isEmpty()) {
            m_absolutePath.chop(1);
            gconf_client_remove_dir(m_client, m_absolutePath.constData(), 0);
        }

        cancelNotifications();

        g_object_unref(m_client);
    }
#endif
}

void ConfigurationGroup::classBegin()
{
    const int propertyChangedIndex = staticMetaObject.indexOfMethod("propertyChanged()");
    Q_ASSERT(propertyChangedIndex != -1);

    const QMetaObject * const metaObject = this->metaObject();
    if (metaObject == &staticMetaObject)
        return;

    for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
        const QMetaProperty property = metaObject->property(i);

        if (property.hasNotifySignal()) {
            QMetaObject::connect(this, property.notifySignalIndex(), this, propertyChangedIndex);
        }
    }
}

void ConfigurationGroup::componentComplete()
{
#ifndef GCONF_DISABLED
    m_client = gconf_client_get_default();

    if (m_path.startsWith(QLatin1Char('/'))) {
        if (!m_parent) {
            GError *error = 0;
            gconf_client_add_dir(
                        m_client,
                        m_path.toUtf8().constData(),
                        GCONF_CLIENT_PRELOAD_RECURSIVE,
                        &error);
            if (error) {
                qmlInfo(this) << "Failed to enable notifications for path " << m_path;
                qmlInfo(this) << error->message;
                g_error_free(error);
            }
        }
        resolveProperties(QByteArray());
    } else if (m_parent && !m_absolutePath.isEmpty()) {
        resolveProperties(m_parent->m_absolutePath);
    }
#endif
}

QString ConfigurationGroup::path() const
{
    return m_path;
}

void ConfigurationGroup::setPath(const QString &path)
{
    if (m_path != path) {
        m_path = path;
        emit pathChanged();

#ifndef GCONF_DISABLED
        if (m_client && !m_absolutePath.isEmpty()) {
            if (!m_parent) {
                m_absolutePath.chop(1);
                gconf_client_remove_dir(m_client, m_absolutePath.constData(), 0);
            }

            if (m_path.isEmpty()) {
                m_absolutePath = QByteArray();
                cancelNotifications();
            } else if (m_parent && !m_path.startsWith(QLatin1Char('/'))) {
                resolveProperties(m_parent->m_absolutePath);
            } else {
                if (!m_parent) {
                    GError *error = 0;
                    gconf_client_add_dir(
                                m_client,
                                m_path.toUtf8().constData(),
                                GCONF_CLIENT_PRELOAD_RECURSIVE,
                                &error);
                    if (error) {
                        qmlInfo(this) << "Failed to register listener for path " << m_path;
                        qmlInfo(this) << error->message;
                        g_error_free(error);
                    }
                }
                resolveProperties(QByteArray());
            }
        }
#endif
    }
}

QDeclarativeListProperty<QObject> ConfigurationGroup::data()
{
    return QDeclarativeListProperty<QObject>(this, 0, data_append, data_count, data_at, data_clear);
}

void ConfigurationGroup::data_append(QDeclarativeListProperty<QObject> *property, QObject *value)
{
    ConfigurationGroup *settings = static_cast<ConfigurationGroup *>(property->object);
    settings->m_data.append(value);
    if (ConfigurationGroup *child = qobject_cast<ConfigurationGroup *>(value)) {
        settings->m_children.append(child);
        child->m_parent = settings;
    }
}

QObject *ConfigurationGroup::data_at(QDeclarativeListProperty<QObject> *property, int index)
{
    return static_cast<ConfigurationGroup *>(property->object)->m_data.at(index);
}

int ConfigurationGroup::data_count(QDeclarativeListProperty<QObject> *property)
{
    return static_cast<ConfigurationGroup *>(property->object)->m_data.count();
}

void ConfigurationGroup::data_clear(QDeclarativeListProperty<QObject> *)
{
}

#ifndef GCONF_DISABLED

#endif

void ConfigurationGroup::propertyChanged()
{
#ifndef GCONF_DISABLED
    if (m_absolutePath.isEmpty())
        return;

    const int notifyIndex = senderSignalIndex();
    const QMetaObject * const metaObject = this->metaObject();

    for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
        const QMetaProperty property = metaObject->property(i);
        if (i != m_readPropertyIndex && property.notifySignalIndex() == notifyIndex) {
            const QByteArray key = m_absolutePath + property.name();
            const QVariant variant = property.read(this);

            GError *error = 0;
            if (GConfValue *value = Configuration::fromVariant(variant)) {
                gconf_client_set(m_client, key.constData(), value, &error);
                gconf_value_free(value);
            } else if (variant.type() == QVariant::Invalid
                       || variant.userType() == qMetaTypeId<QVariantList>()) {
                gconf_client_unset(m_client, key.constData(), &error);
            }

            if (error) {
                qmlInfo(this) << "Failed to write value for " << key << variant;
                qmlInfo(this) << error->message;
                g_error_free(error);
            }
        }
    }
#endif
}

void ConfigurationGroup::resolveProperties(const QByteArray &parentPath)
{
#ifndef GCONF_DISABLED
    cancelNotifications();

    GError *error = 0;

    m_absolutePath = parentPath + m_path.toUtf8();
    m_notifyId = gconf_client_notify_add(m_client, m_absolutePath.constData(), notify, this, 0, &error);
    if (error) {
        qmlInfo(this) << "Failed to register notifications for " << m_absolutePath;
        qmlInfo(this) << error->message;
        g_error_free(error);
        error = 0;
    }
    m_absolutePath += '/';

    const QMetaObject * const metaObject = this->metaObject();
    if (metaObject != &staticMetaObject) {
        for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
            const QMetaProperty property = metaObject->property(i);
            const QByteArray key = m_absolutePath + property.name();

            GConfValue *value = gconf_client_get(m_client, key.constData(), &error);
            if (error) {
                qmlInfo(this) << "Failed to get value for " << key;
                qmlInfo(this) << error->message;
                g_error_free(error);
                error = 0;
            } else if (value) {
                m_readPropertyIndex = i;
                readValue(property, value);
                m_readPropertyIndex = -1;
                gconf_value_free(value);
            }
        }
    }

    for (int i = 0; i < m_children.count(); ++i) {
        m_children.at(i)->resolveProperties(m_absolutePath);
    }

#else
    Q_UNUSED(parentPath);
#endif
}

#ifndef GCONF_DISABLED

void ConfigurationGroup::readValue(const QMetaProperty &property, GConfValue *value)
{
    int typeHint = 0;
    if (value->type == GCONF_VALUE_PAIR)
        typeHint = property.read(this).type();
    QVariant variant = Configuration::toVariant(value, typeHint);
    if (variant.isValid()) {
        property.write(this, variant);
    }
}

void ConfigurationGroup::cancelNotifications()
{
    if (m_notifyId) {
        gconf_client_notify_remove(m_client, m_notifyId);
        m_notifyId = 0;
    }
}

void ConfigurationGroup::notify(GConfClient *, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
    ConfigurationGroup * const settings = static_cast<ConfigurationGroup *>(user_data);
    if (cnxn_id != settings->m_notifyId)
        return;

    const QByteArray key = gconf_entry_get_key(entry);
    const int pathLength = key.lastIndexOf('/');
    if (pathLength + 1 == settings->m_absolutePath.count()
            && key.startsWith(settings->m_absolutePath)) {
        const QMetaObject *const metaObject = settings->metaObject();
        settings->m_readPropertyIndex = metaObject->indexOfProperty(key.mid(pathLength + 1));
        if (settings->m_readPropertyIndex >= metaObject->propertyOffset()) {
            settings->readValue(
                        metaObject->property(settings->m_readPropertyIndex),
                        gconf_entry_get_value(entry));
        }
        settings->m_readPropertyIndex = -1;
    }
}

#endif
