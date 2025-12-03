#include "rx8010.hpp"

#
#include "logger.hpp"

namespace wibot {
LOGGER("rx8010")
#define RX8010_ADDRESS 0x64U

/* rx8010 time register */
#define REG_SEC  0x10U
#define REG_MIN  0x11U
#define REG_HOUR 0x12U
#define REG_WEEK 0x13U
#define REG_DAY  0x14U
#define REG_MON  0x15U
#define REG_YEAR 0x16U

#define REG_RES1 0x17U

/* rx8010 alarm register */
#define REG_ALM_MIN      0x18U
#define REG_ALM_HOUR     0x19U
#define REG_ALM_WEEK_DAY 0x1AU

/* rx8010 control register */
#define REG_TIME_CNT0 0x1BU
#define REG_TIME_CNT1 0x1CU
#define RGE_EXT       0x1DU
#define REG_FLAG      0x1EU
#define REG_CTRL      0x1FU

#define REG_USER_RAM_BEGIN 0x20U

#define REG_RES2     0x30U
#define REG_RES3     0x31U
#define REG_IRQ_CTRL 0x32U

/* rx8010 flag register bit positions */
#define VALUE_FLAG_VLF (1 << 1)
#define VALUE_FLAG_AF  (1 << 3)
#define VALUE_FLAG_TF  (1 << 4)
#define VALUE_FLAG_UF  (1 << 5)

/* rx8010 ctrl register bit positions */
#define VALUE_CTRL_TSTP (1 << 2)
#define VALUE_CTRL_AIE  (1 << 3)
#define VALUE_CTRL_TIE  (1 << 4)
#define VALUE_CTRL_UIE  (1 << 5)
#define VALUE_CTRL_STOP (1 << 6)
#define VALUE_CTRL_TEST (1 << 7)

/* rx8010 ext register bit positions */
#define VALUE_EXT_TSEL0 (1 << 0)
#define VALUE_EXT_TSEL1 (1 << 1)
#define VALUE_EXT_TSEL2 (1 << 2)
#define VALUE_EXT_WADA  (1 << 3)
#define VALUE_EXT_TE    (1 << 4)
#define VALUE_EXT_USEL  (1 << 5)
#define VALUE_EXT_FSEL0 (1 << 6)
#define VALUE_EXT_FSEL1 (1 << 7)

/* rx8010 alarm select */
#define ALM_ENABLE(value)  ((~0x80) & value) /* active when bit7 is low */
#define ALM_DISABLE(value) (0x80 | value)

#define RX8010_DEVICE_EVENT_DONE 0x01

Result Rx8010::_readI2c(u16 address, void* data, u32 dataSize) {
    auto ar = _i2c.readReg(address, Slice((uint8_t*)data, dataSize));
    return ar.wait(TIMEOUT_FOREVER);
};
Result Rx8010::_writeI2c(u16 address, void* data, u32 dataSize) {
    auto ar = _i2c.writeReg(address, Slice((uint8_t*)data, dataSize));
    return ar.wait(TIMEOUT_FOREVER);
};
Rx8010::Rx8010(I2cMaster& i2c) : _i2c(i2c) {
    _i2c.setTransitionConfig(RX8010_ADDRESS >> 1);
};

Result Rx8010::porInit() {
    u8 data;
    os::sleep(50);

    auto rst = _readI2c(REG_USER_RAM_BEGIN, (void*)&data, 1);
    if (rst != Result::kOk) {
        return rst;
    }

    rst = _readI2c(REG_FLAG, (void*)&data, 1);
    if (rst != Result::kOk) {
        return rst;
    }

    if (data & VALUE_FLAG_VLF) {
        {
            LOG_E("rx8010 is not running");
            data &= ~VALUE_FLAG_VLF;
            rst = _writeI2c(REG_FLAG, (void*)&data, 1);
            if (rst != Result::kOk) {
                return rst;
            }
            os::sleep(10);
        }
        while (data & VALUE_FLAG_VLF)

            // software reset?
            data &= 0x7C;
        rst = _writeI2c(REG_FLAG, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }

        data = 0xD8;
        rst  = _writeI2c(REG_RES1, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }

        data = 0x00;
        rst  = _writeI2c(REG_RES2, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }

        data = 0x08;
        rst  = _writeI2c(REG_RES3, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }

        rst = _readI2c(REG_IRQ_CTRL, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }
        data &= 0xF8;
        rst = _writeI2c(REG_IRQ_CTRL, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }
        setDateTime(DateTime());
    } else {
        data &= 0x7C;
        rst = _writeI2c(REG_FLAG, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }

        data = 0xD8;
        rst  = _writeI2c(REG_RES1, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }

        data = 0x00;
        rst  = _writeI2c(REG_RES2, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }

        data = 0x08;
        rst  = _writeI2c(REG_RES3, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }

        rst = _readI2c(REG_IRQ_CTRL, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }
        data &= 0xF8;
        rst = _writeI2c(REG_IRQ_CTRL, &data, 1);
        if (rst != Result::kOk) {
            return rst;
        }
    }

    return Result::kOk;
};

Result Rx8010::getDateTime(DateTime& datetime) {
    u8   data[7];
    auto rst = _readI2c(REG_SEC, data, 7);
    if (rst != Result::kOk) {
        return rst;
    }
    datetime.second = Codex::bcdToByte(data[0]);
    datetime.minute = Codex::bcdToByte(data[1]);
    datetime.hour   = Codex::bcdToByte(data[2]);
    datetime.day    = Codex::bcdToByte(data[4]);
    datetime.month  = Codex::bcdToByte(data[5]);
    datetime.year   = Codex::bcdToByte(data[6]);
    return Result::kOk;
};

Result Rx8010::setDateTime(const DateTime& datetime) {
    u8   ctrl;
    u8   data[3];
    auto rst = _readI2c(REG_CTRL, &ctrl, 1);
    if (rst != Result::kOk) {
        return rst;
    }
    ctrl |= VALUE_CTRL_STOP;
    rst = _writeI2c(REG_CTRL, &ctrl, 1);
    if (rst != Result::kOk) {
        return rst;
    }
    data[0] = Codex::byteToBcd(datetime.second);
    data[1] = Codex::byteToBcd(datetime.minute);
    data[2] = Codex::byteToBcd(datetime.hour);
    rst     = _writeI2c(REG_SEC, data, 3);
    if (rst != Result::kOk) {
        return rst;
    }
    data[0] = Codex::byteToBcd(datetime.day);
    data[1] = Codex::byteToBcd(datetime.month);
    data[2] = Codex::byteToBcd(datetime.year);
    rst     = _writeI2c(REG_DAY, data, 3);
    if (rst != Result::kOk) {
        return rst;
    }

    ctrl &= ~VALUE_CTRL_STOP;
    rst = _writeI2c(REG_CTRL, &ctrl, 1);

    return rst;
}

#if 0
	static irqreturn_t rx8010_irq_1_handler(int irq, void *dev_id)
	{
		struct i2c_client *client = dev_id;
		struct rx8010_data *rx8010 = i2c_get_clientdata(client);
		int flagreg;

		mutex_lock(&rx8010->rtc->ops_lock);

		flagreg = i2c_smbus_read_byte_data(client, RX8010_FLAG);

		if (flagreg <= 0) {
			mutex_unlock(&rx8010->rtc->ops_lock);
			return IRQ_NONE;
		}

		if (flagreg & RX8010_FLAG_VLF)
			dev_warn(&client->dev, "Frequency stop detected\n");

		if (flagreg & RX8010_FLAG_TF) {
			flagreg &= ~RX8010_FLAG_TF;
			rtc_update_irq(rx8010->rtc, 1, RTC_PF | RTC_IRQF);
		}

		if (flagreg & RX8010_FLAG_AF) {
			flagreg &= ~RX8010_FLAG_AF;
			rtc_update_irq(rx8010->rtc, 1, RTC_AF | RTC_IRQF);
		}

		if (flagreg & RX8010_FLAG_UF) {
			flagreg &= ~RX8010_FLAG_UF;
			rtc_update_irq(rx8010->rtc, 1, RTC_UF | RTC_IRQF);
		}

		i2c_smbus_write_byte_data(client, RX8010_FLAG, flagreg);

		mutex_unlock(&rx8010->rtc->ops_lock);
		return IRQ_HANDLED;
	}

