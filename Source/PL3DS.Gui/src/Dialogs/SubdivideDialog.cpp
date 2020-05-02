#include "PL3DS.Gui/Dialogs/SubdivideDialog.h"
#include "ui_SubdivideDialog.h"

#include <QPushButton>

namespace UI
{
    SubdivideDialog::SubdivideDialog(SubdivideParameters& io_params, QDialog* ip_parent)
        : QDialog(ip_parent)
        , m_params(io_params)
    {
        Ui::SubdivideDialog ui;
        ui.setupUi(this);

        ui.mp_spin_edge_length->setValue(m_params.m_edge_length_threshold);
        ui.mp_check_smoothing->setChecked(m_params.m_appy_smoothing);

        m_commit_data = [=]
        {
            m_params.m_edge_length_threshold = ui.mp_spin_edge_length->value();
            m_params.m_appy_smoothing = ui.mp_check_smoothing->isChecked();
        };

        connect(ui.mp_button_box->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &QDialog::accept);
        connect(ui.mp_button_box->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::accept);
    }

    void SubdivideDialog::accept()
    {
        m_commit_data();
        QDialog::accept();
    }

    SubdivideDialog::~SubdivideDialog() = default;
}

