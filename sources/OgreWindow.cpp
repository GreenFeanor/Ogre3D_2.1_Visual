#include "OgreWindow.hpp"


OgreWindow::OgreWindow()
  : mRoot(0)
  , mCamera(0)
  , mSceneManager(0)
  , mResourcesCfg("")
  , mPluginsCfg("")
  , mSceneNode(0)
  , mShutDown(false)
{

}

OgreWindow::~OgreWindow()
{
  //  assert( !mRoot && "deinitialize() not called!!!" );
  //  Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
  windowClosed(mRenderWindow);
  delete mRoot;
}

void OgreWindow::go()
{
  mResourcesCfg = "resources.cfg";
  mPluginsCfg = "plugins.cfg";

  if (!setup())
    return;

  mRoot->startRendering();

  // Clean up
  destroyScene();
}

void OgreWindow::destroyScene()
{

}

bool OgreWindow::setup()
{
  mRoot = new Ogre::Root(mPluginsCfg);

  setupResources();
  initialize();
  loadResources();
  chooseSceneManager();
  createCamera();
  createScene();
  mWorkspace = setupCompositor();
  createFrameListener();

  return true;
}

void OgreWindow::setupResources()
{
  Ogre::ConfigFile cf;
  cf.load(mResourcesCfg);

  // Go through all sections & settings in the file
  Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

  Ogre::String secName, typeName, archName;
  while (seci.hasMoreElements())
  {
    secName = seci.peekNextKey();
    Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
    Ogre::ConfigFile::SettingsMultiMap::iterator i;
    for (i = settings->begin(); i != settings->end(); ++i)
    {
      typeName = i->first;
      archName = i->second;

      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            archName, typeName, secName);
    }
  }
}

void OgreWindow::initialize()
{

  mRoot = OGRE_NEW Ogre::Root (mPluginsCfg, "ogre.cfg", "Ogre.log");
  mRoot->loadPlugin("/usr/lib/OGRE/RenderSystem_GL3Plus.so");
  Ogre::RenderSystem *renderSystem = mRoot->getRenderSystemByName("OpenGL 3+ Rendering Subsystem");
  renderSystem->setConfigOption("FSAA", "4");
  renderSystem->setConfigOption("Full Screen", "No");
  renderSystem->setConfigOption("Video Mode", "1280 x 800 @ 32-bit colour");
  renderSystem->setConfigOption("VSync", "No");
  renderSystem->setConfigOption("RTT Preferred Mode", "FBO");


  //  mRoot->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes" );
  mRoot->setRenderSystem(renderSystem);
  mRoot->initialise(false);

  Ogre::ConfigOptionMap& cfgOpts = mRoot->getRenderSystem()->getConfigOptions();

  int width   = 1280;
  int height  = 720;

  Ogre::ConfigOptionMap::iterator opt = cfgOpts.find( "Video Mode" );
  if( opt != cfgOpts.end() )
  {
    //Get the width and height
    Ogre::String::size_type widthEnd = opt->second.currentValue.find(' ');
    // we know that the height starts 3 characters after the width and goes until the next space
    Ogre::String::size_type heightEnd = opt->second.currentValue.find(' ', widthEnd+3);
    // Now we can parse out the values
    width   = Ogre::StringConverter::parseInt( opt->second.currentValue.substr( 0, widthEnd ) );
    height  = Ogre::StringConverter::parseInt( opt->second.currentValue.substr(
                                                 widthEnd+3, heightEnd ) );
  }
  Ogre::NameValuePairList params;
  Ogre::String windowTitle = "OgreWindow";
  //  params["hidden"] = true;

  params.insert( std::make_pair("title", windowTitle) );
  params.insert( std::make_pair("gamma", "true") );
  params.insert( std::make_pair("FSAA", cfgOpts["FSAA"].currentValue) );
  params.insert( std::make_pair("vsync", cfgOpts["VSync"].currentValue) );

  mRenderWindow = Ogre::Root::getSingleton().createRenderWindow(windowTitle, width, height, false, &params);
}

