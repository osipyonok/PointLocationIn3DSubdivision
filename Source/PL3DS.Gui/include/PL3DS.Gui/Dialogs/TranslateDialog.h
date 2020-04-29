#pragma once

#include <QDialog>

#include <functional>

namespace UI
{
    struct TranslateParameters
    {
        double m_translate_x = 0;
        double m_translate_y = 0;
        double m_translate_z = 0;

        double m_rotate_x = 0;
        double m_rotate_y = 0;
        double m_rotate_z = 0;

        double m_scale = 1;
    };

    class TranslateDialog : public QDialog
    {
        Q_OBJECT

    public:
        TranslateDialog(TranslateParameters& io_params, QDialog* ip_parent = nullptr);
        ~TranslateDialog() override;

    public slots:
        void accept() override;

    private:
        TranslateParameters& m_params;
        std::function<void()> m_commit_data;
    };
}
