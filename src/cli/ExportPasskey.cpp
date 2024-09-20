#include "ExportPasskey.h"

#include "Utils.h"
#include "core/Group.h"
#include "core/Tools.h"

#include <QCommandLineParser>
#include <QJsonDocument>
#include <QJsonObject>

ExportPasskey::ExportPasskey()
{
    name = QString("export-passkey");
    description = QObject::tr("Export a passkey from the database.");
    positionalArguments.append({QString("entry"), QObject::tr("Name of the passkey entry to export."), QString("")});
}

int ExportPasskey::executeWithDatabase(QSharedPointer<Database> database, QSharedPointer<QCommandLineParser> parser)
{
    auto& out = Utils::STDOUT;
    auto& err = Utils::STDERR;

    const QStringList args = parser->positionalArguments();
    const QString& entryPath = args.at(1);

    Entry* entry = database->rootGroup()->findEntryByPath(entryPath);
    if (!entry) {
        err << QObject::tr("Could not find entry with path %1.").arg(entryPath) << Qt::endl;
        return EXIT_FAILURE;
    }

    QJsonObject passkeyObject;
    passkeyObject["relyingParty"] = entry->attributes()->value(QStringLiteral("KPEX_PASSKEY_RELYING_PARTY"));
    passkeyObject["url"] = entry->url();
    //passkeyObject["username"] = passkeyUtils()->getUsernameFromEntry(entry);
    passkeyObject["username"] = 
        entry->attributes()->hasKey(QStringLiteral("KPXC_PASSKEY_USERNAME"))
               ? entry->attributes()->value(QStringLiteral("KPXC_PASSKEY_USERNAME"))
               : entry->attributes()->value(QStringLiteral("KPEX_PASSKEY_USERNAME"));
    //passkeyObject["credentialId"] = passkeyUtils()->getCredentialIdFromEntry(entry);
    passkeyObject["credentialId"] = 
        entry->attributes()->hasKey(QStringLiteral("KPEX_PASSKEY_GENERATED_USER_ID"))
               ? entry->attributes()->value(QStringLiteral("KPEX_PASSKEY_GENERATED_USER_ID"))
               : entry->attributes()->value(QStringLiteral("KPEX_PASSKEY_CREDENTIAL_ID"));
    passkeyObject["userHandle"] = entry->attributes()->value(QStringLiteral("KPEX_PASSKEY_USER_HANDLE"));
    passkeyObject["privateKey"] = entry->attributes()->value(QStringLiteral("KPEX_PASSKEY_PRIVATE_KEY_PEM"));
    
    QJsonDocument document(passkeyObject);

    out << document.toJson();

    return EXIT_SUCCESS;
}