void OgreWindow::loadResources()
{
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation("/home/guillaume/Bureau/Ogre2-1/build/models", "FileSystem","FileSysyem");
  registerHlms();
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void OgreWindow::chooseSceneManager()
{
  Ogre::InstancingThreadedCullingMethod threadedCullingMethod =
      Ogre::INSTANCING_CULLING_SINGLETHREAD;
  const size_t numThreads = std::max<size_t>( 1, Ogre::PlatformInformation::getNumLogicalCores() );

  mSceneManager = mRoot->createSceneManager( Ogre::ST_GENERIC,
                                             numThreads,
                                             threadedCullingMethod,
                                             "ExampleSMInstance" );
  //  mSceneManager->addRenderQueueListener( mOverlaySystem );
}

void OgreWindow::createCamera()
{
  mCamera = mSceneManager->createCamera( "Main Camera" );

  // Position it at 500 in Z direction
  mCamera->setPosition(Ogre::Vector3(20, 20, -35));
  // Look back along -Z
  mCamera->lookAt(Ogre::Vector3( 0, 7, 0 ));
  mCamera->setNearClipDistance( 1.0f );
  mCamera->setFarClipDistance( 100.0f );
  mCamera->setAutoAspectRatio( true );
}

void OgreWindow::createFrameListener()
{
  size_t windowHnd = 0;
  std::ostringstream windowHndStr;

  mRenderWindow->getCustomAttribute("WINDOW", &windowHnd);
  windowHndStr << windowHnd;

  mRoot->addFrameListener(this);
}

bool OgreWindow::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
  if(mRenderWindow->isClosed())
    return false;

  if(mShutDown)
    return false;
  //mCameraMan->frameRenderingQueued(evt);
  MoveCamera();
  return true;
}

void OgreWindow::MoveCamera()
{
  Ogre::Vector3 CamMovementDir;
  CamMovementDir = Ogre::Vector3(1, 0, 0);
  CamMovementDir.normalise();
  CamMovementDir *= 0.1f;
  mCamera->moveRelative(CamMovementDir);
  mCamera->lookAt(0, 8, 0);

  //  Ogre::Vector3 dLightMovementDir(1,0,0);
  //  dLightMovementDir.normalise();
  //  dLightMovementDir *= 0.1f;
  //  mNodePLight->setPosition(dLightMovementDir);
}

void OgreWindow::createScene()
{
  //  mSceneManager->setAmbientLight(Ogre::ColourValue::White, Ogre::ColourValue::Green, Ogre::Vector3(10.0, 10.0, 10.0));

  //First directional light
  Ogre::Light* light = mSceneManager->createLight();
  mNodeDLight = mSceneManager->getRootSceneNode()->createChildSceneNode();
  mNodeDLight->attachObject( light );
  light->setPowerScale(Ogre::Math::PI);
  light->setDiffuseColour( 0.8f, 0.8f, 1.0f );
  light->setSpecularColour( 0.5f, 0.5f, 1.0f );
  light->setType( Ogre::Light::LT_DIRECTIONAL );
  light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );
  light->setCastShadows(true);

  //Second Point light
  Ogre::Light* light2 = mSceneManager->createLight();
  mNodePLight = mSceneManager->getRootSceneNode( )->createChildSceneNode();
  mNodePLight->attachObject( light2 );
  light2->setDiffuseColour( 0.8f, 0.8f, 1.0f );
  light2->setSpecularColour( 0.5f, 0.5f, 1.0f );
  light2->setPowerScale(Ogre::Math::PI);
  light2->setCastShadows(false); //Not working
  light2->setType(Ogre::Light::LT_POINT);
  light2->setAttenuationBasedOnRadius( 10.0f, 0.01f );
  mNodePLight->setPosition(0,5,-8);

  //third Spot light
  Ogre::Light *light3 = mSceneManager->createLight();
  Ogre::SceneNode *nodeSpLight = mSceneManager->getRootSceneNode()->createChildSceneNode();
  nodeSpLight->attachObject(light3);
  light3->setDiffuseColour(0.0, 1.0, 0.0);
  light3->setSpecularColour(0.1, 0.9, 0.1);
  light3->setPowerScale(Ogre::Math::PI * 5.0);
  light3->setType(Ogre::Light::LT_SPOTLIGHT);
  light3->setAttenuationBasedOnRadius( 10.0f, 0.01f );
  light3->setSpotlightRange(Ogre::Degree(1), Ogre::Degree(40));
  light3->setCastShadows(true);
  nodeSpLight->setPosition(15, 5, 0);
  Ogre::Vector3 dir(8, 2, 0);
  dir.normalise();
  nodeSpLight->setDirection(dir);
