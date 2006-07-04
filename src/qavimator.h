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


#ifndef QAVIMATOR_H
#define QAVIMATOR_H

#include <qtimer.h>

#define UNTITLED_NAME "Untitled.avm"
#define PLAY_IMAGE "data/play.png"
#define PAUSE_IMAGE "data/pause.png"
#define KEY_IMAGE "data/key.png"
#define NOKEY_IMAGE "data/nokey.png"

#include "mainapplicationform.h"
#include "rotation.h"

class Prop;

class qavimator : public MainApplicationForm
{
  Q_OBJECT

  public:
    qavimator();
    ~qavimator();

    double frameData[512];

  signals:
    void enableRotation(bool state);
    void enablePosition(bool state);
    void enableProps(bool state);
    void resetCamera();
    void protectFrame(bool state);

  protected:
    void setSliderValue(QSlider* slider,QLineEdit* edit,float value);

  protected slots:
    void readSettings();

    void partClicked(const QString& partName,Rotation rot,RotationLimits rotLimits,Position pos);
    void partDragged(const QString& partName,double changeX,double changeY,double changeZ);
    void propClicked(Prop* prop);
    void backgroundClicked();

    void cb_PartChoice();
    void cb_RotRoller(int dummy);
    void cb_RotValue();
    void cb_PosRoller(int dummy);
    void cb_PosValue();

    void propPosChanged(int dummy);
    void propScaleChanged(int dummy);
    void propRotChanged(int dummy);

    void cb_timeout();
    void cb_PlayBtn();
    void cb_fpsValue(int fps);
    void cb_FrameSlider(int position);

    void figureChanged(int shape);
    void numFramesChanged(int num);
    void keyframeButtonToggled(bool on);

    void setCurrentFrame(int frame);

    // ------- Menu Action Slots (Callbacks) --------
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void fileLoadProps();
    void fileSaveProps();
    void fileExit();

    void editCopy();
    void editPaste();

    void optionsSkeleton(bool on);
    void optionsJointLimits(bool on);
    void optionsLoop(bool on);
    void optionsProtectFirstFrame(bool on);

    void newPropButtonClicked();
    void selectProp(const QString& name);
    void clearProps();

  protected:
    void setCurrentFile(const QString& fileName);
    void enableInputs(bool state);
    void updateFps();

    void updateKeyBtn();
    void updateInputs();

    void setX(float x);
    void setY(float y);
    void setZ(float z);

    float getX();
    float getY();
    float getZ();

    void setXPos(float x);
    void setYPos(float y);
    void setZPos(float z);

    float getXPos();
    float getYPos();
    float getZPos();

    QString currentFile;
    // last path used for open or save
    QString lastPath;
    QTimer timer;

    bool playing;
    bool loop;
    bool frameDataValid;
    // if set the first frame of an animation is protected
    bool protectFirstFrame;
    // will be true if a frame is protected
    bool protect;
};

#endif


/*

#include <string.h>
#include <FL/Fl_PNG_Image.h>
#include "AnimationView.h"
#include "Main.h"

*/
