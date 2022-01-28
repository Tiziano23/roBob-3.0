class Robot
{
public:
    enum State
    {
        IDLE,
        FOLLOW_LINE,
        MANEUVER,
        AVOID_OBSTACLE
    };

    Robot() {}

    State getState() { return currentState; }
    State getLastState() { return lastState; }
    void setState(State s)
    {
        if (s != currentState) lastState = currentState;
        currentState = s;
    }
    void revertState()
    {
        setState(lastState);
    }

private:
    State currentState = IDLE;
    State lastState = IDLE;
};