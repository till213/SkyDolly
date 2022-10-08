#ifndef CONST_H
#define CONST_H

#include <string_view>

namespace Const {

    /*!
     * An invalid ID indicates that the object has not yet been (successfully) persisted.
     */
    constexpr std::int64_t InvalidId {-1};

    constexpr const char *LogbookExtension {".sdlog"};

    constexpr const char *BackupNeverSymId = "NEVER";
    constexpr const char *BackupNowSymId = "NOW";
    constexpr const char *BackupMonthlySymId = "MONTH";
    constexpr const char *BackupWeeklySymId = "WEEK";
    constexpr const char *BackupDailySymId = "DAY";
    constexpr const char *BackupAlwaysSymId = "ALWAYS";
}

#endif // CONST_H
