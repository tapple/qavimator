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

#include <qapplication.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qaction.h>
#include <qsettings.h>

#include "qavimator.h"
#include "animationview.h"
#include "rotation.h"
#include "prop.h"

#define FILTER "Animation Files (*.bvh *.avm)"
#define PRECISION 100

qavimator::qavimator() : MainApplicationForm( 0, "qavimator", WDestructiveClose )
{
  playing=false;
  frameDataValid = false;

  readSettings();

  connect(animationView,SIGNAL(partClicked(const QString&,
                                           Rotation,
                                           RotationLimits,
                                           Position)),
                     this,SLOT(partClicked(const QString&,
                                           Rotation,
                                           RotationLimits,
                                           Position)));

  connect(animationView,SIGNAL(partDragged(const QString&,
                                           double,double,double)),
                     this,SLOT(partDragged(const QString&,
                                           double,double,double)));

  connect(animationView,SIGNAL(backgroundClicked()),this,SLOT(backgroundClicked()));

  connect(xSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_RotRoller(int)));
  connect(ySlider,SIGNAL(valueChanged(int)),this,SLOT(cb_RotRoller(int)));
  connect(zSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_RotRoller(int)));

  connect(xRotationEdit,SIGNAL(returnPressed()),this,SLOT(cb_RotValue()));
  connect(yRotationEdit,SIGNAL(returnPressed()),this,SLOT(cb_RotValue()));
  connect(zRotationEdit,SIGNAL(returnPressed()),this,SLOT(cb_RotValue()));

  connect(xRotationEdit,SIGNAL(lostFocus()),this,SLOT(cb_RotValue()));
  connect(yRotationEdit,SIGNAL(lostFocus()),this,SLOT(cb_RotValue()));
  connect(zRotationEdit,SIGNAL(lostFocus()),this,SLOT(cb_RotValue()));

  connect(xPositionSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_PosRoller(int)));
  connect(yPositionSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_PosRoller(int)));
  connect(zPositionSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_PosRoller(int)));

  connect(xPositionEdit,SIGNAL(returnPressed()),this,SLOT(cb_PosValue()));
  connect(yPositionEdit,SIGNAL(returnPressed()),this,SLOT(cb_PosValue()));
  connect(zPositionEdit,SIGNAL(returnPressed()),this,SLOT(cb_PosValue()));

  connect(xPositionEdit,SIGNAL(lostFocus()),this,SLOT(cb_PosValue()));
  connect(yPositionEdit,SIGNAL(lostFocus()),this,SLOT(cb_PosValue()));
  connect(zPositionEdit,SIGNAL(lostFocus()),this,SLOT(cb_PosValue()));

  connect(this,SIGNAL(enablePosition(bool)),xPositionLabel,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enablePosition(bool)),yPositionLabel,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enablePosition(bool)),zPositionLabel,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enablePosition(bool)),xPositionSlider,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enablePosition(bool)),yPositionSlider,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enablePosition(bool)),zPositionSlider,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enablePosition(bool)),xPositionEdit,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enablePosition(bool)),yPositionEdit,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enablePosition(bool)),zPositionEdit,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enableRotation(bool)),xRotationLabel,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),yRotationLabel,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),zRotationLabel,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enableRotation(bool)),xSlider,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),ySlider,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),zSlider,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enableRotation(bool)),xRotationEdit,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),yRotationEdit,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),zRotationEdit,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enableRotation(bool)),fpsLabel,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),fpsSpin,SLOT(setEnabled(bool)));

  connect(fpsSpin,SIGNAL(valueChanged(int)),this,SLOT(cb_fpsValue(int)));

  connect(positionSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_FrameSlider(int)));
  connect(playButton,SIGNAL(clicked()),this,SLOT(cb_PlayBtn()));
  connect(&timer,SIGNAL(timeout()),this,SLOT(cb_timeout()));

  connect(this,SIGNAL(resetCamera()),animationView,SLOT(resetCamera()));
  connect(this,SIGNAL(protectFrame(bool)),animationView,SLOT(protectFrame(bool)));

  xSlider->setPageStep(10*PRECISION);
  ySlider->setPageStep(10*PRECISION);
  zSlider->setPageStep(10*PRECISION);
  xPositionSlider->setPageStep(10*PRECISION);
  yPositionSlider->setPageStep(10*PRECISION);
  zPositionSlider->setPageStep(10*PRECISION);

  positionSlider->setPageStep(1);

