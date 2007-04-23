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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <qdialog.h>

#include "settingsdialogform.h"

/*
  @author Zi Ree <Zi Ree @ Second Life>
*/

class SettingsDialog : public SettingsDialogForm
{
  Q_OBJECT

  public:
    SettingsDialog(QWidget* parent=0,const char* name=0,bool modal=false,WFlags f=0);
    ~SettingsDialog();

  signals:
    configChanged();

  protected slots:
    void accept();
    void acceptOk();
    void reject();

    void useFogToggled(bool state);
    void floorTranslucencyChanged(int value);
};

#endif
