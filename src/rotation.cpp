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
#include "rotation.h"

Rotation::Rotation(const QString& part,double rx,double ry, double rz)
{
  bodyPart=part;
  x=rx;
  y=ry;
  z=rz;
}

Rotation::~Rotation()
{
}

RotationLimits::RotationLimits(const QString& joint,double rxMin,double rxMax,
                                                    double ryMin,double ryMax,
                                                    double rzMin,double rzMax)
{
  jointName=joint;

  xMin=rxMin;
  yMin=ryMin;
  zMin=rzMin;
  xMax=rxMax;
  yMax=ryMax;
  zMax=rzMax;
}

RotationLimits::~RotationLimits()
{
}

Position::Position(const QString& part,double px,double py, double pz)
{
  bodyPart=part;
  x=px;
  y=py;
  z=pz;
}

Position::~Position()
{
}