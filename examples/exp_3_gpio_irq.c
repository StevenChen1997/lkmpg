#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

struct my_button {
    struct gpio_desc *gpiod;
    int irq;
    unsigned int debounce_ms;
    struct delayed_work work;
    int state;
};

static void my_button_work(struct work_struct *work)
{
    struct my_button *btn =
        container_of(to_delayed_work(work), struct my_button, work);

    btn->state = gpiod_get_value(btn->gpiod);
    pr_info("button state=%d\n", btn->state);
}

static irqreturn_t my_button_irq(int irq, void *dev_id)
{
    struct my_button *btn = dev_id;

    mod_delayed_work(system_wq, &btn->work,
                     msecs_to_jiffies(btn->debounce_ms));

    return IRQ_HANDLED;
}

static int my_button_probe(struct platform_device *pdev)
{
    struct my_button *btn;
    int ret;

    btn = devm_kzalloc(&pdev->dev, sizeof(*btn), GFP_KERNEL);
    if (!btn)
        return -ENOMEM;

    btn->debounce_ms = 20;
    of_property_read_u32(pdev->dev.of_node,
                         "debounce-ms", &btn->debounce_ms);

    btn->gpiod = devm_gpiod_get(&pdev->dev, "button", GPIOD_IN);
    if (IS_ERR(btn->gpiod))
        return PTR_ERR(btn->gpiod);

    btn->irq = gpiod_to_irq(btn->gpiod);
    if (btn->irq < 0)
        return btn->irq;

    INIT_DELAYED_WORK(&btn->work, my_button_work);

    ret = devm_request_irq(&pdev->dev, btn->irq, my_button_irq,
                           IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                           "my_button_irq", btn);
    if (ret)
        return ret;

    platform_set_drvdata(pdev, btn);

    dev_info(&pdev->dev, "my button driver probed\n");
    return 0;
}

static void my_button_remove(struct platform_device *pdev)
{
    struct my_button *btn = platform_get_drvdata(pdev);

    cancel_delayed_work_sync(&btn->work);
}

static const struct of_device_id my_button_of_match[] = {
    { .compatible = "demo,my-button" },
    { }
};
MODULE_DEVICE_TABLE(of, my_button_of_match);

static struct platform_driver my_button_driver = {
    .probe  = my_button_probe,
    .remove = my_button_remove,
    .driver = {
        .name = "my_button",
        .of_match_table = my_button_of_match,
    },
};

module_platform_driver(my_button_driver);

MODULE_LICENSE("GPL");
