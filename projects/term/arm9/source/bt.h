#ifndef _BT_H_
#define _BT_H_


typedef struct
{
	char addr[13];		// address (null-terminated string)
	char name[17];		// friendly name (as above)
	char cod[7];		// class of device (as above)
} bt_device;


/**
 *	connect to a device
 *
 *	@param addr		address (null-terminated string)
 *	@return			true if successful, false if not
 */
bool bt_connect(const char *addr);


/**
 *	return if the device is connected or not
 *
 *	@return			true if connected, false if not
 */
bool bt_connected();


/**
 *	disconnect from the current connection
 */
void bt_disconnect();


/**
 *	initiate a device scan
 *
 *	@param list		pointer that will hold the location of 
 *				the device list
 *	@param timeout		time to scan in seconds
 *	@return			number of devices found
 */
uint8 bt_scan(bt_device **list, uint8 timeout);


/**
 *	set the friendly name of the device
 *
 *	@param name		name string (up to 16 characters long)
 *	@return			true if successful, false if not
 */
bool bt_set_name(const char *name);


#endif	// _BT_H_
