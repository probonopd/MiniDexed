#include "pattern.hpp"

Pattern::Pattern() : size(1), step(0), range(1)
{
}

Pattern::~Pattern()
{
}

void Pattern::setPatternSize(int size)
{
	this->size = size;
}

void Pattern::setStep(int step)
{
	this->step = step;
}

void Pattern::setCycleRange(int range)
{
	this->range = range;
}

int Pattern::getSize()
{
	return size;
}

int Pattern::getStep()
{
	return step;
}

int Pattern::getDirection()
{
	return direction;
}

PatternUp::PatternUp()
{
	reset();
}

PatternUp::~PatternUp()
{
}

void PatternUp::setDirection(int direction)
{
	this->direction = abs(direction);
}

void PatternUp::reset()
{
	step = 0;
	direction = 1;
}

void PatternUp::goToNextStep()
{
	if (size > 0) {
		step = (step + 1) % size;
	} else {
		step = 0;
	}
}

PatternDown::PatternDown()
{
	reset();
}

PatternDown::~PatternDown()
{
}

void PatternDown::setDirection(int direction)
{
	this->direction = abs(direction) * -1;
}

void PatternDown::reset()
{
	step = 0;
	direction = -1;
}

void PatternDown::goToNextStep()
{
	if (size > 0) {
		step = (step + direction < 0) ? size - 1 : step + direction;
	} else {
		step = 0;
	}
}

PatternUpDown::PatternUpDown()
{
	reset();
}

PatternUpDown::~PatternUpDown()
{
}

void PatternUpDown::setDirection(int direction)
{
	this->direction = direction;
}

void PatternUpDown::reset()
{
	step = 0;
	direction = 1;
}

void PatternUpDown::goToNextStep()
{
	if (size > 1) {
		int nextStep = step + direction;
		direction = (nextStep >= size) ? -1 : direction;
		direction = (nextStep < 0) ? 1 : direction;
		step += direction;
	} else {
		step = 0;
	}
}

PatternUpDownAlt::PatternUpDownAlt()
{
	reset();
}

PatternUpDownAlt::~PatternUpDownAlt()
{
}

void PatternUpDownAlt::setDirection(int direction)
{
	this->direction = direction;
}

void PatternUpDownAlt::reset()
{
	step = 0;
	direction = 1;
	checked = false;
	skip = false;
}

void PatternUpDownAlt::goToNextStep()
{
	if (size > 1) {
		int nextStep = step + direction;

		if (!checked) {
			if (nextStep >= size) {
				direction = -1;
				skip = true;
				checked = true;
			}
			if (nextStep < 0) {
				direction = 1;
				skip = true;
				checked = true;
			}
		}

		if (!skip) {
			step += direction;
			checked = false;
		}
		skip = false;
	} else {
		step = 0;
		//TODO init other values
	}
}

PatternRandom::PatternRandom()
{
	reset();
}

PatternRandom::~PatternRandom()
{
}

void PatternRandom::setDirection(int direction)
{
	this->direction = direction;
}

void PatternRandom::reset()
{
	goToNextStep();
}

void PatternRandom::goToNextStep()
{
    step = rand() % size;
}

PatternCycle::PatternCycle()
{
	reset();
}

PatternCycle::~PatternCycle()
{
}

void PatternCycle::setDirection(int direction)
{
	this->direction = abs(direction);
}

void PatternCycle::reset()
{
	step = 0;
	tempStep = 0;
	direction = 1;
}

void PatternCycle::goToNextStep()
{
	if (size >= 1) {
		int nextStep = tempStep + direction;

		if (range > 0 && size > 0) {
			if (nextStep >= size) {
				step = (step + 1) % range;
			}
			tempStep = (tempStep + direction) % size;
		}
	} else {
		step = 0;
		tempStep = 0;
	}
}
