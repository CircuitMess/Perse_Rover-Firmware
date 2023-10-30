#ifndef PERSE_ROVER_DEVICECONTROLLER_H
#define PERSE_ROVER_DEVICECONTROLLER_H

#include <optional>
#include <functional>
#include <string>
#include "Util/Events.h"
#include "Util/Threaded.h"
#include "Services/TCPServer.h"

enum DeviceControlType{
	Remote,
	Local,
};

template <typename T>
class DeviceController{
public:
	explicit DeviceController(const std::string& name) : control(Remote), eventQueue(10), dcListenThread(std::function([this]() {this->processCommandQueue();}), name.c_str(), 4 * 1024){
		Events::listen(Facility::Comm, &eventQueue);
		Events::listen(Facility::TCP, &eventQueue);
		dcListenThread.start();
	}

	virtual ~DeviceController(){
		dcListenThread.stop();
		Events::unlisten(&eventQueue);
	}

	inline void setControl(DeviceControlType value){
		if (control == Local && value == Remote){
			T state = getDefaultState();
			if (queuedState.has_value()){
				state = queuedState.value();
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
	virtual void processEvent(const Event& event) = 0;

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
			return;
		}

		if (event.facility == Facility::TCP) {
			if (auto* tcpEvent = (TCPServer::Event*)event.data) {
				if (tcpEvent->status == TCPServer::Event::Status::Disconnected) {
					write(getDefaultState());
					setControl(DeviceControlType::Remote);
				}
			}
		}
		else if (event.facility == Facility::Comm) {
			processEvent(event);
		}

		free(event.data);
	}
};

#endif //PERSE_ROVER_DEVICECONTROLLER_H
