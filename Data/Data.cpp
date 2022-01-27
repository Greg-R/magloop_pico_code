#include "Data.h"



Data::Data(){}


void Data::computeSlopes() {
countPerHertz[0] = (float) ((float) bandLimitPositionCounts[0][1] - (float) bandLimitPositionCounts[0][0]) / (float) ((float) HIGHEND40M - (float) LOWEND40M);
countPerHertz[1] = (float) ((float) bandLimitPositionCounts[1][1] - (float) bandLimitPositionCounts[1][0]) / (float) ((float) HIGHEND30M - (float) LOWEND30M);
countPerHertz[2] = (float) ((float) bandLimitPositionCounts[2][1] - (float) bandLimitPositionCounts[2][0]) / (float) ((float) HIGHEND20M - (float) LOWEND20M);
hertzPerStepperUnitVVC[0]=(float) ((float) HIGHEND40M - (float) LOWEND40M)/((float) bandLimitPositionCounts[0][1] - (float) bandLimitPositionCounts[0][0]);
hertzPerStepperUnitVVC[1]=(float) ((float) HIGHEND30M - (float) LOWEND30M)/((float) bandLimitPositionCounts[1][1] - (float) bandLimitPositionCounts[1][0]);
hertzPerStepperUnitVVC[2]=(float) ((float) HIGHEND20M - (float) LOWEND20M)/((float) bandLimitPositionCounts[2][1] - (float) bandLimitPositionCounts[2][0]);
}