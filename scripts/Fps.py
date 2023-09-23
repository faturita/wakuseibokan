import time


class Fps:
    def __init__(self):
        self.num_frames = 0
        self.fps = 0

    def tic(self):
        self.start = time.time()
        self.num_frames = 0

    def toc(self):
        self.end = time.time()
        self.seconds = self.end - self.start

        ffps = self.num_frames / self.seconds

        return ffps

    def step(self):
        self.num_frames = self.num_frames + 1

    def steptoc(self):
        self.step()
        ffps = self.toc()
        if (self.end-self.start)>1:
            self.fps = ffps
            self.tic()

if __name__ == "__main__":


    ffps = Fps()
    ffps.tic()
    for i in range(1,1000):
        ffps.step()

    print ("Estimated frames per second: {0}".format(ffps.toc()))

    for i in range(1,1000000):
        ffps.steptoc()
        print ("Estimated frames per second: {0}".format(ffps.fps))

    # Start time
    start = time.time()

    num_frames = 0

    while (True):

        # End time
        end = time.time()

        # Time elapsed
        seconds = end - start
        print ("Time taken : {0} seconds".format(seconds))

        num_frames = num_frames + 1

        if (seconds>1):
            break


    # Calculate frames per second
    fps  = num_frames / seconds;
    print ("Estimated frames per second : {0}".format(fps))
