from cgitb import html
from time import time
import cv2
import imutils
from imutils.video import VideoStream
import time
import math

import generated.drive_control_pb2 as drive_control_pb2
import os


spin_msg = drive_control_pb2.ActualSpeed()
spin_msg.left = -0.1
spin_msg.right = 0.1

halt_msg = drive_control_pb2.Halt()
halt_msg.halt = True

'''
out_dir = "test_output"
with open(os.path.join(out_dir, "spin.protobuf"), "w") as f:
    f.write(str(spin_msg))

with open(os.path.join(out_dir, "halt.protobuf"), "w") as f:
    f.write(str(halt_msg))
'''

# Import correct ARUCO dictionary and set up params
arucoDict = cv2.aruco.Dictionary_get(cv2.aruco.DICT_4X4_50)
arucoParams = cv2.aruco.DetectorParameters_create()

# Open vid stream for main camera
vid_stream = VideoStream(src=0).start()
time.sleep(2.0)

marker_dict = {
    0 : "Start",
    1 : "Post 1",
    2 : "Post 2",
    3 : "Post 3",
    4 : "Gate Left",
    5 : "Gate Right"
}

# Continuously analyze frames from video stream
while(True):

    # Get frame from camera
    frame = vid_stream.read()
    frame = imutils.resize(frame, width=1000)

    # Check for ARUCO code
    (corners, ids, rejected) = cv2.aruco.detectMarkers(
        frame, 
        arucoDict, 
        parameters=arucoParams
    )

    # If we found an ARUCO code
    if len(corners):
        ids = ids.flatten()

        for (markerCorner, markerID) in zip(corners, ids):
            corners = markerCorner.reshape((4,2))
            (topLeft, topRight, bottomLeft, bottomRight) = corners

            # Get corners of the code
            topLeft = (int(topLeft[0]), int(topLeft[1]))
            topRight = (int(topRight[0]), int(topRight[1]))
            bottomLeft = (int(bottomLeft[0]), int(bottomLeft[1]))
            bottomRight = (int(bottomRight[0]), int(bottomRight[1]))

            # Calculates center
            center = (int((topLeft[0] + bottomLeft[0]) / 2), int((topLeft[1] + bottomLeft[1]) / 2))
            
            # Draws bounding box and center circle
            cv2.line(frame, topLeft, topRight, (0, 255, 0), 2)
            cv2.line(frame, topRight, bottomLeft, (0, 255, 0), 2)
            cv2.line(frame, bottomRight, bottomLeft, (0, 255, 0), 2)
            cv2.line(frame, bottomRight, topLeft, (0, 255, 0), 2)

            if center[0] < 450 or center[0] > 550:
                cv2.circle(frame, center, int(math.sqrt((topLeft[0]-bottomLeft[0])**2 + (topLeft[1]-bottomLeft[1])**2) * 0.05), (0,0,255), 2)
            else:
                cv2.circle(frame, center, int(math.sqrt((topLeft[0]-bottomLeft[0])**2 + (topLeft[1]-bottomLeft[1])**2) * 0.05), (0,255,0), 2)

            # Draws label
            cv2.putText(
                frame, 
                marker_dict[markerID],
                (topLeft[0], topLeft[1] - 15),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.5, 
                (0, 255, 0),
                2
            )

    # Shows the camera feed
    cv2.imshow("Camera Feed", frame)
    key = cv2.waitKey(1) & 0xFF

    # Breaks out of loop when we press q
    if key == ord("q"):
        break

# Clean-up
cv2.destroyAllWindows()
vid_stream.stop()

