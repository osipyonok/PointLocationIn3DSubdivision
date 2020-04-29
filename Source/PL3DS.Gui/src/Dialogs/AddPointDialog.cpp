#include "PL3DS.Gui/Dialogs/AddPointDialog.h"
#include "ui_AddPointDialog.h"

#include <QPushButton>

namespace UI
{
    AddPointDialog::AddPointDialog(AddPointParameters& io_params, QDialog* ip_parent)
        : QDialog(ip_parent)
        , m_params(io_params)
    {
        Ui::AddPointDialog ui;
        ui.setupUi(this);

        ui.mp_spin_x->setValue(m_params.m_x);
        ui.mp_spin_y->setValue(m_params.m_y);
        ui.mp_spin_z->setValue(m_params.m_z);

        m_commit_data = [=]
        {
            m_params.m_x = ui.mp_spin_x->value();
            m_params.m_y = ui.mp_spin_y->value();
            m_params.m_z = ui.mp_spin_z->value();
        };

        connect(ui.mp_button_box->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &QDialog::accept);
        connect(ui.mp_button_box->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::accept);
    }
    
    AddPointDialog::~AddPointDialog() = default;

    void AddPointDialog::accept()
    {
        m_commit_data();

        QDialog::accept();
    }

}
