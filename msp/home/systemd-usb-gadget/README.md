# Systemd unit for configuring USB gadgets

This implements the procedure described in the Kernel's
[gadget_configfs.txt][] as a system unit.

## Installation

If you're running on a Pi, you will first need to enable the
appropriate device overlays:

- If you plan to use your
  USB port as both a host and a gadget, then enable OTG mode by
  adding the following to `/boot/config.txt`:

         dtoverlay=dwc2,dr_mode=otg

- If you will be operating exclusively as a gadget, then instead use:

        dtoverlay=dwc2,dr_mode=peripheral

After editing `/boot/config.txt`, you must reboot for the changes
to become active.

To install the systemd unit, run `make install` in the source
directory, which will place the support scripts into `/sbin` and the
systemd unit into `/etc/systemd/system`.

## Configuration

Create one or files in `/etc/gadget` named `<gadget_name>.conf`.
These files **must** contain the following configuration keys:

- `USB_FUNCTIONS` -- List of functions implemented by the gadget.

These files **may** contain any of the following configuration keys:

- `USB_IDVENDOR` -- Vendor ID. Defaults to `0x1b6d`, the Linux
  Foundation.
- `USB_IDPRODUCT` -- Product ID. Defaults to `0x0104`, "Multifunction
  Composite Gadget"
- `USB_BCDDEVICE` -- Device version. Defaults to `0x0100` (for 1.0.0).
- `USB_BCDUSB` -- Maximum supported USB version. Defaults to `0x0200`
  (for USB 2.0).
- `USB_SERIALNUMBER` -- Device serial number. Defaults to
  `deadbeef0000`.
- `USB_PRODUCT` -- Product name. Defaults to `Pi Zero Gadget`.
- `USB_MANUFACTURER` -- Manufacturer name. Defaults to `Linux`.
- `USB_MAXPOWER` -- Maximum power required. Defaults to `250`.

For example, to create a serial gadget named `g0`, I would create the
file `/etc/gadget/g0.conf` with the following contents:

    USB_FUNCTIONS=acm.usb0

Or to create a gadget that offered both a serial interface and an
ethernet interface:

    USB_FUNCTIONS="acm.usb0 rndis.usb0"

To enable the gadget at boot, run:

    systemctl enable usb-gadget@g0

To create the gadget immediately:

    systemctl start usb-gadget@g0

To remove the gadget after starting it:

    systemctl stop usb-gadget@g0

[gadget_configfs.txt]: https://www.kernel.org/doc/Documentation/usb/gadget_configfs.txt
