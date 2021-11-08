#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
#include <QTimer>
#include <QSaveFile>
#include <QStandardPaths>
#include <QImage>

#include "ShareConnectionLogic.h"

#include "configurators/cloak_configurator.h"
#include "configurators/vpn_configurator.h"
#include "configurators/openvpn_configurator.h"
#include "configurators/shadowsocks_configurator.h"
#include "configurators/ssh_configurator.h"

#include "defines.h"
#include <functional>

#include "../uilogic.h"

ShareConnectionLogic::ShareConnectionLogic(UiLogic *logic, QObject *parent):
    PageLogicBase(logic, parent),
    m_textEditShareOpenVpnCodeText{},
    m_pushButtonShareOpenVpnCopyEnabled{false},
    m_pushButtonShareOpenVpnSaveEnabled{false},
    m_lineEditShareShadowSocksStringText{},
    m_labelShareShadowSocksQrCodeText{},
    m_labelShareShadowSocksServerText{},
    m_labelShareShadowSocksPortText{},
    m_labelShareShadowSocksMethodText{},
    m_labelShareShadowSocksPasswordText{},
    m_plainTextEditShareCloakText{},
    m_textEditShareFullCodeText{},
    m_textEditShareAmneziaCodeText{},
    m_pushButtonShareFullCopyText{tr("Copy")},
    m_pushButtonShareAmneziaCopyText{tr("Copy")},
    m_pushButtonShareOpenVpnCopyText{tr("Copy")},
    m_pushButtonShareShadowSocksCopyText{tr("Copy")},
    m_pushButtonShareCloakCopyText{tr("Copy")},
    m_pushButtonShareAmneziaGenerateEnabled{true},
    m_pushButtonShareAmneziaCopyEnabled{true},
    m_pushButtonShareAmneziaGenerateText{tr("Generate config")},
    m_pushButtonShareOpenVpnGenerateEnabled{true},
    m_pushButtonShareOpenVpnGenerateText{tr("Generate config")}
{
    // TODO consider move to Component.onCompleted
    //updateSharingPage(uiLogic()->selectedServerIndex, m_settings.serverCredentials(uiLogic()->selectedServerIndex), uiLogic()->selectedDockerContainer);
}


void ShareConnectionLogic::onPushButtonShareFullCopyClicked()
{
    QGuiApplication::clipboard()->setText(textEditShareFullCodeText());
    set_pushButtonShareFullCopyText(tr("Copied"));

    QTimer::singleShot(3000, this, [this]() {
        set_pushButtonShareFullCopyText(tr("Copy"));
    });
}

void ShareConnectionLogic::onPushButtonShareFullSaveClicked()
{
    if (textEditShareFullCodeText().isEmpty()) return;

    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save AmneziaVPN config"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "*.amnezia");
    QSaveFile save(fileName);
    save.open(QIODevice::WriteOnly);
    save.write(textEditShareFullCodeText().toUtf8());
    save.commit();
}

void ShareConnectionLogic::onPushButtonShareAmneziaCopyClicked()
{
    if (textEditShareAmneziaCodeText().isEmpty()) return;

    QGuiApplication::clipboard()->setText(textEditShareAmneziaCodeText());
    set_pushButtonShareAmneziaCopyText(tr("Copied"));

    QTimer::singleShot(3000, this, [this]() {
        set_pushButtonShareAmneziaCopyText(tr("Copy"));
    });
}

void ShareConnectionLogic::onPushButtonShareAmneziaSaveClicked()
{
    if (textEditShareAmneziaCodeText().isEmpty()) return;

    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save AmneziaVPN config"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "*.amnezia");
    QSaveFile save(fileName);
    save.open(QIODevice::WriteOnly);
    save.write(textEditShareAmneziaCodeText().toUtf8());
    save.commit();
}

void ShareConnectionLogic::onPushButtonShareOpenVpnCopyClicked()
{
    QGuiApplication::clipboard()->setText(textEditShareOpenVpnCodeText());
    set_pushButtonShareOpenVpnCopyText(tr("Copied"));

    QTimer::singleShot(3000, this, [this]() {
        set_pushButtonShareOpenVpnCopyText(tr("Copy"));
    });
}

void ShareConnectionLogic::onPushButtonShareShadowSocksCopyClicked()
{
    QGuiApplication::clipboard()->setText(lineEditShareShadowSocksStringText());
    set_pushButtonShareShadowSocksCopyText(tr("Copied"));

    QTimer::singleShot(3000, this, [this]() {
        set_pushButtonShareShadowSocksCopyText(tr("Copy"));
    });
}

void ShareConnectionLogic::onPushButtonShareCloakCopyClicked()
{
    QGuiApplication::clipboard()->setText(plainTextEditShareCloakText());
    set_pushButtonShareCloakCopyText(tr("Copied"));

    QTimer::singleShot(3000, this, [this]() {
        set_pushButtonShareCloakCopyText(tr("Copy"));
    });
}

