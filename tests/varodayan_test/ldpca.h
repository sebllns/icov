#pragma once


#include <string>
void printArray(double* arr, int size, char* name);

//returns entire accumulated syndrome of source wrt code of source rate 1
void encodeBits(double *source, char *ladderFile, double *accumulatedSyndrome);

//decodeBits() finds the minimum rate for which the decoded bitstream matches
//the transmitted portion of the accumulated syndrome.
//The number of residual bit errors is also calculated.
void decodeBits(double *LLR_intrinsic, double *accumulatedSyndrome, double *source, char *ladderFile,
                double *decoded, double *rate, double *numErrors);

//For implementation outline of beliefPropagation(), refer to
//W. E. Ryan, "An Introduction to LDPC Codes," in CRC Handbook for Coding
//and Signal Processing for Recording Systems (B. Vasic, ed.) CRC Press, 2004.
//available online (as of May 8, 2006) at
//http://www.ece.arizona.edu/~ryan/New%20Folder/ryan-crc-ldpc-chap.pdf

//beliefPropagation() runs several iterations belief propagation until
//either the decoded bitstream agrees with the transmitted portion of
//accumulated syndrome or convergence or the max number of iterations.
//Returns 1 if decoded bitstream agrees with
//transmitted portion of accumulated syndrome.
int beliefPropagation(int *ir, int *jc, int m, int n, int nzmax,
                      double *LLR_intrinsic, double *syndrome,
                      double *decoded);

void write_vector(const std::string& filename, double* data, int size);
