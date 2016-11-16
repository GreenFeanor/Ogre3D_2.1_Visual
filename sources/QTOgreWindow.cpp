#include "QTOgreWindow.hpp"

Ogre::NameGenerator QTOgreWindow::ng("COMP");

QTOgreWindow::QTOgreWindow(QWindow *parent)
  : QWindow(parent)
  , m_update_pending(false)
  , m_animating(false)
  , m_ogreRoot(NULL)
  , m_ogreWindow(NULL)
  , m_ogreCamera(NULL)
  , m_resourcesCfg("")
  , m_pluginsCfg("")
  , m_cameraMan(0)
  , renderTexture_(NULL)
{
  setAnimating(true);
  installEventFilter(this);
  m_ogreBackground = Ogre::ColourValue(0.0, 0.0, 0.0);
}

QTOgreWindow::~QTOgreWindow()
{
  delete m_ogreRoot;
}

void QTOgreWindow::renderNow()
{
  if (!isExposed())
    return;
  if(m_ogreRoot == NULL)
    setup();
  render();

  if(m_animating)
    renderLater();
}

bool QTOgreWindow::setup()
{
  m_resourcesCfg = "resources.cfg";
  m_pluginsCfg = "plugins.cfg";

  m_ogreRoot = new Ogre::Root(m_pluginsCfg);

  setupResources(); // Défini les ressources
  initialize();     // Configuration manuelle du RenderSystem
  loadResources();  // Chargement des ressources définies
  chooseSceneManager(); // Création du scene Manager
  createCamera();   // Création de la camera
  createScene();    // Ajout du contenu dans la scene
  mWorkspace = setupCompositor(); //
  //renderTexture();
  createFrameListener();

  return true;
}

void QTOgreWindow::setupResources()
{
  Ogre::ConfigFile cf;
  cf.load(m_resourcesCfg);

  // Go through all sections & settings in the file
  Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

  Ogre::String secName, typeName, archName;
  while (seci.hasMoreElements())
  {
    secName = seci.peekNextKey();
    Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
    Ogre::ConfigFile::SettingsMultiMap::iterator i;

    // Charges les ressources une a une
    for (i = settings->begin(); i != settings->end(); ++i)
    {
      typeName = i->first;
      archName = i->second;

      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            archName, typeName, secName);
    }

  }
}

