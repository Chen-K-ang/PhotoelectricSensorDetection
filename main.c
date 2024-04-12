#include <reg52.h>

#define PHOTOSHOT_TIME  10     /* 快门间隔时间，单位：根据定时器确定，间隔时间为0则持续输出*/
#define TIME1_ACC_PRC   10     /* 定时器装载值，单位ms，取值范围1~65*/
#define PHOTOSHOT_COUNT 3      /* 快门输出3次 */
#define PHOTOSHOT_PIN   P2^0   /* 快门输出引脚 */
#define ELEC_RECV_PIN   P3^1   /* 光电检测输入引脚 */

sbit photoshot = PHOTOSHOT_PIN;
sbit elec_recv = ELEC_RECV_PIN;

unsigned char _elec_recv_buf = 1; /* 前一状态光电输入 */
unsigned char  elec_recv_buf = 1; /* 此刻状态光电输入 */
unsigned int th1_data = 0;
unsigned int tl1_data = 0;

bit time1_init(void);

int main(void)
{
	if (time1_init()) {
		while (1) {
			elec_recv_buf = elec_recv;
			/* 上升沿触发 */
			if (elec_recv_buf && !_elec_recv_buf)
				TR1  = 1;  /* 打开定时器 */
			_elec_recv_buf = elec_recv_buf;
			photoshot = 0;
		}
	} else {
		while (1) {
			/* 光电有阻碍时持续输出 */
			while (elec_recv)
				photoshot = 1;
			photoshot = 0;
		}
	}
}

bit time1_init(void)
{
	if (PHOTOSHOT_TIME == 0)
		return 0;

	TMOD |= 0X10;  /* 选择为定时器1模式，工作方式1，仅用TR1打开启动 */

	th1_data = (65536 - TIME1_ACC_PRC * 1000) / 256;  /* 定时器初载值，根据用户输入 */
	tl1_data = (65536 - TIME1_ACC_PRC * 1000) % 256;

	TH1  = th1_data;
	TL1  = tl1_data;

	ET1  = 1;      /* 打开定时器1中断允许 */
	EA   = 1;      /* 打开总中断 */
	TR1  = 0;      /* 关闭定时器 */

	return 1;
}

void output_high_pin_time1_interrput() interrupt 3
{
	static unsigned char shot_cuont = 0;
	static unsigned char times = 0;

	TH1  = th1_data;   /* 重载值，根据用户输入 */
	TL1  = tl1_data;

	times++;
	if (times >= PHOTOSHOT_TIME) {
		times = 0;
		shot_cuont++;
		photoshot = 1;
	}

	if (shot_cuont >= PHOTOSHOT_COUNT) {
		TR1 = 0;   /* 关闭定时器 */
		shot_cuont = 0;
	}
}
