#ifndef _H_PATTERN_
#define _H_PATTERN_

#include <stdlib.h>
#include <time.h>

class Pattern {

enum {
	ARP_UP = 0,
	ARP_DOWN
};

public:
	Pattern();
	virtual ~Pattern();
	void setPatternSize(int size);
	void setStep(int step);
	void setCycleRange(int range);
	int getSize();
	int getStepSize();
	int getStep();
	int getDirection();
	virtual void setDirection(int direction) = 0;
	virtual void reset() = 0;
	virtual void goToNextStep() = 0;
protected:
	int size;
	int step;
	int direction;
	int range;
};

class PatternUp : public Pattern {
public:
	PatternUp();
	~PatternUp();
	void setDirection(int direction) override;
	void reset() override;
	void goToNextStep() override;
};

class PatternDown : public Pattern {
public:
	PatternDown();
	~PatternDown();
	void setDirection(int direction) override;
	void reset() override;
	void goToNextStep() override;
};

class PatternUpDown : public Pattern {
public:
	PatternUpDown();
	~PatternUpDown();
	void setDirection(int direction) override;
	void reset() override;
	void goToNextStep() override;
};

class PatternUpDownAlt : public Pattern {
public:
	PatternUpDownAlt();
	~PatternUpDownAlt();
	void setDirection(int direction) override;
	void reset() override;
	void goToNextStep() override;
private:
	bool checked;
	bool skip;
};

class PatternRandom : public Pattern {
public:
	PatternRandom();
	~PatternRandom();
	void setDirection(int direction) override;
	void reset() override;
	void goToNextStep() override;
};

class PatternCycle : public Pattern {
public:
	PatternCycle();
	~PatternCycle();
	void setDirection(int direction) override;
	void reset() override;
	void goToNextStep() override;
private:
	int tempStep;
};

#endif // _H_PATTERN_
