/*
 * Copyright (C) 2022 Unionman Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio_keys.h>
#include <linux/property.h>

#ifdef CONFIG_DRIVERS_HDF_INPUT
#include "hdf_hid_adapter.h"
#endif

#define DRV_NAME "gpio-keypad"

struct gpio_keys_button_data {
    struct gpio_desc *gpiod;
    int last_state;
    int count;
    int threshold;
};

struct gpio_keypad_dev {
    struct input_dev *input;
    struct device *dev;
    const struct gpio_keys_platform_data *pdata;
    unsigned long rel_axis_seen[BITS_TO_LONGS(REL_CNT)];
    unsigned long abs_axis_seen[BITS_TO_LONGS(ABS_CNT)];
    struct gpio_keys_button_data data[];
};

#ifdef CONFIG_DRIVERS_HDF_INPUT
static InputDevice *g_hidInputDev = NULL;
#endif

#ifdef CONFIG_DRIVERS_HDF_INPUT
static InputDevice *HidRegisterHdfKeyPadDev(void)
{
    InputDevice *inputDev = NULL;
    HidInfo hidInfo;

    hidInfo.devType = INDEV_TYPE_KEYBOARD;
    hidInfo.eventType[0] = SET_BIT(EV_KEY);
    hidInfo.keyCode[3L] = SET_BIT(KEY_POWER);
    hidInfo.devName = "gpio_keypad";
    inputDev = HidRegisterHdfInputDev(&hidInfo);

    return inputDev;
}
#endif

static void gpio_keys_button_event(struct input_dev *input, const struct gpio_keys_button *button, int state)
{
    struct gpio_keypad_dev *bdev = input_get_drvdata(input);
    unsigned int type = button->type ?: EV_KEY;

    if (type == EV_REL) {
        if (state) {
            input_event(input, type, button->code, button->value);
            __set_bit(button->code, bdev->rel_axis_seen);
        }
    } else if (type == EV_ABS) {
        if (state) {
            input_event(input, type, button->code, button->value);
            __set_bit(button->code, bdev->abs_axis_seen);
        }
    } else {
        input_event(input, type, button->code, state);
        input_sync(input);
    }
}

static void gpio_keypad_check_state(struct input_dev *input, const struct gpio_keys_button *button,
                                    struct gpio_keys_button_data *bdata)
{
    int state;

    state = gpiod_get_value_cansleep(bdata->gpiod);
    if (state < 0) {
        dev_err(input->dev.parent, "failed to get gpio state: %d\n", state);
    } else {
        gpio_keys_button_event(input, button, state);

        if (state != bdata->last_state) {
            bdata->count = 0;
            bdata->last_state = state;

#ifdef CONFIG_DRIVERS_HDF_INPUT
            InputManager *inputManager = GetInputManager();
            if ((g_hidInputDev == NULL) || (inputManager == NULL)) {
                return;
            }

            if (button->code == KEY_POWER) {
                HidReportEvent(g_hidInputDev, EV_KEY, KEY_POWER, !state);
                HidReportEvent(g_hidInputDev, EV_SYN, SYN_REPORT, 0);
            }
#endif
        }
    }
}

static void gpio_keypad_poll(struct input_dev *input)
{
    struct gpio_keypad_dev *bdev = input_get_drvdata(input);
    const struct gpio_keys_platform_data *pdata = bdev->pdata;
    int i;

    memset(bdev->rel_axis_seen, 0, sizeof(bdev->rel_axis_seen));
    memset(bdev->abs_axis_seen, 0, sizeof(bdev->abs_axis_seen));

    for (i = 0; i < pdata->nbuttons; i++) {
        struct gpio_keys_button_data *bdata = &bdev->data[i];

        if (bdata->count < bdata->threshold) {
            bdata->count++;
            gpio_keys_button_event(input, &pdata->buttons[i], bdata->last_state);
        } else {
            gpio_keypad_check_state(input, &pdata->buttons[i], bdata);
        }
    }

    for_each_set_bit(i, input->relbit, REL_CNT)
    {
        if (!test_bit(i, bdev->rel_axis_seen)) {
            input_event(input, EV_REL, i, 0);
        }
    }

    for_each_set_bit(i, input->absbit, ABS_CNT)
    {
        if (!test_bit(i, bdev->abs_axis_seen)) {
            input_event(input, EV_ABS, i, 0);
        }
    }

    input_sync(input);
}

static int gpio_keypad_open(struct input_dev *input)
{
    struct gpio_keypad_dev *bdev = input_get_drvdata(input);
    const struct gpio_keys_platform_data *pdata = bdev->pdata;

    if (pdata->enable) {
        pdata->enable(bdev->dev);
    }

    return 0;
}

static void gpio_keypad_close(struct input_dev *input)
{
    struct gpio_keypad_dev *bdev = input_get_drvdata(input);
    const struct gpio_keys_platform_data *pdata = bdev->pdata;

    if (pdata->disable) {
        pdata->disable(bdev->dev);
    }
}

static struct gpio_keys_platform_data *gpio_keypad_get_devtree_pdata(struct device *dev)
{
    struct gpio_keys_platform_data *pdata;
    struct gpio_keys_button *button;
    struct fwnode_handle *child;
    int nbuttons;

    nbuttons = device_get_child_node_count(dev);
    if (nbuttons == 0) {
        return ERR_PTR(-EINVAL);
    }

    pdata = devm_kzalloc(dev, sizeof(*pdata) + nbuttons * sizeof(*button), GFP_KERNEL);
    if (!pdata) {
        return ERR_PTR(-ENOMEM);
    }

    button = (struct gpio_keys_button *)(pdata + 1);

    pdata->buttons = button;
    pdata->nbuttons = nbuttons;

    pdata->rep = device_property_present(dev, "autorepeat");
    device_property_read_u32(dev, "poll-interval", &pdata->poll_interval);

    device_property_read_string(dev, "label", &pdata->name);

    device_for_each_child_node(dev, child)
    {
        if (fwnode_property_read_u32(child, "linux,code", &button->code)) {
            dev_err(dev, "button without keycode\n");
            fwnode_handle_put(child);
            return ERR_PTR(-EINVAL);
        }

        fwnode_property_read_string(child, "label", &button->desc);

        if (fwnode_property_read_u32(child, "linux,input-type", &button->type)) {
            button->type = EV_KEY;
        }

        if (fwnode_property_read_u32(child, "linux,input-value", (u32 *)&button->value)) {
            button->value = 1;
        }

        button->wakeup = fwnode_property_read_bool(child, "wakeup-source") ||
                         /* legacy name */
                         fwnode_property_read_bool(child, "gpio-key,wakeup");

        if (fwnode_property_read_u32(child, "debounce-interval", &button->debounce_interval)) {
            button->debounce_interval = 5L;
        }

        button++;
    }

    return pdata;
}

