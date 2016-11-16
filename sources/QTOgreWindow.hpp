#ifndef QTOGREWINDOW_H
#define QTOGREWINDOW_H
/*
Qt headers
*/
#include <QtWidgets/QApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QWindow>

#include "SdkQtCameraMan.hpp"
/*
Ogre3D header
*/

//Includes OlderCode
#include <OgreCamera.h>
#include <OgreItem.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>

//newInclude
#include <OgreConfigFile.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreLogManager.h>
#include <OgreFrameListener.h>
#include <OgreWindowEventUtilities.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <OgreMeshManager.h>
#include <OgreMeshManager2.h>
#include <OgreMesh2.h>
#include <OgreMesh2Serializer.h>
//HLMS
#include <Hlms/Unlit/OgreHlmsUnlit.h>
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <OgreHlmsManager.h>
#include <OgreArchiveManager.h>

//rendertexture
#include <OgreNameGenerator.h>
#include <OgreRenderTexture.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTargetListener.h>

class QTOgreWindow : public QWindow, public Ogre::FrameListener, public Ogre::RenderTargetListener
{
  Q_OBJECT

public:
  explicit QTOgreWindow(QWindow *parent = NULL);
  ~QTOgreWindow();

  virtual void render(QPainter *painter);
  virtual void render();
  virtual void createScene();

  void setAnimating(bool animating);

public slots:

  virtual void renderLater();
  virtual void renderNow();

  virtual bool eventFilter(QObject *target, QEvent *event);

signals:
  void entitySelected(Ogre::Item* entity);

protected:
  /*
    Ogre3D pointers added here. Useful to have the pointers here for use by the window later.
    */
  Ogre::Root* m_ogreRoot;
  Ogre::RenderWindow* m_ogreWindow;
  Ogre::SceneManager* m_ogreSceneMgr;
  Ogre::Camera* m_ogreCamera;
  Ogre::ColourValue m_ogreBackground;
  Ogre::CompositorWorkspace *mWorkspace;

  OgreQtBites::SdkQtCameraMan* m_cameraMan;

  //RenderTarget
  Ogre::TexturePtr texture_;
  Ogre::RenderTexture *renderTexture_;
  Ogre::Camera *Cam_;
  static Ogre::NameGenerator ng;

  Ogre::String m_resourcesCfg;
  Ogre::String m_pluginsCfg;

  bool m_update_pending;
  bool m_animating;

  /*
    The below methods are what is actually fired when they keys on the keyboard are hit.
    Similar events are fired when the mouse is pressed or other events occur.
    */
  virtual void keyPressEvent(QKeyEvent * ev);
  virtual void keyReleaseEvent(QKeyEvent * ev);
  virtual void mouseMoveEvent(QMouseEvent* e);
  virtual void wheelEvent(QWheelEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);
  virtual void mousePressEvent( QMouseEvent* e );

  virtual void exposeEvent(QExposeEvent *event);
  virtual bool event(QEvent *event);

  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

  //ajout pour fusion Ogre(fonctionnel) et QT
  virtual bool setup();
  virtual void setupResources();
  virtual void chooseSceneManager();
  virtual void createCamera();
  virtual void loadResources();
  virtual void createFrameListener();

  //Merge OgreWindow
  void initialize();
  void registerHlms(void);
  Ogre::CompositorWorkspace* setupCompositor();

  //renderTexture
  void renderTexture();

  void log(Ogre::String msg);
  void log(QString msg);

};

#endif // QTOGREWINDOW_H
