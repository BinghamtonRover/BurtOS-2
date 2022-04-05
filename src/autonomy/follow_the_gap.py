# need to be adjusted to actual camera's settings
width = 10.0
hfov = 60.0

threshold = 5.0

current_angle = 0.0

# simple version of https://ieeexplore.ieee.org/document/8014220

final_heading_angle = 0
gap_center_angle = 0
goal_point_angle = 0
min_dist_to_obstacle = 1 # not sure what this is
safety_coefficient = 5 # not sure what this is either...
safety_ratio = safety_coefficient / min_dist_to_obstacle

# for testing purposes, pretend image has dimensions 0 - 10
obstacles = [(5,9)]

last_obstacle = 0
max_size = 0

# add rightmost edge of camera view
obstacles.append((width, width))

# find angle to largest gap
for o in obstacles:
    size = o[0] - last_obstacle
    mid = (o[0] + last_obstacle) / 2.0
    # formula from https://github.com/IntelRealSense/librealsense/issues/9297
    angle = ((mid - width / 2.0) / (width / 2.0)) * (hfov / 2.0)
    last_obstacle = o[1]
    if size > max_size:
        max_size = size
        gap_center_angle = angle

final_heading_num = safety_ratio * gap_center_angle + goal_point_angle
final_heading_den = safety_ratio + 1
final_heading_angle = final_heading_num / final_heading_den

print("Current angle:", current_angle)
print("Final heading angle:", final_heading_angle)
if current_angle - final_heading_angle > threshold:
    print("turn left")
elif current_angle - final_heading_angle < -threshold:
    print("turn right")
else:
    print("go straight")

# if this works, implement improvements mentioned in paper
