#pragma once

#include "stm32f4xx_hal.h"
#include <string.h>
#include <functional>

template<class T, size_t size>
class CircularBuffer {
public:
	void Put(T item) {
		buf_[wloc_] = item;
		__DSB();
		wloc_ = (wloc_ + 1) % (size - 1);
	}

	T Get() {
		T item = buf_[rloc_];
		__DSB();
		rloc_ = (rloc_ + 1) % (size - 1);
		return item;
	}

	void Reset() {
		wloc_ = 0;
		rloc_ = 0;
	}

	bool Readable() {
		return wloc_ != rloc_;
	}

private:
	T buf_[size];
	volatile size_t wloc_;
	volatile size_t rloc_;

};

template<class T, size_t size>
class InterruptTxBuffer {
public:

	InterruptTxBuffer(std::function<void(T*, size_t)> transmit_function) :
			transmit_function_(transmit_function) {
	}

	void Put(T item) {
		loading_ = true;
		__DSB();
		buf_[wloc_] = item;
		__DSB();
		wloc_ = (wloc_ + 1) % (size - 1);
		loading_ = false;
		if (!transmitting_) {
			StartTransfer();
		}
	}

	bool Full() {
		return wloc_ + size;
	}

	void Reset() {
		wloc_ = 0;
	}

	void StartTransfer() {
		if (!loading_ && wloc_) {
			memcpy(transmitbuf_, buf_, wloc_);
			transmit_function_(transmitbuf_, wloc_);
			transmitting_ = true;
			//__DSB();
			wloc_ = 0;
		} else {
			transmitting_ = false;
		}
	}

private:
	T buf_[size];
	T transmitbuf_[size];

	std::function<void(T*, size_t)> transmit_function_;

	volatile size_t wloc_ { };
	bool transmitting_ { };
	bool loading_ { };
};

template<class T, size_t size>
class InterruptRxCircularBuffer {
public:
	InterruptRxCircularBuffer(std::function<void(T*)> enable_rx_interrupt) :
			enable_rx_interrupt_(enable_rx_interrupt) {
	}

	void Start() {
		Prime();
	}

	T Get() {
		return buffer.Get();
	}

	void Reset() {
		buffer.Reset();
	}

	bool Readable() {
		return buffer.Readable();
	}

	void Put() {
		buffer.Put(incoming_byte_);
		Prime();
	}

private:
	T incoming_byte_ { };
	CircularBuffer<T, size> buffer;

	void Prime() {
		enable_rx_interrupt_(&incoming_byte_);
	}

	std::function<void(T*)> enable_rx_interrupt_;

};

