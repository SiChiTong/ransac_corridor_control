#include "ransac2Dline.hpp"

int ransac_2Dline(float **data, int n, int maxT, float threshold,
                  float *bestModel, int *bestInliers, int verbose) {

    if(verbose)
        printf("Start RANSAC, n=%d, maxT=%d, t=%.2f\n\n", n, maxT, threshold);

    *bestInliers = 0;

    int T = maxT;
    int inliers = 0;
    int Tcount = 0;
    int ndata = n;
    int nr = 2;
    int i;

    float **randSet = (float **) malloc(nr * sizeof(float *));
    if(randSet == NULL) { perror("out of memory\n"); exit(0); }

    for(i = 0; i < nr; i++)
    {
        randSet[i] = (float *) malloc(2 * sizeof(float));
        if(randSet[i] == NULL) { perror("out of memory\n"); exit(0); }
    }

    float **conSet = (float **) malloc(n * sizeof(float *));
    if(conSet == NULL) { perror("out of memory\n"); exit(0); }

    for(i = 0; i < n; i++)
    {
        conSet[i] = (float *) malloc(2 * sizeof(float));
        if(conSet[i] == NULL) { perror("out of memory\n"); exit(0); }
    }

    float fracInliers = 0;
    float randModel[3];
    float point[2];
    float pNoOutliers = 0;
    float p = 0.99;

    srand(time(NULL)); // set rand seed

    while(T > Tcount)
    {
        if(verbose)
            printf("\n#%d ITERATION >>>>>>>>>>>>>>>>>>>>>>>\n", Tcount);

        // Select 2 points at random to form a trial model
        if(randomSelect(randSet, nr, data, &ndata) == -1)
            break;

        if(verbose)
            printf(" selected points: (%.3f, %.3f) and (%.3f, %.3f)\n", 
                    randSet[0][0], randSet[0][1], randSet[1][0], 
                    randSet[1][1]);

        // Fit model to the random selection of data points
        twoPointsLine(randModel, randSet);

        if(verbose)
            printf(" rand model: %.3f*x + %.3f*y + %.3f = 0\n", randModel[0], 
            randModel[1], randModel[2]);

        inliers = 0;
        fracInliers = 0;

        // Evaluate distances between points and model.
        // Given a threshold, create a consensus set with the points
        // that are inliers.
        for(i = 0; i < n; i++)
        {
            point[0] = data[i][0];
            point[1] = data[i][1];

            if(fitModel_line(point, randModel, threshold))
            {
                conSet[inliers][0] = point[0];
                conSet[inliers][1] = point[1];
                inliers++;
            }
        }

        if(verbose)
            printf(" inliers = %d\n", inliers);

        if(inliers > *bestInliers)  // Largest set of inliers.
        {
            if(verbose)
                printf(" >> IT'S THE BEST MODEL !!! <<\n");

            estimateModel_line(bestModel, conSet, inliers);
            *bestInliers = inliers;

            if(verbose)
            printf(" reestimated model: %.3f*x + %.3f*y + %.3f = 0\n",
                    bestModel[0], bestModel[1], bestModel[2]);

            // Reestimate T, the number of trials to ensure we pick,
            // with probability p, a data set free of outliers.
            fracInliers = (float)inliers/n;
            pNoOutliers = 1 - pow(fracInliers, 2);
            T = log(1-p)/log(pNoOutliers);
        }

        Tcount++;
        if(Tcount > maxT)
            break;
    }

    if(bestInliers==0)
    {
        printf("\n### ERROR: ransac was unable to find a useful solution.\n");
        return(-1);
    }

    for(i = 0; nr < 2; i++){
        free(randSet[i]);
    }
    free(randSet);
    for(i = 0; i < n; i++){
        free(conSet[i]);
    }
    free(conSet);

    return(0);
}

int randomSelect(float **sel, int nsel, float **data, int *ndata) {

    int r = 0;
    int k = *ndata;
    int i;

    if(nsel > *ndata)
    {
        printf("randomSelect: unable to select %d points from dataset[%d]\n", 
                nsel, *ndata);
        return -1;
    }

    for(i = 0; i < nsel; i++, k--)
    {
        r = rand()%(k);

        sel[i][0] = data[r][0];
        sel[i][1] = data[r][1];

        data[r][0] = data[k-1][0];
        data[r][1] = data[k-1][1];
        
        data[k-1][0] = sel[i][0];
        data[k-1][1] = sel[i][1];
    }

    *ndata = k;
    //printf("ndata = %d\n", *ndata);

    return 0;
}

int fitModel_line(float *point, float *l, float threshold) {
    // Estimate distance between point and model
    // d = abs(a*x + b*y + c)/sqrt(a^2 + b^2)

    float d=0;

    d = fabs(l[0]*point[0] + l[1]*point[1] + l[2])/sqrt(pow(l[0], 2) + pow(l[1], 2));

    if(d<=threshold)
        return 1;
    else
        return 0;
}

void estimateModel_line(float *l, float **P, int n) {
    if(n<=2) {
        perror("Need at least tree points\n");
        l[0] = 0;
        l[1] = 0;
        l[2] = 0;
        return;
    }
    
    float x_bar = 0.0;
    float y_bar = 0.0;
    for(int i=0; i!=n; ++i){
        x_bar += P[i][0];
        y_bar += P[i][1];
    }
    x_bar = x_bar/n;
    y_bar = y_bar/n;

    float c0 = 0.0;
    float c1 = 0.0;
    for(int i=0; i!=n; ++i){
        c0 +=  (P[i][0] - x_bar)*(P[i][1] - y_bar);
        c1 += -pow(P[i][0] - x_bar,2) + pow(P[i][1] - y_bar,2);

    // Line in normal vector representation
    float phi, r;
    phi = 0.5*atan2(-2*c0,c1);
    r = x_bar*cos(phi) + y_bar*sin(phi);

    // Line in tradicional representation
    float angCoeff, linCoeff;
    angCoeff = -cos(phi)/(sin(phi) + 1e-6);
    linCoeff = r/(sin(phi) + 1e-6);

    // Line in representation ax + by + c = 0
    l[0] = -angCoeff;
    l[1] = 1.0;
    l[2] = -linCoeff;
    }

}

void twoPointsLine(float *l, float **P){
    l[0] = 1;
    l[1] = -((P[0][0]-P[1][0])/(P[0][1]-P[1][1]));
    l[2] = -l[0]*P[0][0] -l[1]*P[0][1];
}
