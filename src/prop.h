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
#ifndef PROP_H
#define PROP_H

/**
  @author Zi Ree
*/

class Prop
{
  public:
    typedef enum
    {
      Box=0,
      Prism=1
    } PropType;


    Prop(PropType type,const QString& name);
    ~Prop();

    void draw();
    void setPosition(double xp,double yp,double zp);
    void setScale(double scx,double scy,double scz);
    void setType(PropType type);

  protected:
    QString name;

    PropType type;

    double x,y,z;
    double xs,ys,zs;
};

#endif
