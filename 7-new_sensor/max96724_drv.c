/*  */

#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <media/camera_common.h>
#include <linux/module.h>

#include "max96724.h"

#define max96724_TX11_PIPE_X_EN_ADDR 0x90B
#define max96724_TX45_PIPE_X_DST_CTRL_ADDR 0x92D
#define max96724_PIPE_X_SRC_0_MAP_ADDR 0x90D

#define max96724_PHY_CLK (20)

#define MAX_USE_OV_NO_OG 0

struct max96724 {
	struct i2c_client *i2c_client;
	struct regmap *regmap;
	struct mutex lock;
	u8 link_lock_status;
	u8 ismaster;
};

typedef struct 
{
	u16 addr;
	u8 val;
}MAX96724_REG_SET;


#define CHECKNULL(x) if(x==NULL){ \
		dev_err(NULL, " %s: ##x## NULL ptr\n",__func__); \
		return -EINVAL; \
		}

static int vercheck=0;
static int check_version(struct i2c_client *client);
static int check_device_id_rev(struct i2c_client *client);

static int max96724_write_reg(struct device *dev,
	u16 addr, u8 val)
{
	struct max96724 *priv;
	int err;
	priv = dev_get_drvdata(dev);
	err = regmap_write(priv->regmap, addr, val);
	if (err)
		dev_err(dev, " %s:i2c write failed, 0x%x = %x\n",
		__func__, addr, val);
	usleep_range(100, 110);
	if(!err)
		dev_info(dev,"96724:w addr(0x%04x)->0x%02x\n",addr,val);
	return err;
}

static int max96724_read_reg(struct device *dev,
	u16 addr, u32* val)
{
	struct max96724 *priv;
	int err;

	priv = dev_get_drvdata(dev);

	err = regmap_read(priv->regmap, addr, val);
	if (err)
		dev_err(dev,
		" %s:i2c read failed, 0x%x\n",
		__func__, addr);

	/* delay before next i2c command as required for SERDES link */
	usleep_range(100, 110);
	// if(!err)
	// 	dev_info(dev,"96724:r addr(0x%04x)<-0x%02x\n",addr,(u8)(*val));
	return err;
}

int z_max96724_lock_link(struct device *dev)
{
	struct max96724 *priv = NULL;

	CHECKNULL(dev);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);

	mutex_lock(&priv->lock);
	dev_info(dev," mutex lock");
	return 0;
}
EXPORT_SYMBOL(z_max96724_lock_link);

int z_max96724_print_reg_status(struct device *dev)
{
	// uint8_t i = 0;
    // struct reg_8 regValue;
	// u32 Value=0; 
	// dev_info(dev, "===========max96724=========\r\n");
	// for (i = 0; i < (sizeof(g_max96724_err_reg) / sizeof(g_max96724_err_reg[0])); i++)
	// {
	// 	regValue.addr = g_max96724_err_reg[i].addr;
	// 	max96724_read_reg(dev, regValue.addr, &Value);
	// 	regValue.val = Value&0xff;
	// 	// vc_info(dev, "read %s 0x%x %s\r\n", g_err_reg[i].str, g_err_reg[i].addr, g_err_reg[i].bitfield);
	// 	sensor_reg_info_print(dev, 0, &regValue);
	// 	msleep_range(5);
	// }
	return 0;
}

EXPORT_SYMBOL(z_max96724_print_reg_status);

int z_max96724_unlock_link(struct device *dev)
{
	struct max96724 *priv = NULL;

	CHECKNULL(dev);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);

	dev_info(dev," mutex unlock");
	mutex_unlock(&priv->lock);
	return 0;
}
EXPORT_SYMBOL(z_max96724_unlock_link);

int z_max96724_monopolize_link(struct device *dev, int link)
{
	struct max96724 *priv = NULL;
	u32 value;

	CHECKNULL(dev);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);

	dev_info(dev, " %s: monopolize link_%c\n", __func__, 'A'+link);
	max96724_read_reg(dev,0x0006,&value);
	value = (value & 0xF0) | (1<<link);
	max96724_write_reg(dev,0x0006,value);
	mdelay(150);
	return 0;
}
EXPORT_SYMBOL(z_max96724_monopolize_link);

int z_max96724_enable_link(struct device *dev, int link)
{
    struct max96724 *priv = NULL;
 
    CHECKNULL(dev);
    priv = dev_get_drvdata(dev);
    CHECKNULL(priv);
 
    dev_info(dev, " %s: enable link_%c\n", __func__, 'A'+link);
    priv->link_lock_status |= (1<<link);
    return 0;
}
EXPORT_SYMBOL(z_max96724_enable_link);

