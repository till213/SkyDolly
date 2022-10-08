#ifndef CONST_H
#define CONST_H

#include <string_view>

namespace Const {

    /*!
     * An invalid ID indicates that the object has not yet been (successfully) persisted.
     */
    constexpr std::int64_t InvalidId {-1};

    constexpr std::string_view LogbookExtension {".sdlog"};

    constexpr char BackupNeverSymId[] = "NEVER";
    constexpr char BackupNowSymId[] = "NOW";
    constexpr char BackupMonthlySymId[] = "MONTH";
    constexpr char BackupWeeklySymId[] = "WEEK";
    constexpr char BackupDailySymId[] = "DAY";
    constexpr char BackupAlwaysSymId[] = "ALWAYS";
}

#endif // CONST_H
