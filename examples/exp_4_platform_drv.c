#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>

#define REG_STATUS 0x00
#define REG_CTRL   0x04

struct my_regs {
    void __iomem *base;
};

static ssize_t status_show(struct device *dev,
                           struct device_attribute *attr,
                           char *buf)
{
    struct my_regs *priv = dev_get_drvdata(dev);
    u32 val = readl(priv->base + REG_STATUS);

    return sysfs_emit(buf, "0x%08x\n", val);
}

static ssize_t ctrl_store(struct device *dev,
                          struct device_attribute *attr,
                          const char *buf,
                          size_t count)
{
    struct my_regs *priv = dev_get_drvdata(dev);
    unsigned long val;
    int ret;

    ret = kstrtoul(buf, 0, &val);
    if (ret)
        return ret;

    writel((u32)val, priv->base + REG_CTRL);

    return count;
}

static DEVICE_ATTR_RO(status);
static DEVICE_ATTR_WO(ctrl);

static int my_regs_probe(struct platform_device *pdev)
{
    struct my_regs *priv;
    struct resource *res;
    int ret;

    priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    priv->base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(priv->base))
        return PTR_ERR(priv->base);

    platform_set_drvdata(pdev, priv);

    ret = device_create_file(&pdev->dev, &dev_attr_status);
    if (ret)
        return ret;

    ret = device_create_file(&pdev->dev, &dev_attr_ctrl);
    if (ret) {
        device_remove_file(&pdev->dev, &dev_attr_status);
        return ret;
    }

    dev_info(&pdev->dev, "my_regs probed\n");
    return 0;
}

static void my_regs_remove(struct platform_device *pdev)
{
    device_remove_file(&pdev->dev, &dev_attr_ctrl);
    device_remove_file(&pdev->dev, &dev_attr_status);
}

static const struct of_device_id my_regs_of_match[] = {
    { .compatible = "demo,my-regs" },
    { }
};
MODULE_DEVICE_TABLE(of, my_regs_of_match);

static struct platform_driver my_regs_driver = {
    .probe  = my_regs_probe,
    .remove = my_regs_remove,
    .driver = {
        .name = "my_regs",
        .of_match_table = my_regs_of_match,
    },
};

module_platform_driver(my_regs_driver);

MODULE_LICENSE("GPL");
