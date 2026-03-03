# Kalman Filter

## Introduction 
This project demonstrates how a Kalman filter can be used to smooth noisy market price data. In this case, for futures contracts like ES (S&P 500), NQ (Nasdaq), or CL (Crude Oil).

Financial markets are full of noisy data, each price tick isn’t always meaningful. The Kalman filter acts like a smart averaging system that learns over time how much to trust new prices versus its current belief. The filter keeps track of a “fair value” estimate of the instrument.
Each time a new price tick arrives, it decides:
Should I move my estimate toward this new tick (if I think it’s real)?
Or should I ignore it a bit (if I think it’s just noise)?


This decision depends on how “trustworthy” the market data looks at that moment, captured by the Kalman Gain (K).


## Concept Overview 
The Kalman filter works like a weighing system between two ideas:
What you already believe (prior estimate)
 → e.g., your current fair value for ES is 5050.0
What the market is showing you (new measurement)
 → e.g., a new tick at 5052.0


It combines both using a weighted average:
New Estimate=Old Estimate+K×(New Tick−Old Estimate)
Here, K (the Kalman Gain) decides how much to move:
If K is high, you trust the new tick and move a lot.
If K is low, you think it’s noise and move a little.


Over time, the filter keeps updating both:
the estimate of fair value
the confidence level (uncertainty)


This helps smooth prices and track trends more accurately than raw tick data

To know more please go to Kalman_Filter.pdf

## Makefile
Build: Make

Run: Make run

Clean: Make clean


