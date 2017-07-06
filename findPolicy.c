/* Author : Ranjan Mishra
 * Date : 5th May 2017
 * Description : Policy
 */

#include <stdio.h>
#include <math.h>
#include <time.h>

#define MAX_PRICE 20
#define MIN_PRICE 0

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y)  ((X) > (Y) ? (X) : (Y))


/* Description of the state and dynamics of the system
   
   State is three dimensional, price in the exchange, price in the dark pool
   and the amount of stock holding

   Dyanmics of the prices in both dark pool and exchange are given

   Following are the assumptions of the dynamics of the market
    
    Price of the stock is given as below function
    
    p_t+1 = p_t( 1+ h1(x_t) + h2(e_t) )
    
    where p_t+1 = price of stock at t+1
          p_t   = price of stock at t
          h1 and h2 are functions 
          e_t = random variable drwan from N(0,1)
          x_t = stocks executed or applied to market
          
          h1(x_t) = [ 0 if x_t < 100, 0.0001 *x_t if 100 < x_t < 10000 and 0.00005 * (x_t)^2 if x_t > 10000 ]
          h2(e_t) = [ 0 if 4*e_t if e_t < -0.3 or e_t > 0.3 and 0 if -0.3 < e_t < 0.3] 
          
    And the ploicy function is taken as parametric function of k_t as follows
    
    x_t = X_t * exp(t-T)/k_t
    where X_t is amount of stocks available to sell at time t
    t is the current time
    T is the total time available
    k_t is the coefficient to be determined
    
    
    Algorithm for the process goes as follows
    
    1. Let the price range be 0 to 100 and its integer
    2. Calculate the transition probabilities for above range of prices
    3. Initialize the value for k as policy
    4. Improve the policy as per policy iteration method
*/

// function to calculate the transition probability 

typedef struct {
	unsigned priceExchange;
	unsigned priceDark;
	unsigned amount; 
}state;

typedef struct {
	unsigned actionExchange;
	unsigned actionDark;
	double val;
}actionValue;

actionValue value[10][100][100][1000];
state transitions[9];
double prob[9];

void init(){
	int i = 0;
	for(i=0;i<9;i++){
		transitions[i].priceExchange = 0;
		transitions[i].priceDark = 0;
		transitions[i].amount = 0;
	}

	printf("transition initilization done!\n");
}

unsigned impact(unsigned price, unsigned amount){

	if(amount <= 100)
		return price;
	else if(amount <= 1000)
		return (unsigned) price*(1 - (0.00008*amount));
	else 
		return (unsigned) price*(1 - (0.01*log(amount)));
}

void transition(state st, unsigned darkAmount, unsigned exchangeAmount){
	int i, j;
	unsigned impactedPriceExchange, impactedPriceDark;
	impactedPriceExchange = impact(st.priceExchange, exchangeAmount);
	impactedPriceDark = impact(st.priceDark, darkAmount);

	unsigned k = 0;
	for(i=0;i<3;i++){
		for(j=0;j<3;j++){

			transitions[k].priceDark = min(10, impactedPriceDark + j);
			transitions[k].priceExchange = min(10, impactedPriceExchange + i);
			transitions[k].amount = st.amount - (darkAmount + exchangeAmount);
			k++;
		}

	}
	


}

void assignProbability(){
	prob[0] = 0.04;
	prob[1] = 0.06;
	prob[2] = 0.1;
	prob[3] = 0.06;
	prob[4] = 0.09;
	prob[5] = 0.15;
	prob[6] = 0.1;
	prob[7] = 0.15;
	prob[8] = 0.25;

	printf("Probability assignement done!\n");
}


void initializeValue(){
	unsigned i,j,k;
 	for(i=0;i<100;i++){
 		for(j=0;j<100;j++){
 			for(k=0;k<1000;k++){
 				value[0][i][j][k].val = i*k;
 			}
 		}
 	}

 	printf("value initilization done!\n");
}

/* value at the final state is nothing but multiple of price in the primary
   exchange and amount of stocks availble, we need not to explicitly calculate the value at time N-1
   we start with one step ahead and go backward
   we will let k vary from 0 to 100
 */

//double value[100][100][100];


double getValue(unsigned t, state st, unsigned darkAmount, unsigned exchangeAmount){
	double val=0;
	unsigned i=0;
	transition(st, darkAmount, exchangeAmount);

	for(i=0;i<9;i++){
		val+= prob[i]*value[t][transitions[i].priceExchange][transitions[i].priceDark][transitions[i].amount].val;
	}
	
	return val;

}


void printPower(){
	int i;
	double val = 0;
	unsigned x = 0;
	for(i=-100;i<0;i++){
		val = pow(2.7, i);
		x = 1000*val;
		printf("%u\n", x);
	}
}

int main(){
	int i, j, k, l, a;
	int x = 0, z =0;
	time_t start_t, end_t;

	int t=0;

	double diff_t;
	double max = 0, val = 0;
	unsigned action = 0, darkAction = 0;
    printf("Starting of the program...\n\n");
    time(&start_t);
    state st;

    init();
    assignProbability();
    initializeValue();
    double temp = 0;
    unsigned w = 0;
    int cal = 0;
	for(t=9;t>0;t--){
		//printf("%d", t);
		for(i=0;i<100;i++){
			//printf("%d", w++);
			st.priceExchange = k;
			for(j=0;j<100;j++){
				st.priceDark = j;
				
				for(a=0;a<1000;a++){
					//printf("%d\n", w++); 
					max = 0;
					action = 0;
					darkAction = 0;
					st.amount = a;
					for(k=0;k<10;k++){
						//printf("%d", w);
						for(l=0;l<10;l++){
							cal = l*(t-10);		
							temp = pow(2.7, cal);
							//printf("%f\t%d\n", temp, cal);
							z = a*temp;
							cal = k*(t-10);	
							temp = pow(2.7, cal);
							x = temp*(a-z);
							val = x*i + z*j + getValue(10-t-1, st, z, x);

							if (val > max){
								max = val;
								action = k;
								darkAction = l;
							}

						}
					}

					value[10-t][i][j][a].val = max;
					value[10-t][i][j][a].actionExchange = action;
					value[10-t][i][j][a].actionDark = darkAction;
				}
				
				
			}
		}

		printf("completed %d%", t-10);

	}
	//printPower();
	
	time(&end_t);
	diff_t = difftime(end_t, start_t);
	printf("\n");

	printf("Execution time = %f\n", diff_t);

	printf("Value at prices : 12, 16 is : %f", value[9][12][16][99].val);
	printf("Dark action and exchange action : %u and %u ", value[9][12][16][99].actionDark, value[9][12][16][99].actionExchange);
	//printf("Exiting of the program...\n");

	return 0;
}

