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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QObject>
# include <QQmlEngine>
#else
# include <qdeclarative.h>
#endif

#include <QVariant>

#ifndef GCONF_DISABLED
#include <gconf/gconf-value.h>
#include <gconf/gconf-client.h>
#endif

class Configuration : public QObject
{
    Q_OBJECT
public:
    Configuration(QObject *parent = 0);
    ~Configuration();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    static QObject *factory(QQmlEngine *, QJSEngine *);
#else
    static Configuration *qmlAttachedProperties(QObject *);
#endif

    Q_INVOKABLE QVariant read(const QString &key);
    Q_INVOKABLE void write(const QString &key, const QVariant &value);
    Q_INVOKABLE void clear(const QString &key);

#ifndef GCONF_DISABLED
    static QVariant toVariant(GConfValue *value, int typeHint = 0);
    static GConfValue *fromVariant(const QVariant &variant);

private:
    GConfClient *m_client;
#endif
};

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
QML_DECLARE_TYPE(Configuration)
QML_DECLARE_TYPEINFO(Configuration, QML_HAS_ATTACHED_PROPERTIES)
#endif

#endif
