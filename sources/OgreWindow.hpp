#ifndef __OgreWindow_h_
#define __OgreWindow_h_

//Include Sample
//#include "OgrePrerequisites.h"
//#include "OgreColourValue.h"
#include "Overlay/OgreOverlayPrerequisites.h"
#include "Threading/OgreUniformScalableTask.h"

//Includes OlderCode
#include <OgreCamera.h>
#include <OgreItem.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>

//newInclude
#include <OgreConfigFile.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreLogManager.h>
#include <OgreFrameListener.h>
#include <OgreWindowEventUtilities.h>
#include <SDL/SDL.h>
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

class OgreWindow : public Ogre::FrameListener/*, public Ogre::UniformScalableTask*/, public Ogre::WindowEventListener
{
public:
  OgreWindow();
  virtual ~OgreWindow();

  virtual void go();

protected:
  virtual bool setup();
  virtual void initialize();
  virtual void createScene();
  virtual void createCamera();
  virtual void createFrameListener();
  //  virtual void createResources();
  virtual void setupResources();
  virtual void loadResources();
  virtual void destroyScene();
  virtual void chooseSceneManager();
  virtual void windowClosed(Ogre::RenderWindow* rw);
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

  //test implementation hlms auto
  virtual void registerHlms();

  virtual Ogre::CompositorWorkspace* setupCompositor(void);

  Ogre::Root                  *mRoot;
  Ogre::RenderWindow          *mRenderWindow;
  Ogre::SceneManager          *mSceneManager;
  Ogre::Camera                *mCamera;
  Ogre::CompositorWorkspace   *mWorkspace;
  //  Ogre::String                mResourcePath;
  bool                        mShutDown;

  Ogre::SceneNode             *mSceneNode;

  Ogre::v1::OverlaySystem     *mOverlaySystem;

  //String : Plugin and Resources .config
  Ogre::String                mResourcesCfg;
  Ogre::String                mPluginsCfg;

  //std Cast ????
  std::vector<Ogre::Item*> mMeshes;

  //Functions control
  virtual void MoveCamera();
  Ogre::SceneNode * mNodeDLight;
  Ogre::SceneNode * mNodePLight;
};

#endif // #ifndef __OgreWindow_h_
