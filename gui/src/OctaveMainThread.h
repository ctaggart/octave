#ifndef OCTAVEMAINTHREAD_H
#define OCTAVEMAINTHREAD_H

#include <QThread>
class OctaveMainThread:public QThread
{
  Q_OBJECT
public:
  OctaveMainThread (QObject * parent);

signals:
  void ready();

protected:
  void run ();
};

#endif // OCTAVEMAINTHREAD_H
