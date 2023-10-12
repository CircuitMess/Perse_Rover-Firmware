#ifndef PERSE_ROVER_DEVICECONTROLLER_H
#define PERSE_ROVER_DEVICECONTROLLER_H

#include <optional>
#include <functional>
#include <string>
#include "Util/Events.h"
#include "Util/Threaded.h"

enum DeviceControlType{
    Remote,
    Local,
};

template <typename T>
class DeviceController{
public:
    explicit DeviceController(const std::string& name) : control(Remote), eventQueue(10), dcListenThread(std::function(std::bind(&DeviceController::processCommandQueue, this)), name.c_str()){
        Events::listen(Facility::Comm, &eventQueue);
    }

    virtual ~DeviceController(){
        dcListenThread.stop();
        Events::unlisten(&eventQueue);
    }

    inline void setControl(DeviceControlType value){
        if (control == Local && value == Remote){
            const T& state = getDefaultState();
            if (queuedState.has_value()){
                state = queuedState;
                queuedState.reset();
            }

            write(state);
            sendState(state);
        }

        control = value;
    }

    inline void setLocally(const T& state){
        if (control == Remote){
            return;
        }

        write(state);
    }

protected:
    virtual void write(const T& state) = 0;
    virtual T getDefaultState() const = 0;
    virtual void sendState(const T& state) const = 0;
    virtual T processStateFromEvent(const Event& event) const = 0;

    inline void setRemotely(const T& state){
        if (control == Local){
            queuedState = state;
            return;
        }

        write(state);
        sendState(state);
    }

private:
    DeviceControlType control;
    std::optional<T> queuedState;
    EventQueue eventQueue;
    ThreadedClosure dcListenThread;

private:
    void processCommandQueue(){
        Event event = {};
        if (!eventQueue.get(event, portMAX_DELAY))
        {
            vTaskDelay(1);
            return;
        }

        setRemotely(processStateFromEvent(event));
    }
};

#endif //PERSE_ROVER_DEVICECONTROLLER_H
