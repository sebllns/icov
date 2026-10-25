#include "ldpca.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <chrono>

#define max(a, b) (((a) > (b)) ? (a) : (b))

 void write_vector(const std::string& filename, double* data, int size)
{
     return;
    std::ofstream file(filename);
    file << size;
    for(int i = 0; i < size; i++)
    {
        file  << " " << data[i];
    }
}

int getMatrixElement(int *matrix, int row, int col, int n, int totalnumInc)
{
    return matrix[(row * n + col) % (n * totalnumInc)];
}

void setMatrixElement(int value, int *matrix, int row, int col, int n, int totalnumInc)
{
    matrix[(row * n + col) % (n * totalnumInc)] = value;
}

void encodeBits(double *source, char *ladderFile, double *accumulatedSyndrome)
{
    FILE *fp;
    int n, m, nzmax, *ir, *jc;
    int numCodes, totalNumInc, numInc, *txSeq;
    int k, l;

    int *h_matrix;

    fp = fopen(ladderFile, "r");
    //65 396 1188 66
    fscanf(fp, "%d", &numCodes);
    fscanf(fp, "%d", &n);
    fscanf(fp, "%d", &nzmax);
    fscanf(fp, "%d", &totalNumInc);

    ir = new int[nzmax];
    jc = new int[n + 1];
    txSeq = new int[totalNumInc];

    h_matrix = new int[n * totalNumInc];



    for (k = 0; k < n + 1; k++)
        fscanf(fp, "%d", jc + k);
    for (k = 0; k < numCodes; k++)
    {
        fscanf(fp, "%d", &numInc);
        for (l = 0; l < numInc; l++)
            fscanf(fp, "%d", txSeq + l);
        for (l = 0; l < nzmax; l++)
            fscanf(fp, "%d", ir + l);
    }
    m = (n / totalNumInc) * numInc;

    fclose(fp);

    for (k = 0; k < m; k++)
        accumulatedSyndrome[k] = (double)0;

    //source*H'
    for (k = 0; k < n; k++)
    {
        for (l = jc[k]; l < jc[k + 1]; l++)
        {
            accumulatedSyndrome[ir[l]] += source[k];


//            std::cout << (ir[l] * n + k) % (n * totalNumInc) << std::endl;

            h_matrix[(ir[l] * n + k) % (n * totalNumInc)] = 1;
            //setMatrixElement(1, h_matrix, ir[l], k, n, totalNumInc);
        }
    }

//    for(int h = 0; h < n * totalNumInc ; h++)
//    {
//        std::cout << h_matrix[h] <<  ", ";
//    }

    //accumulate
    for (k = 1; k < m; k++)
        accumulatedSyndrome[k] += accumulatedSyndrome[k - 1];

    //mod 2
    for (k = 0; k < m; k++)
        accumulatedSyndrome[k] = (double)((int)accumulatedSyndrome[k] % 2);
}

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
                      double *decoded)
{
    int iteration, k, l, sameCount;
    double *LLR_extrinsic, *check_LLR, *check_LLR_mag, *rowTotal, *LLR_overall;

    LLR_extrinsic = new double[nzmax];
    check_LLR = new double[nzmax];
    check_LLR_mag = new double[nzmax];
    rowTotal = new double[m];
    LLR_overall = new double[n];

    sameCount = 0;
    for (k = 0; k < n; k++)
        decoded[k] = 0;

    //initialize variable-to-check messages
    for (k = 0; k < n; k++)
        for (l = jc[k]; l < jc[k + 1]; l++)
            LLR_extrinsic[l] = LLR_intrinsic[k];


//    std::cout <<"LLR_extrinsic"<<std::endl;
    write_vector(std::string("/home/sbelleno/_ICOV/WIP/aff3ct-tests/icov-test/build/bin/LLR_extrinsic"), LLR_extrinsic, nzmax);

    for (iteration = 0; iteration < 100; iteration++)
    {
        //Step 1: compute check-to-variable messages

        for (k = 0; k < nzmax; k++)
        {
            check_LLR[k] = (double)((LLR_extrinsic[k] < 0) ? -1 : 1);
            check_LLR_mag[k] = ((LLR_extrinsic[k] < 0) ? -LLR_extrinsic[k] : LLR_extrinsic[k]);

            //std::cout << check_LLR_mag[k] * check_LLR[k] << std::endl;
        }

        for (k = 0; k < m; k++)
            rowTotal[k] = (double)((syndrome[k] == 1) ? -1 : 1); //1-2*S
        for (k = 0; k < nzmax; k++)
            rowTotal[ir[k]] *= check_LLR[k];
        for (k = 0; k < nzmax; k++)
            check_LLR[k] = check_LLR[k] * rowTotal[ir[k]];
        //sign of check-to-variable messages





        for (k = 0; k < nzmax; k++)
            check_LLR_mag[k] = -log(tanh(max(check_LLR_mag[k], 0.000000001) / 2));


//        std::cout <<"check_LLR MAG "<<std::endl;
        write_vector(std::string("/home/sbelleno/_ICOV/WIP/aff3ct-tests/icov-test/build/bin/check_LLR_mag_avant"), check_LLR_mag, nzmax);

//        getchar();

        for (k = 0; k < m; k++)
            rowTotal[k] = (double)0;
        for (k = 0; k < nzmax; k++)
            rowTotal[ir[k]] += check_LLR_mag[k];
        for (k = 0; k < nzmax; k++)
            check_LLR_mag[k] = -log(tanh(max(rowTotal[ir[k]] - check_LLR_mag[k], 0.000000001) / 2));
        //magnitude of check-to-variable messages
        // 2 * atanh(t) = log((1 + t) / (1 - t)

        for (k = 0; k < nzmax; k++)
        {
            check_LLR[k] = check_LLR[k] * check_LLR_mag[k]; // SIGNE * MAG

            //std::cout << check_LLR[k] << ", ";
        }


//        std::cout <<"check_LLR FINAUX"<<std::endl;
        write_vector(std::string("/home/sbelleno/_ICOV/WIP/aff3ct-tests/icov-test/build/bin/check_LLR"), check_LLR, nzmax);
//        getchar();
        //check-to-variable messages = check_LLR
        //Step 2: compute variable-to-check messages

        for (k = 0; k < n; k++)
        {
            LLR_overall[k] = LLR_intrinsic[k];
            for (l = jc[k]; l < jc[k + 1]; l++)
                LLR_overall[k] += check_LLR[l];
        }

        for (k = 0; k < n; k++)
            for (l = jc[k]; l < jc[k + 1]; l++)
                LLR_extrinsic[l] = LLR_overall[k] - check_LLR[l];
        //variable-to-check messages = LLR_extrinsic

        printArray(LLR_overall, n, "LLR_overall");


        printArray(LLR_extrinsic, n, "LLR_extrinsic");

        //Step 3: test convergence and syndrome condition

        l = 0;
        for (k = 0; k < n; k++)
            if (decoded[k] == ((LLR_overall[k] < 0) ? 1 : 0))
                l++;
            else
                decoded[k] = ((LLR_overall[k] < 0) ? 1 : 0);

        sameCount = ((l == n) ? sameCount + 1 : 0);

        if (sameCount == 5)
        {
            //std::cout << "m=" << m << " STOP AT ITER " << iteration << std::endl;
            return 0; //convergence (to wrong answer)
        }

        for (k = 0; k < m; k++)
            rowTotal[k] = syndrome[k];

        printArray(rowTotal, m, "rowtotal");

        for (k = 0; k < n; k++)
        {
            for (l = jc[k]; l < jc[k + 1]; l++)
            {
                rowTotal[ir[l]] += decoded[k];
//std::cout << ir[l] << " " << decoded[k] <<std::endl;

            }
        }

        printArray(rowTotal, m, "rowtotal");

        for (k = 0; k < m; k++)
            if (((int)rowTotal[k] % 2) != 0)
                break;
            else if (k == m - 1)
                return 1; //all syndrome checks satisfied
    }

    return 0;
}