static void gpio_keypad_set_abs_params(struct input_dev *input, const struct gpio_keys_platform_data *pdata,
                                       unsigned int code)
{
    int i, min = 0, max = 0;

    for (i = 0; i < pdata->nbuttons; i++) {
        const struct gpio_keys_button *button = &pdata->buttons[i];

        if (button->type != EV_ABS || button->code != code) {
            continue;
        }

        if (button->value < min) {
            min = button->value;
        }
        if (button->value > max) {
            max = button->value;
        }
    }

    input_set_abs_params(input, code, min, max, 0, 0);
}

static const struct of_device_id gpio_keypad_of_match[] = {
    {
        .compatible = "amlogic, gpio_keypad",
    },
    {},
};
MODULE_DEVICE_TABLE(of, gpio_keypad_of_match);

static int gpio_keypad_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct fwnode_handle *child = NULL;
    const struct gpio_keys_platform_data *pdata = dev_get_platdata(dev);
    struct gpio_keypad_dev *bdev;
    struct input_dev *input;
    int error;
    int i;

    if (!pdata) {
        pdata = gpio_keypad_get_devtree_pdata(dev);
        if (IS_ERR(pdata)) {
            return PTR_ERR(pdata);
        }
    }

    if (!pdata->poll_interval) {
        dev_err(dev, "missing poll_interval value\n");
        return -EINVAL;
    }

    bdev = devm_kzalloc(dev, struct_size(bdev, data, pdata->nbuttons), GFP_KERNEL);
    if (!bdev) {
        dev_err(dev, "no memory for private data\n");
        return -ENOMEM;
    }

    input = devm_input_allocate_device(dev);
    if (!input) {
        dev_err(dev, "no memory for input device\n");
        return -ENOMEM;
    }

    input_set_drvdata(input, bdev);

    input->name = "gpio_keypad";
    input->phys = "gpio_keypad/input0";

    input->id.bustype = BUS_HOST;
    input->id.vendor = 0x0001;
    input->id.product = 0x0001;
    input->id.version = 0x0100;

    input->open = gpio_keypad_open;
    input->close = gpio_keypad_close;

    __set_bit(EV_KEY, input->evbit);
    if (pdata->rep) {
        __set_bit(EV_REP, input->evbit);
    }

    for (i = 0; i < pdata->nbuttons; i++) {
        const struct gpio_keys_button *button = &pdata->buttons[i];
        struct gpio_keys_button_data *bdata = &bdev->data[i];
        unsigned int type = button->type ?: EV_KEY;

        if (button->wakeup) {
            dev_err(dev, DRV_NAME " does not support wakeup\n");
            fwnode_handle_put(child);
            return -EINVAL;
        }

        if (!dev_get_platdata(dev)) {
            /* No legacy static platform data */
            child = device_get_next_child_node(dev, child);
            if (!child) {
                dev_err(dev, "missing child device node\n");
                return -EINVAL;
            }

            bdata->gpiod = devm_fwnode_gpiod_get(dev, child, NULL, GPIOD_IN, button->desc);
            if (IS_ERR(bdata->gpiod)) {
                error = PTR_ERR(bdata->gpiod);
                if (error != -EPROBE_DEFER) {
                    dev_err(dev, "failed to get gpio: %d\n", error);
                }
                fwnode_handle_put(child);
                return error;
            }
        } else if (gpio_is_valid(button->gpio)) {
            /*
             * Legacy GPIO number so request the GPIO here and
             * convert it to descriptor.
             */
            unsigned flags = GPIOF_IN;

            if (button->active_low) {
                flags |= GPIOF_ACTIVE_LOW;
            }

            error = devm_gpio_request_one(dev, button->gpio, flags, button->desc ?: DRV_NAME);
            if (error) {
                dev_err(dev, "unable to claim gpio %u, err=%d\n", button->gpio, error);
                return error;
            }

            bdata->gpiod = gpio_to_desc(button->gpio);
            if (!bdata->gpiod) {
                dev_err(dev, "unable to convert gpio %u to descriptor\n", button->gpio);
                return -EINVAL;
            }
        }

        bdata->last_state = -1;
        bdata->threshold = DIV_ROUND_UP(button->debounce_interval, pdata->poll_interval);

        input_set_capability(input, type, button->code);
        if (type == EV_ABS) {
            gpio_keypad_set_abs_params(input, pdata, button->code);
        }
    }

    fwnode_handle_put(child);

    bdev->input = input;
    bdev->dev = dev;
    bdev->pdata = pdata;

    error = input_setup_polling(input, gpio_keypad_poll);
    if (error) {
        dev_err(dev, "unable to set up polling, err=%d\n", error);
        return error;
    }

    input_set_poll_interval(input, pdata->poll_interval);

    error = input_register_device(input);
    if (error) {
        dev_err(dev, "unable to register polled device, err=%d\n", error);
        return error;
    }

    /* report initial state of the buttons */
    for (i = 0; i < pdata->nbuttons; i++) {
        gpio_keypad_check_state(input, &pdata->buttons[i], &bdev->data[i]);
    }

    input_sync(input);

