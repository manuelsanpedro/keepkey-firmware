#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/stm32/rcc.h>

#include <keepkey_board.h>

/**
 * These are the addresses assigned to the USB interface.  These
 * are from the perspective of the host, so ENDPOINT_ADDRESS_IN is the 
 * KeepKey endpoint, and ENDPOINT_ADDRESS_OUT is the Host endpoint.
 */
#define ENDPOINT_ADDRESS_IN         (0x81)
#define ENDPOINT_ADDRESS_OUT        (0x01)

/**
 * Control buffer for use by the USB stack.  We just allocate the
 * space for it.
 */
#define USBD_CONTROL_BUFFER_SIZE 128
static uint8_t usbd_control_buffer[USBD_CONTROL_BUFFER_SIZE];

/**
 * USB Device state structure.
 */
static usbd_device *usbd_dev = NULL;

/**
 * This optional callback is configured by the user to handle receive events.
 */
usb_rx_callback_t user_rx_callback = NULL;

/*
 * Used to track the initialization of the USB device.  Set to true after the
 * USB stack is configured.
 */
static bool usb_configured = false;

static const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = USB_SEGMENT_SIZE,
	.idVendor = 0x534c,
	.idProduct = 0x0001,
	.bcdDevice = 0x0100,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

/* got via usbhid-dump from CP2110 */
static const uint8_t hid_report_descriptor[] = {
	0x06, 0x00, 0xFF, 0x09, 0x01, 0xA1, 0x01, 0x09, 0x01, 0x75, 0x08, 0x95, 0x40, 0x26, 0xFF, 0x00,
	0x15, 0x00, 0x85, 0x01, 0x95, 0x01, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x02,
	0x95, 0x02, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x03, 0x95, 0x03, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x04, 0x95, 0x04, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x05, 0x95, 0x05, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x06,
	0x95, 0x06, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x07, 0x95, 0x07, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x08, 0x95, 0x08, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x09, 0x95, 0x09, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x0A,
	0x95, 0x0A, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x0B, 0x95, 0x0B, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x0C, 0x95, 0x0C, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x0D, 0x95, 0x0D, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x0E,
	0x95, 0x0E, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x0F, 0x95, 0x0F, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x10, 0x95, 0x10, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x11, 0x95, 0x11, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x12,
	0x95, 0x12, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x13, 0x95, 0x13, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x14, 0x95, 0x14, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x15, 0x95, 0x15, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x16,
	0x95, 0x16, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x17, 0x95, 0x17, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x18, 0x95, 0x18, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x19, 0x95, 0x19, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x1A,
	0x95, 0x1A, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x1B, 0x95, 0x1B, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x1C, 0x95, 0x1C, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x1D, 0x95, 0x1D, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x1E,
	0x95, 0x1E, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x1F, 0x95, 0x1F, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x20, 0x95, 0x20, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x21, 0x95, 0x21, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x22,
	0x95, 0x22, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x23, 0x95, 0x23, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x24, 0x95, 0x24, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x25, 0x95, 0x25, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x26,
	0x95, 0x26, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x27, 0x95, 0x27, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x28, 0x95, 0x28, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x29, 0x95, 0x29, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x2A,
	0x95, 0x2A, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x2B, 0x95, 0x2B, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x2C, 0x95, 0x2C, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x2D, 0x95, 0x2D, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x2E,
	0x95, 0x2E, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x2F, 0x95, 0x2F, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x30, 0x95, 0x30, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x31, 0x95, 0x31, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x32,
	0x95, 0x32, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x33, 0x95, 0x33, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x34, 0x95, 0x34, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x35, 0x95, 0x35, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x36,
	0x95, 0x36, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x37, 0x95, 0x37, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x38, 0x95, 0x38, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x39, 0x95, 0x39, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x3A,
	0x95, 0x3A, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x3B, 0x95, 0x3B, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x3C, 0x95, 0x3C, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01,
	0x91, 0x02, 0x85, 0x3D, 0x95, 0x3D, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x3E,
	0x95, 0x3E, 0x09, 0x01, 0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x3F, 0x95, 0x3F, 0x09, 0x01,
	0x81, 0x02, 0x09, 0x01, 0x91, 0x02, 0x85, 0x40, 0x95, 0x01, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x41,
	0x95, 0x01, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x42, 0x95, 0x06, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x43,
	0x95, 0x01, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x44, 0x95, 0x02, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x45,
	0x95, 0x04, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x46, 0x95, 0x02, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x47,
	0x95, 0x02, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x50, 0x95, 0x08, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x51,
	0x95, 0x01, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x52, 0x95, 0x01, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x60,
	0x95, 0x0A, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x61, 0x95, 0x3F, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x62,
	0x95, 0x3F, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x63, 0x95, 0x3F, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x64,
	0x95, 0x3F, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x65, 0x95, 0x3E, 0x09, 0x01, 0xB1, 0x02, 0x85, 0x66,
	0x95, 0x13, 0x09, 0x01, 0xB1, 0x02, 0xC0,
};

