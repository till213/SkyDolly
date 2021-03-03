#ifndef CSVEXPORT_H
#define CSVEXPORT_H

class QIODevice;

class Aircraft;
class CSVExportPrivate;

#include "../KernelLib.h"

class CSVExport
{
public:
    KERNEL_API CSVExport();
    KERNEL_API ~CSVExport();

    KERNEL_API bool exportData(const Aircraft &aircraft, QIODevice &io) const;

private:
    CSVExportPrivate *d;
};

#endif // CSVEXPORT_H
