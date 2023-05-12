//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "app/TaskGenerator.h"
#include "app/messages/LoadBalanceTimerMessage_m.h"

using namespace task_offloading;

void TaskGenerator::vehicleHandler()
{
    bool randomTimeReached = simTime() > par("randomTimeHelpMessage").doubleValue() + newRandomTime;
    bool isBus = findHost()->getIndex() == busIndex;
    bool moreDataToLoad = par("computationLoad").doubleValue() > 0;

    if (randomTimeReached && isBus && !(helpReceived) && !(sentHelpMessage) && moreDataToLoad) {
        // Help message creation
        HelpMessage* helpRequest = new HelpMessage();
        populateWSM(helpRequest);

        // Fill the map with BUS vehicle info
        double busLoad = par("randomVehicleLoadActual").doubleValue() * par("busVehicleLoad").doubleValue();
        double busFreq = par("randomCpuVehicleFreq").doubleValue();
        std::string hostBUSIndex = "node0";
        helpers[busIndex] = HelperVehicleInfo(hostBUSIndex, busLoad, busFreq, busIndex);
        helpers[busIndex].setVehicleAngle(traciVehicle->getAngle());

        // Color the bus
        findHost()->getDisplayString().setTagArg("i", 1, "red");

        // Fill the data of the help request message
        helpRequest->setVehicleIndex(findHost()->getIndex());
        helpRequest->setMinimumLoadRequested(par("minimumVehicleLoadActual").doubleValue());

        // Send the help message
        sendDown(helpRequest);

        // Emit the signal that help requested has been sent
        emit(startHelp, simTime());

        // Send statistics for the start of the task
        emit(startTask, simTime());

        // Schedule timer for the help request
        LoadBalanceTimerMessage* loadBalanceMsg = new LoadBalanceTimerMessage();
        populateWSM(loadBalanceMsg);
        loadBalanceMsg->setSimulationTime(simTime());
        scheduleAt(simTime() + par("busWaitingTime").doubleValue(), loadBalanceMsg);

        sentHelpMessage = true;
    } else if (!moreDataToLoad) {
        findHost()->getDisplayString().setTagArg("i", 1, "white");
    }
}
