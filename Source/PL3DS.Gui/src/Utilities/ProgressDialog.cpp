#include "PL3DS.Gui/Utilities/ProgressDialog.h"

#include "ui_ProgressDialog.h"

#include <QDebug>
#include <QFutureWatcher>
#include <QString>
#include <QTimer>
#include <QTime>

#include <QtConcurrent/QtConcurrentRun>

#include <memory>

namespace
{
    const auto EMPTY_TITLE = QStringLiteral("Operation is running, please wait...");
    const auto NAMED_TITLE = QStringLiteral("Operation \"%1\" is running, please wait...");
    const auto ELAPSED_TIME_PLACEHOLDER = QStringLiteral("Elapsed time: %1");

    inline auto _TimeToStr(const int i_duration_seconds)
    {
        auto seconds = std::max(i_duration_seconds, 0);
        if (seconds < 60)
            return QStringLiteral("%1 sec.").arg(QString::number(seconds));

        auto minutes = seconds / 60;
        seconds %= 60;
        if (seconds == 0)
            return QStringLiteral("%1 min.").arg(QString::number(minutes));

        return QStringLiteral("%1 min. %2 sec.").arg(QString::number(minutes)).arg(QString::number(seconds));
    }
}

namespace UI
{
    void RunInThread(std::function<void()> i_func, const QString& i_title)
    {
        auto p_dialog = std::make_unique<ProgressDialog>(i_title);
        auto p_future_watcher = std::make_unique<QFutureWatcher<void>>();
        
        bool is_connected = false;
        is_connected = QObject::connect(p_future_watcher.get(), &QFutureWatcherBase::finished, p_dialog.get(), &QDialog::close);
        Q_ASSERT(is_connected);

        p_future_watcher->setParent(p_dialog.get());
        p_dialog->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);
        p_dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
        p_dialog->setWindowFlag(Qt::WindowType::WindowMinimizeButtonHint, false);
        p_dialog->setWindowFlag(Qt::WindowType::WindowContextHelpButtonHint, false);
        
        p_future_watcher->setFuture(QtConcurrent::run(i_func));

        p_future_watcher.release();
        p_dialog.release()->exec();

        Q_UNUSED(is_connected);
    }

    ProgressDialog::ProgressDialog(const QString& i_title, QDialog* ip_parent/* = nullptr*/)
        : QDialog(ip_parent)
        , mp_timer(new QTimer(this))
    {
        Ui::ProgressDialog ui;
        ui.setupUi(this);

        ui.mp_label_title->setText(i_title.isEmpty() ? EMPTY_TITLE : NAMED_TITLE.arg(i_title));
        ui.mp_label_elapsed_time->setText(ELAPSED_TIME_PLACEHOLDER.arg(_TimeToStr(0)));
    
        bool is_connected = false;
        is_connected = connect(mp_timer.data(), &QTimer::timeout, this, [this, ui]
        {
            ++m_elapsed_seconds;
            ui.mp_label_elapsed_time->setText(ELAPSED_TIME_PLACEHOLDER.arg(_TimeToStr(m_elapsed_seconds)));
        });

        Q_ASSERT(is_connected);
        Q_UNUSED(is_connected);
    }

    void ProgressDialog::showEvent(QShowEvent* ip_event)
    {
        m_elapsed_seconds = 0;
        mp_timer->setInterval(1000); 
        mp_timer->setSingleShot(false);
        mp_timer->start();

        QDialog::showEvent(ip_event);
    }

    void ProgressDialog::closeEvent(QCloseEvent* ip_event)
    {
        mp_timer->stop();
               
        QDialog::closeEvent(ip_event);
    }

    ProgressDialog::~ProgressDialog() = default;
}
