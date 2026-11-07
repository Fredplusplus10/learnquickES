#include "buildinfo.h"

#include <QCoreApplication>
#include <QSysInfo>

QString BuildInfo::appVersion() const
{
    return QCoreApplication::applicationVersion();
}

// The Qt the app is running against, which can differ from the one it was built with.
QString BuildInfo::qtVersion() const
{
    return QString::fromLatin1(qVersion());
}

QString BuildInfo::compiler() const
{
    // clang-cl defines _MSC_VER too, so Clang is checked first.
#if defined(__clang__)
    return QStringLiteral("Clang %1.%2.%3").arg(__clang_major__).arg(__clang_minor__).arg(__clang_patchlevel__);
#elif defined(_MSC_VER)
    return QStringLiteral("MSVC %1.%2").arg(_MSC_VER / 100).arg(_MSC_VER % 100);
#elif defined(__GNUC__)
    return QStringLiteral("GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#else
    return QStringLiteral("unknown");
#endif
}

QString BuildInfo::platform() const
{
    return QSysInfo::prettyProductName();
}

QString BuildInfo::architecture() const
{
    return QSysInfo::currentCpuArchitecture();
}
