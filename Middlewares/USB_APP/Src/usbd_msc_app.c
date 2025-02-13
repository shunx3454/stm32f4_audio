#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_storage.h"

USBD_HandleTypeDef USBD_Device;

void usbd_msc_init(void)
{
    /* Init MSC Application */
    USBD_Init(&USBD_Device, &MSC_Desc, 0);

    /* Add Supported Class */
    USBD_RegisterClass(&USBD_Device, USBD_MSC_CLASS);

    /* Add Storage callbacks for MSC Class */
    USBD_MSC_RegisterStorage(&USBD_Device, &USBD_DISK_fops);

    /* Start Device Process */
    USBD_Start(&USBD_Device);
}