	/**
	 * @description: get time from rx8010
	 * @param {type}
	 * @return:
	 */
	static int rx8010_get_time(struct device *dev, struct rtc_time *dt)
	{
		struct rx8010_data *rx8010 = dev_get_drvdata(dev);
		u8 date[7];
		int flagreg;
		int err;

		flagreg = i2c_smbus_read_byte_data(rx8010->client, RX8010_FLAG);
		if (flagreg < 0)
			return flagreg;

		if (flagreg & RX8010_FLAG_VLF) {
			dev_warn(dev, "Frequency stop detected\n");
			return -EINVAL;
		}

		err = i2c_smbus_read_i2c_block_data(rx8010->client, RX8010_SEC,
							7, date);
		if (err != 7)
			return err < 0 ? err : -EIO;

		dt->tm_sec = bcd2bin(date[RX8010_SEC - RX8010_SEC] & 0x7f);
		dt->tm_min = bcd2bin(date[RX8010_MIN - RX8010_SEC] & 0x7f);
		dt->tm_hour = bcd2bin(date[RX8010_HOUR - RX8010_SEC] & 0x3f);
		dt->tm_mday = bcd2bin(date[RX8010_MDAY - RX8010_SEC] & 0x3f);
		dt->tm_mon = bcd2bin(date[RX8010_MONTH - RX8010_SEC] & 0x1f) - 1;
		dt->tm_year = bcd2bin(date[RX8010_YEAR - RX8010_SEC]) + 100;
		dt->tm_wday = ffs(date[RX8010_WDAY - RX8010_SEC] & 0x7f);

		return 0;
	}

