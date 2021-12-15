/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *   Christian Dywan <christian.dywan@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NOSONAPPFILTERBEHAVIOR_H
#define NOSONAPPFILTERBEHAVIOR_H

#include <QSortFilterProxyModel>
#include <QRegularExpression>

namespace nosonapp
{

class FilterBehavior : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)
    Q_PROPERTY(QRegularExpression pattern READ pattern WRITE setPattern NOTIFY patternChanged)

public:
    explicit FilterBehavior(QObject *parent = 0);

    QString property() const;
    void setProperty(const QString& property);
    QRegularExpression pattern() const;
    void setPattern(QRegularExpression pattern);

Q_SIGNALS:
    void propertyChanged();
    void patternChanged();

private:
    QString m_property;
    QRegularExpression m_pattern;
};

}

#endif // NOSONAPPFILTERBEHAVIOR_H
