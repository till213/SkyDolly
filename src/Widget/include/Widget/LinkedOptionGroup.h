/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef LINKEDOPTIONGROUP_H
#define LINKEDOPTIONGROUP_H

#include <memory>

#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QVariant>

#include "WidgetLib.h"

struct LinkedOptionGroupPrivate;

/*! \todo This class needs some serious API rework ;) */
class WIDGET_API LinkedOptionGroup : public QWidget
{
    Q_OBJECT
public:
    explicit LinkedOptionGroup(QWidget *parent = nullptr) noexcept;
    LinkedOptionGroup(const LinkedOptionGroup &rhs) = delete;
    LinkedOptionGroup(LinkedOptionGroup &&rhs) = delete;
    LinkedOptionGroup &operator=(const LinkedOptionGroup &rhs) = delete;
    LinkedOptionGroup &operator=(LinkedOptionGroup &&rhs) = delete;
    ~LinkedOptionGroup() override;

    void addOption(const QString &name, const QVariant &optionValue, const QString &toopTip = QString()) noexcept;

    /*!
     * Sets all options to \c false.
     *
     * \sa optionToggled
     */
    void clearOptions() noexcept;

    /*!
     * Sets the option identified by \p optionValue to \p enable.
     *
     * \param optionValue
     *        the option value to enable or disable
     * \param enable
     *        set to \c true in order to enable the \p optionValue; \c false else
     * \sa optionToggled
     */
    void setOptionEnabled(const QVariant &optionValue, bool enable) noexcept;

signals:
    void optionToggled(const QVariant &optionValue, bool enable);

private:
    const std::unique_ptr<LinkedOptionGroupPrivate> d;

    void initUi() noexcept;

private slots:
    void onButtonToggled(bool enable) noexcept;
};

#endif // LINKEDOPTIONGROUP_H