#ifdef CONFIG_DRIVERS_HDF_INPUT
    HidInfo *info = (HidInfo *)kmalloc(sizeof(HidInfo), GFP_KERNEL);
    if (info == NULL) {
        printk("%s: malloc failed\n", __func__);
        return -EINVAL;
    }

    info->devName = input->name;
    memcpy(info->devProp, input->propbit, sizeof(unsigned long) * BITS_TO_LONGS(INPUT_PROP_CNT));
    memcpy(info->eventType, input->evbit, sizeof(unsigned long) * BITS_TO_LONGS(EV_CNT));
    memcpy(info->keyCode, input->keybit, sizeof(unsigned long) * BITS_TO_LONGS(KEY_CNT));
    memcpy(info->relCode, input->relbit, sizeof(unsigned long) * BITS_TO_LONGS(REL_CNT));
    memcpy(info->absCode, input->absbit, sizeof(unsigned long) * BITS_TO_LONGS(ABS_CNT));
    memcpy(info->miscCode, input->mscbit, sizeof(unsigned long) * BITS_TO_LONGS(MSC_CNT));
    memcpy(info->ledCode, input->ledbit, sizeof(unsigned long) * BITS_TO_LONGS(LED_CNT));
    memcpy(info->soundCode, input->sndbit, sizeof(unsigned long) * BITS_TO_LONGS(SND_CNT));
    memcpy(info->forceCode, input->ffbit, sizeof(unsigned long) * BITS_TO_LONGS(FF_CNT));
    memcpy(info->switchCode, input->swbit, sizeof(unsigned long) * BITS_TO_LONGS(SW_CNT));
    for (i = 0; i < BITS_TO_LONGS(ABS_CNT); i++) {
        if (input->absbit[i] != 0) {
            memcpy(info->axisInfo, input->absinfo, sizeof(struct input_absinfo) * ABS_CNT);
            break;
        }
    }

    info->bustype = input->id.bustype;
    info->vendor = input->id.vendor;
    info->product = input->id.product;
    info->version = input->id.version;
    SendInfoToHdf(info);
    kfree(info);
    info = NULL;

    g_hidInputDev = HidRegisterHdfKeyPadDev();
    if (g_hidInputDev == NULL) {
        pr_err("HidRegisterHdfKeyPadDev error\n");
        return -EINVAL;
    }
#endif

    return 0;
}

static struct platform_driver gpio_keypad_driver = {
    .probe = gpio_keypad_probe,
    .driver =
        {
            .name = DRV_NAME,
            .of_match_table = gpio_keypad_of_match,
        },
};
module_platform_driver(gpio_keypad_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("AlgoIdeas <yu19881234@163.com>");
MODULE_DESCRIPTION("GPIO KeyPad driver");
MODULE_ALIAS("platform:" DRV_NAME);