void ShareConnectionLogic::onPushButtonShareAmneziaGenerateClicked()
{
    set_pushButtonShareAmneziaGenerateEnabled(false);
    set_pushButtonShareAmneziaCopyEnabled(false);
    set_pushButtonShareAmneziaGenerateText(tr("Generating..."));
    qApp->processEvents();

    ServerCredentials credentials = m_settings.serverCredentials(uiLogic()->selectedServerIndex);
    QJsonObject containerConfig = m_settings.containerConfig(uiLogic()->selectedServerIndex, uiLogic()->selectedDockerContainer);
    containerConfig.insert(config_key::container, ContainerProps::containerToString(uiLogic()->selectedDockerContainer));

    ErrorCode e = ErrorCode::NoError;
    for (Protocol p: ContainerProps::protocolsForContainer(uiLogic()->selectedDockerContainer)) {
        QJsonObject protoConfig = m_settings.protocolConfig(uiLogic()->selectedServerIndex, uiLogic()->selectedDockerContainer, p);

        QString cfg = VpnConfigurator::genVpnProtocolConfig(credentials, uiLogic()->selectedDockerContainer, containerConfig, p, &e);
        if (e) {
            cfg = "Error generating config";
            break;
        }
        protoConfig.insert(config_key::last_config, cfg);

        containerConfig.insert(ProtocolProps::protoToString(p), protoConfig);
    }

    QByteArray ba;
    if (!e) {
        QJsonObject serverConfig = m_settings.server(uiLogic()->selectedServerIndex);
        serverConfig.remove(config_key::userName);
        serverConfig.remove(config_key::password);
        serverConfig.remove(config_key::port);
        serverConfig.insert(config_key::containers, QJsonArray {containerConfig});
        serverConfig.insert(config_key::defaultContainer, ContainerProps::containerToString(uiLogic()->selectedDockerContainer));


        ba = QJsonDocument(serverConfig).toJson().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        set_textEditShareAmneziaCodeText(QString("vpn://%1").arg(QString(ba)));
    }
    else {
        set_textEditShareAmneziaCodeText(tr("Error while generating connection profile"));
    }

    set_pushButtonShareAmneziaGenerateEnabled(true);
    set_pushButtonShareAmneziaCopyEnabled(true);
    set_pushButtonShareAmneziaGenerateText(tr("Generate config"));
}

void ShareConnectionLogic::onPushButtonShareOpenVpnGenerateClicked()
{
    set_pushButtonShareOpenVpnGenerateEnabled(false);
    set_pushButtonShareOpenVpnCopyEnabled(false);
    set_pushButtonShareOpenVpnSaveEnabled(false);
    set_pushButtonShareOpenVpnGenerateText(tr("Generating..."));

    ServerCredentials credentials = m_settings.serverCredentials(uiLogic()->selectedServerIndex);
    const QJsonObject &containerConfig = m_settings.containerConfig(uiLogic()->selectedServerIndex, uiLogic()->selectedDockerContainer);

    ErrorCode e = ErrorCode::NoError;
    QString cfg = OpenVpnConfigurator::genOpenVpnConfig(credentials, uiLogic()->selectedDockerContainer, containerConfig, &e);
    cfg = OpenVpnConfigurator::processConfigWithExportSettings(cfg);

    set_textEditShareOpenVpnCodeText(QJsonDocument::fromJson(cfg.toUtf8()).object()[config_key::config].toString());

    set_pushButtonShareOpenVpnGenerateEnabled(true);
    set_pushButtonShareOpenVpnCopyEnabled(true);
    set_pushButtonShareOpenVpnSaveEnabled(true);
    set_pushButtonShareOpenVpnGenerateText(tr("Generate config"));
}

void ShareConnectionLogic::onPushButtonShareOpenVpnSaveClicked()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save OpenVPN config"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "*.ovpn");

    QSaveFile save(fileName);
    save.open(QIODevice::WriteOnly);
    save.write(textEditShareOpenVpnCodeText().toUtf8());
    save.commit();
}


