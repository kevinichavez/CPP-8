#ifndef TIMER_H

const int TIMER_STARTED{ 0 }, TIMER_STOPPED{ 1 }, TIMER_PAUSED{ 2 };

class Timer {
public:
    //Initializes variables
    Timer();

    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //Gets the timer's time
    unsigned long getTicks();

    //Checks the status of the timer
    int getTimerStatus() { return m_status; }

private:
    //The clock time when the timer started
    unsigned long m_startTicks;

    //The ticks stored when the timer was paused
    unsigned long m_pausedTicks;

    //The timer status
    int m_status;
};

#endif // !TIMER_H