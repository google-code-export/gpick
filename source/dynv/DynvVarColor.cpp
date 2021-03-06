/*
 * Copyright (c) 2009-2012, Albertas Vyšniauskas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of the software author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "DynvVarColor.h"
#include "DynvVariable.h"
#include "DynvIO.h"
#include "../Endian.h"
#include <string.h>

#include <sstream>
using namespace std;

static int dynv_var_color_create(struct dynvVariable* variable){
	if ((variable->ptr_value = new float[4])){
		return 0;
	}
	return -1;
}

static int dynv_var_color_destroy(struct dynvVariable* variable){
	if (variable->ptr_value){
		delete [] (float*)variable->ptr_value;
		return 0;
	}
	return -1;
}

static int dynv_var_color_set(struct dynvVariable* variable, void* value, bool deref){
	if (!variable->ptr_value) return -1;
	memcpy(variable->ptr_value, *(void**)value, sizeof(float[4]));

	/*((float*)variable->ptr_value)[0] = ((float*)value)[0];
	((float*)variable->ptr_value)[1] = ((float*)value)[1];
	((float*)variable->ptr_value)[2] = ((float*)value)[2];
	((float*)variable->ptr_value)[3] = ((float*)value)[3];*/
	return 0;
}

static int dynv_var_color_get(struct dynvVariable* variable, void** value, bool *deref){
	if (variable->ptr_value){
		*value = &variable->ptr_value;
		return 0;
	}
	return -1;
}

static int dynv_var_color_serialize(struct dynvVariable* variable, struct dynvIO* io){
	if (!variable->ptr_value) return -1;
	uint32_t written;

	uint32_t length=16;
	length=UINT32_TO_LE(length);

	dynv_io_write(io, &length, 4, &written);

	uint32_t value[4];
	memcpy(value, variable->ptr_value, 16);
	value[0]=UINT32_TO_LE(value[0]);
	value[1]=UINT32_TO_LE(value[1]);
	value[2]=UINT32_TO_LE(value[2]);
	value[3]=UINT32_TO_LE(value[3]);

	if (dynv_io_write(io, value, 16, &written)==0){
		if (written!=16) return -1;
	}else return -1;
	return 0;
}

static int dynv_var_color_deserialize(struct dynvVariable* variable, struct dynvIO* io){
	if (!variable->ptr_value) return -1;

	uint32_t read;
	uint32_t length;
	uint32_t value[4];
	dynv_io_read(io, &length, 4, &read);

	if (dynv_io_read(io, value, 16, &read)==0){
		if (read==16){
			value[0]=UINT32_FROM_LE(value[0]);
			value[1]=UINT32_FROM_LE(value[1]);
			value[2]=UINT32_FROM_LE(value[2]);
			value[3]=UINT32_FROM_LE(value[3]);
			memcpy(variable->ptr_value, value, 16);
			return 0;
		}
	}
	return -1;
}

static int serialize_xml(struct dynvVariable* variable, ostream& out){
	if (variable->ptr_value){
		float* color = (float*)variable->ptr_value;
		out << color[0] <<" "<< color[1] <<" "<< color[2] <<" "<< color[3];
	}
	return 0;
}

static int deserialize_xml(struct dynvVariable* variable, const char *data){
	stringstream ss(stringstream::in);
	ss.str(data);
	float *c = (float*)variable->ptr_value;
	ss >> c[0] >> c[1] >> c[2] >> c[3];
	return 0;
}

struct dynvHandler* dynv_var_color_new(){
	struct dynvHandler* handler=dynv_handler_create("color");

	handler->create=dynv_var_color_create;
	handler->destroy=dynv_var_color_destroy;
	handler->set=dynv_var_color_set;
	handler->get=dynv_var_color_get;
	handler->serialize=dynv_var_color_serialize;
	handler->deserialize=dynv_var_color_deserialize;

	handler->serialize_xml=serialize_xml;
	handler->deserialize_xml=deserialize_xml;

	handler->data_size = sizeof(float*);

	return handler;
}
