#include <QCoreApplication>

#include "activityjumper.hpp"

int main(int ac, char **av)
{
  QCoreApplication a(ac, av);

  ActivityJumper c(&a);

  return a.exec();
}
