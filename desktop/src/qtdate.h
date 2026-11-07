#pragma once

#include "lq/date.h"

#include <QDate>

// The engine counts days with std::chrono; Qt shows and stores them as QDate.
inline lq::Date toLqDate(QDate date)
{
    return lq::makeDate(date.year(), static_cast<unsigned>(date.month()), static_cast<unsigned>(date.day()));
}

inline QDate toQDate(lq::Date date)
{
    const std::chrono::year_month_day ymd{date};
    return QDate(static_cast<int>(ymd.year()), static_cast<int>(static_cast<unsigned>(ymd.month())),
                 static_cast<int>(static_cast<unsigned>(ymd.day())));
}
