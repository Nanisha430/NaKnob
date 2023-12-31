#include "PlaygroundModel.h"
#include "App/Accounts/Account_Master.h"
#include "hal/motor.h"
#include <Arduino.h>
#include "hal/hal.h"
using namespace Page;

static int32_t knob_value = 50;
static int now_pos = 0;
static int last_pos = 0;
static int32_t MAX_VALUE = 100;
static int32_t MIN_VALUE = 0;
static bool is_outbound = false;
static SuperDialMotion knob_direction = SUPER_DIAL_NULL;
static int32_t arc_offset = 0; // 超过界限以后，显示arch长度

void PlaygroundModel::GetKnobStatus(PlaygroundMotorInfo *info)
{
    info->xknob_value = knob_value;
    info->motor_pos = now_pos;
    info->angle_offset = arc_offset;
    info->knob_direction = knob_direction;
    knob_direction = SUPER_DIAL_NULL;
}

void PlaygroundModel::SetPlaygroundMode(int16_t mode)
{
    playgroundMode = mode;
    //ChangeMotorMode(playgroundMode);
    knob_value = 0;
    switch (playgroundMode)
    {
    case PLAYGROUND_MODE_NO_EFFECTS:
        break;
    case PLAYGROUND_MODE_FINE_DETENTS:
        // This mode is default
        MAX_VALUE = 100;
        MIN_VALUE = 0;

        break;
    case PLAYGROUND_MODE_BOUND:
        MAX_VALUE = 12;
        MIN_VALUE = 0;
     
        break;
    case PLAYGROUND_MODE_ON_OFF:
        MAX_VALUE = 1;
        MIN_VALUE = 0;
        break;
    case APP_MODE_SUPER_DIAL:
        break;
        
    default:
        break;
    }
}

void PlaygroundModel::ChangeMotorMode(int mode)
{
    // Serial.printf("MenuModel: Change Motor Mode\n");
    AccountSystem::Motor_Info_t info;
    info.cmd = AccountSystem::MOTOR_CMD_CHANGE_MODE;
    info.motor_mode = mode;
    // 第一个参数是通知发布者，即本 Account 应该 subscribe 第一个参数指向的 Account
    account->Notify("Motor", &info, sizeof(info));
}
static int onEvent(Account *account, Account::EventParam_t *param)
{

    MotorStatusInfo *info = (MotorStatusInfo *)(param->data_p);

    now_pos = info->position;
    is_outbound = info->is_outbound;
    knob_direction = SUPER_DIAL_NULL;
    if (is_outbound)
    {
        arc_offset = (int)(info->angle_offset);
    }
    else
    {
        arc_offset = 0;
    }
    if (now_pos > last_pos)
    {
        knob_value++;
        knob_direction = SUPER_DIAL_RIGHT;
        if (knob_value > MAX_VALUE)
        {
            knob_value = MAX_VALUE;
        }
        last_pos = now_pos;
    }
    else if (now_pos < last_pos)
    {
        knob_value--;
        knob_direction = SUPER_DIAL_LEFT;
        if (knob_value < MIN_VALUE)
        {
            knob_value = MIN_VALUE;
        }
       
        last_pos = now_pos;
    }

    return 0;
}

void PlaygroundModel::Init()
{
    knob_value = 0;
    account = new Account("PlaygroundModel", AccountSystem::Broker(), 0, this);

    account->SetEventCallback(onEvent);
    account->Subscribe("MotorStatus");
    account->Subscribe("Motor");
    playgroundMode = 0;
}

void PlaygroundModel::Deinit()
{
    if (account)
    {
        delete account;
        account = nullptr;
    }
}