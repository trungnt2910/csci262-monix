#ifndef CONSOLE_H_INCLUDED
#define CONSOLE_H_INCLUDED

#include <iosfwd>

class Console
{
private:
    class ConsoleData* m_data;
protected:
    Console(std::istream& input, std::ostream& output);
    virtual ~Console();

    virtual void Exit();
public:
    virtual std::istream& GetInput();
    virtual std::ostream& GetOutput();

    // Runs the console until it exits. Returns the exit code.
    int Run();
    // Interrupts the console, for example, when somebody presses Ctrl + C.
    void Interrupt(bool newPrompt = true);
};

// Thrown by a command to tell the console to exit.
class ExitException
{
private:
    int m_status;
public:
    ExitException(int status = 0) : m_status(status) { }
    int GetStatusCode() const { return m_status; }
};

extern class Console& Console;

#endif // CONSOLE_H_INCLUDED
