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

#ifdef MACOSX
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <qstring.h>

#include "prop.h"

Prop::Prop(PropType newType,const QString& newName)
{
  setType(newType);
  propName=newName;
  createVertices();
}

void Prop::draw()
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glBegin(GL_QUADS);

  glColor4f(0.3,0.4,1.0, 1);

  QPtrList<Vertex> vertices=getVertices(type);

  for(int index=0;index<vertices.count();index++)
  {
    Vertex* v=vertices.at(index);
    glVertex3f(x+v->x()*xs,
               y+v->y()*ys,
               z+v->z()*zs);
  } // for


  // TODO: rotation

  glEnd();
}

void Prop::setPosition(double xp,double yp,double zp)
{
  x=xp;
  y=yp;
  z=zp;
}

void Prop::setScale(double scx,double scy,double scz)
{
  xs=scx;
  ys=scy;
  zs=scz;
}

void Prop::setRotation(double rx,double ry,double rz)
{
  xr=rx;
  yr=ry;
  zr=rz;
}

void Prop::setType(PropType newType)
{
  type=newType;
}

const QString& Prop::name() const
{
  return propName;
}

const QPtrList<Vertex> Prop::getVertices(PropType type)
{
  QPtrList<Vertex> vertices;
  if(type==Box)
  {
    vertices=boxVertices;
  }
  return vertices;
}

void Prop::createVertices()
{
  boxVertices.append(new Vertex(-0.5,-0.5,-0.5));
  boxVertices.append(new Vertex( 0.5,-0.5,-0.5));
  boxVertices.append(new Vertex( 0.5, 0.5,-0.5));
  boxVertices.append(new Vertex(-0.5, 0.5,-0.5));

  boxVertices.append(new Vertex(-0.5,-0.5,-0.5));
  boxVertices.append(new Vertex(-0.5,-0.5, 0.5));
  boxVertices.append(new Vertex(-0.5, 0.5, 0.5));
  boxVertices.append(new Vertex(-0.5, 0.5,-0.5));

  boxVertices.append(new Vertex( 0.5,-0.5,-0.5));
  boxVertices.append(new Vertex( 0.5,-0.5, 0.5));
  boxVertices.append(new Vertex( 0.5, 0.5, 0.5));
  boxVertices.append(new Vertex( 0.5, 0.5,-0.5));

  boxVertices.append(new Vertex(-0.5,-0.5, 0.5));
  boxVertices.append(new Vertex( 0.5,-0.5, 0.5));
  boxVertices.append(new Vertex( 0.5, 0.5, 0.5));
  boxVertices.append(new Vertex(-0.5, 0.5, 0.5));

  boxVertices.append(new Vertex(-0.5,-0.5,-0.5));
  boxVertices.append(new Vertex( 0.5,-0.5,-0.5));
  boxVertices.append(new Vertex( 0.5,-0.5, 0.5));
  boxVertices.append(new Vertex(-0.5,-0.5, 0.5));

  boxVertices.append(new Vertex(-0.5, 0.5,-0.5));
  boxVertices.append(new Vertex( 0.5, 0.5,-0.5));
  boxVertices.append(new Vertex( 0.5, 0.5, 0.5));
  boxVertices.append(new Vertex(-0.5, 0.5, 0.5));
}

Prop::~Prop()
{
}

Vertex::Vertex(double xc,double yc,double zc)
{
  xp=xc;
  yp=yc;
  zp=zc;
}

Vertex::~Vertex()
{
}

double Vertex::x()
{
  return xp;
}

double Vertex::y()
{
  return yp;
}

double Vertex::z()
{
  return zp;
}
