# A video capture class based on OpenCv
This is an video capture class designed to make randomly get a frame from the
video stream easily.

Example code:
```
    NvpVideoCap cap;
    if (cap.Open(0) < 0) {
        printf("Open camera error!\n");
        return -1;
    }
    while (1)
    {
        cv::Mat img;
        cap.GetOneFrame(img);
        cv::imshow("test", img);
        cv::waitKey(10);
    }
    return 0;
```
