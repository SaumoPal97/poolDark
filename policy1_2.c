/* Author : Ranjan Mishra
 * Date : 5th May 2017
 * Description : Policy
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define MAX_PRICE 100
#define MIN_PRICE 0


#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y)  ((X) > (Y) ? (X) : (Y))

#define MAX_AMOUNT 10000
#define TIME 50
#define TRANSITION_LEVEL 9 

#define MAX_POLICY 10
#define DARK_LEVEL 3
#define DARK_POOL 90


const double darkExp = 0.000001;
const double expVal = 2.72;
double a = 5000;
double b = 0.5;
const double gam = 1.893;
/*
 Let the price changes take place only in exchange and the price is derived in dark pool from the exchange

 The dynamics of the price change in the exchange is given as below


 Price of the stock is given as below function
    
    p_t+1 = p_t( 1+ h1(x_t) + h2(e_t) )
    
    where p_t+1 = price of stock at t+1
          p_t   = price of stock at t
          h1 and h2 are functions 
          x_t = stocks applied to market
          e_t = random variable drwan from N(0,1)
          
          h1(x_t) = [ 0 if x_t < 100, 0.0001 *x_t if 100 < x_t < 10000 and 0.00005 * (x_t)^2 if x_t > 10000 ]
          h2(e_t) = [ 0 if 4*e_t if e_t < -0.3 or e_t > 0.3 and 0 if -0.3 < e_t < 0.3] 
          
    And the ploicy function is taken as parametric function of k_t as follows
    
    x_t = X_t * exp(t-T)/k_t
    where X_t is amount of stocks available to sell at time t
    t is the current time
    T is the total time available
    k_t is the coefficient to be determined

	In the dark pool the execution of the order is uncertain and it is highly unlikely.
	We use technique to understand the nature of distribution in the data.

    
    
    Algorithm for the process goes as follows
    
    1. Let the price range be 0 to 100 and its integer
    2. Calculate the transition probabilities for above range of prices
    3. Initialize the value for k as policy
    4. Improve the policy as per policy iteration method

  We will use Gumbell distribution for the dark pool availability and 

*/




// function to calculate the transition probability 

typedef struct {
  int priceExchange;
  //unsigned priceDark;
  int amount; 
}state;

typedef struct {
  int actionExchange;
  int actionDark;
  double val;
}actionValue;

actionValue value[TIME][MAX_PRICE][MAX_AMOUNT][DARK_POOL];
//state transitions[TRANSITION_LEVEL];
double prob[TRANSITION_LEVEL];
double probDark[DARK_LEVEL];





void assignProbability(){
  prob[0] = 0.07;
  prob[1] = 0.09;
  prob[2] = 0.1;
  prob[3] = 0.1;
  prob[4] = 0.09;
  prob[5] = 0.15;
  prob[6] = 0.1;
  prob[7] = 0.15;
  prob[8] = 0.15;

  printf("Probability assignement done!\n\n");
}


void initDarkProb(){
  probDark[0] = 0.3; 
  probDark[1] = 0.3; 
  probDark[2] = 0.4;
}


double getDarkVal(int priceExchange, int amountDark){
  double priceDark, val;
  if(amountDark != 0)
    return 0.0;

  priceDark = probDark[0]*max(0, priceExchange-1) + probDark[1]*priceExchange + probDark[2]*(priceExchange+1 );
  val = priceDark*(1.0 - darkExp*amountDark)*amountDark;

  return val;

}


int impactExchange(int priceExchange, int amountExchange){

  if(amountExchange <= 4 || priceExchange < 3)
    return priceExchange;
  else if(amountExchange <= 40)
    return (int) priceExchange*(1.0 - (0.000008*amountExchange));
  else 
    return (int) priceExchange*(1.0 - (0.01*log(amountExchange)));

}