void QTOgreWindow::initialize()
{
  m_ogreRoot = OGRE_NEW Ogre::Root (m_pluginsCfg, "ogre.cfg", "Ogre.log");
  m_ogreRoot->loadPlugin("/usr/lib/OGRE/RenderSystem_GL3Plus.so");
  Ogre::RenderSystem *renderSystem = m_ogreRoot->getRenderSystemByName("OpenGL 3+ Rendering Subsystem");
  renderSystem->setConfigOption("FSAA", "4");
  renderSystem->setConfigOption("Full Screen", "No");
  renderSystem->setConfigOption("Video Mode", "1280 x 800 @ 32-bit colour");
  renderSystem->setConfigOption("VSync", "No");
  renderSystem->setConfigOption("RTT Preferred Mode", "FBO");

  m_ogreRoot->setRenderSystem(renderSystem);
  m_ogreRoot->initialise(false);

  Ogre::ConfigOptionMap& cfgOpts = m_ogreRoot->getRenderSystem()->getConfigOptions();

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
  params["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(this->winId()));
  params["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(this->winId()));

  params.insert( std::make_pair("title", windowTitle) );
  params.insert( std::make_pair("gamma", "false") );
  params.insert( std::make_pair("FSAA", cfgOpts["FSAA"].currentValue) );
  params.insert( std::make_pair("vsync", cfgOpts["VSync"].currentValue) );

  m_ogreWindow = Ogre::Root::getSingleton().createRenderWindow(windowTitle, width, height, false, &params);
}

void QTOgreWindow::loadResources()
{
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation("/home/yann/Project/Ogre_21_Labo/build/models/", "FileSystem","FileSysyem");
  registerHlms();
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void QTOgreWindow::registerHlms(void)
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
  hlmsPbs->setShadowSettings(Ogre::HlmsPbs::PCF_2x2);
}

Ogre::CompositorWorkspace* QTOgreWindow::setupCompositor()
{

  Ogre::String name = ng.generate();
  Ogre::IdString idName = name;
  Ogre::CompositorManager2* CompositorManager = m_ogreRoot->getCompositorManager2();
  Ogre::CompositorWorkspaceDef * compositorWS = CompositorManager->addWorkspaceDefinition(idName);
  compositorWS->connectOutput("ShadowMapDebuggingRenderingNode", 0);
  return CompositorManager->addWorkspace(m_ogreSceneMgr, m_ogreWindow, m_ogreCamera, idName, true);
}

void QTOgreWindow::chooseSceneManager()
{
  Ogre::InstancingThreadedCullingMethod threadedCullingMethod =
      Ogre::INSTANCING_CULLING_SINGLETHREAD;
  const size_t numThreads = std::max<size_t>( 1, Ogre::PlatformInformation::getNumLogicalCores() );

  m_ogreSceneMgr = m_ogreRoot->createSceneManager( Ogre::ST_GENERIC,
                                                   numThreads,
                                                   threadedCullingMethod,
                                                   "ExampleSMInstance" );
}

void QTOgreWindow::createCamera()
{
  m_ogreCamera = m_ogreSceneMgr->createCamera( "Main Camera" );

  // Position it at 500 in Z direction
  m_ogreCamera->setPosition(Ogre::Vector3(20, 20, -35));
  // Look back along -Z
  m_ogreCamera->lookAt(Ogre::Vector3( 0, 7, 0 ));
  m_ogreCamera->setNearClipDistance( 1.0f );
  m_ogreCamera->setFarClipDistance( 100000.0f );
  m_ogreCamera->setAutoAspectRatio( true );
  //m_ogreCamera->getSceneManager()->getRootSceneNode()
  m_cameraMan = new OgreQtBites::SdkQtCameraMan(m_ogreCamera);
}

void QTOgreWindow::createScene()
{
  //First directional light
  Ogre::Light* light = m_ogreSceneMgr->createLight();
  Ogre::SceneNode *NodeDLight = m_ogreSceneMgr->getRootSceneNode()->createChildSceneNode();
  NodeDLight->attachObject( light );
  light->setPowerScale(Ogre::Math::PI);
  light->setDiffuseColour( 0.8f, 0.8f, 1.0f );
  light->setSpecularColour( 0.5f, 0.5f, 1.0f );
  light->setType( Ogre::Light::LT_DIRECTIONAL );
  light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

  //Second Point light
  Ogre::Light* light2 = m_ogreSceneMgr->createLight();
  Ogre::SceneNode *nodePoLight = m_ogreSceneMgr->getRootSceneNode( )->createChildSceneNode();
  nodePoLight->attachObject( light2 );
  light2->setDiffuseColour( 0.8f, 0.8f, 1.0f );
  light2->setSpecularColour( 0.5f, 0.5f, 1.0f );
  light2->setPowerScale(Ogre::Math::PI);
  light2->setCastShadows(false); //Not working
  light2->setType(Ogre::Light::LT_POINT);
  light2->setAttenuationBasedOnRadius( 10.0f, 0.01f );
  nodePoLight->setPosition(0,5,-8);

  //third Spot light
  Ogre::Light *light3 = m_ogreSceneMgr->createLight();
  Ogre::SceneNode *nodeSpLight = m_ogreSceneMgr->getRootSceneNode()->createChildSceneNode();
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


  m_ogreSceneMgr->setForward3D( true, 4,4,5,96,3,200 );

  Ogre::v1::MeshPtr v1Mesh;
  Ogre::MeshPtr v2Mesh;



  /////////////////////////////////////////////// ??? ///////////////////////////////////////////////////////
  ///-----------------------------------------------------------------------------------------------------///
  v1Mesh = Ogre::v1::MeshManager::getSingleton().load("Tractor_model.mesh",
                                                      Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                      Ogre::v1::HardwareBuffer::HBU_STATIC,
                                                      Ogre::v1::HardwareBuffer::HBU_STATIC);

  v2Mesh = Ogre::MeshManager::getSingleton().createManual(
        "Tractor_model.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

  v2Mesh->importV1( v1Mesh.get(), true, true, true);

  v1Mesh->unload();

  Ogre::Item* Tractor = m_ogreSceneMgr->createItem("Tractor_model.mesh Imported",
                                                      Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                      Ogre::SCENE_DYNAMIC);

  Ogre::SceneNode *TractorNode = m_ogreSceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
  TractorNode->attachObject(Tractor);
  TractorNode->setPosition(10, 10, 10);
  TractorNode->scale(Ogre::Vector3(0.07));





  ////////////////////////////////////////////////////////NINJA///////////////////////////////////////////////////
  ///----------------------------------------------------------------------------------------------------------///

 /* v1Mesh = Ogre::v1::MeshManager::getSingleton().load("ninja.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                      Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);



  v2Mesh = Ogre::MeshManager::getSingleton().createManual(
        "ninja.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

  v2Mesh->importV1( v1Mesh.get(), true, true, true);

  v1Mesh->unload();

  Ogre::Item* item = m_ogreSceneMgr->createItem("ninja.mesh Imported",
                                                Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                Ogre::SCENE_DYNAMIC);
  Ogre::SceneNode *SceneNode = m_ogreSceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
  SceneNode->attachObject(item);
  SceneNode->setScale(Ogre::Vector3(0.1));
  SceneNode->setPosition(0, 0, 0);
*/

  ////////////////////////////////////////////////////////PLANE///////////////////////////////////////////////////
  ///----------------------------------------------------------------------------------------------------------///

  v1Mesh = Ogre::v1::MeshManager::getSingleton().load("damier_mesh.mesh",
                                                      Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                      Ogre::v1::HardwareBuffer::HBU_STATIC,
                                                      Ogre::v1::HardwareBuffer::HBU_STATIC);

  v2Mesh = Ogre::MeshManager::getSingleton().createManual(
        "damier_mesh.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

  v2Mesh->importV1( v1Mesh.get(), true, true, true);

  v1Mesh->unload();

  Ogre::Item* plane = m_ogreSceneMgr->createItem("damier_mesh.mesh Imported",
                                                 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                 Ogre::SCENE_DYNAMIC);
  Ogre::SceneNode *planeNode = m_ogreSceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
  planeNode->attachObject(plane);
  planeNode->scale(1.0, 1.0, 0.5);
  //  planeNode->setOrientation(0.7, 0.7, 0, 0);



  ///////////////////////////////////////////////CAISSE///////////////////////////////////////////////////////////
  ///----------------------------------------------------------------------------------------------------------///
/*
  v1Mesh = Ogre::v1::MeshManager::getSingleton().load("visuel_caisse.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                      Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);
  v2Mesh = Ogre::MeshManager::getSingleton().createManual(
        "visuel_caisse.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

  v2Mesh->importV1( v1Mesh.get(), true, true, true);

  v1Mesh->unload();

  Ogre::Item* caseVisual = m_ogreSceneMgr->createItem("visuel_caisse.mesh Imported",
                                                      Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                      Ogre::SCENE_DYNAMIC);
  Ogre::SceneNode *caseNode = m_ogreSceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
  caseNode->attachObject(caseVisual);
  caseNode->setPosition(5, 2, 0);
  caseNode->scale(10.0, 10.0, 10.0);
    //planeNode->setOrientation(0.7, 0.7, 0, 0);

*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



  //shadow
  m_ogreSceneMgr->setShadowFarDistance(1000.0f);

  ////////////////////////////////////////////////RENDERTARGET//////////////////////////////////////////////////

}

void QTOgreWindow::createFrameListener()
{
  m_ogreRoot->addFrameListener(this);
}

void QTOgreWindow::render(QPainter *painter)
{
  Q_UNUSED(painter);
}

void QTOgreWindow::render()
{
  Ogre::WindowEventUtilities::messagePump();
  m_ogreRoot->renderOneFrame();
}

void QTOgreWindow::renderLater()
{
  if (!m_update_pending)
  {
    m_update_pending = true;
    QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
  }
}

bool QTOgreWindow::event(QEvent *event)
{
  switch (event->type())
  {
  case QEvent::UpdateRequest:
    m_update_pending = false;
    renderNow();
    return true;

  default:
    return QWindow::event(event);
  }
}

void QTOgreWindow::exposeEvent(QExposeEvent *event)
{
  Q_UNUSED(event);

  if (isExposed())
    renderNow();
}

void QTOgreWindow::setAnimating(bool animating)
{
  m_animating = animating;

  if (animating)
    renderLater();
}

///////////////////////CMD////////////////////////////
/////////////////////////////////////////////////////
void QTOgreWindow::keyPressEvent(QKeyEvent * ev)
{
  if(m_cameraMan)
    m_cameraMan->injectKeyDown(*ev);
}

void QTOgreWindow::keyReleaseEvent(QKeyEvent * ev)
{
  if(m_cameraMan)
    m_cameraMan->injectKeyUp(*ev);
}

void QTOgreWindow::mouseMoveEvent( QMouseEvent* e )
{
  static int lastX = e->x();
  static int lastY = e->y();
  int relX = e->x() - lastX;
  int relY = e->y() - lastY;
  lastX = e->x();
  lastY = e->y();

  if(m_cameraMan && (e->buttons() & Qt::LeftButton))
    m_cameraMan->injectMouseMove(relX, relY);
}

void QTOgreWindow::wheelEvent(QWheelEvent *e)
{
  if(m_cameraMan)
    m_cameraMan->injectWheelMove(*e);
  //  Ogre::Hlms *hlms = m_ogreRoot->getHlmsManager()->getHlms( Ogre::HLMS_PBS );

  //  assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
  //  Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );

  //  pbs->setShadowSettings( static_cast<Ogre::HlmsPbs::ShadowFilter>(
  //                            (pbs->getShadowFilter() + 1) %
  //                            Ogre::HlmsPbs::NumShadowFilter ) );

  renderTexture_->writeContentsToFile("test.png");
}

void QTOgreWindow::mousePressEvent( QMouseEvent* e )
{
  if(m_cameraMan)
    m_cameraMan->injectMouseDown(*e);
}

bool QTOgreWindow::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
  m_cameraMan->frameRenderingQueued(evt);

  return true;
}

void QTOgreWindow::log(Ogre::String msg)
{
  if(Ogre::LogManager::getSingletonPtr() != NULL) Ogre::LogManager::getSingletonPtr()->logMessage(msg);
}

void QTOgreWindow::log(QString msg)
{
  log(Ogre::String(msg.toStdString().c_str()));
}

//resized qWindow
bool QTOgreWindow::eventFilter(QObject *target, QEvent *event)
{
  if (target == this)
  {
    if (event->type() == QEvent::Resize)
    {
      if (isExposed() && m_ogreWindow != NULL)
      {
        m_ogreWindow->resize(this->width(), this->height());
      }
    }
  }

  return false;
}

void QTOgreWindow::mouseReleaseEvent( QMouseEvent* e )
{
  if(m_cameraMan)
    m_cameraMan->injectMouseUp(*e);
  QPoint pos = e->pos();
  Ogre::Ray mouseRay = m_ogreCamera->getCameraToViewportRay(
        (Ogre::Real)pos.x() / m_ogreWindow->getWidth(),
        (Ogre::Real)pos.y() / m_ogreWindow->getHeight());
  Ogre::RaySceneQuery* pSceneQuery = m_ogreSceneMgr->createRayQuery(mouseRay);
  pSceneQuery->setSortByDistance(true);
  Ogre::RaySceneQueryResult vResult = pSceneQuery->execute();
  for (size_t ui = 0; ui < vResult.size(); ui++)
  {
    if (vResult[ui].movable)
    {
      if (vResult[ui].movable->getMovableType().compare("Entity") == 0)
      {
        emit entitySelected((Ogre::Item*)vResult[ui].movable);
      }
    }
  }
  m_ogreSceneMgr->destroyQuery(pSceneQuery);
}

void QTOgreWindow::renderTexture()
{
  texture_ = Ogre::TextureManager::getSingleton().createManual("RttTex",
                                                               Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                               Ogre::TEX_TYPE_2D,
                                                               1920, 1080,
                                                               0,
                                                               Ogre::PF_BYTE_BGR,
                                                               Ogre::TU_RENDERTARGET, 0, false, 16);
  renderTexture_ = texture_->getBuffer()->getRenderTarget();

  /*Cam_ = m_ogreSceneMgr->createCamera( "targetCam" );

  // Position it at 500 in Z direction
  Cam_->setPosition(Ogre::Vector3(-20, 10, 35));
  // Look back along -Z
  Cam_->lookAt(Ogre::Vector3( 0, 7, 0 ));
  Cam_->setNearClipDistance( 1.0f );
  Cam_->setFarClipDistance( 100000.0f );
  Cam_->setAutoAspectRatio( true );*/
  ///////////////////////////////////////////////////////////////////////////////////
  /*const Ogre::String name = ng.generate();
  const Ogre::IdString idName = name;
  //  Ogre::SceneManager *temp = m_ogreSceneMgr;

  Ogre::CompositorManager2* compositorManager = m_ogreRoot->getCompositorManager2();
  Ogre::CompositorWorkspaceDef * compositorWS = compositorManager->addWorkspaceDefinition(idName);
  compositorWS->connectOutput("ShadowMapDebuggingRenderingNode", 0);
  compositorManager->addWorkspace(m_ogreSceneMgr, renderTexture_, Cam_, idName, true);
*/
  ////////////////////////////////////////////////////////////////////////////////
}
