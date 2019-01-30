#ifndef _CFilterDevice_H
#define _CFilterDevice_H

#include "cpplib.h"
#include "CErrorStatus.h"
#include "CAttachDevice.h"

class CFilterDevice;
class CIrp;

typedef CErrorStatus ( NTAPI *IrpDispatchFunc)(
									CFilterDevice * FilterDevice,
									CIrp&Irp,
									void* RefPtr, 
									BOOL& IsIrpCompleted);

class CFilterDevice
{
public:
	CFilterDevice(void);
	~CFilterDevice(void);
	
public:
	CErrorStatus Create( PDRIVER_OBJECT Driver );

	CErrorStatus 
		Init(	PDRIVER_OBJECT			Driver,
				PUNICODE_STRING			Registry,
				const UINT32			DeviceExtSize
				);

	CErrorStatus Create(
					PDRIVER_OBJECT			DriverObject,
					const UINT32			DeviceExtSize,
					const WCHAR*			DeviceName, 
					DEVICE_TYPE				DeviceType, 
					UINT32				DeviceChars, 
					BOOL					Exclusive, 
					IrpDispatchFunc			DispatchFunc, 
					void					*RefPtr);

	CErrorStatus	Link( const WCHAR* LinkName);
	CErrorStatus	Unlink();
	void			Delete();
 
	PDEVICE_OBJECT
	Device();


	static 
	CAttachDevice&  
	AttachDevice();

	PDRIVER_OBJECT  DriverObject() const {return mDriverObject;};

	operator PDEVICE_OBJECT() {return mDeviceObject;};
	operator const PDEVICE_OBJECT() const {return mDeviceObject;};
	BOOL			IsAttached() const{return mIsAttached;};
	BOOL			WeCreated() const {return mWeCreated;};
	BOOL			WeLinked()	const {return mWeLinked;};

	CErrorStatus ToAttachDevice( WCHAR* DeviceName );

	CErrorStatus
		DispatchIrp( 
		CIrp& Irp,
		BOOL& IsIrpCompleted );

private:
	/*
	* ...�豸����...
	*/
	DEVICE_OBJECT*	mDeviceObject;

	/*
	* ...��������...
	*/
	DRIVER_OBJECT*	mDriverObject;

	/*
	Attach�豸
	*/
	CAttachDevice mAttachDevice;

	/*
	* ...�豸����(DeviceName)...
	*/
	UNICODE_STRING	mDeviceName;

	/*
	* ...�豸��������(LinkName)...
	*/
	UNICODE_STRING	mDeviceLinkName;

	/*
	* ...�����豸...
	*/
	void*			mRefPtr;

	/*
	* ...�豸�Ƿ񱻸���Attached...
	*/
	BOOL			mIsAttached;

	/*
	* ...�豸�Ƿ񱻴���Created...
	*/
	BOOL			mWeCreated;

	/*
	* ...�豸�Ƿ�����Linked...
	*/
	BOOL			mWeLinked;

	/*
	*
	*/
	BOOL			mIsRelease;

	/*
	* ...�豸�������豸...
	*/

	PFILE_OBJECT	mFileObjectToUse;
	IrpDispatchFunc	mDispatchFunc;

	static WCHAR* sDeviceName;
	static WCHAR* sLinkName;

};

#endif