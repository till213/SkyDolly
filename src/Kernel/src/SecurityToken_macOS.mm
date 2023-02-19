/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#import <Foundation/NSURL.h>
#import <Foundation/NSError.h>
#import <Foundation/NSString.h>

#import <QByteArray>
#import <QString>

#import "SecurityToken.h"

#include "SecurityToken.h"

namespace
{
    bool startAccess(const QByteArray &securityTokenData)
    {
        NSData *bookmarkData;
        NSError *error = nil;
        BOOL bookmarkDataIsStale;
        NSURL *url;
        bool result;

        if (!securityTokenData.isNull()) {
            bookmarkData = securityTokenData.toNSData();
            url = [NSURL URLByResolvingBookmarkData:bookmarkData
                                            options:NSURLBookmarkResolutionWithSecurityScope
                                      relativeToURL:nil
                                bookmarkDataIsStale:&bookmarkDataIsStale
                                              error:&error];
            if (error == nil && bookmarkDataIsStale == NO) {
                [url startAccessingSecurityScopedResource];
                result = true;
            } else {
                result = false;
            }
        } else {
            result = true;
        }

        return result;
    }

    void stopAccess(const QByteArray &securityTokenData)
    {
        NSData *bookmarkData;
        NSError *error = nil;
        BOOL bookmarkDataIsStale;
        NSURL *url;

        if (!securityTokenData.isNull()) {
            bookmarkData = securityTokenData.toNSData();
            url = [NSURL URLByResolvingBookmarkData:bookmarkData
                                            options:NSURLBookmarkResolutionWithSecurityScope
                                      relativeToURL:nil
                                bookmarkDataIsStale:&bookmarkDataIsStale
                                              error:&error];

            [url stopAccessingSecurityScopedResource];
        }
    }
}

class SecurityTokenPrivate
{
public:
    SecurityTokenPrivate(const QByteArray theSecurityTokenData)
        : securityTokenData(theSecurityTokenData),
          refCount(1),
          valid(false)
    {}

    // make a copy of the original security token data, as the original data
    // in RecentFile might get deleted while moving position in the recent file list
    const QByteArray securityTokenData;
    int refCount;
    bool valid;
};

// public

void SecurityToken::retain()
{
    ++d->refCount;
}

void SecurityToken::release()
{
    --d->refCount;
    if (d->refCount == 0) {
        delete this;
    }
}

bool SecurityToken::isValid() const
{
    return d->valid;
}

SecurityToken *SecurityToken::create(const QByteArray &securityTokenData)
{
    SecurityToken *result = new SecurityToken(securityTokenData);
    return result;
}

QByteArray SecurityToken::createSecurityTokenData(const QString &filePath)
{
    QByteArray result;
    NSString *nsFilePath;
    NSURL *url;
    NSData *bookmarkData;
    NSError *error;

    nsFilePath = filePath.toNSString();
    url = [NSURL fileURLWithPath:nsFilePath isDirectory:NO];
    error = nil;
    bookmarkData = [url bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope
             includingResourceValuesForKeys:nil
                              relativeToURL:nil // app-scoped bookmark
                                      error:&error];
    if (bookmarkData != nil) {
        result.fromNSData(bookmarkData);
    }

    return result;
}

#ifdef DEBUG
void SecurityToken::debugTokenToFilePath(const QByteArray &securityTokenData)
{
    NSData *bookmarkData;
    NSError *error = nil;
    BOOL bookmarkDataIsStale;
    NSURL *url;

    if (!securityTokenData.isNull()) {
        bookmarkData = securityTokenData.toNSData();
        url = [NSURL URLByResolvingBookmarkData:bookmarkData
                                        options:NSURLBookmarkResolutionWithSecurityScope
                                  relativeToURL:nil
                            bookmarkDataIsStale:&bookmarkDataIsStale
                                          error:&error];

        NSString *filePath = [url absoluteString];
        if (url != nil) {
            NSLog(@"SecurityToken::tokenToFilePath: %@", filePath);
        } else {
            qDebug("SecurityToken::tokenToFilePath: Returned URL is NULL!");
        }
    } else {
        qDebug("SecurityToken::tokenToFilePath: token data is NULL!");
    }
}
#endif

// protected

SecurityToken::SecurityToken(const QByteArray &securityTokenData)
{
    d = new SecurityTokenPrivate(securityTokenData);
    d->valid = startAccess(securityTokenData);
#ifdef DEBUG
    qDebug("SecurityToken::SecurityToken(): called. Valid: %d", d->valid);
    debugTokenToFilePath(d->securityTokenData);
#endif
}

SecurityToken::~SecurityToken()
{
#ifdef DEBUG
    qDebug("SecurityToken::~SecurityToken(): called.");
    debugTokenToFilePath(d->securityTokenData);
#endif
    stopAccess(d->securityTokenData);
    delete d;
}
