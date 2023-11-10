#pragma once

#include "buffer.h"


class SerialPort {
public:
	virtual void Start() =0;

	virtual void PutC(char c) =0;

	virtual bool Readable() =0;

	virtual char GetC() =0;

	virtual void RxCmpltCb() =0;

	virtual void TxCmpltCb() =0;
};
