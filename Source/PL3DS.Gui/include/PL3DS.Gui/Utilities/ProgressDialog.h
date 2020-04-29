#pragma once

#include <QDialog>
#include <QPointer>

#include <functional>

class QString;
class QTimer;

namespace UI
{
    void RunInThread(std::function<void()> i_func, const QString& i_title);

    class ProgressDialog : public QDialog
    {
        Q_OBJECT

    public:
        ProgressDialog(const QString& i_title, QDialog* ip_parent = nullptr);
        ~ProgressDialog() override;

        void showEvent(QShowEvent* ip_event) override;
        void closeEvent(QCloseEvent* ip_event) override;

    private:
        QPointer<QTimer> mp_timer;
        int m_elapsed_seconds = 0;
    };
}
