#ifndef COPROCESSOR_H
#define COPROCESSOR_H

#include <QObject>
#include "display_interface.h"

class Coprocessor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasCoprocessor READ hasCoprocessor)
   public:
    Coprocessor(QObject *parent = 0);
    ~Coprocessor();

    bool hasCoprocessor() const;

   private:
    org::asteroid::display *display;
};

#endif  // COPROCESSOR_H