//  animationView->addProp(Prop::Box,60,50,0,40,40,40);
//  animationView->addProp(Prop::Box,50,30,20,30,20,10);

  fileNew();
}

qavimator::~qavimator()
{
  fileExit();
}

void qavimator::readSettings()
{
  QSettings settings;
  settings.setPath("DeZiRee","QAvimator",QSettings::User);
  settings.beginGroup("/qavimator");

  // if no settings found, start up with defaults
  int width=850;
  int height=600;
  int figureType=0;
  loop=true;
  protectFirstFrame=true;
  bool skeleton=false;
  bool jointLimits=true;

  bool settingsFound=settings.readBoolEntry("/settings");
  if(settingsFound)
  {
    loop=settings.readBoolEntry("/loop");
    skeleton=settings.readBoolEntry("/skeleton");
    jointLimits=settings.readBoolEntry("/joint_limits");
    protectFirstFrame=settings.readBoolEntry("/protect_first_frame");

    int width=settings.readNumEntry("/mainwindow_width");
    int height=settings.readNumEntry("/mainwindow_height");

    // sanity
    if(width<50) width=50;
    if(height<50) height=50;

    figureType=settings.readNumEntry("/figure");

    settings.endGroup();
  }

  resize(width,height);

  optionsLoopAction->setOn(loop);
  optionsSkeletonAction->setOn(skeleton);
  optionsJointLimitsAction->setOn(jointLimits);
  // prevent a signal to be sent to yet uninitialized animation view
  optionsProtectFirstFrameAction->blockSignals(true);
  optionsProtectFirstFrameAction->setOn(protectFirstFrame);
  optionsProtectFirstFrameAction->blockSignals(false);
  figureCombo->setCurrentItem(figureType);
  figureChanged(figureType);
}

