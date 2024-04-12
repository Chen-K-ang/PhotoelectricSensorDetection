#include <reg52.h>

#define PHOTOSHOT_TIME  10     /* ���ż��ʱ�䣬��λ�����ݶ�ʱ��ȷ�������ʱ��Ϊ0��������*/
#define TIME1_ACC_PRC   10     /* ��ʱ��װ��ֵ����λms��ȡֵ��Χ1~65*/
#define PHOTOSHOT_COUNT 3      /* �������3�� */
#define PHOTOSHOT_PIN   P2^0   /* ����������� */
#define ELEC_RECV_PIN   P3^1   /* ������������� */

sbit photoshot = PHOTOSHOT_PIN;
sbit elec_recv = ELEC_RECV_PIN;

unsigned char _elec_recv_buf = 1; /* ǰһ״̬������� */
unsigned char  elec_recv_buf = 1; /* �˿�״̬������� */
unsigned int th1_data = 0;
unsigned int tl1_data = 0;

bit time1_init(void);

int main(void)
{
	if (time1_init()) {
		while (1) {
			elec_recv_buf = elec_recv;
			/* �����ش��� */
			if (elec_recv_buf && !_elec_recv_buf)
				TR1  = 1;  /* �򿪶�ʱ�� */
			_elec_recv_buf = elec_recv_buf;
			photoshot = 0;
		}
	} else {
		while (1) {
			/* ������谭ʱ������� */
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

	TMOD |= 0X10;  /* ѡ��Ϊ��ʱ��1ģʽ��������ʽ1������TR1������ */

	th1_data = (65536 - TIME1_ACC_PRC * 1000) / 256;  /* ��ʱ������ֵ�������û����� */
	tl1_data = (65536 - TIME1_ACC_PRC * 1000) % 256;

	TH1  = th1_data;
	TL1  = tl1_data;

	ET1  = 1;      /* �򿪶�ʱ��1�ж����� */
	EA   = 1;      /* �����ж� */
	TR1  = 0;      /* �رն�ʱ�� */

	return 1;
}

void output_high_pin_time1_interrput() interrupt 3
{
	static unsigned char shot_cuont = 0;
	static unsigned char times = 0;

	TH1  = th1_data;   /* ����ֵ�������û����� */
	TL1  = tl1_data;

	times++;
	if (times >= PHOTOSHOT_TIME) {
		times = 0;
		shot_cuont++;
		photoshot = 1;
	}

	if (shot_cuont >= PHOTOSHOT_COUNT) {
		TR1 = 0;   /* �رն�ʱ�� */
		shot_cuont = 0;
	}
}