int z_max96724_restore_link(struct device *dev)
{
	struct max96724 *priv = NULL;
	u32 value;

	CHECKNULL(dev);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);

	dev_info(dev, " restore links\n");
	max96724_read_reg(dev,0x0006,&value);
	max96724_write_reg(dev,0x0006,(value&0xF0)|(priv->link_lock_status&0x0F));
	mdelay(150);
	return 0;
}
EXPORT_SYMBOL(z_max96724_restore_link);

int z_max96724_check_link_status(struct device *dev, int link)
{
	struct max96724 *priv = NULL;
	CHECKNULL(dev);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);
	if(priv->link_lock_status & (0x1<<link)){
		return 1;
	}else{
		return 0;
	}
}
EXPORT_SYMBOL(z_max96724_check_link_status);

int z_max96724_set_link_bandwidth(struct device *dev, int link, int gbps)
{
	struct max96724 *priv = NULL;
	u32 value;
	u16 reg = 0x0010;
	u8 regval = 0x01;
	if(gbps!=3){ //6Gbps
		regval = 0x02;
	}

	CHECKNULL(dev);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);

	dev_info(dev, " set link_%c to %dGbps\n",link+'A',gbps);
	if(link>=2){
		reg++, link-=2;
	}
	max96724_read_reg(dev,reg,&value);
	value = (value & (~(0x3<<(link*4)))) | (regval<<(link*4));
	max96724_write_reg(dev,reg,value & 0xFF);
	max96724_write_reg(dev,0x0018,(1<<link));
	mdelay(300);
	return 0;
}
EXPORT_SYMBOL(z_max96724_set_link_bandwidth);

static int max96724_setup_pipeline(struct device *dev,struct gmsl_link_ctx *g_ctx)
{
	struct max96724 *priv = NULL;
	int pipe_id = 0;
	u32 i = 0;
	u8 dst_vc = 0,src_vc = 0;

	CHECKNULL(dev);
	CHECKNULL(g_ctx);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);
	vc_info(dev,"96724: num_streams=%d\n",g_ctx->num_streams);
	for (i = 0; i < g_ctx->num_streams; i++) {
		pipe_id = g_ctx->des_link;
		dst_vc = g_ctx->des_link;
		dev_info(dev, " g_ctx->des_link:%d receive stream_id=%d, datatype=0x%x\n", \
									g_ctx->des_link, i, g_ctx->streams[i]);
		//todo
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_TX11_PIPE_X_EN_ADDR+0, 0x07);
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_TX11_PIPE_X_EN_ADDR+1, 0x00);
		//接口板子的4lane 最终接入的是portA -> serial_c cam0
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_PIPE_X_SRC_0_MAP_ADDR+0, g_ctx->streams[i] | (src_vc << 6));
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_PIPE_X_SRC_0_MAP_ADDR+1, g_ctx->streams[i] | (dst_vc << 6));
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_PIPE_X_SRC_0_MAP_ADDR+2, 0x00 | (src_vc << 6));
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_PIPE_X_SRC_0_MAP_ADDR+3, 0x00 | (dst_vc << 6));
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_PIPE_X_SRC_0_MAP_ADDR+4, 0x01 | (src_vc << 6));
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_PIPE_X_SRC_0_MAP_ADDR+5, 0x01 | (dst_vc << 6)); //0x912		
		max96724_write_reg(dev, (0x40*pipe_id)+max96724_TX45_PIPE_X_DST_CTRL_ADDR+0, 0x15);  // 0x15=controller1  0x1A=controller2
	}

	return 0;
}

int z_max96724_start_streaming(struct device *dev, struct gmsl_link_ctx *g_ctx)
{
	struct max96724 *priv = NULL;
	int err;
	u32 val;
	
	CHECKNULL(dev);
	CHECKNULL(g_ctx);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);

	dev_info(dev," 96724 start streaming, link=%c - enter\n",'A'+g_ctx->des_link);

	mutex_lock(&priv->lock);
	err = max96724_setup_pipeline(dev, g_ctx);
	if (err)
		return err;

	max96724_read_reg(dev,0xf4,&val);
	max96724_write_reg(dev,0xf4, (val | (1<<g_ctx->des_link))&0xff);

	// reset link
	max96724_write_reg(dev,0x018, (1<<g_ctx->des_link));
	mdelay(150);
	
	max96724_write_reg(dev,0x0903,0x10);
	max96724_write_reg(dev,0x0903,0x33);

	max96724_write_reg(dev,0x0943,0x10);
	max96724_write_reg(dev,0x0943,0x33);

	// //skew
	// if(g_ctx->des_link==0 || g_ctx->des_link==1){
	// 	max96724_write_reg(dev,0x0943,0x10);
	// 	max96724_write_reg(dev,0x0943,0x33);
	// }else{
	// 	max96724_write_reg(dev,0x0983,0x10);
	// 	max96724_write_reg(dev,0x0983,0x33);
	// }

	mutex_unlock(&priv->lock);
	dev_info(dev," 96724 start streaming - exit\n");
	return 0;
}
EXPORT_SYMBOL(z_max96724_start_streaming);

