#ifndef KEEPASSXC_IMPORT_PASSKEY_H
#define KEEPASSXC_IMPORT_PASSKEY_H

#include "DatabaseCommand.h"

class ImportPasskey : public DatabaseCommand
{
public:
    ImportPasskey();

    int executeWithDatabase(QSharedPointer<Database> db, QSharedPointer<QCommandLineParser> parser) override;
};

#endif // KEEPASSXC_IMPORT_PASSKEY_H