	static int rx8010_set_time(struct device *dev, struct rtc_time *dt)
	{
		struct rx8010_data *rx8010 = dev_get_drvdata(dev);
		u8 date[7];
		int ctrl, flagreg;
		int ret;

		if ((dt->tm_year < 100) || (dt->tm_year > 199))
			return -EINVAL;

		/* set STOP bit before changing clock/calendar */
		ctrl = i2c_smbus_read_byte_data(rx8010->client, RX8010_CTRL);
		if (ctrl < 0)
			return ctrl;
		rx8010->ctrlreg = ctrl | RX8010_CTRL_STOP;
		ret = i2c_smbus_write_byte_data(rx8010->client, RX8010_CTRL,
						rx8010->ctrlreg);
		if (ret < 0)
			return ret;

		date[RX8010_SEC - RX8010_SEC] = bin2bcd(dt->tm_sec);
		date[RX8010_MIN - RX8010_SEC] = bin2bcd(dt->tm_min);
		date[RX8010_HOUR - RX8010_SEC] = bin2bcd(dt->tm_hour);
		date[RX8010_MDAY - RX8010_SEC] = bin2bcd(dt->tm_mday);
		date[RX8010_MONTH - RX8010_SEC] = bin2bcd(dt->tm_mon + 1);
		date[RX8010_YEAR - RX8010_SEC] = bin2bcd(dt->tm_year - 100);
		date[RX8010_WDAY - RX8010_SEC] = bin2bcd(1 << dt->tm_wday);

		ret = i2c_smbus_write_i2c_block_data(rx8010->client,
							 RX8010_SEC, 7, date);
		if (ret < 0)
			return ret;

		/* clear STOP bit after changing clock/calendar */
		ctrl = i2c_smbus_read_byte_data(rx8010->client, RX8010_CTRL);
		if (ctrl < 0)
			return ctrl;
		rx8010->ctrlreg = ctrl & ~RX8010_CTRL_STOP;
		ret = i2c_smbus_write_byte_data(rx8010->client, RX8010_CTRL,
						rx8010->ctrlreg);
		if (ret < 0)
			return ret;

		flagreg = i2c_smbus_read_byte_data(rx8010->client, RX8010_FLAG);
		if (flagreg < 0) {
			return flagreg;
		}

		if (flagreg & RX8010_FLAG_VLF)
			ret = i2c_smbus_write_byte_data(rx8010->client, RX8010_FLAG,
							flagreg & ~RX8010_FLAG_VLF);

		return 0;
	}
#endif

#if 0
	static int rx8010_read_alarm(struct device *dev, struct rtc_wkalrm *t)
	{
		struct rx8010_data *rx8010 = dev_get_drvdata(dev);
		struct i2c_client *client = rx8010->client;
		u8 alarmvals[3];
		int flagreg;
		int err;

		err = i2c_smbus_read_i2c_block_data(client, RX8010_ALMIN, 3, alarmvals);
		if (err != 3)
			return err < 0 ? err : -EIO;

		flagreg = i2c_smbus_read_byte_data(client, RX8010_FLAG);
		if (flagreg < 0)
			return flagreg;

		t->time.tm_sec = 0;
		t->time.tm_min = bcd2bin(alarmvals[0] & 0x7f);
		t->time.tm_hour = bcd2bin(alarmvals[1] & 0x3f);

		if (!(alarmvals[2] & RX8010_ALARM_AE))
			t->time.tm_mday = bcd2bin(alarmvals[2] & 0x7f);

		t->enabled = !!(rx8010->ctrlreg & RX8010_CTRL_AIE);
		t->pending = (flagreg & RX8010_FLAG_AF) && t->enabled;

		return 0;
	}

