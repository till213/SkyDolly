#ifndef CONST_H
#define CONST_H

#include <string_view>

namespace Const {
    constexpr std::string_view LogbookExtension {".sdlog"};

    constexpr char BackupNeverSymId[] = "NEVER";
    constexpr char BackupNowSymId[] = "NOW";
    constexpr char BackupMonthlySymId[] = "MONTH";
    constexpr char BackupWeeklySymId[] = "WEEK";
    constexpr char BackupDailySymId[] = "DAY";
    constexpr char BackupAlwaysSymId[] = "ALWAYS";
}

#endif // CONST_H
