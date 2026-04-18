/* App/Sensor/app_sensor.c */
#include "app_interact.h"
#include "app.h"
#include "bluetooth.h"
#include "usart.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>

// 包含消息头文件
#include <std_msgs/msg/int32.h>
#include <geometry_msgs/msg/twist.h> // 引入Twist消息用于速度传递

// 声明外部变量
extern float target_vx, target_vy, target_wz;
extern float current_vx, current_vy, current_wz; // 新增：当前速度

// 底层传输函数声明
bool cubemx_transport_open(struct uxrCustomTransport * transport);
bool cubemx_transport_close(struct uxrCustomTransport * transport);
size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);

void * microros_allocate(size_t size, void * state);
void microros_deallocate(void * pointer, void * state);
void * microros_reallocate(void * pointer, size_t size, void * state);
void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);

// --- 新增：接收目标速度的回调函数 ---
void cmd_vel_callback(const void * msgin)
{
    const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
    // 将接收到的ROS 2消息赋值给单片机控制变量
    target_vx = msg->linear.x;
    target_vy = msg->linear.y;
    target_wz = msg->angular.z;
    
    // 如果需要调试，可以取消注释下面这行 (需配置好printf)
    // printf("Received cmd_vel: vx=%.2f, vy=%.2f, wz=%.2f\n", target_vx, target_vy, target_wz);
}

void ProtocolProcessTask(void *argument)
{
    /* micro-ROS configuration */
    rmw_uros_set_custom_transport(
      true,
      (void *) &huart2,
      cubemx_transport_open,
      cubemx_transport_close,
      cubemx_transport_write,
      cubemx_transport_read);

    rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
    freeRTOS_allocator.allocate = microros_allocate;
    freeRTOS_allocator.deallocate = microros_deallocate;
    freeRTOS_allocator.reallocate = microros_reallocate;
    freeRTOS_allocator.zero_allocate =  microros_zero_allocate;

    if (!rcutils_set_default_allocator(&freeRTOS_allocator)) {
        printf("Error on default allocators (line %d)\n", __LINE__);
    }

    /* 声明 micro-ROS 对象 */
    rclc_support_t support;
    rcl_node_t node;
    rclc_executor_t executor; // 新增：执行器，用于处理回调
    rcl_allocator_t allocator = rcl_get_default_allocator();

    // 声明发布者和订阅者
    rcl_publisher_t pub_int;
    rcl_publisher_t pub_vel;
    rcl_subscription_t sub_cmd_vel;

    // 声明消息体
    std_msgs__msg__Int32 msg_int;
    geometry_msgs__msg__Twist msg_current_vel;
    geometry_msgs__msg__Twist msg_target_vel; // 用于接收

    // 1. 初始化 support 和 node
    rclc_support_init(&support, 0, NULL, &allocator);
    rclc_node_init_default(&node, "stm32_chassis_node", "", &support);

    // 2. 创建 Publisher (原有的计数器)
    rclc_publisher_init_default(
      &pub_int,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "chassis_counter");

    // 3. 创建 Publisher (发布当前实际速度)
    rclc_publisher_init_default(
      &pub_vel,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
      "current_velocity");

    // 4. 创建 Subscriber (订阅 Linux 发来的目标控制速度)
    rclc_subscription_init_default(
      &sub_cmd_vel,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
      "cmd_vel");

    // 5. 初始化 Executor (参数 1 表示只有 1 个订阅者需要处理)
    rclc_executor_init(&executor, &support.context, 1, &allocator);
    rclc_executor_add_subscription(&executor, &sub_cmd_vel, &msg_target_vel, &cmd_vel_callback, ON_NEW_DATA);

    msg_int.data = 0;

    for(;;)
    {
        // --- 发送数据 ---
        
        // 1. 发送计数器
        rcl_publish(&pub_int, &msg_int, NULL);
        msg_int.data++;

        // --- 发送数据前，先给盒子“洗个澡” (全部清零) ---
        // 这样 z, roll, pitch 就会变成干净的 0.0，而不是 e-127
        memset(&msg_current_vel, 0, sizeof(geometry_msgs__msg__Twist));

        // 2. 发送当前速度给 Linux
        // 这里把单片机底层的 current 变量装填到 ROS 2 消息结构体中
        msg_current_vel.linear.x = current_vx;
        msg_current_vel.linear.y = current_vy;
        msg_current_vel.angular.z = current_wz;
        rcl_publish(&pub_vel, &msg_current_vel, NULL);


        // --- 接收数据 ---
        
        // 让 executor 运行，处理订阅的收包逻辑。
        // 超时时间设置为 100ms。如果有新消息，它会自动调用 cmd_vel_callback。
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));

        // osDelay(100);  // 因为 spin_some 已经有了 100ms 的阻塞等待时间，这里只需要很小的让出CPU即可
        osDelay(10); 
    }
}