	static int rx8010_set_alarm(struct device *dev, struct rtc_wkalrm *t)
	{
		struct i2c_client *client = to_i2c_client(dev);
		struct rx8010_data *rx8010 = dev_get_drvdata(dev);
		u8 alarmvals[3];
		int extreg, flagreg;
		int err;

		flagreg = i2c_smbus_read_byte_data(client, RX8010_FLAG);
		if (flagreg < 0) {
			return flagreg;
		}

		if (rx8010->ctrlreg & (RX8010_CTRL_AIE | RX8010_CTRL_UIE)) {
			rx8010->ctrlreg &= ~(RX8010_CTRL_AIE | RX8010_CTRL_UIE);
			err = i2c_smbus_write_byte_data(rx8010->client, RX8010_CTRL,
							rx8010->ctrlreg);
			if (err < 0) {
				return err;
			}
		}

		flagreg &= ~RX8010_FLAG_AF;
		err = i2c_smbus_write_byte_data(rx8010->client, RX8010_FLAG, flagreg);
		if (err < 0)
			return err;

		alarmvals[0] = bin2bcd(t->time.tm_min);
		alarmvals[1] = bin2bcd(t->time.tm_hour);
		alarmvals[2] = bin2bcd(t->time.tm_mday);

		err = i2c_smbus_write_i2c_block_data(rx8010->client, RX8010_ALMIN,
							 2, alarmvals);
		if (err < 0)
			return err;

		extreg = i2c_smbus_read_byte_data(client, RX8010_EXT);
		if (extreg < 0)
			return extreg;

		extreg |= RX8010_EXT_WADA;
		err = i2c_smbus_write_byte_data(rx8010->client, RX8010_EXT, extreg);
		if (err < 0)
			return err;

		if (alarmvals[2] == 0)
			alarmvals[2] |= RX8010_ALARM_AE;

		err = i2c_smbus_write_byte_data(rx8010->client, RX8010_ALWDAY,
						alarmvals[2]);
		if (err < 0)
			return err;

		if (t->enabled) {
			if (rx8010->rtc->uie_rtctimer.enabled)
				rx8010->ctrlreg |= RX8010_CTRL_UIE;
			if (rx8010->rtc->aie_timer.enabled)
				rx8010->ctrlreg |=
					(RX8010_CTRL_AIE | RX8010_CTRL_UIE);

			err = i2c_smbus_write_byte_data(rx8010->client, RX8010_CTRL,
							rx8010->ctrlreg);
			if (err < 0)
				return err;
		}

		return 0;
	}

