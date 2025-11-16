class VideoStream {
private:
    std::ifstream videoFile;
    std::string fileName;
    int frameNbr;
public:
    VideoStream(std::string fName);
    bool openFile();
    int getNextFrame(char* frameBuf, int bufSize);
};