int z_max96724_stop_streaming(struct device *dev, struct gmsl_link_ctx *g_ctx)
{
	struct max96724 *priv = NULL;
	u32 val;
	u8 tmp;

	CHECKNULL(dev);
	CHECKNULL(g_ctx);
	priv = dev_get_drvdata(dev);
	CHECKNULL(priv);

	dev_info(dev," 96724 stop streaming, link_%c - enter\n",'A'+g_ctx->des_link);
	mutex_lock(&priv->lock);

	max96724_read_reg(dev,0xf4,&val);
	tmp = (val & (~(1<<g_ctx->des_link))) & 0xff;
	max96724_write_reg(dev,0xf4, tmp );
	if(tmp==0){
		max96724_write_reg(dev,0x018, 0x0f);
		dev_info(dev," all stopped, reset links\n");
		mdelay(30);
	}

	mutex_unlock(&priv->lock);
	dev_info(dev," 96724 stop streaming - exit\n");
	return 0;
}
EXPORT_SYMBOL(z_max96724_stop_streaming);

const struct of_device_id max96724_of_match[] = {
	{ .compatible = "bopixel,max96724", },
	{ },
};
MODULE_DEVICE_TABLE(of, max96724_of_match);

static int max96724_parse_dt(struct max96724 *priv,
				struct i2c_client *client)
{
	struct device_node *node = client->dev.of_node;
	const struct of_device_id *match;

	if (!node)
		return -EINVAL;

	match = of_match_device(max96724_of_match, &client->dev);
	if (!match) {
		dev_err(&client->dev, "Failed to find matching dt id\n");
		return -EFAULT;
	}

	return 0;
}

static struct regmap_config max96724_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
};

// static int get_mipifreq(void)
// {
// 	struct file *file;
// 	loff_t pos = 0;
// 	char buf[10];
// 	int freq = 0;
// 	ssize_t ret;

// 	file = filp_open("/etc/mipifreq", O_RDONLY, 0);
// 	if (IS_ERR(file)) {
// 		printk(" no mipifreq define file\n");
// 		goto defaultval;
// 	}

// 	ret = kernel_read(file, buf, 4, &pos);
// 	filp_close(file, NULL);
// 	if (ret > 0) {
// 		buf[ret] = 0;
// 		printk(" read from mipifreq: %s\n", buf);
// 		freq = simple_strtoul(buf,NULL,10);
// 		freq = freq / 100;
// 		if(freq<2 || freq>25){
// 			printk(" mipifreq out of range: %d, use default\n", freq);
// 			goto defaultval;
// 		}
// 		return freq;
// 	} else {
// 		printk(" failed to read from mipifreq\n");
// 		goto defaultval;
// 	}

// defaultval:
// 	return (max96724_PHY_CLK&0x1f);
// }

//1440x1080 @ 50fps RAW10 的VPG出图配置参数
//水平总宽度HTS_2000
// static MAX96724_REG_SET g_vpg_set[] = {
// 	{0x01DC, 0x00}, //配置基础时钟参数
// 	//写入垂直时序VS
// 	// {0x1055, }, //5行*2000
// 	// {0x1056, },
// 	// {0x1057, },
// 	{0x0150, 0xFB}, //配置使能开关
// 	{0x0151, 0x20}, //配置渐变图案
// 	{},
// };

// static int max96724_vpg_set(struct device *dev)
// {
// 	int err = 0;
// 	int i = 0;
// 	int num = sizeof(g_vpg_set) / sizeof(g_vpg_set[0]);
// 	vc_info(dev, "need set vpg reg num:%d \r\n", num);
// 	for (i = 0; i < num; i++)
// 	{
// 		vc_reg_info(dev, 1, g_vpg_set[i]);
// 		// err = max96724_write_reg(dev, g_vpg_set[i].addr, g_vpg_set[i].val);
// 	}
// 	return err;
// }

