/***************************************************************************
 *   Copyright (C) 2006 by Zi Ree   *
 *   Zi Ree @ SecondLife   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qspinbox.h>
#include <qapplication.h>

#include "settings.h"
#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget* parent,const char* name,bool modal,WFlags f)
: SettingsDialogForm(parent,name,modal,f)
{
  useFogCheckbox->setChecked(Settings::fog());
  floorTranslucencySpin->setValue(Settings::floorTranslucency());
  easeInCheck->setChecked(Settings::easeIn());
  easeOutCheck->setChecked(Settings::easeOut());
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::accept()
{
  qDebug("accept()");

  Settings::setFog(useFogCheckbox->isChecked());
  Settings::setFloorTranslucency(floorTranslucencySpin->value());
  Settings::setEaseIn(easeInCheck->isChecked());
  Settings::setEaseOut(easeOutCheck->isChecked());
  emit configChanged();
  qApp->processEvents();
}

void SettingsDialog::acceptOk()
{
  qDebug("acceptOk()");
  accept();
  QDialog::accept();
}

void SettingsDialog::reject()
{
  qDebug("reject()");
  QDialog::reject();
}

void SettingsDialog::useFogToggled(bool state)
{
  qDebug("useFogToggled(%d)",state);
}

void SettingsDialog::floorTranslucencyChanged(int value)
{
  qDebug("floorTranslucencyChanged(%d)",value);
}

void SettingsDialog::easeInToggled(bool state)
{
  qDebug("easeInToggled(%d)",state);
}

void SettingsDialog::easeOutToggled(bool state)
{
  qDebug("easeOutToggled(%d)",state);
}
