#pragma once

#include <QDialog>

#include <functional>

namespace UI
{
    struct SubdivideParameters
    {
        double m_edge_length_threshold = 0.5;
        bool m_appy_smoothing = true;
    };

    class SubdivideDialog : public QDialog
    {
        Q_OBJECT

    public:
        SubdivideDialog(SubdivideParameters& io_params, QDialog* ip_parent = nullptr);
        ~SubdivideDialog() override;

    public slots:
        void accept() override;

    private:
        SubdivideParameters& m_params;
        std::function<void()> m_commit_data;
    };
}