static int max96724_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct max96724 *priv;
	int err = 0;
	u32 val = 0;
	struct device_node *node = client->dev.of_node;
	// int mipifreq = 0;
	dev_info(&client->dev, "branch:%s %s time:%s\n", GIT_BRANCH, GIT_COMMIT, PROGRAM_DATE);
	priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
	priv->i2c_client = client;
	priv->regmap = devm_regmap_init_i2c(priv->i2c_client,
											&max96724_regmap_config);
	if (IS_ERR(priv->regmap))
	{
		dev_err(&client->dev,
				"regmap init failed: %ld\n", PTR_ERR(priv->regmap));
		return -ENODEV;
	}
	err = max96724_parse_dt(priv, client);
	if (err)
	{
		dev_err(&client->dev, " unable to parse dt\n");
		return -EFAULT;
	}
	mutex_init(&priv->lock);
	dev_set_drvdata(&client->dev, priv);
	err=check_device_id_rev(client);
	if(err){
		dev_err(&client->dev, " failed on check id & rev\n");
		return -EFAULT;
	}

	if (of_get_property(node, "is-master", NULL)) {
		priv->ismaster = 1;
		dev_info(&client->dev, " this 96724 is master\n");
	}else{
		priv->ismaster = 0;
		dev_info(&client->dev, " this 96724 is slave\n");
	}

	if( check_version(client)<0 ){
		return -EFAULT;
	}
	err = max96724_read_reg(&client->dev, 0x000D, &val);
	vc_info(&client->dev, "max96724 id:0x%x\n", val);
	if(0!=err)
	{
		return -ENODEV;
	}

	err=max96724_write_reg(&client->dev, 0x0013, 0x40); //reset all
	mdelay(100);
	err=max96724_write_reg(&client->dev, 0x0013, 0x00);

#if MAX_USE_OV_NO_OG
	max96724_write_reg(&client->dev,0xf0,0x40); //pipe1 GMSLB, Z-pipe; pipe0 GMSLA, Z-pipe
	max96724_write_reg(&client->dev,0xf1,0xc8); //pipe3 GMSLD, Z-pipe; pipe2 GMSLC, Z-pipe
#else
	max96724_write_reg(&client->dev,0xf0,0x40); //pipe1 GMSLB, Z-pipe; pipe0 GMSLA, Z-pipe
	max96724_write_reg(&client->dev,0xf1,0xc8); //pipe3 GMSLD, Z-pipe; pipe2 GMSLC, Z-pipe
#endif
	max96724_write_reg(&client->dev,0xf4,0x00); //disable pipes 0-4 (default val)

	err=max96724_read_reg(&client->dev,0x0006,&val);
	err=max96724_write_reg(&client->dev,0x0006,(val&0xF0)|(priv->link_lock_status&0x0F));

#if !(MAX_USE_OV_NO_OG)

#if 1 //配置8bit的参数
	max96724_write_reg(&client->dev,0x0414,0x10); // BACKTOP : BACKTOP21 | bpp8dbl0 (bpp8dbl0): Process BPP=8 as 16-bit color
	max96724_write_reg(&client->dev,0x0417,0x10); // BACKTOP : BACKTOP24 | bpp8dbl0_mode (bpp8dbl0_mode): Write alternative map enabled
	max96724_write_reg(&client->dev,0x0414,0x30); // BACKTOP : BACKTOP21 | bpp8dbl1 (bpp8dbl1): Process BPP=8 as 16-bit color
	max96724_write_reg(&client->dev,0x0417,0x30); // BACKTOP : BACKTOP24 | bpp8dbl1_mode (bpp8dbl1_mode): Write alternative map enabled
	max96724_write_reg(&client->dev,0x0414,0x70); // BACKTOP : BACKTOP21 | bpp8dbl2 (bpp8dbl2): Process BPP=8 as 16-bit color
	max96724_write_reg(&client->dev,0x0417,0x70); // BACKTOP : BACKTOP24 | bpp8dbl2_mode (bpp8dbl2_mode): Write alternative map enabled
	max96724_write_reg(&client->dev,0x0414,0xF0); // BACKTOP : BACKTOP21 | bpp8dbl3 (bpp8dbl3): Process BPP=8 as 16-bit color
	max96724_write_reg(&client->dev,0x0417,0xF0); // BACKTOP : BACKTOP24 | bpp8dbl3_mode (bpp8dbl3_mode): Write alternative map enabled
	max96724_write_reg(&client->dev,0x0973,0x02); // MIPI_TX__1 : MIPI_TX51 | ALT_MEM_MAP8 (ALT_MEM_MAP8 CTRL1): Alternate memory map enabled
#endif
	// max96724_write_reg(&client->dev,0x933,0x07); //配置8bit数据
	// max96724_write_reg(&client->dev,0x973,0x07); 
	// max96724_write_reg(&client->dev,0x933,0x01); //配置12bit数据
	// max96724_write_reg(&client->dev,0x973,0x01);