//  nodeSpLight->lookAt(Ogre::Vector3(0,0,0), Ogre::Node::TS_WORLD);


  mSceneManager->setForward3D( true, 4,4,5,96,3,200 );

  Ogre::v1::MeshPtr v1Mesh;
  Ogre::MeshPtr v2Mesh;

  v1Mesh = Ogre::v1::MeshManager::getSingleton().load("ninja.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                      Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);
  v2Mesh = Ogre::MeshManager::getSingleton().createManual(
        "ninja.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

  v2Mesh->importV1( v1Mesh.get(), true, true, true);

  v1Mesh->unload();

  Ogre::Item* item = mSceneManager->createItem("ninja.mesh Imported",
                                               Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                               Ogre::SCENE_DYNAMIC);
  mSceneNode = mSceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
  mSceneNode->attachObject(item);
  mSceneNode->setScale(Ogre::Vector3(0.1));
  mSceneNode->setPosition(0, 0, 0);

  ////////////////////////////////////////////////////////PLANE///////////////////////////////////////////////////
  v1Mesh = Ogre::v1::MeshManager::getSingleton().load("damier_mesh.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                      Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);
  v2Mesh = Ogre::MeshManager::getSingleton().createManual(
        "damier_mesh.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

  v2Mesh->importV1( v1Mesh.get(), true, true, true);

  v1Mesh->unload();

  Ogre::Item* plane = mSceneManager->createItem("damier_mesh.mesh Imported",
                                                Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                Ogre::SCENE_DYNAMIC);
  Ogre::SceneNode *planeNode = mSceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
  planeNode->attachObject(plane);
  planeNode->scale(1.0, 1.0, 0.5);
//  planeNode->setOrientation(0.7, 0.7, 0, 0);

  //shadow
  mSceneManager->setShadowFarDistance(1000.0f);

}

Ogre::CompositorWorkspace* OgreWindow::setupCompositor()
{
  Ogre::CompositorManager2* pCompositorManager = mRoot->getCompositorManager2();
  const Ogre::String workspaceName = "ShadowMapDebuggingWorkspace";
  const Ogre::IdString workspaceNameHash("ShadowMapDebuggingWorkspace");

  if( !pCompositorManager->hasWorkspaceDefinition( workspaceNameHash ) )
  {
    pCompositorManager->createBasicWorkspaceDef( workspaceName, Ogre::ColourValue::Black,
                                                 Ogre::IdString() );
  }
  return pCompositorManager->addWorkspace(mSceneManager, mRenderWindow, mCamera, workspaceNameHash, true);

}

void OgreWindow::windowClosed(Ogre::RenderWindow* rw)
{
  // Only close for window that created OIS (the main window in these demos)
  //    if(rw == mRenderWindow)
  //    {
  //        if(mInputManager)
  //        {
  //            mInputManager->destroyInputObject(mMouse);
  //            mInputManager->destroyInputObject(mKeyboard);

  //            OIS::InputManager::destroyInputSystem(mInputManager);
  //            mInputManager = 0;
  //        }
  //    }
}

//test implementation HLMS
void OgreWindow::registerHlms(void)
{
  Ogre::Archive *archiveLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(
        "/usr/share/OGRE/Media/Hlms/Common/GLSL",
        "FileSystem", true );

  Ogre::ArchiveVec library;
  library.push_back( archiveLibrary );

  Ogre::Archive *archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load("/usr/share/OGRE/Media/Hlms/Unlit/GLSL",
                                                                              "FileSystem", true );

  Ogre::HlmsUnlit *hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit, &library );
  Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsUnlit );

  Ogre::Archive *archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load("/usr/share/OGRE/Media/Hlms/Pbs/GLSL",
                                                                            "FileSystem", true );
  Ogre::HlmsPbs *hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &library );
  Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
}
