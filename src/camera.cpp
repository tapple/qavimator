/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Copyright (C) 2006 by Vinay Pulim.
 * All rights reserved.
 *
 */

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include "camera.h"

Camera::Camera()
{
  reset();
}

void Camera::rotate(float x, float y)
{
  rotX += x;
  if (rotX < 2) rotX = 2;
  if (rotX > 80) rotX = 80;
  rotY += y;
  if (rotY < 0) rotY += 360;
  if (rotY > 360) rotY -= 360;
}

void Camera::pan(float x, float y, float z)
{
  panX -= x/2;
  if (panX < -500) panX = -500;
  if (panX > 500) panX = 500;
  panY += y/2;
  if (panY < 5) panY = 5;
  if (panY > 500) panY = 500;
  panZ += z;
  if (panZ < 10) panZ = 10;
  if (panZ > 1000) panZ = 1000;
}

void Camera::setModelView()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(panX, panY, panZ, panX, panY, 0, 0, 1, 0);
  glRotatef(rotX, 1, 0, 0);
  glRotatef(rotY, 0, 1, 0);
}

void Camera::reset()
{
  rotX = 10;
  rotY = 0;
  panX = 0;
  panY = 40;
  panZ = 100;
}

float Camera::xRotation() const
{
  return rotX;
}

float Camera::yRotation() const
{
  return rotY;
}
