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

#include "qavimator.h"
#include "animationview.h"

#define FILTER "Animation Files (*.bvh *.avm)"
#define PRECISION 100

qavimator::qavimator() : MainApplicationForm( 0, "qavimator", WDestructiveClose )
{
  playing=false;
  loop=true;
  frameDataValid = false;

/* FIXME: char fileName[256];
  sprintf(fileName, "%s/%s", execPath, PLAY_IMAGE);
  playImage = new Fl_PNG_Image(fileName);
  sprintf(fileName, "%s/%s", execPath, PAUSE_IMAGE);
  pauseImage = new Fl_PNG_Image(fileName);
  sprintf(fileName, "%s/%s", execPath, KEY_IMAGE);
  keyImage = new Fl_PNG_Image(fileName);
  sprintf(fileName, "%s/%s", execPath, NOKEY_IMAGE);
  nokeyImage = new Fl_PNG_Image(fileName);*/

  resize(850,600);

  connect(animationView,SIGNAL(callback()),this,SLOT(cb_AnimationView()));

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

  xSlider->setPageStep(10*PRECISION);
  ySlider->setPageStep(10*PRECISION);
  zSlider->setPageStep(10*PRECISION);
  xPositionSlider->setPageStep(10*PRECISION);
  yPositionSlider->setPageStep(10*PRECISION);
  zPositionSlider->setPageStep(10*PRECISION);

  positionSlider->setPageStep(1);

  // by default protect the first animation frame from being edited, this helps
  // with Second Life import
  protectFirstFrame=true;
  protect=true;

  fileNew();
}

qavimator::~qavimator()
{
}

void qavimator::cb_AnimationView()
{
  const char *partName = animationView->getSelectedPart();
  double x, y, z;
  double xMin, xMax, yMin, yMax, zMin, zMax;

  if (partName) {
    for(int index=0;index<editPartCombo->count();index++)
      if(editPartCombo->text(index)==partName) editPartCombo->setCurrentItem(index);

    Animation *anim = animationView->getAnimation();

    cb_PartChoice();
    animationView->getChangeValues(&x, &y, &z);
    anim->getRotationLimits(partName, &xMin, &xMax,
    &yMin, &yMax, &zMin, &zMax);

    // check for joint updates, if one if the values is not 0, assume drag -- Zi Ree
    bool doDrag=(x || y || z);
    // check if this frame is protected
    if(!protect)
    {
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

      x = getX()+x;
      y = getY()+y;
      z = getZ()+z;

      if (x < xMin) x = xMin;
      if (x > xMax) x = xMax;
      if (y < yMin) y = yMin;
      if (y > yMax) y = yMax;
      if (z < zMin) z = zMin;
      if (z > zMax) z = zMax;

      setX(x);
      setY(y);
      setZ(z);

      // I think this was specifically designed to set a keyframe upon selecting a
      // part.  Disabled so you can check whether a keyframe is set on a specific
      // part.  -- Lex Neva
      //cb_RotRoller(w->xRotRoller, NULL);
      if(doDrag) cb_RotRoller(0);
    }

  } else {
    // make key button reflect that nothing is selected
    updateKeyBtn();
  }
}

void qavimator::cb_PartChoice()
{
  animationView->selectPart(editPartCombo->currentText());
  updateInputs();
}

void qavimator::cb_RotRoller(int)
{
  qDebug("qavimator::cb_RotRoller()");

  double x = getX();
  double y = getY();
  double z = getZ();

  setX(x);
  setY(y);
  setZ(z);

  if (editPartCombo->currentText()) {
    qDebug("cb_RotRoller -> setRotation()");
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
    qDebug("cb_RotValue -> setRotation()");
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
    anim->getRotation(editPartCombo->currentText(), &x,&y,&z);

    anim->getRotationLimits(editPartCombo->currentText(), &xMin, &xMax, &yMin, &yMax,
                            &zMin, &zMax);

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
    positionSlider->blockSignals(true);
    positionSlider->setValue(anim->getFrame());
    positionSlider->blockSignals(false);
    currentFrameLabel->setText(QString::number(anim->getFrame()+1));
    framesSpin->setValue(anim->getNumberOfFrames());
// FIXME:    if (anim->useRotationLimits()) w->limits->setonly();
  }
  if (editPartCombo->currentText()=="hip") {
    emit enablePosition(!protect);
    anim->getPosition("hip", &x,&y,&z);

    setXPos(x);
    setYPos(y);
    setZPos(z);
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
  qDebug(QString("enableInputs(%1)").arg(state));
  if(protect) state=false;
  qDebug(QString("protect==%1").arg(protect));

  emit enableRotation(state);
  if (editPartCombo->currentText()=="hip") emit enablePosition(state);

  keyframeButton->setEnabled(state);
}

void qavimator::cb_timeout()
{
  if (playing) {
    Animation *anim = animationView->getAnimation();
    if (anim) {
      animationView->repaint();
      anim->stepForward();
      //      if (!anim->stepForward())
      //	playing = false;

      if (anim->getFrame() == (anim->getNumberOfFrames() - 1) && !loop)
      {
        timer.stop();
        playing = false;
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
    timer.start(animationView->getAnimation()->frameTime()*1000);

  updateInputs();
}

void qavimator::cb_fpsValue(int fps) {
  qDebug(QString("qavimator::cb_fpsValue(%1)").arg(fps));
  animationView->getAnimation()->setFrameTime(1.0/(double) fps);
}

void qavimator::cb_FrameSlider(int position)
{
  qDebug(QString("cb_FrameSlider(%1)").arg(position));

  // check if we are at the first frame and if it's protected
  if(position==0 && protectFirstFrame) protect=true;
  else protect=false;

  if(protect) qDebug("Protected!");

  playing = false;
  animationView->getAnimation()->setFrame(position);
  updateInputs();
  animationView->repaint();
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

  editPartCombo->setCurrentItem(1);

  playing = false;
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
    editPartCombo->setCurrentItem(1);
    updateInputs();
    updateFps();

//    playBtn->label("Play");
//    partChc->clear();
//    addJoints(w->partChc, w->viewWin->getAnimation()->getJoints());
//    partChc->value(1);
//    partChc_cb(w->partChc, w);
//    playState = STOPPED;
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

// Menu Action: File / QExit
void qavimator::fileExit()
{
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
    animationView->getAnimation()->setFrameData(frameData);
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
  // don't send FPS change back to Animation object
  framesSpin->blockSignals(true);
  if (animationView->getAnimation()->frameTime() != 0.0)
    fpsSpin->setValue(1 / (animationView->getAnimation()->frameTime()));
  // re-enable FPS signal
  framesSpin->blockSignals(false);
}

// convenience function to set window title in a defined way
void qavimator::setCurrentFile(const QString& fileName)
{
  qDebug("current file "+fileName);
  currentFile=fileName;
  setCaption("qavimator ["+currentFile+"]");
}
