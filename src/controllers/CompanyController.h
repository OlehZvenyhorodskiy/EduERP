#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <spdlog/spdlog.h>

namespace eduerp::ctrl {

/**
 * @brief Manages company CRUD and exposes data to QML.
 */
class CompanyController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString currentCompanyName READ currentCompanyName NOTIFY companyChanged)
    Q_PROPERTY(QString industryTemplate READ industryTemplate NOTIFY companyChanged)
    Q_PROPERTY(double initialBudget READ initialBudget NOTIFY companyChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)

public:
    explicit CompanyController(QObject* parent = nullptr)
        : QObject(parent)
        , m_networkManager(new QNetworkAccessManager(this))
    {}

    QString currentCompanyName() const { return m_companyName; }
    QString industryTemplate() const { return m_industryTemplate; }
    double initialBudget() const { return m_initialBudget; }
    bool isLoading() const { return m_isLoading; }

    Q_INVOKABLE void createCompany(const QString& name, const QString& industry, double budget) {
        spdlog::info("CompanyController: Creating company '{}' in '{}'", name.toStdString(), industry.toStdString());
        m_isLoading = true; emit loadingChanged();

        QJsonObject body;
        body["name"] = name;
        body["industry_template"] = industry;
        body["initial_budget"] = budget;

        QNetworkRequest request(QUrl(m_apiBaseUrl + "/companies"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

        auto* reply = m_networkManager->post(request, QJsonDocument(body).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            m_isLoading = false; emit loadingChanged();
            if (reply->error() == QNetworkReply::NoError) {
                auto doc = QJsonDocument::fromJson(reply->readAll());
                auto data = doc.object()["data"].toObject();
                m_companyName = data["name"].toString();
                m_industryTemplate = data["industry_template"].toString();
                m_initialBudget = data["initial_budget"].toDouble();
                emit companyChanged();
                emit companyCreated();
            }
            reply->deleteLater();
        });
    }

    Q_INVOKABLE void loadCompany(int companyId) {
        m_isLoading = true; emit loadingChanged();

        QNetworkRequest request(QUrl(m_apiBaseUrl + "/companies/" + QString::number(companyId)));
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

        auto* reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            m_isLoading = false; emit loadingChanged();
            if (reply->error() == QNetworkReply::NoError) {
                auto doc = QJsonDocument::fromJson(reply->readAll());
                auto data = doc.object()["data"].toObject();
                m_companyName = data["name"].toString();
                m_industryTemplate = data["industry_template"].toString();
                m_initialBudget = data["initial_budget"].toDouble();
                emit companyChanged();
            }
            reply->deleteLater();
        });
    }

    void setAccessToken(const QString& token) { m_accessToken = token; }

signals:
    void companyChanged();
    void companyCreated();
    void loadingChanged();

private:
    QNetworkAccessManager* m_networkManager;
    QString m_companyName;
    QString m_industryTemplate;
    double m_initialBudget = 100000.00;
    bool m_isLoading = false;
    QString m_accessToken;
    QString m_apiBaseUrl = "http://localhost:8080/api/v1";
};

} // namespace eduerp::ctrl
