/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef SECURITYTOKEN_H
#define SECURITYTOKEN_H

#include <QByteArray>

#include "KernelLib.h"

class SecurityTokenPrivate;

/*!
 * \brief Provides file security tokens for permanent access across application restarts.
 *
 * The security token is reference-counted with #create, #retain and #release. Each create/retain call must be
 * balanced with the corresponding release call.
 *
 * For an introduction to Security-Scoped Bookmarks ("security token data") also refer to:
 * https://developer.apple.com/library/mac/documentation/security/conceptual/AppSandboxDesignGuide/AppSandboxInDepth/AppSandboxInDepth.html#//apple_ref/doc/uid/TP40011183-CH3-SW16
 */
class SecurityToken
{
public:

    /*!
     * Increases the reference counter by 1. Call this method if you want to keep
     * a reference to \em this instance.
     */
    KERNEL_API void retain();

    /*!
     * Decreases the reference counter by 1. If the reference counter reaches 0 then
     * \em this instance is \p deleted. This instance is then invalid and no
     * further instance methods must be called after a release.
     */
    KERNEL_API void release();

    KERNEL_API bool isValid() const;

    /*!
     * Creates an instance of the SecurityToken. The reference count will be 1. In
     * order to \p delete the object call #release.
     *
     * \param securityTokenData
     *        the security token data which has been created with createSecurityTokenData()
     * \sa #release()
     * \sa #createSecurityTokenData(const QString &)
     */
    KERNEL_API static SecurityToken *create(const QByteArray &securityTokenData);

    /*!
     * Creates a security access token data for the given \p filePath in order to access
     * files after the restart of a sandboxed application. The token
     * generation is platform-specific:
     * - On OS X Security-Scoped Bookmarks are generated
     * - On all other platforms an empty QByteArray is currently returned (= no
     *   security token)
     */
    KERNEL_API static QByteArray createSecurityTokenData(const QString &filePath);

#ifdef DEBUG
    KERNEL_API static void debugTokenToFilePath(const QByteArray &securityTokenData);
#endif

protected:
    explicit SecurityToken(const QByteArray &securityTokenData);
    virtual ~SecurityToken();

private:
    Q_DISABLE_COPY(SecurityToken)

    SecurityTokenPrivate *d;
};

#endif // SECURITYTOKEN_H
