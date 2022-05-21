/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) Oliver Knoll
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
#ifndef VERSION_H
#define VERSION_H

#include <memory>

#include <QString>
#include <QStringView>
#include <QDate>

#include "KernelLib.h"

class VersionPrivate;

/*!
 * The default constructor creates an instance which represents the current \e application version.
 * Other constructors exist to construct \e document versions, for example, which can be compared against
 * the \e application version.
 *
 * The version string has the form "[major].[minor].[patch]", for example "1.2.3".
 */
class KERNEL_API Version
{
public:
    /*!
     * Creates this Version with the current values of the application version.
     */
    Version() noexcept;

    /*!
     * Creates this Version with \p majorNo, \p minorNo and \p patchNo values. This
     * Version can then be compared against the application version, for example.
     *
     * \param majorNo
     *        major version number
     * \param minorNo
     *        minor version number
     * \param patchNo
     *        sub-minor version number
     */
    Version(int majorNo, int minorNo, int patchNo) noexcept;

    /*!
     * Creates this Version by parsing the \p version string value. This
     * Version can then be compared against the application version, for example.
     *
     * \param version
     *        a QString containing the version of the expected [major].[minor].[patch]
     * \sa #operator<()
     */
    Version(QStringView version) noexcept;
    ~Version() noexcept;

    void fromString(QStringView version) noexcept;

    /*!
     * Returns the major version number.
     *
     * \return the major version number
     */
    int getMajor() const noexcept;

    /*!
     * Returns the minor version number.
     *
     * \return the minor version number
     */
    int getMinor() const noexcept;

    /*!
     * Returns the patch version number.
     *
     * \return the patch version number
     */
    int getPatch() const noexcept;

    /*!
     * Returns a string representation of the form "[major].[minor].[patch]" of this Version
     * which can be used to construct another Version instance.
     *
     * \return a QString containing a user-friendly version value
     * \sa Version(QStringView)
     */
    QString toString() const noexcept;

    /*!
     * Returns whether the version is 0.0.0.
     *
     * \return \c true if all major, minor and patch numbers are zero (0.0.0); \c false else
     */
    bool isNull() const noexcept;

    /*!
     * Assignmnent operator.
     *
     * \param rhs
     *        the other Version to assign from
     */
    void operator=(const Version &rhs) noexcept;

    /*!
     * Compares this Version against \c rhs for equality.
     *
     * \param rhs
     *        the other Version to compare with
     * \return \c true if this Version is equal with the \c rhs version;
     *         \c false else
     */
    bool operator==(const Version &rhs) noexcept;

    /*!
     * Compares this Version against \c rhs for greater or equal.
     *
     * \param rhs
     *        the other Version to compare with
     * \return \c true if this Version is equal or greater than the \c rhs version;
     *         \c false else
     */
    bool operator>=(const Version &rhs) noexcept;

    /*!
     * Compares this Version against \c rhs for smaller.
     *
     * \param rhs
     *        the other Version to compare with
     * \return \c true if this Version is smaller than the \c rhs version;
     *         \c false else
     */
    bool operator<(const Version &rhs) noexcept;

    /*!
     * A cool code name - every application needs this ;)
     *
     * \return a QString containing a cool code name
     */
    static const QString getCodeName() noexcept;

    /*!
     * A "fancy user readable version" which follows the pattern yy.mm.
     *
     * Example: 11.02 (= "released February 2011")
     *
     * \return a QString containing the "user readable version"
     */
    static const QString getUserVersion() noexcept;

    /*!
     * The version number which follows the pattern version.minor.patch.
     *
     * Example: 1.2.3
     *
     * \return a QString containing the application version.
     */
    static const QString getApplicationVersion() noexcept;

    /*!
     * Returns the organisation name.
     *
     * \return a QString the name of the organisation
     */
    static const QString getOrganisationName() noexcept;

    /*!
     * Returns the application name which can be displayed in dialog captions.
     *
     * \return a QString containing the application name
     */
    static const QString getApplicationName() noexcept;

    static QLatin1String getGitHash() noexcept;

private:
    std::unique_ptr<VersionPrivate> d;
};

#endif // VERSION_H
