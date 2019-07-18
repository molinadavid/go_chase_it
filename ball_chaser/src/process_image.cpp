#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

#include <ros/console.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    client.call(srv);
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    // Prepare the linear and angular velocities
    float x = 0.0;
    float z = 0.0;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    // Loop through each pixel in the image and check for white pixels
    int height = img.height;
    int steps = img.step;
    int col = -1;
    for (int i = 0; i < height ; i++)
    {
        for (int j = 0; j < steps; j++)
        {
            int red_px = img.data[i * steps + j];
            int green_px = img.data[i * steps  + (j + 1)];
            int blue_px = img.data[i * steps + (j + 2)];
            if (red_px == white_pixel && green_px == white_pixel && blue_px == white_pixel)
            {
                ROS_WARN("Step white: %d", j);
                col = j / 3;
                break;
            }
        }
    }
    
    // Process the data based on the col number
    // If the col is -1 it means that the white ball was not found
    if (col == -1)
    {
        x = 0.0;
        z = 0.0;
    }
    else
    {
        int width = img.width;
        // Find if the col is on the first third of the image and move left
        if (col <= (width/3))
        {
            x = 0.1;
            z = 0.2;
        }
        // Find if the column is on the middle third of the image and move straight
        else if (col > (width / 3) && col <= (2 * (width / 3)))
        {
             x = 0.4;
             z = 0.0;
        }
        // Move to the right
        else
        {
            x = 0.1;
            z = -0.2;
        }
    }
    // Send the command to drive the robot
    drive_robot(x, z);
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;
    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}