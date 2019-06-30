#pragma once

#ifndef SURFFACE_H
#define SURFFACE_H

#include <vector>
#include <iostream>
#include "Point.h"
using namespace std;

class CSurfFace
{
public:
	vector<shared_ptr<CPoint>> mPoints;
	float mColor;
	pair<size_t, size_t> mCPoints;
	inline CSurfFace(vector<shared_ptr<CPoint>> aPoints, float aColor, pair<size_t, size_t> aCPoints);
	inline CSurfFace() {};
	inline ~CSurfFace();

private:

};

CSurfFace::CSurfFace(vector<shared_ptr<CPoint>> aPoints, float aColor, pair<size_t, size_t> aCPoints) {
	this->mPoints = aPoints;
	this->mColor = aColor;
	this->mCPoints = aCPoints;
}

CSurfFace::~CSurfFace()
{
}

#endif