// slot gets called by AnimationView::mousePressEvent()
void qavimator::partClicked(const QString& partName,Rotation rot,RotationLimits limits,Position pos)
{
  if(partName)
  {
    for(int index=0;index<editPartCombo->count();index++)
      if(editPartCombo->text(index)==partName) editPartCombo->setCurrentItem(index);

    setX(rot.x);
    setY(rot.y);
    setZ(rot.z);

    if(partName=="hip")
    {
      enablePosition(!protect);

      setXPos(pos.x);
      setYPos(pos.y);
      setZPos(pos.z);

      xSlider->setRange(-359*PRECISION, 359*PRECISION);
      ySlider->setRange(-359*PRECISION, 359*PRECISION);
      zSlider->setRange(-359*PRECISION, 359*PRECISION);
    }
    else
    {
      enablePosition(false);

      xSlider->setRange(limits.xMin*PRECISION, limits.xMax*PRECISION);
      ySlider->setRange(limits.yMin*PRECISION, limits.yMax*PRECISION);
      zSlider->setRange(limits.zMin*PRECISION, limits.zMax*PRECISION);
    }

    // show the user if this part has a key frame here
    updateKeyBtn();
  }
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::partDragged(const QString& partName,double x,double y,double z)
{
  if(partName)
  {
    // check if this frame is protected
    if(!protect)
    {
      double newX=getX()+x;
      double newY=getY()+y;
      double newZ=getZ()+z;

      Animation *anim = animationView->getAnimation();
      RotationLimits rotLimits=anim->getRotationLimits(partName);

      double xMin=rotLimits.xMin;
      double yMin=rotLimits.yMin;
      double zMin=rotLimits.zMin;
      double xMax=rotLimits.xMax;
      double yMax=rotLimits.yMax;
      double zMax=rotLimits.zMax;

      if (newX < xMin) newX = xMin;
      if (newX > xMax) newX = xMax;
      if (newY < yMin) newY = yMin;
      if (newY > yMax) newY = yMax;
      if (newZ < zMin) newZ = zMin;
      if (newZ > zMax) newZ = zMax;

      setX(newX);
      setY(newY);
      setZ(newZ);

      animationView->getAnimation()->setRotation(partName,newX,newY,newZ);
      animationView->repaint();

      updateKeyBtn();
    }
  }
  else qDebug("partDragged(): partName==\"\"!");
}

// slot gets called by AnimationView::mouseButtonClicked()
void qavimator::backgroundClicked()
{
  updateKeyBtn();
}

// slot gets called by body part dropdown list
void qavimator::cb_PartChoice()
{
  // selectPart will fire partClicked event, so we don't bother
  // about updating controls ourselves here
  animationView->selectPart(editPartCombo->currentText());
  animationView->setFocus();
}

void qavimator::cb_RotRoller(int)
{
  double x = getX();
  double y = getY();
  double z = getZ();

  setX(x);
  setY(y);
  setZ(z);

  if (editPartCombo->currentText()) {
    animationView->getAnimation()->setRotation(editPartCombo->currentText(), x, y, z);
    animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::cb_RotValue()
{
  double x = xRotationEdit->text().toDouble();
  double y = yRotationEdit->text().toDouble();
  double z = zRotationEdit->text().toDouble();

  double min_x = xSlider->minValue()/PRECISION;
  double min_y = ySlider->minValue()/PRECISION;
  double min_z = zSlider->minValue()/PRECISION;

  double max_x = xSlider->maxValue()/PRECISION;
  double max_y = ySlider->maxValue()/PRECISION;
  double max_z = zSlider->maxValue()/PRECISION;

  if (x<min_x) x = min_x;  if (y<min_y) y = min_y;  if (z<min_z) z = min_z;
  if (x>max_x) x = max_x;  if (y>max_y) y = max_y;  if (z>max_z) z = max_z;

  setX(x);
  setY(y);
  setZ(z);

  if (editPartCombo->currentText()) {
    animationView->getAnimation()->setRotation(editPartCombo->currentText(), x, y, z);
    animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::cb_PosRoller(int)
{
  double x = getXPos();
  double y = getYPos();
  double z = getZPos();

  setXPos(x);
  setYPos(y);
  setZPos(z);

  if (editPartCombo->currentText()) {
    animationView->getAnimation()->setPosition(editPartCombo->currentText(), x, y, z);
    animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::cb_PosValue()
{
  double x = xPositionEdit->text().toDouble();
  double y = yPositionEdit->text().toDouble();
  double z = zPositionEdit->text().toDouble();

  double min_x = xPositionSlider->minValue()/PRECISION;
  double min_y = yPositionSlider->minValue()/PRECISION;
  double min_z = zPositionSlider->minValue()/PRECISION;

  double max_x = xPositionSlider->maxValue()/PRECISION;
  double max_y = yPositionSlider->maxValue()/PRECISION;
  double max_z = zPositionSlider->maxValue()/PRECISION;

  if (x<min_x) x = min_x;  if (y<min_y) y = min_y;  if (z<min_z) z = min_z;
  if (x>max_x) x = max_x;  if (y>max_y) y = max_y;  if (z>max_z) z = max_z;

  setXPos(x);
  setYPos(y);
  setZPos(z);

  if (editPartCombo->currentText()) {
    animationView->getAnimation()->setPosition(editPartCombo->currentText(), x, y, z);
    animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::updateInputs()
{
  double x=0, y=0, z=0;
  Animation *anim = animationView->getAnimation();

  if (anim) {
    double xMin, xMax, yMin, yMax, zMin, zMax;

    Rotation rot=anim->getRotation(editPartCombo->currentText());
    x=rot.x;
    y=rot.y;
    z=rot.z;

    RotationLimits rotLimits=anim->getRotationLimits(editPartCombo->currentText());
    xMin=rotLimits.xMin;
    yMin=rotLimits.yMin;
    zMin=rotLimits.zMin;
    xMax=rotLimits.xMax;
    yMax=rotLimits.yMax;
    zMax=rotLimits.zMax;

    if (editPartCombo->currentText()=="hip") {
      xSlider->setRange(-359*PRECISION, 359*PRECISION);
      ySlider->setRange(-359*PRECISION, 359*PRECISION);
      zSlider->setRange(-359*PRECISION, 359*PRECISION);
    }
    else
    {
      xSlider->setRange(xMin*PRECISION, xMax*PRECISION);
      ySlider->setRange(yMin*PRECISION, yMax*PRECISION);
      zSlider->setRange(zMin*PRECISION, zMax*PRECISION);
    }

    setX(x);
    setY(y);
    setZ(z);

    positionSlider->setMaxValue(anim->getNumberOfFrames()-1);

    framesSpin->setValue(anim->getNumberOfFrames());
  }
  if (editPartCombo->currentText()=="hip") {
    emit enablePosition(!protect);
    Position pos=anim->getPosition("hip");

    setXPos(pos.x);
    setYPos(pos.y);
    setZPos(pos.z);
  }
  else {
    emit enablePosition(false);
  }
  playButton->setText(playing ? "||" : ">");

  updateKeyBtn();

  if (playing)
    enableInputs(false);
  else
    enableInputs(true);

  if (frameDataValid)
    editPasteAction->setEnabled(true);
  else
    editPasteAction->setEnabled(false);
}

void qavimator::updateKeyBtn()
{
  // make sure we don't send a toggle signal on display update
  keyframeButton->blockSignals(true);
  keyframeButton->setOn(animationView->getAnimation()->isKeyFrame(animationView->getSelectedPart()));
  // re-enable toggle signal
  keyframeButton->blockSignals(false);
}

void qavimator::enableInputs(bool state) 
{
  if(protect) state=false;

  emit enableRotation(state);
  if (editPartCombo->currentText()=="hip") emit enablePosition(state);

  keyframeButton->setEnabled(state);
}

void qavimator::cb_timeout()
{
  if (playing) {
    Animation *anim = animationView->getAnimation();
    if (anim) {

      // don't show protected frames color on playback to avoid flicker
      protectFrame(false);
      anim->stepForward();

      if (anim->getFrame() == (anim->getNumberOfFrames() - 1) && !loop)
      {
        timer.stop();
        playing=false;
      }
      else
      {
      }

      updateInputs();
      return;
    }
  }
}

void qavimator::cb_PlayBtn()
{
  playing = !playing;
  if (playing)
    timer.start((int)(animationView->getAnimation()->frameTime()*1000));
  else
  {
    // take care of locks, key frames ...
    cb_FrameSlider(positionSlider->value());
  }

  updateInputs();
}

void qavimator::cb_fpsValue(int fps) {
  animationView->getAnimation()->setFrameTime(1.0/(double) fps);
}

void qavimator::cb_FrameSlider(int position)
{
  // check if we are at the first frame and if it's protected
  if(position==0 && protectFirstFrame) protect=true;
  else protect=false;

  playing=false;
  animationView->getAnimation()->setFrame(position);
  emit protectFrame(protect);

  updateInputs();
}

void qavimator::figureChanged(int shape)
{
  if(shape==0)
    animationView->setFigure(AnimationView::FEMALE);
  else
    animationView->setFigure(AnimationView::MALE);
  animationView->repaint();
}

void qavimator::numFramesChanged(int num)
{
  animationView->getAnimation()->setNumberOfFrames(num);
  updateInputs();
}

void qavimator::keyframeButtonToggled(bool on)
{
  animationView->getAnimation()->toggleKeyFrame(animationView->getSelectedPart());
  animationView->repaint();
}

// ------ Menu Action Slots (Callbacks) -----------

// Menu action: File / New
void qavimator::fileNew()
{
  setCurrentFile(UNTITLED_NAME);

  animationView->setAnimation(new Animation());

  // FIXME: code duplication
  connect(animationView->getAnimation(),SIGNAL(currentFrame(int)),this,SLOT(setCurrentFrame(int)));

  editPartCombo->setCurrentItem(1);

  playing=false;

  if(protectFirstFrame)
    // skip first frame, since it's protected anyway
    animationView->getAnimation()->setFrame(1);
  else
    animationView->getAnimation()->setFrame(0);

  // show frame as unprotected
  emit protectFrame(false);
  protect=false;

  updateInputs();
  updateFps();
}

// Menu action: File / Open ...
void qavimator::fileOpen()
{
  QString file=QFileDialog::getOpenFileName(QString::null,
                                            FILTER,
                                            this,
                                            "file_open_dialog",
                                            tr("Select Animation File"),
                                            0,
                                            false);
  if (file) {
    setCurrentFile(file);
    animationView->setAnimation(new Animation(file));
    // FIXME: code duplication
    connect(animationView->getAnimation(),SIGNAL(currentFrame(int)),this,SLOT(setCurrentFrame(int)));
    editPartCombo->setCurrentItem(1);
    updateInputs();
    updateFps();
  }
}

// Menu Action: File / Save
void qavimator::fileSave() {
  if(currentFile==UNTITLED_NAME)
    fileSaveAs();
  else
    animationView->getAnimation()->saveBVH(currentFile);
}

// Menu Action: File / Save As...
void qavimator::fileSaveAs()
{
  QString file=QFileDialog::getSaveFileName(currentFile,
                                            FILTER,
                                            this,
                                            "file_save_as_dialog",
                                            tr("Save Animation File"),
                                            0,
                                            false);
  if (file) {
    setCurrentFile(file);
    animationView->getAnimation()->saveBVH(file);
  }
}

// Menu Action: File / Exit
void qavimator::fileExit()
{
  QSettings settings;
  settings.setPath("DeZiRee","QAvimator",QSettings::User);

  settings.beginGroup("/qavimator");

  // make sure we know next time, that there actually was a settings file
  settings.writeEntry("/settings",true);

  settings.writeEntry("/loop",loop);
  settings.writeEntry("/skeleton",optionsSkeletonAction->isOn());
  settings.writeEntry("/joint_limits",optionsJointLimitsAction->isOn());
  settings.writeEntry("/protect_first_frame",optionsProtectFirstFrameAction->isOn());

  settings.writeEntry("/figure",figureCombo->currentItem());
  settings.writeEntry("/mainwindow_width",size().width());
  settings.writeEntry("/mainwindow_height",size().height());

  settings.endGroup();
  qApp->quit();
}

// Menu Action: Edit / Copy
void qavimator::editCopy()
{
  animationView->getAnimation()->getFrameData(frameData);
  frameDataValid = true;
  updateInputs();
}

// Menu Action: Edit / Paste
void qavimator::editPaste()
{
  if (frameDataValid)
  {
    animationView->getAnimation()->setFrameData(frameData);
    animationView->repaint();
    updateInputs();
  }
}

// Menu Action: Options / Skeleton
void qavimator::optionsSkeleton(bool on)
{
  if(on)
    animationView->showSkeleton();
  else
    animationView->hideSkeleton();
}

// Menu Action: Options / Loop
void qavimator::optionsLoop(bool on)
{
  loop=on;
}

// Menu Action: Options / Joint Limits
void qavimator::optionsJointLimits(bool on)
{
  animationView->getAnimation()->useRotationLimits(on);
  animationView->repaint();
  updateInputs();
}

// Menu Action: Oprions / Protect First Frame
void qavimator::optionsProtectFirstFrame(bool on)
{
  protectFirstFrame=on;
  if(on && positionSlider->value()==0) protect=true;
  else protect=false;

  emit protectFrame(protect);
  updateInputs();
}

void qavimator::setX(float x)
{
  setSliderValue(xSlider,xRotationEdit,x);
}

void qavimator::setY(float y)
{
  setSliderValue(ySlider,yRotationEdit,y);
}

void qavimator::setZ(float z)
{
  setSliderValue(zSlider,zRotationEdit,z);
}

float qavimator::getX()
{
  return xSlider->value()/PRECISION;
}

float qavimator::getY()
{
  return ySlider->value()/PRECISION;
}

float qavimator::getZ()
{
  return zSlider->value()/PRECISION;
}

void qavimator::setXPos(float x)
{
  setSliderValue(xPositionSlider,xPositionEdit,x);
}

void qavimator::setYPos(float y)
{
  setSliderValue(yPositionSlider,yPositionEdit,y);
}

void qavimator::setZPos(float z)
{
  setSliderValue(zPositionSlider,zPositionEdit,z);
}

float qavimator::getXPos()
{
  return xPositionSlider->value()/PRECISION;
}

float qavimator::getYPos()
{
  return yPositionSlider->value()/PRECISION;
}

float qavimator::getZPos()
{
  return zPositionSlider->value()/PRECISION;
}

// helper function to prevent feedback between the two widgets
void qavimator::setSliderValue(QSlider* slider,QLineEdit* edit,float value)
{
  slider->blockSignals(true);
  edit->blockSignals(true);
  slider->setValue(value*PRECISION);
  edit->setText(QString::number(value));
  edit->blockSignals(false);
  slider->blockSignals(false);
}

void qavimator::updateFps()
{
  double frameTime=animationView->getAnimation()->frameTime();

  // guard against division by zero
  if(frameTime!=0.0)
  {
    // don't send FPS change back to Animation object
    framesSpin->blockSignals(true);
    fpsSpin->setValue((int)(1/frameTime));
    // re-enable FPS signal
    framesSpin->blockSignals(false);
  }
}

// convenience function to set window title in a defined way
void qavimator::setCurrentFile(const QString& fileName)
{
  currentFile=fileName;
  setCaption("qavimator ["+currentFile+"]");
}

// this slot gets called from Animation::setFrame(int)
void qavimator::setCurrentFrame(int frame)
{
  positionSlider->blockSignals(true);
  positionSlider->setValue(frame);
  currentFrameLabel->setText(QString::number(frame+1));
  positionSlider->blockSignals(false);
}
