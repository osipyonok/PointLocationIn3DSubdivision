#pragma once

#include <QDialog>

#include <functional>

namespace UI
{
    struct AddPointParameters
    {
        double m_x = 0;
        double m_y = 0;
        double m_z = 0;
    };

    class AddPointDialog : public QDialog
    {
        Q_OBJECT

    public:
        AddPointDialog(AddPointParameters& io_params, QDialog* ip_parent = nullptr);
        ~AddPointDialog() override;

    public slots:
        void accept() override;

    private:
        AddPointParameters& m_params;
        std::function<void()> m_commit_data;
    };
}