void ShareConnectionLogic::updateSharingPage(int serverIndex, const ServerCredentials &credentials,
                                DockerContainer container)
{
    uiLogic()->selectedDockerContainer = container;
    uiLogic()->selectedServerIndex = serverIndex;


    // Full access
    if (container == DockerContainer::None) {

        const QJsonObject &server = m_settings.server(uiLogic()->selectedServerIndex);

        QByteArray ba = QJsonDocument(server).toJson().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

        set_textEditShareFullCodeText(QString("vpn://%1").arg(QString(ba)));
    }
    else {
        for (Protocol p : ContainerProps::protocolsForContainer(container)) {
            if (p == Protocol::OpenVpn) {
                QString cfg = tr("Press Generate config");
                set_textEditShareOpenVpnCodeText(cfg);
                set_pushButtonShareOpenVpnCopyEnabled(false);
                set_pushButtonShareOpenVpnSaveEnabled(false);
            }
            else if (p == Protocol::ShadowSocks) {
                QJsonObject protoConfig = m_settings.protocolConfig(serverIndex, container, Protocol::ShadowSocks);
                QString cfg = protoConfig.value(config_key::last_config).toString();

                if (cfg.isEmpty()) {
                    const QJsonObject &containerConfig = m_settings.containerConfig(serverIndex, container);

                    ErrorCode e = ErrorCode::NoError;
                    cfg = ShadowSocksConfigurator::genShadowSocksConfig(credentials, container, containerConfig, &e);
                }

                QJsonObject ssConfig = QJsonDocument::fromJson(cfg.toUtf8()).object();

                QString ssString = QString("%1:%2@%3:%4")
                        .arg(ssConfig.value("method").toString())
                        .arg(ssConfig.value("password").toString())
                        .arg(ssConfig.value("server").toString())
                        .arg(ssConfig.value("server_port").toString());

                ssString = "ss://" + ssString.toUtf8().toBase64();
                set_lineEditShareShadowSocksStringText(ssString);

                QImage qr = updateQRCodeImage(ssString);
                set_labelShareShadowSocksQrCodeText(imageToBase64(qr));

                set_labelShareShadowSocksServerText(ssConfig.value("server").toString());
                set_labelShareShadowSocksPortText(ssConfig.value("server_port").toString());
                set_labelShareShadowSocksMethodText(ssConfig.value("method").toString());
                set_labelShareShadowSocksPasswordText(ssConfig.value("password").toString());

            }
            else if (p == Protocol::Cloak) {
                set_plainTextEditShareCloakText(QString(""));

                QJsonObject protoConfig = m_settings.protocolConfig(serverIndex, container, Protocol::Cloak);
                QString cfg = protoConfig.value(config_key::last_config).toString();

                if (cfg.isEmpty()) {
                    const QJsonObject &containerConfig = m_settings.containerConfig(serverIndex, container);

                    ErrorCode e = ErrorCode::NoError;
                    cfg = CloakConfigurator::genCloakConfig(credentials, container, containerConfig, &e);

                    //set_pushButtonShareCloakCopyEnabled(true);
                }

                QJsonObject cloakConfig = QJsonDocument::fromJson(cfg.toUtf8()).object();
                cloakConfig.remove(config_key::transport_proto);
                cloakConfig.insert("ProxyMethod", "shadowsocks");

                set_plainTextEditShareCloakText(QJsonDocument(cloakConfig).toJson());
            }
        }

    }

    //ui->toolBox_share_connection->addItem(ui->page_share_amnezia, tr("  Share for Amnezia client"));

    // Amnezia sharing
    //    QJsonObject exportContainer;
    //    for (Protocol p: protocolsForContainer(container)) {
    //        QJsonObject protocolConfig = containerConfig.value(ProtocolProps::protoToString(p)).toObject();
    //        protocolConfig.remove(config_key::last_config);
    //        exportContainer.insert(ProtocolProps::protoToString(p), protocolConfig);
    //    }
    //    exportContainer.insert(config_key::container, containerToString(container));

    //    ui->textEdit_share_amnezia_code->setPlainText(QJsonDocument(exportContainer).toJson());

    set_textEditShareAmneziaCodeText(tr(""));
}


QImage ShareConnectionLogic::updateQRCodeImage(const QString &text)
{
    int levelIndex = 1;
    int versionIndex = 0;
    bool bExtent = true;
    int maskIndex = -1;

    m_qrEncode.EncodeData( levelIndex, versionIndex, bExtent, maskIndex, text.toUtf8().data() );

    int qrImageSize = m_qrEncode.m_nSymbleSize;

    int encodeImageSize = qrImageSize + ( QR_MARGIN * 2 );
    QImage encodeImage( encodeImageSize, encodeImageSize, QImage::Format_Mono );

    encodeImage.fill( 1 );

    for ( int i = 0; i < qrImageSize; i++ )
        for ( int j = 0; j < qrImageSize; j++ )
            if ( m_qrEncode.m_byModuleData[i][j] )
                encodeImage.setPixel( i + QR_MARGIN, j + QR_MARGIN, 0 );

    return encodeImage;
}

QString ShareConnectionLogic::imageToBase64(const QImage &image)
{
    QByteArray ba;
    QBuffer bu(&ba);
    bu.open(QIODevice::WriteOnly);
    image.save(&bu, "PNG");
    return "data:image/png;base64," + QString::fromLatin1(ba.toBase64().data());
}