#endif

//  MIPI D-PHY Configuration
    max96724_write_reg(&client->dev,0x8a0,0x24); // 2x4 mode
#if MAX_USE_OV_NO_OG
	max96724_write_reg(&client->dev,0x90a,0xc0); //4 lanes, D-Phy, 2bits VC
	max96724_write_reg(&client->dev,0x94a,0xc0); //4 lanes, D-Phy, 2bits VC
#else
	max96724_write_reg(&client->dev,0x94a,0xC0); //4 lanes, D-Phy, 2bits VC
#endif
	max96724_write_reg(&client->dev,0x8a3,0xe4); //phy lane mapping
	max96724_write_reg(&client->dev,0x8a4,0xe4); //phy lane mapping
	max96724_write_reg(&client->dev,0x8a5,0x00); 
	max96724_write_reg(&client->dev,0x943,0x07);
	max96724_write_reg(&client->dev,0x944,0x01);

	//配置时钟2.5Gbps参数
	max96724_write_reg(&client->dev, 0x1D00, 0xF4);
	max96724_write_reg(&client->dev, 0x0418, 0x39);
	max96724_write_reg(&client->dev, 0x1D00, 0xF5);
	max96724_write_reg(&client->dev, 0x08A2, 0x34);
	max96724_write_reg(&client->dev, 0x040B, 0x02);

	// max96724_write_reg(&client->dev, 0x0420, 0xF1);

	// mipifreq = get_mipifreq();
	// max96724_write_reg(&client->dev,0x415,0x20|mipifreq);
	// max96724_write_reg(&client->dev,0x418,0x20|mipifreq);
	// max96724_write_reg(&client->dev,0x41b,0x20|mipifreq);
	// max96724_write_reg(&client->dev,0x41e,0x20|mipifreq);

	// max96724_write_reg(&client->dev,0x0027,0x00); //disable unconcerned error report
	// max96724_write_reg(&client->dev,0x0029,0x00); //disable unconcerned error report
	// max96724_vpg_set(&client->dev);
	vc_info(&client->dev, "probe end !!!!!!!!!\n");
	return err;
}

static int check_device_id_rev(struct i2c_client *client)
{
	int err=0;
	unsigned int value=0;
	err=max96724_read_reg(&client->dev,0x004C,&value);
	vc_info(&client->dev,"device revision=0x%x\n",(value&0x0f));
	return err;
}

static int check_version(struct i2c_client *client)
{
	struct max96724 *priv = NULL;
	const uint16_t ver0addr = 0x0316; //ver0 : mfp7
	const uint16_t ver1addr = 0x0319; //ver1 : mfp8

	uint16_t veraddrs[2] = {ver0addr,ver1addr};
	uint32_t ver=0,tmp;
	int i=0,err=0;

	priv = dev_get_drvdata(&client->dev);
	CHECKNULL(priv);

	if(priv->ismaster){
		for(i=0;i<2;i++){
			err=max96724_read_reg(&client->dev,veraddrs[i],&tmp);
			tmp = (tmp & 0x8)>>3;
			ver = ver | (tmp<<i);
		}
		dev_info(&client->dev," version=%d\n",ver);
		if(ver==0x01){
			vercheck=1;
		}else{
			vercheck=-1;
			err=-1;
		}
	}else{
		if(vercheck<0){
			err=-1;
		}
	}

	return err;
}

static int max96724_remove(struct i2c_client *client)
{
    struct max96724 *priv;
    priv = dev_get_drvdata(&client->dev);
    if (priv) {
        mutex_destroy(&priv->lock);
        dev_set_drvdata(&client->dev, NULL);
    }
    return 0;
}

static const struct i2c_device_id max96724_id[] = {
	{ "max96724", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, max96724_id);

static struct i2c_driver max96724_i2c_driver = {
	.driver = {
		.name = "z_max96724",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(max96724_of_match),
	},
	.probe = max96724_probe,
	.remove = max96724_remove,
	.id_table = max96724_id,
};

static int __init max96724_init(void)
{
	return i2c_add_driver(&max96724_i2c_driver);
}

static void __exit max96724_exit(void)
{
	i2c_del_driver(&max96724_i2c_driver);
}

module_init(max96724_init);
module_exit(max96724_exit);

MODULE_DESCRIPTION("GMSL Deserializer driver max96724");
MODULE_AUTHOR("yz");
MODULE_VERSION(BL_NV_MAX_VERSION);
MODULE_LICENSE("GPL v2");
