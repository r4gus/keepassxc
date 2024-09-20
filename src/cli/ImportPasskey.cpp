#include "ImportPasskey.h"

#include "Utils.h"
#include "core/Global.h"
#include "core/Merger.h"
#include "core/Tools.h"

#include <QCommandLineParser>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

static int KEEPASSXCBROWSER_PASSKEY_ICON = 13;

ImportPasskey::ImportPasskey()
{
    name = QString("import-passkey");
    description = QObject::tr("Import a passkey into the database.");
    positionalArguments.append({QString("passkey-file"), QObject::tr("Path of the passkey to import."), QString("")});
}

int ImportPasskey::executeWithDatabase(QSharedPointer<Database> database, QSharedPointer<QCommandLineParser> parser)
{
    auto& err = Utils::STDERR;

    const QStringList args = parser->positionalArguments();
    const QString& fileName = args.at(1);
    
    // Read JSON file containing the passkey
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        err << QObject::tr("Cannot open file \"%1\" for reading.").arg(fileName) << Qt::endl;
        return EXIT_FAILURE;
    }

    const auto fileData = file.readAll();
    
    // Deserialize passkey
    QJsonParseError jsonErr;
    QJsonDocument doc(QJsonDocument::fromJson(fileData, &jsonErr));
    if (doc.isNull()) {
        err << QObject::tr("Cannot import passkey: \"%1\"").arg(jsonErr.errorString()) << Qt::endl;
        return EXIT_FAILURE;
    }
    QJsonObject passkeyObject = doc.object();
    
    // Verify all data is present and well-formed
    const auto privateKey = passkeyObject["privateKey"].toString();
    const auto missingKeys = Tools::getMissingValuesFromList<QString>(
        passkeyObject.keys(),
        QStringList() << "relyingParty" << "url" << "username" << "credentialId" << "userHandle" << "privateKey");

    if (!missingKeys.isEmpty()) {
        err << QObject::tr("Cannot import passkey. The following data is missing: \"%1\"").arg(missingKeys.join(", ")) << Qt::endl;
        return EXIT_FAILURE;
    }

    if (!privateKey.startsWith("-----BEGIN PRIVATE KEY-----") 
            || !privateKey.trimmed().endsWith("-----END PRIVATE KEY-----")) {
        err << QObject::tr("Cannot import passkey: private key is missing or malformed") << Qt::endl;
        return EXIT_FAILURE;
    }

    const auto relyingParty = passkeyObject["relyingParty"].toString();
    const auto url = passkeyObject["url"].toString();
    const auto username = passkeyObject["username"].toString();
    const auto credentialId = passkeyObject["credentialId"].toString();
    const auto userHandle = passkeyObject["userHandle"].toString();

    // Select group
    Group* group = database->rootGroup();
    // TODO: add additional logic for group selection
    // For now we just add it to /
    
    // Create new entry
    auto* entry = new Entry();

    entry->beginUpdate();

    entry->setUuid(QUuid::createUuid());
    entry->setGroup(group);
    entry->setTitle(QObject::tr("%1 (Passkey)").arg(relyingParty));
    entry->setUsername(username);
    entry->setUrl(url);
    entry->setIcon(KEEPASSXCBROWSER_PASSKEY_ICON);

    entry->attributes()->set(QStringLiteral("KPEX_PASSKEY_USERNAME"), username);
    entry->attributes()->set(QStringLiteral("KPEX_PASSKEY_CREDENTIAL_ID"), credentialId, true);
    entry->attributes()->set(QStringLiteral("KPEX_PASSKEY_PRIVATE_KEY_PEM"), privateKey, true);
    entry->attributes()->set(QStringLiteral("KPEX_PASSKEY_RELYING_PARTY"), relyingParty);
    entry->attributes()->set(QStringLiteral("KPEX_PASSKEY_USER_HANDLE"), userHandle, true);
    entry->addTag(QObject::tr("Passkey"));

    entry->endUpdate();

    // Remove blank entry history
    entry->removeHistoryItems(entry->historyItems());

    QString errorMessage;
    if (!database->save(Database::Atomic, {}, &errorMessage)) {
        err << QObject::tr("Writing the database failed %1.").arg(errorMessage) << Qt::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
