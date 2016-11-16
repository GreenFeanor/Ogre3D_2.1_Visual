#include "QTOgreWindow.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QTOgreWindow* ogreWindow = new QTOgreWindow();
  ogreWindow->show();

  return app.exec();
}

