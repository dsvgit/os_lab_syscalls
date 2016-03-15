#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/processor.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#define DRV_NAME "tplink_driver"
#define DRV_VERSION "dev"
#define DRV_DESCRIPTION "IC Plus IP100A Fast Ethernet Adapter"

MODULE_AUTHOR("Sergey Dedkov dsv.mail@yandex.ru");
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL");

static struct pci_device_id ids[] = {
	{ 0x13F0, 0x0200, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 6 },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ids);

struct tplink_private {
	spinlock_t lock;
	struct pci_dev *pci_dev;
	void __iomem *base;
};

static int tplink_dummy_open(struct net_device *ndev)
{
	return 0;
}

static int tplink_dummy_stop(struct net_device *ndev)
{
	return 0;
}

static int tplink_dummy_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	return 0;
}

static const struct net_device_ops tplink_net_device_ops = {
	.ndo_open = tplink_dummy_open,
	.ndo_stop = tplink_dummy_stop,
	.ndo_start_xmit = tplink_dummy_start_xmit
};

static int __devinit tplink_dummy_init_netdev(struct pci_dev *pdev, unsigned long ioaddr)
{
	struct net_device *ndev = pci_get_drvdata(pdev);
	struct tplink_private *pp;
	int irq;
	unsigned long i;

	if (!ndev)
		return -ENODEV;

	irq = pdev->irq;
	pp = netdev_priv(ndev);
	ndev->base_addr = ioaddr;
	ndev->irq = irq;
	pp->pci_dev = pdev;
	pp->base = (void *)ioaddr;
	spin_lock_init(&pp->lock);

	/* read first 6 bytes of PROM */
	for(i = 0; i < 6; i++)
		ndev->dev_addr[i] = ioread8((void *)ioaddr + i);
	if (!is_valid_ether_addr(ndev->dev_addr))
		random_ether_addr(ndev->dev_addr);

	/* init net_dev_ops */
	ndev->netdev_ops = &tplink_net_device_ops;

	if (register_netdev(ndev))
		return -ENODEV;
	netdev_info(ndev, "%s %pM\n", DRV_DESCRIPTION, ndev->dev_addr);

	return 0;
}

static int probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	printk("test probe");

	struct tplink_private *pp;
	struct net_device *ndev;
	void __iomem *ioaddr;
#ifdef USE_IO_OPS
	int bar = 0;
#else
	int bar = 1;
#endif

#ifndef MODULE
	pr_info_once("%s version %s\n", DRV_DESCRIPTION, DRV_VERSION);
#endif

	if (pci_enable_device(pdev))
		return -ENODEV;
	/* enables bus-mastering for device pdev */
	pci_set_master(pdev);

	ndev = alloc_etherdev(sizeof(*pp));
	if (!ndev)
		goto out;
	/* register ourself under /sys/class/net/ */
	SET_NETDEV_DEV(ndev, &pdev->dev);

	if (pci_request_regions(pdev, DRV_NAME))
		goto out_netdev;

	ioaddr = pci_iomap(pdev, bar, 0x20);
	if (!ioaddr)
		goto out_res;
	pci_set_drvdata(pdev, ndev);

	if (tplink_dummy_init_netdev(pdev, (unsigned long)ioaddr))
		goto out_res_unmap;

	return 0;

out_res_unmap:
	pci_iounmap(pdev, ioaddr);
out_res:
	pci_release_regions(pdev);
out_netdev:
	free_netdev(ndev);
out:
	pci_disable_device(pdev);
	return -ENODEV;
}

static void remove(struct pci_dev *pdev)
{
	struct net_device *ndev = pci_get_drvdata(pdev);
	struct tplink_private *pp;
	pp = netdev_priv(ndev);
	unregister_netdev(ndev);
	pci_iounmap(pdev, pp->base);
	free_netdev(ndev);
	pci_disable_device(pdev);
	pci_release_regions(pdev);
	pci_set_drvdata(pdev, NULL);
}

static struct pci_driver pci_driver = {
	.name = DRV_NAME,
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};

static int __init pci_skel_init(void)
{
	return pci_register_driver(&pci_driver);
}

static void __exit pci_skel_exit(void)
{
	pci_unregister_driver(&pci_driver);
}

module_init(pci_skel_init);
module_exit(pci_skel_exit);