#ifndef KEEPASSXC_EXPORT_PASSKEY_H
#define KEEPASSXC_EXPORT_PASSKEY_H

#include "DatabaseCommand.h"

class ExportPasskey : public DatabaseCommand
{
public:
    ExportPasskey();

    int executeWithDatabase(QSharedPointer<Database> db, QSharedPointer<QCommandLineParser> parser) override;
};

#endif // KEEPASSXC_EXPORT_PASSKEY_H
