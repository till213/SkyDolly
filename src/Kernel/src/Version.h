/* This file is part of the Screenie project.
   Screenie is a fancy screenshot composer.

   Copyright (C) 2011 Oliver Knoll <till.oliver.knoll@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef VERSION_H
#define VERSION_H

#include <QString>
#include <QDate>

#include "KernelLib.h"

class VersionPrivate;

/*!
 * The default constructor creates an instance which represents the current \em application version.
 * Other constructors exist to construct \em document versions, for example, which can be compared against
 * the \em application version.
 *
 * The version string has the form "[major].[minor].[patch]", for example "1.2.3".
 */
class Version
{
public:
    /*!
     * Creates this Version with the current values of the application version.
     */
    KERNEL_API Version();

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
    KERNEL_API Version(int majorNo, int minorNo, int patchNo);

    /*!
     * Creates this Version by parsing the \p version string value. This
     * Version can then be compared against the application version, for example.
     *
     * \param version
     *        a QString containing the version of the expected [major].[minor].[patch]
     * \sa #operator<
     */
    KERNEL_API Version(const QString &version);
    KERNEL_API ~Version();

    /*!
     * Returns the major version number.
     *
     * \return the major version number
     */
    KERNEL_API int getMajor();

    /*!
     * Returns the minor version number.
     *
     * \return the minor version number
     */
    KERNEL_API int getMinor();

    /*!
     * Returns the patch version number.
     *
     * \return the patch version number
     */
    KERNEL_API int getPatch();

    /*!
     * Returns a string representation of the form "[major].[minor].[patch]" of this Version
     * which can be used to construct another Version instance.
     *
     * \return a QString containing a user-friendly version value
     * \sa Version(const QString &)
     */
    KERNEL_API QString toString();

    KERNEL_API bool operator==(const Version &other);

    KERNEL_API bool operator>=(const Version &other);

    KERNEL_API bool operator<(const Version &other);

    /*!
     * A cool code name - every application needs this ;)
     *
     * \return a QString containing a cool code name
     */
    KERNEL_API static QString getCodeName();

    /*!
     * A "fancy user readable version" which follows the pattern yy.mm.
     *
     * Example: 11.02 (= "released February 2011")
     *
     * \return a QString containing the "user readable version"
     */
    KERNEL_API static QString getUserVersion();

    /*!
     * The version number which follows the pattern version.minor.patch.
     *
     * Example: 1.2.3
     *
     * \return a QString containing the application version.
     */
    KERNEL_API static QString getApplicationVersion();

    /*!
     * Returns the application name which can be displayed in dialog captions.
     *
     * \return a QString containing the application name
     */
    KERNEL_API static QString getApplicationName();

private:
    Q_DISABLE_COPY(Version)

    VersionPrivate *d;
};

#endif // VERSION_H
