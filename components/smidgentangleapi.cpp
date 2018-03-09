#include "smidgentangleapi.h"

SmidgenTangleAPI::SmidgenTangleAPI(QObject *parent) :
    AbstractTangleAPI(parent), m_currentRequest(RequestType::NO_REQUEST)
{

}

void SmidgenTangleAPI::startAPIRequest(RequestType request, const QStringList &argList)
{

}

void SmidgenTangleAPI::stopCurrentAPIRequest()
{
    if (m_currentRequest == RequestType::NO_REQUEST) return;

    //TODO kill process
}