/*******************************************************************************************************/
//decodeBits() finds the minimum rate for which the decoded bitstream matches
//the transmitted portion of the accumulated syndrome.
//The number of residual bit errors is also calculated.
void decodeBits(double *LLR_intrinsic, double *accumulatedSyndrome, double *source, char *ladderFile,
                double *decoded, double *rate, double *numErrors)
{
    FILE *fp;
    int n, m, nzmax, *ir, *jc;
    int numCodes, totalNumInc, numInc, *txSeq;
    int code, k, currIndex, prevIndex;
    double *syndrome;

    fp = fopen(ladderFile, "r");

    fscanf(fp, "%d", &numCodes);
    fscanf(fp, "%d", &n);
    fscanf(fp, "%d", &nzmax);
    fscanf(fp, "%d", &totalNumInc);

    ir = new int[nzmax];
    jc = new int[n + 1];
    txSeq = new int[totalNumInc]; //actual length: numInc
    syndrome = new double[n];     //actual length: m

    for (k = 0; k < n + 1; k++)
        fscanf(fp, "%d", jc + k);

    auto start = std::chrono::steady_clock::now();

    //iterate through codes of increasing rate
    for (code = 0; code < numCodes; code++)
    {

        fscanf(fp, "%d", &numInc);
        for (k = 0; k < numInc; k++)
            fscanf(fp, "%d", txSeq + k);
        for (k = 0; k < nzmax; k++)
            fscanf(fp, "%d", ir + k);
        m = (n / totalNumInc) * numInc;

//        std::cout << "code=" << code <<", m=" << m << std::endl;


        rate[0] = ((double)m) / ((double)n);

        if (rate[0] == 1)
        {
            fclose(fp);
            for (k = 0; k < n; k++)
                decoded[k] = source[k]; //result of Gaussian elimination
            numErrors[0] = 0;
            return;
        }

        currIndex = txSeq[0];
        syndrome[0] = accumulatedSyndrome[currIndex];

//        std::cout << "__currIndex "<<currIndex<<std::endl;
//        std::cout << "__syndrome k=0"<<": s="<<syndrome[0]<<std::endl;



        for (k = 1; k < m; k++)
        {
            prevIndex = currIndex;

            //CODE CYCLIQUE DU AU MODULO + OFFSET
            currIndex = txSeq[k % numInc] + (k / numInc) * totalNumInc;
            //CALCUL SYNDROME
            syndrome[k] = (double)(((int)(accumulatedSyndrome[currIndex] + accumulatedSyndrome[prevIndex])) % 2);

//            std::cout << "__prevIndex="<<prevIndex<<std::endl;
//            std::cout << "__currIndex="<<currIndex<<std::endl;
//            std::cout << "__acc="<<accumulatedSyndrome[currIndex]<<std::endl;
//            std::cout << "__k="<<k<<": s="<<syndrome[k]<<std::endl;
        }

//        getchar();


//       std::cout << "********************************************************\n\n";

//        std::cout << m << std::endl;
        printArray(syndrome, n, "syndrome");


        if (beliefPropagation(ir, jc, m, n, nzmax, LLR_intrinsic, syndrome, decoded))
        {
            std::cout << "code=" << code <<", m=" << m << std::endl;

            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

            //ends at code=45, m=282
            fclose(fp);
            numErrors[0] = 0;
            for (k = 0; k < n; k++)
                numErrors[0] += (double)(decoded[k] != source[k]);
            return;
        }


//        std::cout << m << std::endl;
//        getchar();

    }
}

void printArray(double *arr, int n, char* name)
{
    return;
    std::cout << "\n\n********************************************************************************************\n" << name << "[";
    for (int i = 0; i < n - 1; i++)
    {
        std::cout << arr[i] << ", ";
    }

    std::cout << arr[n-1] << "]\n" << std::endl;

    getchar();
}



