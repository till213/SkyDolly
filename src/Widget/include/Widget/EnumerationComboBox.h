/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef ENUMERATIONCOMBOBOX_H
#define ENUMERATIONCOMBOBOX_H

#include <memory>
#include <unordered_set>
#include <cstdint>

#include <QComboBox>

#include "WidgetLib.h"

struct EnumerationComboBoxPrivate;

/*!
 * A combo box showing the the persisted enumeration values.
 */
class WIDGET_API EnumerationComboBox : public QComboBox
{
    Q_OBJECT
public:
    using IgnoredIds = std::unordered_set<std::int64_t>;

    enum struct Mode: std::uint8_t {
        Editable,
        NonEditable
    };

    explicit EnumerationComboBox(QString enumerationName, Mode mode, QWidget *parent = nullptr) noexcept;
    explicit EnumerationComboBox(QWidget *parent = nullptr) noexcept;
    EnumerationComboBox(const EnumerationComboBox &rhs) = delete;
    EnumerationComboBox(EnumerationComboBox &&rhs) = delete;
    EnumerationComboBox &operator=(const EnumerationComboBox &rhs) = delete;
    EnumerationComboBox &operator=(EnumerationComboBox &&rhs) = delete;
    ~EnumerationComboBox() override;

    QString getEnumerationName() const;
    void setEnumerationName(QString name) noexcept;

    Mode getMode() const noexcept;
    void setMode(Mode mode) noexcept;

    std::int64_t getCurrentId() const noexcept;
    void setCurrentId(std::int64_t id) noexcept;

    IgnoredIds getIgnoredIds() const noexcept;
    void setIgnoredIds(IgnoredIds ignoredIds) noexcept;

private:
    const std::unique_ptr<EnumerationComboBoxPrivate> d;

    void initUi() noexcept;
    void initAutoCompleter() noexcept;
};

#endif // ENUMERATIONCOMBOBOX_H