	static int rx8010_alarm_irq_enable(struct device *dev,
					   unsigned int enabled)
	{
		struct i2c_client *client = to_i2c_client(dev);
		struct rx8010_data *rx8010 = dev_get_drvdata(dev);
		int flagreg;
		u8 ctrl;
		int err;

		ctrl = rx8010->ctrlreg;

		if (enabled) {
			if (rx8010->rtc->uie_rtctimer.enabled)
				ctrl |= RX8010_CTRL_UIE;
			if (rx8010->rtc->aie_timer.enabled)
				ctrl |= (RX8010_CTRL_AIE | RX8010_CTRL_UIE);
		} else {
			if (!rx8010->rtc->uie_rtctimer.enabled)
				ctrl &= ~RX8010_CTRL_UIE;
			if (!rx8010->rtc->aie_timer.enabled)
				ctrl &= ~RX8010_CTRL_AIE;
		}

		flagreg = i2c_smbus_read_byte_data(client, RX8010_FLAG);
		if (flagreg < 0)
			return flagreg;

		flagreg &= ~RX8010_FLAG_AF;
		err = i2c_smbus_write_byte_data(rx8010->client, RX8010_FLAG, flagreg);
		if (err < 0)
			return err;

		if (ctrl != rx8010->ctrlreg) {
			rx8010->ctrlreg = ctrl;
			err = i2c_smbus_write_byte_data(rx8010->client, RX8010_CTRL,
							rx8010->ctrlreg);
			if (err < 0)
				return err;
		}

		return 0;
	}

	static int rx8010_ioctl(struct device *dev, unsigned int cmd, unsigned long arg)
	{
		struct i2c_client *client = to_i2c_client(dev);
		struct rx8010_data *rx8010 = dev_get_drvdata(dev);
		int ret, tmp;
		int flagreg;

		switch (cmd) {
		case RTC_VL_READ:
			flagreg = i2c_smbus_read_byte_data(rx8010->client, RX8010_FLAG);
			if (flagreg < 0)
				return flagreg;

			tmp = !!(flagreg & RX8010_FLAG_VLF);
			if (copy_to_user((void __user *)arg, &tmp, sizeof(int)))
				return -EFAULT;

			return 0;

		case RTC_VL_CLR:
			flagreg = i2c_smbus_read_byte_data(rx8010->client, RX8010_FLAG);
			if (flagreg < 0) {
				return flagreg;
			}

			flagreg &= ~RX8010_FLAG_VLF;
			ret = i2c_smbus_write_byte_data(client, RX8010_FLAG, flagreg);
			if (ret < 0)
				return ret;

			return 0;

		default:
			return -ENOIOCTLCMD;
		}
	}

	static struct rtc_class_ops rx8010_rtc_ops = {
		.read_time = rx8010_get_time,
		.set_time = rx8010_set_time,
		.ioctl = rx8010_ioctl,
	};

	static int rx8010_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
	{
		struct i2c_adapter *adapter = client->adapter;
		struct rx8010_data *rx8010;
		int err = 0;

		if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA
			| I2C_FUNC_SMBUS_I2C_BLOCK)) {
			dev_err(&adapter->dev, "doesn't support required functionality\n");
			return -EIO;
		}

		rx8010 = devm_kzalloc(&client->dev, sizeof(struct rx8010_data),
					  GFP_KERNEL);
		if (!rx8010)
			return -ENOMEM;

		rx8010->client = client;
		i2c_set_clientdata(client, rx8010);

		err = rx8010_init_client(client);
		if (err)
			return err;

		if (client->irq > 0) {
			dev_info(&client->dev, "IRQ %d supplied\n", client->irq);
			err = devm_request_threaded_irq(&client->dev, client->irq, NULL,
							rx8010_irq_1_handler,
							IRQF_TRIGGER_LOW | IRQF_ONESHOT,
							"rx8010", client);

			if (err) {
				dev_err(&client->dev, "unable to request IRQ\n");
				client->irq = 0;
			} else {
				rx8010_rtc_ops.read_alarm = rx8010_read_alarm;
				rx8010_rtc_ops.set_alarm = rx8010_set_alarm;
				rx8010_rtc_ops.alarm_irq_enable = rx8010_alarm_irq_enable;
			}
		}

		rx8010->rtc = devm_rtc_device_register(&client->dev, client->name,
			&rx8010_rtc_ops, THIS_MODULE);

		if (IS_ERR(rx8010->rtc)) {
			dev_err(&client->dev, "unable to register the class device\n");
			return PTR_ERR(rx8010->rtc);
		}

		rx8010->rtc->max_user_freq = 1;

		return err;
	}
#endif

}  // namespace wibot
