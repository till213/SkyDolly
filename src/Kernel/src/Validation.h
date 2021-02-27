#ifndef VALIDATION_H
#define VALIDATION_H

namespace Validation
{
    /*!
     * Checks whether the given \c value is in the range [min, max].
     */
    template <typename T>
        bool inRange(const T &value, const T &min, const T &max) {
        return !(value < min) && !(max < value);
    }
};

#endif // VALIDATION_H