static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
	.hid_descriptor = {
		.bLength = sizeof(hid_function),
		.bDescriptorType = USB_DT_HID,
		.bcdHID = 0x0111,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
	},
	.hid_report = {
		.bReportDescriptorType = USB_DT_REPORT,
		.wDescriptorLength = sizeof(hid_report_descriptor),
	}
};

static const struct usb_endpoint_descriptor hid_endpoints[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = ENDPOINT_ADDRESS_IN,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = USB_SEGMENT_SIZE,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = ENDPOINT_ADDRESS_OUT,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = USB_SEGMENT_SIZE,
	.bInterval = 10,
}};

static const struct usb_interface_descriptor hid_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_HID,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,
	.endpoint = hid_endpoints,
	.extra = &hid_function,
	.extralen = sizeof(hid_function),
}};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = hid_iface,
}};

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,
	.interface = ifaces,
};

static const char *usb_strings[] = {
	"KeepKey",
	"KeepKey App",
	"No Serial",
        ""
};
#define NUM_USB_STRINGS (sizeof(usb_strings) / sizeof(usb_strings[0]) - 1)

static int hid_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *, struct usb_setup_data *))
{
	(void)complete;
	(void)dev;

	if ((req->bmRequestType != ENDPOINT_ADDRESS_IN) ||
	    (req->bRequest != USB_REQ_GET_DESCRIPTOR) ||
	    (req->wValue != 0x2200))
		return 0;

	/* Handle the HID report descriptor. */
	*buf = (uint8_t *)hid_report_descriptor;
	*len = sizeof(hid_report_descriptor);
	return 1;
}

static void hid_rx_callback(usbd_device *dev, uint8_t ep)
{
    (void)ep;

    /*
     * Receive into the message buffer.
     */
    UsbMessage m;
    uint16_t rx = usbd_ep_read_packet(dev, 
                                      ENDPOINT_ADDRESS_OUT, 
                                      m.message, 
                                      USB_SEGMENT_SIZE);

    if(rx && user_rx_callback)
    {
        m.len = rx;
        user_rx_callback(&m);
    }
}

/**
 * Called by the usb infrastructure in response to a config event.
 */
static void hid_set_config_callback(usbd_device *dev, uint16_t wValue)
{
	(void)wValue;

	usbd_ep_setup(dev, ENDPOINT_ADDRESS_IN,  USB_ENDPOINT_ATTR_INTERRUPT, USB_SEGMENT_SIZE, 0);
	usbd_ep_setup(dev, ENDPOINT_ADDRESS_OUT, USB_ENDPOINT_ATTR_INTERRUPT, USB_SEGMENT_SIZE, hid_rx_callback);

	usbd_register_control_callback(
		dev,
		USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
		USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
		hid_control_request);

        usb_configured = true;
}

bool usb_init(void)
{
    /*
     * Already initialized.
     */
    if(usbd_dev != NULL)
    {
        return false;
    }

    gpio_mode_setup(USB_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USB_GPIO_PORT_PINS);
    gpio_set_af(USB_GPIO_PORT, GPIO_AF10, USB_GPIO_PORT_PINS);

    static char serial_number[100];
    desig_get_unique_id_as_string(serial_number, sizeof(serial_number));
    usb_strings[NUM_USB_STRINGS-1] = serial_number;

   

    usbd_dev = usbd_init(&otgfs_usb_driver, 
                         &dev_descr, 
                         &config, 
                         usb_strings,
                         NUM_USB_STRINGS, 
                         usbd_control_buffer, 
                         sizeof(usbd_control_buffer));
    if(usbd_dev == NULL)
    {
        return false;
    }

    usbd_register_set_config_callback(usbd_dev, hid_set_config_callback);

    return true;
}

bool usb_poll(void)
{

    usbd_poll(usbd_dev);

    return true;
}

bool usb_tx(void* message, uint32_t len)
{
    uint8_t tmp_buffer[USB_SEGMENT_SIZE];
    tmp_buffer[0] = '?';

    uint32_t send_ct = 0;

    while(send_ct < len)
    {
        if(send_ct)
        {
            //TODO Replace with more elegant solution that
            //monitors the usb transmit for last transmit complete
            //status.
            delay(100);
        } else {
        	delay(100);
        }
        /*
         * Need to truncate to 63 because 
         * 1) maintain messaging compatability with trezor 
         * 2) opencm3 doesn't support fragments packets currently.
         */
        uint32_t rem = len-send_ct;
        uint32_t ct = rem < USB_SEGMENT_SIZE-1 ? rem : USB_SEGMENT_SIZE-1;
        memcpy(tmp_buffer+1, message+send_ct, ct);

        /*
         * Chunk out the message 63 bytes at a time (with the trezor '?' as the first byte)
         */
        uint16_t tx = usbd_ep_write_packet(usbd_dev, ENDPOINT_ADDRESS_IN, tmp_buffer, USB_SEGMENT_SIZE);
        send_ct += ct;
        assert(tx != 0);
    }
    return(true);
}

void usb_set_rx_callback(usb_rx_callback_t callback)
{
    user_rx_callback = callback;
}

