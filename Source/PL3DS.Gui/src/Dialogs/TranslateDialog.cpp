#include "PL3DS.Gui/Dialogs/TranslateDialog.h"

#include "ui_TranslateDialog.h"

#include <QPushButton>

namespace UI
{
    TranslateDialog::TranslateDialog(TranslateParameters& io_params, QDialog* ip_parent)
        : QDialog(ip_parent)
        , m_params(io_params)
    {
        Ui::TransformDialog ui;
        ui.setupUi(this);
        
        ui.mp_spin_translate_x->setValue(m_params.m_translate_x);
        ui.mp_spin_translate_y->setValue(m_params.m_translate_y);
        ui.mp_spin_translate_z->setValue(m_params.m_translate_z);
        ui.mp_spin_rotate_x->setValue(m_params.m_rotate_x);
        ui.mp_spin_rotate_y->setValue(m_params.m_rotate_y);
        ui.mp_spin_rotate_z->setValue(m_params.m_rotate_z);
        ui.mp_spin_scale->setValue(m_params.m_scale);

        m_commit_data = [=]
        {
            m_params.m_translate_x = ui.mp_spin_translate_x->value();
            m_params.m_translate_y = ui.mp_spin_translate_y->value();
            m_params.m_translate_z = ui.mp_spin_translate_z->value();
            m_params.m_rotate_x = ui.mp_spin_rotate_x->value();
            m_params.m_rotate_y = ui.mp_spin_rotate_y->value();
            m_params.m_rotate_z = ui.mp_spin_rotate_z->value();
            m_params.m_scale = ui.mp_spin_scale->value();
        };
    }

    TranslateDialog::~TranslateDialog() = default;

    void TranslateDialog::accept()
    {
        m_commit_data();

        QDialog::accept();
    }
}
