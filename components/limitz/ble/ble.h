#pragma once

#include <stddef.h>


typedef union
{
	uint16_t u16;
	uint32_t u32;
	uint8_t  u128[16];
} ble_uuid_t;

typedef uint8_t ble_address_t[6];

typedef struct _ble_attr_t
{
	const char* name; //optionally set in code

	ble_uuid_t* datatype,;
	size_t 	    datasize;
	void*       data;
	struct _ble_attr_t* _next;
	struct _ble_attr_t* _child;

} ble_attribute_t;

typedef struct
{
	const char* name; //optionally set in code
	ble_uuid_t  uuid;

	size_t datasize;
	void*  data;

} ble_characteristic_t;

typedef struct _ble_service_t
{
	const char*         name; //optionally set in code

	ble_uuid_t          uuid;
	ble_characteristic* characteristics;	
	struct _ble_service_t* _next;
} ble_service_t;

typedef struct
{
	char* name;


	ble_address_t  address;
	ble_service_t* services;

} ble_peripheral_t;

typedef struct
{

} ble_central_t;
