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

#include "configuration.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QQmlInfo>
#else
# include <QDeclarativeInfo>
#endif

#include <QMetaProperty>
#include <QPoint>
#include <QSize>
#include <QStringList>

Configuration::Configuration(QObject *parent)
    : QObject(parent = 0)
#ifndef GCONF_DISABLED
    , m_client(0)
#endif
{
#ifndef GCONF_DISABLED
    m_client = gconf_client_get_default();
#endif
}

Configuration::~Configuration()
{
#ifndef GCONF_DISABLED
    g_object_unref(m_client);
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
QObject *Configuration::factory(QQmlEngine *, QJSEngine *)
{
    return new Configuration;
}
#else
Configuration *Configuration::qmlAttachedProperties(QObject *parent)
{
    return new Configuration(parent);
}
#endif

#ifndef GCONF_DISABLED


template <typename T> GConfValue *toGConfValue(const QVariant &) { return 0; }

template <> GConfValue *toGConfValue<QString>(const QVariant &variant)
{
    GConfValue *value = gconf_value_new(GCONF_VALUE_STRING);
    gconf_value_set_string(value, variant.toString().toUtf8());
    return value;
}

template <> GConfValue *toGConfValue<int>(const QVariant &variant)
{
    GConfValue *value = gconf_value_new(GCONF_VALUE_INT);
    gconf_value_set_int(value, variant.toInt());
    return value;
}

template <> GConfValue *toGConfValue<double>(const QVariant &variant)
{
    GConfValue *value = gconf_value_new(GCONF_VALUE_FLOAT);
    gconf_value_set_float(value, variant.toDouble());
    return value;
}

template <> GConfValue *toGConfValue<bool>(const QVariant &variant)
{
    GConfValue *value = gconf_value_new(GCONF_VALUE_BOOL);
    gconf_value_set_bool(value, variant.toBool());
    return value;
}

template <> GConfValue *toGConfValue<QSize>(const QVariant &variant)
{
    const QSize size = variant.toSize();
    GConfValue *value = gconf_value_new(GCONF_VALUE_PAIR);
    gconf_value_set_car_nocopy(value, toGConfValue<int>(size.width()));
    gconf_value_set_cdr_nocopy(value, toGConfValue<int>(size.height()));
    return value;
}

template <> GConfValue *toGConfValue<QSizeF>(const QVariant &variant)
{
    const QSizeF size = variant.toSizeF();
    GConfValue *value = gconf_value_new(GCONF_VALUE_PAIR);
    gconf_value_set_car_nocopy(value, toGConfValue<double>(size.width()));
    gconf_value_set_cdr_nocopy(value, toGConfValue<double>(size.height()));
    return value;
}

template <> GConfValue *toGConfValue<QPoint>(const QVariant &variant)
{
    const QPoint size = variant.toPoint();
    GConfValue *value = gconf_value_new(GCONF_VALUE_PAIR);
    gconf_value_set_car_nocopy(value, toGConfValue<int>(size.x()));
    gconf_value_set_cdr_nocopy(value, toGConfValue<int>(size.y()));
    return value;
}

template <> GConfValue *toGConfValue<QPointF>(const QVariant &variant)
{
    const QPointF size = variant.toPointF();
    GConfValue *value = gconf_value_new(GCONF_VALUE_PAIR);
    gconf_value_set_car_nocopy(value, toGConfValue<double>(size.x()));
    gconf_value_set_cdr_nocopy(value, toGConfValue<double>(size.y()));
    return value;
}

template <> GConfValue *toGConfValue<QStringList>(const QVariant &variant)
{
    const QStringList stringList = variant.value<QStringList>();

    GSList *list = 0;
    for (int i = 0; i < stringList.count(); ++i) {
        GConfValue *value = gconf_value_new(GCONF_VALUE_STRING);
        gconf_value_set_string(value, stringList.at(i).toUtf8());
        list = g_slist_prepend(list, value);
    }

    GConfValue *value = gconf_value_new(GCONF_VALUE_LIST);
    gconf_value_set_list_type(value, GCONF_VALUE_STRING);
    gconf_value_set_list_nocopy(value, g_slist_reverse(list));

    return value;
}

template <typename T> GConfValue *toGConfList(const QVariantList &variantList, GConfValueType type)
{
    GSList *list = 0;
    for (int i = 0; i < variantList.count(); ++i) {
        list = g_slist_prepend(list, toGConfValue<T>(variantList.at(i)));
    }

    GConfValue *value = gconf_value_new(GCONF_VALUE_LIST);
    gconf_value_set_list_type(value, type);
    gconf_value_set_list_nocopy(value, g_slist_reverse(list));

    return value;
}

GConfValue *Configuration::fromVariant(const QVariant &variant)
{
    switch (variant.type()) {
    case QVariant::Invalid:    return 0;
    case QVariant::String:     return toGConfValue<QString>(variant);
    case QVariant::Int:        return toGConfValue<int>(variant);
    case QVariant::UInt:       return toGConfValue<int>(variant);
    case QVariant::Double:     return toGConfValue<double>(variant);
    case QVariant::Bool:       return toGConfValue<bool>(variant);
    case QVariant::StringList: return toGConfValue<QStringList>(variant);
    case QVariant::Point:      return toGConfValue<QPoint>(variant);
    case QVariant::PointF:     return toGConfValue<QPointF>(variant);
    case QVariant::Size:       return toGConfValue<QSize>(variant);
    case QVariant::SizeF:      return toGConfValue<QSizeF>(variant);
    default:
        if (variant.userType() == qMetaTypeId<QVariantList>()) {
            QVariantList list = variant.value<QVariantList>();
            if (list.isEmpty()) {
                return 0;
            } else {
                switch (list.first().type()) {
                case QVariant::Invalid:    return 0;
                case QVariant::String:     return toGConfList<QString>(list, GCONF_VALUE_STRING);
                case QVariant::Int:        return toGConfList<int>(list, GCONF_VALUE_INT);
                case QVariant::UInt:       return toGConfList<int>(list, GCONF_VALUE_INT);
                case QVariant::Double:     return toGConfList<double>(list, GCONF_VALUE_FLOAT);
                case QVariant::Bool:       return toGConfList<bool>(list, GCONF_VALUE_BOOL);
                default:
                    if (variant.userType() == qMetaTypeId<float>()) {
                        return toGConfList<double>(list, GCONF_VALUE_FLOAT);
                    } else if (variant.canConvert<int>()) {
                        return toGConfList<int>(list, GCONF_VALUE_INT);
                    }
                }
            }
        } else if (variant.userType() == qMetaTypeId<float>()) {
            return toGConfValue<double>(variant);
        } else if (variant.canConvert<int>()) {
            return toGConfValue<int>(variant);
        }
        return 0;
    }
}

template <typename T> T fromGConfValue(GConfValue *value) { return T(); }
template <> QString fromGConfValue<QString>(GConfValue *value) { return QString::fromUtf8(gconf_value_get_string(value)); }
template <> int fromGConfValue<int>(GConfValue *value) { return gconf_value_get_int(value); }
template <> double fromGConfValue<double>(GConfValue *value) { return gconf_value_get_float(value); }
template <> bool fromGConfValue<bool>(GConfValue *value) { return gconf_value_get_bool(value); }

template <typename List, typename T> List fromGConfList(GConfValue *value)
{
    List list;
    for (GSList *it = gconf_value_get_list(value); it; it = it->next) {
        list.append(fromGConfValue<T>(reinterpret_cast<GConfValue *>(it->data)));
    }
    return list;
}

QVariant Configuration::toVariant(GConfValue *value, int typeHint)
{
    switch (value->type) {
    case GCONF_VALUE_STRING: return fromGConfValue<QString>(value);
    case GCONF_VALUE_INT:    return fromGConfValue<int>(value);
    case GCONF_VALUE_FLOAT:  return fromGConfValue<double>(value);
    case GCONF_VALUE_BOOL:   return fromGConfValue<bool>(value);
    case GCONF_VALUE_LIST:
        switch (gconf_value_get_list_type(value)) {
        case GCONF_VALUE_STRING: return fromGConfList<QStringList, QString>(value);
        case GCONF_VALUE_INT:    return fromGConfList<QVariantList, int>(value);
        case GCONF_VALUE_FLOAT:  return fromGConfList<QVariantList, double>(value);
        case GCONF_VALUE_BOOL:   return fromGConfList<QVariantList, bool>(value);
        default: return QVariant();
        }
        break;
    case GCONF_VALUE_PAIR: {
        GConfValue *first = gconf_value_get_car(value);
        GConfValue *second = gconf_value_get_cdr(value);
        if (!first || !second) {
            return QVariant();
        } else if (first->type == GCONF_VALUE_INT && second->type == GCONF_VALUE_INT) {
            switch (typeHint) {
            case QVariant::Point:
            case QVariant::PointF:
                return QVariant(QPoint(gconf_value_get_int(first), gconf_value_get_int(second)));
            case QVariant::Size:
            case QVariant::SizeF:
            default:    //
                return QVariant(QSize(gconf_value_get_int(first), gconf_value_get_int(second)));
            }
        } else if (first->type == GCONF_VALUE_FLOAT && second->type == GCONF_VALUE_FLOAT) {
            switch (typeHint) {
            case QVariant::Point:
            case QVariant::PointF:
                return QVariant(QPointF(gconf_value_get_float(first), gconf_value_get_float(second)));
            case QVariant::Size:
            case QVariant::SizeF:
            default:
                return QVariant(QSizeF(gconf_value_get_float(first), gconf_value_get_float(second)));
            }
        }
        return QVariant();
    }
    case GCONF_VALUE_INVALID:
        return QVariant();
    default:
        return QVariant();
    }
}

#endif

QVariant Configuration::read(const QString &key)
{
#ifndef GCONF_DISABLED
    GError *error = 0;
    if (GConfValue *value = gconf_client_get(m_client, key.toUtf8().constData(), &error)) {
        QVariant variant = toVariant(value);
        gconf_value_free(value);
        return variant;
    } else if (error) {
        qmlInfo(this) << "Failed to get value for " << key;
        qmlInfo(this) << error->message;
        g_error_free(error);
    }
#else
    Q_UNUSED(key)
#endif
    return QVariant();

}

void Configuration::write(const QString &key, const QVariant &variant)
{
#ifndef GCONF_DISABLED
    GError *error = 0;
    if (GConfValue *value = toGConfValue<QVariant>(variant)) {
        gconf_client_set(m_client, key.toUtf8().constData(), value, &error);
        gconf_value_free(value);
    } else if (variant.type() == QVariant::Invalid
               || variant.userType() == qMetaTypeId<QVariantList>()) {
        gconf_client_unset(m_client, key.toUtf8().constData(), &error);
    }

    if (error) {
        qmlInfo(this) << "Failed to get value for " << key << variant;
        qmlInfo(this) << error->message;
        g_error_free(error);
    }
#else
    Q_UNUSED(key);
    Q_UNUSED(variant);
#endif
}

void Configuration::clear(const QString &key)
{
#ifndef GCONF_DISABLED
    GError *error = 0;
    gconf_client_recursive_unset(m_client, key.toUtf8().constData(), GConfUnsetFlags(0), &error);
    if (error) {
        qmlInfo(this) << "Failed to unset values for " << key;
        qmlInfo(this) << error->message;
        g_error_free(error);
    }
#else
    Q_UNUSED(key);
#endif
}