double getValue(int time, int amount, int priceExchange, int amountExchange, int amountDark, int previousDark){
  int price= 0, i=0;
  double val = 0, nextVal=0;

  //printf("%d, %d, %d, %d, %d\n", time, amount, priceExchange, amountExchange, amountDark);
  //if(amountDark != 0){
    val = getDarkVal(priceExchange, amountDark);

    //printf("2: %d, %d, %d, %d, %d, %f\n", time, amount, priceExchange, amountExchange, amountDark, val);

    //if(amountExchange != 0)
    price = impactExchange(priceExchange, amountExchange);
    val += price*amountExchange;
    amount = amount - (amountExchange+amountDark);

    //printf("3: %d, %d, %d, %d, %d, %d\n", time, amount, priceExchange, amountExchange, amountDark, price);
    //printf("new amount: %d and price: %d\n", amount, price);

    if(price < 3){
      for(i=0;i<TRANSITION_LEVEL;i++){
        nextVal += prob[i]*value[time][max(price+i-3, 0)][amount][previousDark].val;  
        //printf("next value: %f\n", nextVal);
      }
    }
    else if(price > 93){
      for(i-0;i<TRANSITION_LEVEL;i++){
        nextVal += prob[i]*value[time][min(price+i-3, 99)][amount][previousDark].val;  
        //printf("next value: %f\n", nextVal);
      } 
    }
    else{
      for(i=0;i<TRANSITION_LEVEL;i++){
        nextVal += prob[i]*value[time][(price+i-3)][amount][previousDark].val;  
        //printf("next value: %f\n", nextVal);
      }
    }
  //}
  //printf("Value : %f and nextValue : %f\n", val, nextVal);
  return (val+nextVal); 
  
}


int darkBeta(int beta, int time, int mu, int amount){
  float value = 0;
  value = mu+pow(beta,gam*(time/5.0));
  return min((int) value, amount);
}

void initializeValue(){
  int i,j,k, iterUp, iterDown;
  state st;
  for(i=0;i<MAX_PRICE;i++){    
    for(k=0;k<MAX_AMOUNT;k++){
      for(j=0;j<DARK_POOL;j++){
        value[0][i][k][j].val = k*impactExchange(i,k);  
      } 
      //printf("%d\t%d\t%f\n", i, k, value[0][i][k].val);
    } 
  }
}

int main(){
  int price;
  int time;
  int amount,previousDark;
  int policyExchange, policyDark, z, x;

  double maxValue = 0, stValue;
  int actionDark = 0;
  int actionExchange = 0;
  FILE *fp;

  initializeValue();
  //printf("Steppint into dark pool probability\n");

  initDarkProb();

  //printf("Steppint into probability assignProbability\n");
  assignProbability();

  //printf("Steppint into loop\n");

  for(time=1;time < TIME; time++){
    //printf("Time started : %d\n", time);
    for(price=0;price<MAX_PRICE;price++){


      for(amount=0;amount<MAX_AMOUNT;amount++){
        
        for(previousDark=0;previousDark<DARK_POOL;previousDark++){

          if(amount == 0){
            value[time][price][0][previousDark].val = 0.0;
            value[time][price][0][previousDark].actionExchange = 0.0;
            value[time][price][0][previousDark].actionDark = 0.0;

          }
          else{
            maxValue = 0.0;
            actionDark = MAX_POLICY;
            actionExchange = MAX_POLICY;
            stValue = 0.0;


            for(policyDark=0;policyDark<DARK_POOL;policyDark--){
              z = darkBeta(policyDark,previousDark, time-TIME, amount);

            //z = amount*pow(expVal, policyDark*(time - TIME));
              for(policyExchange=MAX_POLICY;policyExchange>=0;policyExchange--){
                x = (amount-z)* pow(expVal, policyExchange*(time - TIME));
                stValue = getValue(time-1, amount, price, x, z, previousDark);

            //printf("Called function getValue stValue : %f\n", stValue);
            //printf("Called function getValue stValue : %f\n", stValue);
                if(stValue > maxValue){
                  actionExchange = policyExchange;
                  actionDark = policyDark;
                  maxValue = stValue;
                } 
              }
            }

            value[time][price][amount][previousDark].val = maxValue;
            value[time][price][amount][previousDark].actionExchange = actionExchange;
            value[time][price][amount][previousDark].actionDark = actionDark;
          }
        }
      }
    }

    printf("Remaining time : %d\n", TIME-time);

  }

  fp = fopen("output.txt", "w");
  fscanf(fp, "Value\tExchange\tDark\n");
  for(time=0;time < TIME; time++){
    for(price=0;price<MAX_PRICE;price++){
      for(amount=0;amount<MAX_AMOUNT;amount++){
        for(previousDark=0;previousDark<DARK_POOL;previousDark++)
        fprintf(fp, "%f\t%d\t%d\n", value[time][price][amount][previousDark].val, value[time][price][amount][previousDark].actionExchange, value[time][price][amount][previousDark].actionDark);
      }
    }
  }
  
  /*for(amount=10;amount<20;amount++){
   stValue = getValue(0, amount, 0, 0, 0);
   printf("%f\n", stValue);
 }*/
  

fclose(fp);
return 0;
}
